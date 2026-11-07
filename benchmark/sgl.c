/* GFOpenBenchmark: single-precision multiply and divide loops (fpu.h). Four chains, so a pipelined
 * FPU can overlap them; each value is multiplied (or divided) by m and then by n, alternately, with
 * m n close to 1, so it stays normal for billions of steps.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include "fpu.h"

/* volatile so the compiler loads them at run time and can't fold the chains */
static volatile float vm = 1.0001f, vn = 0.9999f;

float sgl_cmul_run(unsigned long passes)
{
    float a = 1, b = 1, c = 1, d = 1, m = vm, n = vn;
    unsigned long p;
    for (p = 0; p < passes; p++) {
        a *= m; b *= n; c *= m; d *= n;
        a *= n; b *= m; c *= n; d *= m;
    }
    return a + b + c + d;
}

float sgl_cdiv_run(unsigned long passes)
{
    float a = 1, b = 1, c = 1, d = 1, m = vm, n = vn;
    unsigned long p;
    for (p = 0; p < passes; p++) {
        a /= m; b /= n; c /= m; d /= n;
        a /= n; b /= m; c /= n; d /= m;
    }
    return a + b + c + d;
}

#ifdef GFB_HAVE_FSGL
/* The 68881's single-precision-mantissa forms, FSGLMUL.X and FSGLDIV.X, on FP0-FP3 by FP4 and FP5. */
#define FSGL_LOOP(op)                                                                                     \
    float r;                                                                                              \
    float m = vm, n = vn, one = 1.0f;                                                                     \
    unsigned long cnt = passes;                                                                           \
    __asm__ __volatile__(                                                                                 \
        "fmove.s %2,%%fp4\n\t"                                                                            \
        "fmove.s %3,%%fp5\n\t"                                                                            \
        "fmove.s %4,%%fp0\n\t"                                                                            \
        "fmove.x %%fp0,%%fp1\n\t"                                                                         \
        "fmove.x %%fp0,%%fp2\n\t"                                                                         \
        "fmove.x %%fp0,%%fp3\n\t"                                                                         \
        "tst.l %1\n\t"                                                                                    \
        "beq.s 2f\n"                                                                                      \
        "1:\n\t"                                                                                          \
        op " %%fp4,%%fp0\n\t" op " %%fp5,%%fp1\n\t" op " %%fp4,%%fp2\n\t" op " %%fp5,%%fp3\n\t"         \
        op " %%fp5,%%fp0\n\t" op " %%fp4,%%fp1\n\t" op " %%fp5,%%fp2\n\t" op " %%fp4,%%fp3\n\t"         \
        "subq.l #1,%1\n\t"                                                                                \
        "bne.s 1b\n"                                                                                      \
        "2:\n\t"                                                                                          \
        "fadd.x %%fp1,%%fp0\n\t"                                                                          \
        "fadd.x %%fp2,%%fp0\n\t"                                                                          \
        "fadd.x %%fp3,%%fp0\n\t"                                                                          \
        "fmove.s %%fp0,%0"                                                                                \
        : "=m"(r), "+d"(cnt)                                                                              \
        : "m"(m), "m"(n), "m"(one)                                                                        \
        : "fp0", "fp1", "fp2", "fp3", "fp4", "fp5", "cc");                                                \
    return r

float fsglmul_run(unsigned long passes) { FSGL_LOOP("fsglmul.x"); }
float fsgldiv_run(unsigned long passes) { FSGL_LOOP("fsgldiv.x"); }
#endif
