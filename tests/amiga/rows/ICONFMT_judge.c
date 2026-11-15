/* ICONFMT's judge (ICONFMT.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "ICONFMT.h"

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

const char *wbgap_iconfmt_name(int k)
{
    /* b to e: after a, in the order they are made, so a desktop laying a drawer out by name (GFOpenBench) adds each at
     * the end */
    static const char *const names[WBGAP_ICONFMT_N] = {"b_classic", "c_newicons", "d_glowicons", "e_png"};
    return k >= 0 && k < WBGAP_ICONFMT_N ? names[k] : "?";
}

int wbgap_iconfmt_judge(int n, const int *near, int pen1_added, char *why, unsigned cap)
{
    int k, bad = 0;
    size_t at = 0;
    if (why && cap) why[0] = 0;
    if (n < 2000) {
        if (why && cap) snprintf(why, cap, "only %d points of the drawer's inside read: nothing to judge", n);
        return WBGAP_SKIP;
    }
    for (k = 0; k < WBGAP_ICONFMT_N; k++) {
        if (near[k] >= WBGAP_ICONFMT_MIN) continue;
        bad++;
        if (why && cap && at < cap) {
            int w = snprintf(why + at, cap - at, "%s%s: %d points its colour", at ? "; " : "", wbgap_iconfmt_name(k), near[k]);
            if (w > 0) at += (size_t)w;
        }
    }
    if (bad && why && cap && at < cap && pen1_added >= 2 * WBGAP_ICONFMT_MIN)
        snprintf(why + at, cap - at, " (pen 1 grew by %d points: a classic picture shown in place)", pen1_added);
    return bad ? WBGAP_FAIL : WBGAP_PASS;
}
