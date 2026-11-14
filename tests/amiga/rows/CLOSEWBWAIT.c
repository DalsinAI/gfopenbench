/* ---- CLOSEWBWAIT: C6's watcher (rows/CLOSEWB.c) ----
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "CLOSEWB.h"

WBGAP_MODE(CLOSEWBWAIT, wbt_closewbwait, SCRIPT)

/* CLOSEWBWAIT file secs: the script starts CLOSEWB with Run >file, then this
 * waits up to secs for it to be over (wbgap_closewb_over) and prints what it
 * wrote. A CloseWorkBench() that never returns (lab t2-killcrazy-wbgap, 14
 * Nov, before amicradle-guest #752) is then FAIL C6 here, and the run goes
 * on to REPORT; the CLOSEWB process is left as it is, with its own code.
 * The FAIL names the last "INFO C6 step:" line CLOSEWB flushed (from the
 * Run file, else WBGAP_C6_STEPS, which CLOSEWB closes after each line), so a
 * run shows which step hung (wbgap_closewb_last_step); an IoErr() on opening
 * the Run file is printed too, as Ackerman asked on #222. */
static int closewb_wait(const char *path, int secs)
{
    static char buf[4096], steps[2048];
    int t, over = 0;
    long n = 0, ioerr = 0;
    BPTR fh;
    for (t = 0; t <= secs * 2 && !over; t++) {
        if (t) Delay(25);
        buf[0] = 0;
        if ((fh = Open((CONST_STRPTR)path, MODE_OLDFILE))) {
            n = Read(fh, buf, sizeof buf - 1);
            buf[n > 0 ? n : 0] = 0;
            Close(fh);
            ioerr = 0;
        } else ioerr = IoErr();
        over = wbgap_closewb_over(buf);
    }
    steps[0] = 0;
    if (!over && (fh = Open((CONST_STRPTR)WBGAP_C6_STEPS, MODE_OLDFILE))) {
        n = Read(fh, steps, sizeof steps - 1);
        steps[n > 0 ? n : 0] = 0;
        Close(fh);
    }
    if (buf[0]) printf("%s%s", buf, buf[strlen(buf) - 1] == '\n' ? "" : "\n");
    if (!over) {
        char why[192], step[96];
        if (ioerr) printf("INFO C6 %s couldn't be opened to read (IoErr %ld)\n", path, ioerr);
        if (wbgap_closewb_last_step(buf, step, sizeof step) || wbgap_closewb_last_step(steps, step, sizeof step))
            snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); last step: %s",
                     secs, step);
        else
            snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); no step written",
                     secs);
        wbt_fail("C6", why);
    }
    return wbt_fails() ? 5 : 0;
}

int wbt_closewbwait(int argc, char **argv)
{
    if (argc < 4) { printf("usage: WBGapTest CLOSEWBWAIT <file> <secs>\n"); return 20; }
    return closewb_wait(argv[2], atoi(argv[3]));
}
