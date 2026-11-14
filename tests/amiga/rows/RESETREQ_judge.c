/* C5a's judge and its prefs helpers (RESETREQ.h), plain C for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "RESETREQ.h"

int wbgap_resetreq_judge(const struct wbgap_resetreq *r, char *why, unsigned cap)
{
    char t[240] = "";
    int v = WBGAP_FAIL;
    if (!r || r->no_prefs) { snprintf(t, sizeof t, "no ScreenMode prefs (ENV:Sys/screenmode.prefs with an SCRM) to change"); v = WBGAP_SKIP; }
    else if (r->mine_failed) snprintf(t, sizeof t, "the row's own window didn't open (a test fault, not the desktop's)");
    else if (r->runner) {                /* the Shell's window stays in the way: up, and Cancel takes it down */
        if (!r->shown) snprintf(t, sizeof t, "no requester came up for the change while a program's window was open");
        else if (!r->cancel_gone) snprintf(t, sizeof t, "Cancel didn't take the requester down (this Shell's window in the way)");
        else if (r->depth_cancel != r->depth_before)
            snprintf(t, sizeof t, "after Cancel the Workbench changed anyway: depth %d, was %d", r->depth_cancel, r->depth_before);
        else {
            snprintf(t, sizeof t, "the reset half needs no Shell window on the Workbench (run WBGapTest RESETREQ with "
                                  "amiga_run); here the requester came up and Cancel took it down, depth kept");
            v = WBGAP_SKIP;
        }
    }
    else if (!r->shown) snprintf(t, sizeof t, "no requester came up for the change while a program's window was open");
    else if (!r->gone) snprintf(t, sizeof t, "the requester stayed after the window in its way closed");
    else if (r->depth_after != r->depth_want)
        snprintf(t, sizeof t, "the Workbench wasn't reset into the new mode: depth %d, asked for %d", r->depth_after, r->depth_want);
    else if (r->cancel_tried && !r->cancel_shown) snprintf(t, sizeof t, "Cancel half: no requester came up");
    else if (r->cancel_tried && !r->cancel_gone) snprintf(t, sizeof t, "Cancel didn't take the requester down");
    else if (r->cancel_tried && r->depth_cancel != r->depth_after)
        snprintf(t, sizeof t, "after Cancel the Workbench changed anyway: depth %d, was %d", r->depth_cancel, r->depth_after);
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", t);
    return v;
}

int wbgap_smod_depth(unsigned char *buf, long len, int depth, int *old)
{
    long i;
    for (i = 12; buf && i + 8 <= len; ) {          /* after FORM <len> PREF: the chunks */
        unsigned long n = (unsigned long)buf[i + 4] << 24 | (unsigned long)buf[i + 5] << 16 | buf[i + 6] << 8 | buf[i + 7];
        if (n > (unsigned long)(len - i - 8)) return 0;   /* past the file: on a 68k a length of $80000000 or more went negative (G'Kar) */
        if (!memcmp(buf + i, "SCRM", 4)) {
            unsigned char *d = buf + i + 8;
            if (n < 28 || i + 8 + 28 > len) return 0;
            if (old) *old = d[24] << 8 | d[25];
            d[24] = (unsigned char)(depth >> 8);
            d[25] = (unsigned char)depth;
            return 1;
        }
        i += 8 + (long)n + (long)(n & 1);
    }
    return 0;
}

int wbgap_other_depth(int depth)
{
    if (depth > 8) return depth == 16 ? 24 : 16;
    return depth > 1 ? depth - 1 : 2;
}
