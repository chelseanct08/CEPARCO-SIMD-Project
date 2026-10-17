#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include "dot.h"

#define RUNS 30
#define BOUNDARY_SIZE 1003

typedef double (*DotKernel)(const double*, const double*, size_t);

double dot_c(const double* A, const double* B, size_t n)
{
    double sum = 0.0;
    size_t i;

    for (i = 0; i < n; i++) {
        sum += A[i] * B[i];
    }

    return sum;
}

double time_kernel(
    DotKernel kernel,
    const double* A,
    const double* B,
    size_t n,
    LARGE_INTEGER frequency)
{
    LARGE_INTEGER start, end;
    int i;

    QueryPerformanceCounter(&start);

    for (i = 0; i < RUNS; i++) {
        kernel(A, B, n);
    }

    QueryPerformanceCounter(&end);

    return (double)(end.QuadPart - start.QuadPart)
        / (double)frequency.QuadPart / RUNS;
}

int check_result(double expected, double actual)
{
    double difference = fabs(expected - actual);
    double tolerance = 1e-8 * (fabs(expected) + 1.0);

    return difference <= tolerance;
}

void initialize_vectors(double* A, double* B, size_t n)
{
    size_t i;

    for (i = 0; i < n; i++) {
        A[i] = sin(i * 0.001);
        B[i] = cos(i * 0.002);
    }
}

void test_boundary(void)
{
    double A[BOUNDARY_SIZE];
    double B[BOUNDARY_SIZE];
    double expected, actual;
    size_t i;

    DotKernel kernels[] = {
        dot_c,
        dot_x64,
        dot_xmm,
        dot_ymm
    };

    const char* names[] = {
        "C",
        "x86-64",
        "SIMD XMM",
        "SIMD YMM"
    };

    initialize_vectors(A, B, BOUNDARY_SIZE);
    expected = dot_c(A, B, BOUNDARY_SIZE);

    printf("\nBoundary Test: n = %d\n", BOUNDARY_SIZE);
    printf("----------------------------------------\n");

    for (i = 0; i < sizeof(kernels) / sizeof(kernels[0]); i++) {
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

    DotKernel kernels[] = {
        dot_c,
        dot_x64,
        dot_xmm,
        dot_ymm
    };

    const char* names[] = {
        "C",
        "x86-64",
        "SIMD XMM",
        "SIMD YMM"
    };

    LARGE_INTEGER frequency;
    size_t size_index, kernel_index;
    double* A;
    double* B;
    double expected, actual, average_time;
    size_t n;

    QueryPerformanceFrequency(&frequency);

    for (size_index = 0;
        size_index < sizeof(sizes) / sizeof(sizes[0]);
        size_index++) {

        n = sizes[size_index];

        printf("\n========================================\n");
        printf("Vector Size: %zu\n", n);
        printf("========================================\n");

        A = (double*)malloc(n * sizeof(double));
        B = (double*)malloc(n * sizeof(double));

        if (A == NULL || B == NULL) {
            printf("Memory allocation failed for n = %zu.\n", n);
            free(A);
            free(B);
            continue;
        }

        initialize_vectors(A, B, n);
        expected = dot_c(A, B, n);

        printf("Reference C Result: %.12f\n\n", expected);

        for (kernel_index = 0;
            kernel_index < sizeof(kernels) / sizeof(kernels[0]);
            kernel_index++) {

            actual = kernels[kernel_index](A, B, n);

            printf("%-10s Result: %.12f | Correctness: %s\n",
                names[kernel_index],
                actual,
                check_result(expected, actual) ? "PASS" : "FAIL");

            average_time = time_kernel(
                kernels[kernel_index],
                A,
                B,
                n,
                frequency);

            printf("           Average Time: %.6f seconds (%d runs)\n\n",
                average_time,
                RUNS);
        }

        free(A);
        free(B);
    }

    test_boundary();

    printf("\nProgram completed.\n");

    return 0;
}