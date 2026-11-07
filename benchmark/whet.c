/* GFOpenBenchmark: Whetstone, double precision (fpu.h). Written for this program from the
 * benchmark's published modules. The integer and branch modules' variables are volatile, so that
 * a modern compiler can't work their loops out at compile time (they have no inputs); the
 * floating-point modules are as the original.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <math.h>
#include "fpu.h"

static double T, T1, T2, E1[5];
static volatile long J, K, L;

static __attribute__((noinline)) void PA(double E[])
{
    int j = 0;
    do {
        E[1] = (E[1] + E[2] + E[3] - E[4]) * T;
        E[2] = (E[1] + E[2] - E[3] + E[4]) * T;
        E[3] = (E[1] - E[2] + E[3] + E[4]) * T;
        E[4] = (-E[1] + E[2] + E[3] + E[4]) / T2;
        j += 1;
    } while (j < 6);
}

static __attribute__((noinline)) void P0(void)
{
    E1[J] = E1[K];
    E1[K] = E1[L];
    E1[L] = E1[J];
}

static __attribute__((noinline)) void P3(double X, double Y, double *Z)
{
    double X1 = X, Y1 = Y;
    X1 = T * (X1 + Y1);
    Y1 = T * (X1 + Y1);
    *Z = (X1 + Y1) / T2;
}

double whet_run(unsigned long loops)
{
    unsigned long i, n2 = 12 * loops, n3 = 14 * loops, n4 = 345 * loops, n6 = 210 * loops, n7 = 32 * loops,
                     n8 = 899 * loops, n9 = 616 * loops, n11 = 93 * loops;
    double X, Y, Z, sum = 0;

    T = 0.499975;
    T1 = 0.50025;
    T2 = 2.0;

    /* module 2: array elements */
    E1[1] = 1.0;
    E1[2] = E1[3] = E1[4] = -1.0;
    for (i = 0; i < n2; i++) {
        E1[1] = (E1[1] + E1[2] + E1[3] - E1[4]) * T;
        E1[2] = (E1[1] + E1[2] - E1[3] + E1[4]) * T;
        E1[3] = (E1[1] - E1[2] + E1[3] + E1[4]) * T;
        E1[4] = (-E1[1] + E1[2] + E1[3] + E1[4]) * T;
    }
    sum += E1[1] + E1[2] + E1[3] + E1[4];

    /* module 3: an array as a parameter */
    for (i = 0; i < n3; i++) PA(E1);
    sum += E1[4];

    /* module 4: conditional jumps */
    J = 1;
    for (i = 0; i < n4; i++) {
        if (J == 1) J = 2; else J = 3;
        if (J > 2) J = 0; else J = 1;
        if (J < 1) J = 1; else J = 0;
    }
    sum += (double)J;

    /* module 6: integer arithmetic */
    J = 1; K = 2; L = 3;
    for (i = 0; i < n6; i++) {
        J = J * (K - J) * (L - K);
        K = L * K - (L - J) * K;
        L = (L - K) * (K + J);
        E1[L - 1] = (double)(J + K + L);
        E1[K - 1] = (double)(J * K * L);
    }
    sum += E1[1] + E1[2];

    /* module 7: trigonometric functions */
    X = Y = 0.5;
    for (i = 0; i < n7; i++) {
        X = T * atan(T2 * sin(X) * cos(X) / (cos(X + Y) + cos(X - Y) - 1.0));
        Y = T * atan(T2 * sin(Y) * cos(Y) / (cos(X + Y) + cos(X - Y) - 1.0));
    }
    sum += X + Y;

    /* module 8: procedure calls */
    X = Y = Z = 1.0;
    for (i = 0; i < n8; i++) P3(X, Y, &Z);
    sum += Z;

    /* module 9: array references */
    J = 1; K = 2; L = 3;
    E1[1] = 1.0; E1[2] = 2.0; E1[3] = 3.0;
    for (i = 0; i < n9; i++) P0();
    sum += E1[1] + E1[2] + E1[3];

    /* module 11: standard functions */
    X = 0.75;
    for (i = 0; i < n11; i++) X = sqrt(exp(log(X) / T1));
    sum += X;
    return sum;
}
