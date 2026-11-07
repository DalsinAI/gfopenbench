/* GFOpenBenchmark: the FPU (fpu.h). Whetstone in MWIPS; LINPACK 100, DAXPY and the single-precision
 * loops in MFLOPS (millions of floating-point operations a second).
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <exec/types.h>
#include "gfb.h"
#include "fpu.h"

static double w_whet(void *u, unsigned long n) { (void)u; return whet_run(n); }
static double w_lp(void *u, unsigned long n) { (void)u; lp_run(n); return 0; }
static double w_daxpy(void *u, unsigned long n) { (void)u; return daxpy_run(n); }
static double w_cmul(void *u, unsigned long n) { (void)u; return sgl_cmul_run(n); }
static double w_cdiv(void *u, unsigned long n) { (void)u; return sgl_cdiv_run(n); }
#ifdef GFB_HAVE_FSGL
static double w_fsglmul(void *u, unsigned long n) { (void)u; return fsglmul_run(n); }
static double w_fsgldiv(void *u, unsigned long n) { (void)u; return fsgldiv_run(n); }
#endif

static void one(const char *key, const char *label, const char *unit, gfb_work w, double scale)
{
    struct gfb_result r;
    memset(&r, 0, sizeof r);
    r.key = key;
    r.label = label;
    r.unit = unit;
    gfb_time(&r, w, NULL, scale, 1);
    if (key[0] == 'l') {        /* LINPACK: was the answer right? */
        double xerr, res = lp_residual(&xerr);
        if (!(res < 10) || !(xerr < 1e-9)) sprintf(r.note, "(WRONG: residual %.2f, |x-1| %.3g)", res, xerr);
        else sprintf(r.note, "(residual %.2f)", res);
    }
    gfb_report(&r);
}

void gfb_fpu_tests(void)
{
    gfb_section("FPU");
    one("whet_mwips", "Whetstone (double)", "MWIPS", w_whet, 0.1);
    one("linpack_mflops", "LINPACK 100 (double)", "MFLOPS", w_lp, LP_FLOPS / 1e6);
    one("daxpy_mflops", "DAXPY, 1000 doubles", "MFLOPS", w_daxpy, 2.0 * DAXPY_N / 1e6);
    one("fmul_s_mflops", "Single multiply (C, FMUL)", "MFLOPS", w_cmul, 8 / 1e6);
    one("fdiv_s_mflops", "Single divide (C, FDIV)", "MFLOPS", w_cdiv, 8 / 1e6);
#ifdef GFB_HAVE_FSGL
    one("fsglmul_mflops", "FSGLMUL.X", "MFLOPS", w_fsglmul, 8 / 1e6);
    one("fsgldiv_mflops", "FSGLDIV.X", "MFLOPS", w_fsgldiv, 8 / 1e6);
#endif
}
