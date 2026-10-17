#ifndef DOT_H
#define DOT_H

#include <stddef.h>

double dot_c(const double* A, const double* B, size_t n);
double dot_x64(const double* A, const double* B, size_t n);
double dot_xmm(const double* A, const double* B, size_t n);
double dot_ymm(const double* A, const double* B, size_t n);

#endif