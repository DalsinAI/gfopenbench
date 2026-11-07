/* GFOpenBenchmark's portable kernels on the host: Dhrystone 2.1 leaves every value its listing says
 * it should, LINPACK 100 solves its system (x all ones, a residual of order 1), and Whetstone, DAXPY
 * and the single-precision loops give finite results.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <math.h>
#include "dhry.h"
#include "fpu.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { printf("FAIL: " __VA_ARGS__); printf("\n"); fails++; } } while (0)

int main(void)
{
    static const unsigned long runs[] = { 1, 2, 100, 50000 };
    struct dhry_result r;
    const char *bad;
    double resid, xerr, w, d;
    float f;
    unsigned i;
    for (i = 0; i < sizeof runs / sizeof runs[0]; i++) {
        dhry_run(runs[i], &r);
        bad = dhry_check(runs[i], &r);
        CHECK(!bad, "dhrystone after %lu runs: %s wrong", runs[i], bad ? bad : "");
    }
    lp_run(3);
    resid = lp_residual(&xerr);
    CHECK(resid > 0 && resid < 10, "linpack normalised residual %g", resid);
    CHECK(xerr < 1e-12, "linpack |x - 1| %g", xerr);
    w = whet_run(10);
    CHECK(isfinite(w), "whetstone %g", w);
    d = daxpy_run(1000);
    CHECK(isfinite(d) && d > 0, "daxpy %g", d);
    f = sgl_cmul_run(100000);
    CHECK(isfinite(f) && f > 0, "single multiply %g", f);
    f = sgl_cdiv_run(100000);
    CHECK(isfinite(f) && f > 0, "single divide %g", f);
    printf("test_benchmark: %s (linpack residual %.2f, whetstone sum %.6f)\n", fails ? "FAILED" : "ok", resid, w);
    return fails != 0;
}
