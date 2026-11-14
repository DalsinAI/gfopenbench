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
 * The FAIL names the last "INFO C6 step:" line CLOSEWB wrote (from
 * WBGAP_C6_STEPS, which CLOSEWB closes after each line, else the Run file),
 * so a run shows which step hung (wbgap_closewb_last_step); an IoErr() on
 * opening the Run file is printed too, as Ackerman asked on #222.
 * When CLOSEWB had already decided C6 (wbgap_closewb_verdict) and only its
 * clean-up hung, C6 is that verdict and an INFO line names the step: the
 * 14 Nov 23:50 lab run hung at "closing and deleting RAM:WBGapC" after the
 * row's judging, on 3.2.3 alone and on GFOS after RESETREQ. */
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
    /* A row that ended has no use for its steps; a hung one keeps them for Type. */
    if (over) DeleteFile((CONST_STRPTR)WBGAP_C6_STEPS);
    if (buf[0]) printf("%s%s", buf, buf[strlen(buf) - 1] == '\n' ? "" : "\n");
    if (!over) {
        char why[192], step[96], said[128];
        int v;
        if (ioerr) printf("INFO C6 %s couldn't be opened to read (IoErr %ld)\n", path, ioerr);
        /* the steps file first: closed after each line, it has every step; buf may be held or short */
        if (!wbgap_closewb_last_step(steps, step, sizeof step)) wbgap_closewb_last_step(buf, step, sizeof step);
        if (wbgap_closewb_verdict(buf, said, sizeof said) != WBGAP_NONE) {
            /* the row's own PASS/FAIL is already in buf, printed above: only the clean-up hung */
            printf("INFO C6 the row decided, then its clean-up didn't end in %d s (left running); last step: %s\n", secs,
                   step[0] ? step : "none");
        } else if ((v = wbgap_closewb_verdict(steps, said, sizeof said)) != WBGAP_NONE) {
            printf("INFO C6 the row decided, then its clean-up didn't end in %d s (left running); last step: %s\n", secs,
                   step[0] ? step : "none");
            wbt_say("C6", v, said);
        } else {
            if (step[0])
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); last step: %s",
                         secs, step);
            else
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); no step written",
                         secs);
            wbt_fail("C6", why);
        }
    }
    return wbt_fails() ? 5 : 0;
}

int wbt_closewbwait(int argc, char **argv)
{
    if (argc < 4) { printf("usage: WBGapTest CLOSEWBWAIT <file> <secs>\n"); return 20; }
    return closewb_wait(argv[2], atoi(argv[3]));
}
