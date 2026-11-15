/* ICONFMT's judge (ICONFMT.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "ICONFMT.h"

const char *wbgap_iconfmt_name(int k)
{
    /* b to e: after a, in the order they are made, so a desktop laying a drawer out by name (GFOpenBench) adds each at
     * the end and moves nothing else */
    static const char *const names[WBGAP_ICONFMT_N] = {"b_classic", "c_newicons", "d_glowicons", "e_png"};
    return k >= 0 && k < WBGAP_ICONFMT_N ? names[k] : "?";
}

int wbgap_iconfmt_near(unsigned long rgb, unsigned long want, unsigned tol)
{
    int sh;
    for (sh = 0; sh <= 16; sh += 8) {
        int d = (int)(rgb >> sh & 0xff) - (int)(want >> sh & 0xff);
        if (d < 0) d = -d;
        if ((unsigned)d > tol) return 0;
    }
    return 1;
}

int wbgap_iconfmt_judge(const int *n, const int *near, const int *pen1, char *why, unsigned cap)
{
    int k, bad = 0;
    size_t at = 0;
    if (why && cap) why[0] = 0;
    for (k = 0; k < WBGAP_ICONFMT_N; k++)
        if (n[k] < 10) {
            if (why && cap) snprintf(why, cap, "only %d points of the %s icon's picture read: nothing to judge", n[k], wbgap_iconfmt_name(k));
            return WBGAP_SKIP;
        }
    for (k = 0; k < WBGAP_ICONFMT_N; k++) {
        if (near[k] * 4 >= n[k]) continue;
        bad++;
        if (why && cap && at < cap) {
            int w = snprintf(why + at, cap - at, "%s%s: %d of %d points its colour%s", at ? "; " : "", wbgap_iconfmt_name(k),
                             near[k], n[k], pen1[k] >= 0 && pen1[k] * 4 >= n[k] ? ", the classic picture (pen 1) shown instead" : "");
            if (w > 0) at += (size_t)w;
        }
    }
    return bad ? WBGAP_FAIL : WBGAP_PASS;
}
