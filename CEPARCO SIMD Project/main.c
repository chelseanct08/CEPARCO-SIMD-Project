#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <windows.h>
#include <intrin.h>
#include "dot.h"

#define RUNS 30
#define BOUNDARY_SIZE 1003

typedef double (*DotKernel)(const double*, const double*, size_t);

static int has_avx_support(void)
{
#if defined(_M_X64)
    int cpu_info[4];
    unsigned __int64 xcr0;

    __cpuid(cpu_info, 1);
    if ((cpu_info[2] & (1 << 27)) == 0 || (cpu_info[2] & (1 << 28)) == 0) {
        return 0;
    }

    xcr0 = _xgetbv(0);
    return (xcr0 & 0x6ULL) == 0x6ULL;
#else
    return 0;
#endif
}

static int has_avx2_support(void)
{
#if defined(_M_X64)
    int cpu_info[4];

    if (!has_avx_support()) {
        return 0;
    }

    __cpuid(cpu_info, 0);
    if (cpu_info[0] < 7) {
        return 0;
    }

    __cpuidex(cpu_info, 7, 0);
    return (cpu_info[1] & (1 << 5)) != 0;
#else
    return 0;
#endif
}

double dot_c(const double* A, const double* B, size_t n)
{
    double sum = 0.0;
    size_t i;

    for (i = 0; i < n; i++) {
        sum += A[i] * B[i];
    }

    return sum;
}

static int time_kernel(
    DotKernel kernel,
    const double* A,
    const double* B,
    size_t n,
    LARGE_INTEGER frequency,
    double* average_time)
{
    LARGE_INTEGER start, end;
    volatile double sink = 0.0;
    int i;

    if (!QueryPerformanceCounter(&start)) {
        return 0;
    }

    for (i = 0; i < RUNS; i++) {
        sink = kernel(A, B, n);
    }

    if (!QueryPerformanceCounter(&end)) {
        return 0;
    }

    (void)sink;
    *average_time = (double)(end.QuadPart - start.QuadPart)
        / (double)frequency.QuadPart / RUNS;
    return 1;
}

int check_result(double expected, double actual)
{
    double difference = fabs(expected - actual);
    double tolerance = 1e-12 + 1e-10 * fabs(expected);

    return difference <= tolerance;
}

void initialize_vectors(double* A, double* B, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        A[i] = sin((double)i * 0.001);
        B[i] = cos((double)i * 0.002);
    }
}

static void run_supported_kernels(
    const double* A,
    const double* B,
    size_t n,
    const int avx_available,
    const int avx2_available,
    double expected,
    LARGE_INTEGER frequency)
{
    DotKernel kernels[4];
    const char* names[4];
    size_t kernel_count = 0;
    size_t kernel_index;
    double actual;
    double average_time;

    if (!avx_available) {
        printf("%-10s SKIPPED: AVX/OSXSAVE/XCR0 support is unavailable.\n", "SIMD XMM");
    }

    if (!avx2_available) {
        printf("%-10s SKIPPED: AVX2 support or OS state is unavailable.\n", "SIMD YMM");
    }

    kernels[kernel_count] = dot_c;
    names[kernel_count] = "C";
    kernel_count++;

    kernels[kernel_count] = dot_x64;
    names[kernel_count] = "x86-64";
    kernel_count++;

    if (avx_available) {
        kernels[kernel_count] = dot_xmm;
        names[kernel_count] = "SIMD XMM";
        kernel_count++;
    }

    if (avx2_available) {
        kernels[kernel_count] = dot_ymm;
        names[kernel_count] = "SIMD YMM";
        kernel_count++;
    }

    for (kernel_index = 0; kernel_index < kernel_count; kernel_index++) {
        actual = kernels[kernel_index](A, B, n);
        printf("%-10s Result: %.12f | %s\n",
            names[kernel_index],
            actual,
            check_result(expected, actual) ? "PASS" : "FAIL");

        if (time_kernel(kernels[kernel_index], A, B, n, frequency, &average_time)) {
            printf("           Average Time: %.9f seconds (%d runs)\n\n",
                average_time,
                RUNS);
        } else {
            printf("           Timing failed: QueryPerformanceCounter was unavailable.\n\n");
        }
    }
}

void test_boundary(const int avx_available, const int avx2_available)
{
    double A[BOUNDARY_SIZE];
    double B[BOUNDARY_SIZE];
    double expected, actual;
    size_t i;
    size_t kernel_count = 0;
    DotKernel kernels[4];
    const char* names[4];

    if (!avx_available) {
        printf("%-10s SKIPPED: AVX/OSXSAVE/XCR0 support is unavailable.\n", "SIMD XMM");
    }

    if (!avx2_available) {
        printf("%-10s SKIPPED: AVX2 support or OS state is unavailable.\n", "SIMD YMM");
    }

    kernels[kernel_count] = dot_c;
    names[kernel_count] = "C";
    kernel_count++;

    kernels[kernel_count] = dot_x64;
    names[kernel_count] = "x86-64";
    kernel_count++;

    if (avx_available) {
        kernels[kernel_count] = dot_xmm;
        names[kernel_count] = "SIMD XMM";
        kernel_count++;
    }

    if (avx2_available) {
        kernels[kernel_count] = dot_ymm;
        names[kernel_count] = "SIMD YMM";
        kernel_count++;
    }

    initialize_vectors(A, B, BOUNDARY_SIZE);
    expected = dot_c(A, B, BOUNDARY_SIZE);

    printf("\nBoundary Test: n = %d\n", BOUNDARY_SIZE);
    printf("----------------------------------------\n");

    for (i = 0; i < kernel_count; i++) {
        actual = kernels[i](A, B, BOUNDARY_SIZE);
        printf("%-10s Result: %.12f | %s\n",
            names[i],
            actual,
            check_result(expected, actual) ? "PASS" : "FAIL");
    }
}

int main(void)
{
    const size_t sizes[] = {
        (size_t)1 << 20,
        (size_t)1 << 26,
        (size_t)1 << 28
    };

    const int avx_available = has_avx_support();
    const int avx2_available = has_avx2_support();
    LARGE_INTEGER frequency;
    size_t size_index;
    double* A;
    double* B;
    double expected;
    size_t n;

    printf("CPU support: AVX %s, AVX2 %s\n",
        avx_available ? "available" : "unavailable",
        avx2_available ? "available" : "unavailable");
    printf("Tolerance: absolute 1e-12 + relative 1e-10 * |reference|\n");

    if (!QueryPerformanceFrequency(&frequency) || frequency.QuadPart == 0) {
        fprintf(stderr, "Unable to obtain a valid performance-counter frequency; benchmarks cannot run.\n");
        return 1;
    }

    for (size_index = 0;
        size_index < sizeof(sizes) / sizeof(sizes[0]);
        size_index++) {

        n = sizes[size_index];

        printf("\n========================================\n");
        printf("Vector Size: %zu\n", n);
        printf("========================================\n");

        if (n > SIZE_MAX / sizeof(double)) {
            printf("SIZE OVERFLOW: n=%zu cannot be represented as an allocation size; continuing.\n", n);
            continue;
        }

        A = (double*)malloc(n * sizeof(double));
        B = (double*)malloc(n * sizeof(double));

        if (A == NULL || B == NULL) {
            printf("ALLOCATION FAILED for n=%zu: unable to allocate both vectors.\n", n);
            free(A);
            free(B);
            continue;
        }

        initialize_vectors(A, B, n);
        expected = dot_c(A, B, n);

        printf("Reference C Result: %.12f\n\n", expected);
        run_supported_kernels(A, B, n, avx_available, avx2_available, expected, frequency);

        free(A);
        free(B);
    }

    test_boundary(avx_available, avx2_available);

    printf("\nProgram completed.\n");

    return 0;
}