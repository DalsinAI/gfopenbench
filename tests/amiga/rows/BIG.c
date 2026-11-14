/* ---- X1: 1000 files ----
 * RAM:WBGapBig, a drawer of 1000 files each with an icon, is opened and timed from the open to the last change of its
 * inside before a second without one (wbgap_big_ticks); under 10 s is a PASS (wbgap_big_judge). The
 * drawer is closed again after, whatever the time.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <intuition/intuition.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "BIG.h"

WBGAP_MODE(BIG, wbt_big, SCRIPT)
WBGAP_ROWMODE(X1, BIG)

int wbt_big(int argc, char **argv)
{
    char path[64], why[64];
    struct Window *w;
    struct DateStamp a, b;
    BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapBig");
    int i, how;
    long ticks;
    (void)argc, (void)argv;
    if (l) UnLock(l);
    wbt_put_icon("RAM:WBGapBig", WBDRAWER, NULL);
    for (i = 0; i < 1000; i++) {
        snprintf(path, sizeof path, "RAM:WBGapBig/file%04d", i);
        wbt_write_file(path, "x\n");
        wbt_put_icon(path, WBPROJECT, NULL);
    }
    DateStamp(&a);
    w = wbt_open_drawer("RAM:WBGapBig", "WBGapBig", 60, &how);
    if (!w) {
        wbt_fail("X1", "RAM:WBGapBig's window didn't open in 60 s");
        wbt_clear_drawer("RAM:WBGapBig", "WBGapBig", "X1");   /* in case it opens late, over the desktop */
        return 10;
    }
    DateStamp(&b);                              /* open; then each change of the inside moves it on */
    {   /* drawn: the inside stops changing for a second. The time is to the last change seen, so the second of
         * stillness and the probe's own looks aren't in it (Hudzen, 14 Nov 2026: X1 measured its polls) */
        struct DateStamp p0, p1;
        unsigned long last, now;
        int still = 0, t;
        long probe;
        DateStamp(&p0);
        last = wbt_inside_sum(w);
        DateStamp(&p1);
        probe = (p1.ds_Minute - p0.ds_Minute) * 3000L + (p1.ds_Tick - p0.ds_Tick);
        printf("INFO X1 window %ld x %ld, one look %ld ms\n", (long)w->Width, (long)w->Height, probe * 20);
        for (t = 0; t < 600 && still < 5; t++) {
            Delay(10);
            now = wbt_inside_sum(w);
            if (now != last) { DateStamp(&b); still = 0; } else still++;
            last = now;
        }
    }
    ticks = wbgap_big_ticks(a.ds_Days, a.ds_Minute, a.ds_Tick, b.ds_Days, b.ds_Minute, b.ds_Tick);
    printf("INFO X1 1000 icons shown in %ld.%02ld s\n", ticks / 50, (ticks % 50) * 2);
    wbt_say("X1", wbgap_big_judge(ticks, why, sizeof why), why);
    /* closed again, and waited for: left open over the desktop's left-out icons, the next rows' clicks landed on its
     * file icons and stopped an unattended run on "has no default tool" (Franklin's lab, 14 November 2026) */
    wbt_clear_drawer("RAM:WBGapBig", "WBGapBig", "X1");
    return wbt_fails() ? 5 : 0;
}
