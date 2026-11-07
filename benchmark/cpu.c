/* GFOpenBenchmark: the integer CPU. Dhrystone 2.1 (dhry.h) as Dhrystones a second and Dhrystone
 * MIPS, and ACInfo SPEED's figure in SysInfo's units (common/sysprobe in amicradle-guest).
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include "gfb.h"
#include "dhry.h"
#include "sysprobe.h"

static struct dhry_result dr;

static double dhry_work(void *user, unsigned long n)
{
    (void)user;
    dhry_run(n, &dr);
    return dr.Int_1_Loc;
}

void gfb_cpu_tests(void)
{
    struct gfb_result r, m;
    struct sp_speed sp;
    ULONG d[GFB_RUNS], x;
    const char *bad;
    int i, j;

    gfb_section("CPU");
    memset(&r, 0, sizeof r);
    r.key = "dhry_dps";
    r.label = "Dhrystone 2.1";
    r.unit = "Dhrystones/s";
    gfb_time(&r, dhry_work, NULL, 1.0, 1);
    if ((bad = dhry_check(r.n, &dr))) {
        sprintf(r.note, "(WRONG: %s after %lu runs)", bad, r.n);
    }
    gfb_report(&r);
    m = r;
    m.key = "dmips";
    m.label = "Dhrystone MIPS (/1757, VAX = 1)";
    m.unit = "DMIPS";
    m.value /= DHRY_VAX_DPS;
    for (i = 0; i < GFB_RUNS; i++) m.runs[i] /= DHRY_VAX_DPS;
    m.note[0] = 0;
    gfb_report(&m);

    /* ACInfo SPEED's loop, as GFOpenMonitor's Speed view runs it: a second under Forbid, three times */
    memset(&r, 0, sizeof r);
    r.key = "sysinfo_mips";
    r.label = "SysInfo-scale MIPS (ACInfo loop)";
    r.unit = "MIPS";
    for (i = 0; i < GFB_RUNS; i++) {
        if (!sp_speed(&sp)) break;
        d[i] = sp.dhry;
        r.runs[i] = (double)sp.dhry / SP_SYSINFO_MIPS;
    }
    if (i < GFB_RUNS) gfb_fail(&r, "no timer.device");
    else {
        for (i = 1; i < GFB_RUNS; i++)
            for (j = i; j > 0 && d[j] < d[j - 1]; j--) { x = d[j]; d[j] = d[j - 1]; d[j - 1] = x; }
        r.value = (double)d[GFB_RUNS / 2] / SP_SYSINFO_MIPS;
        r.seconds = 1.0;
        r.ok = 1;
        strcpy(r.note, "(its count / 958)");
    }
    gfb_report(&r);
}
