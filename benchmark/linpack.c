/* GFOpenBenchmark: LINPACK 100 and DAXPY in double precision (fpu.h). Written for this program
 * from the LINPACK benchmark's published method: matgen's numbers, dgefa (LU with partial
 * pivoting) and dgesl, with rolled BLAS (idamax, dscal, daxpy).
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <math.h>
#include <float.h>
#include "fpu.h"

#define LDA (LP_N + 1)

static double a[LP_N][LDA];    /* column j is a[j][0..n-1], as LINPACK's Fortran order */
static double b[LP_N], x[LP_N];
static int ipvt[LP_N];

/* LINPACK's matgen: the same pseudo-random matrix each time, and b its row sums, so x is all ones. */
static double matgen(void)
{
    int init = 1325, i, j;
    double norma = 0;
    for (j = 0; j < LP_N; j++)
        for (i = 0; i < LP_N; i++) {
            init = 3125 * init % 65536;
            a[j][i] = (init - 32768.0) / 16384.0;
            if (a[j][i] > norma) norma = a[j][i];
        }
    for (i = 0; i < LP_N; i++) b[i] = 0;
    for (j = 0; j < LP_N; j++)
        for (i = 0; i < LP_N; i++) b[i] += a[j][i];
    return norma;
}

static __attribute__((noinline)) void daxpy(int n, double da, const double *dx, double *dy)
{
    int i;
    if (n <= 0 || da == 0) return;
    for (i = 0; i < n; i++) dy[i] += da * dx[i];
}

static __attribute__((noinline)) void dscal(int n, double da, double *dx)
{
    int i;
    for (i = 0; i < n; i++) dx[i] *= da;
}

static __attribute__((noinline)) int idamax(int n, const double *dx)
{
    int i, m = 0;
    double dmax;
    if (n < 1) return -1;
    dmax = fabs(dx[0]);
    for (i = 1; i < n; i++)
        if (fabs(dx[i]) > dmax) { m = i; dmax = fabs(dx[i]); }
    return m;
}

static void dgefa(void)
{
    int j, k, l;
    double t;
    for (k = 0; k < LP_N - 1; k++) {
        l = idamax(LP_N - k, &a[k][k]) + k;
        ipvt[k] = l;
        if (a[k][l] == 0) continue;
        if (l != k) { t = a[k][l]; a[k][l] = a[k][k]; a[k][k] = t; }
        t = -1.0 / a[k][k];
        dscal(LP_N - k - 1, t, &a[k][k + 1]);
        for (j = k + 1; j < LP_N; j++) {
            t = a[j][l];
            if (l != k) { a[j][l] = a[j][k]; a[j][k] = t; }
            daxpy(LP_N - k - 1, t, &a[k][k + 1], &a[j][k + 1]);
        }
    }
    ipvt[LP_N - 1] = LP_N - 1;
}

static void dgesl(void)
{
    int k, l;
    double t;
    for (k = 0; k < LP_N - 1; k++) {
        l = ipvt[k];
        t = b[l];
        if (l != k) { b[l] = b[k]; b[k] = t; }
        daxpy(LP_N - k - 1, t, &a[k][k + 1], &b[k + 1]);
    }
    for (k = LP_N - 1; k >= 0; k--) {
        b[k] /= a[k][k];
        t = -b[k];
        daxpy(k, t, &a[k][0], &b[0]);
    }
}

/* Each solve sets the matrix up again (matgen is O(n^2), as LINPACK's own timing leaves in). */
void lp_run(unsigned long solves)
{
    unsigned long s;
    for (s = 0; s < solves; s++) {
        matgen();
        dgefa();
        dgesl();
    }
}

double lp_residual(double *xerr)
{
    int i, j;
    double norma, resid = 0, normx = 0, err = 0;
    for (i = 0; i < LP_N; i++) x[i] = b[i];
    norma = matgen();
    for (i = 0; i < LP_N; i++) b[i] = -b[i];
    for (j = 0; j < LP_N; j++)            /* b = -b + A x */
        for (i = 0; i < LP_N; i++) b[i] += a[j][i] * x[j];
    for (i = 0; i < LP_N; i++) {
        if (fabs(b[i]) > resid) resid = fabs(b[i]);
        if (fabs(x[i]) > normx) normx = fabs(x[i]);
        if (fabs(x[i] - 1.0) > err) err = fabs(x[i] - 1.0);
    }
    if (xerr) *xerr = err;
    return resid / (LP_N * norma * normx * DBL_EPSILON);
}

static double vx[DAXPY_N], vy[DAXPY_N];

double daxpy_run(unsigned long passes)
{
    unsigned long p;
    int i;
    double s = 0;
    for (i = 0; i < DAXPY_N; i++) { vx[i] = 1.0 + i * 0.001; vy[i] = 0; }
    for (p = 0; p < passes; p++) daxpy(DAXPY_N, 1.0e-6, vx, vy);
    for (i = 0; i < DAXPY_N; i++) s += vy[i];
    return s;
}
