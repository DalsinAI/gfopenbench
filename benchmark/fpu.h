/* GFOpenBenchmark: the floating-point kernels. Portable C (the host tests run them), but for the
 * FSGLMUL and FSGLDIV loops, which are the 68881/68040's own instructions in inline assembly and
 * exist only in an m68k build with the FPU.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#ifndef GFB_FPU_H
#define GFB_FPU_H

/* Whetstone (H. J. Curnow and B. A. Wichmann, 1976), double precision, with the module weights of
 * the C translation everyone quotes (whetstone.c: N2 12, N3 14, N4 345, N6 210, N7 32, N8 899,
 * N9 616, N11 93 per loop). One loop is 100,000 Whetstone instructions: MWIPS = loops / 10 / s.
 * Returns a sum of its results (finite when it worked). */
double whet_run(unsigned long loops);

/* LINPACK 100 (J. Dongarra's benchmark): a 100x100 double system solved by Gaussian elimination
 * with partial pivoting (dgefa, dgesl, rolled BLAS). One solve is 2/3 n^3 + 2 n^2 flops. */
#define LP_N 100
#define LP_FLOPS (2.0 * LP_N * LP_N * LP_N / 3.0 + 2.0 * LP_N * LP_N)
void lp_run(unsigned long solves);
/* After lp_run: the normalised residual of the last solve, |b - Ax| / (n |A| |x| eps), which
 * LINPACK expects to be of order 1; and |x - 1|'s largest term (its right answer is all ones). */
double lp_residual(double *xerr);

/* DAXPY, y = y + a x on two vectors of 1,000 doubles: 2,000 flops a pass. */
#define DAXPY_N 1000
double daxpy_run(unsigned long passes);

/* Single precision in C, four chains, each step x = x * m (or x / m) on a float: the compiler's
 * own instructions (on the 68040 with -m68881, FSMUL.S/FSDIV.S style rounding to single). 8 ops a
 * pass. */
float sgl_cmul_run(unsigned long passes);
float sgl_cdiv_run(unsigned long passes);

#if defined(__mc68000__) && defined(__HAVE_68881__)
#define GFB_HAVE_FSGL 1
/* FSGLMUL.X and FSGLDIV.X themselves, 8 a pass on four registers. */
float fsglmul_run(unsigned long passes);
float fsgldiv_run(unsigned long passes);
#endif

#endif
