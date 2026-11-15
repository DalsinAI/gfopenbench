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
 * clean-up hung, C6 is that verdict and C6a is a FAIL naming the step.
 * CLOSEWB is over when "INFO C6 done" is in the Run file OR the steps file:
 * on 3.2.3 (15 Nov 00:32 lab) CLOSEWB ended, Status listing no WBGapTest,
 * but none of its stdout reached the Run file, so it looked hung; its
 * verdicts are then taken from the steps file (wbgap_closewb_row_verdict). */
static long read_text(const char *path, char *to, long cap, long *ioerr)
{
    BPTR fh;
    long n = 0;
    to[0] = 0;
    if ((fh = Open((CONST_STRPTR)path, MODE_OLDFILE))) {
        n = Read(fh, to, cap - 1);
        to[n > 0 ? n : 0] = 0;
        Close(fh);
        if (ioerr) *ioerr = 0;
    } else if (ioerr) *ioerr = IoErr();
    return n;
}

/* one of CLOSEWB's rows: its line already printed from buf, else from the steps file, else the FAIL or SKIP given */
static void closewb_row(const char *row, const char *buf, const char *steps, int none_state, const char *none_why)
{
    char said[128];
    int v;
    if (wbgap_closewb_row_verdict(buf, row, said, sizeof said) != WBGAP_NONE) return;   /* printed with buf */
    if ((v = wbgap_closewb_row_verdict(steps, row, said, sizeof said)) != WBGAP_NONE) wbt_say(row, v, said);
    else wbt_say(row, none_state, none_why);
}

static int closewb_wait(const char *path, int secs)
{
    static char buf[4096], steps[2048];
    int t, over = 0;
    long ioerr = 0;
    for (t = 0; t <= secs * 2 && !over; t++) {
        if (t) Delay(25);
        read_text(path, buf, sizeof buf, &ioerr);
        read_text(WBGAP_C6_STEPS, steps, sizeof steps, NULL);
        over = wbgap_closewb_over(buf) || wbgap_closewb_over(steps);
    }
    if (buf[0]) printf("%s%s", buf, buf[strlen(buf) - 1] == '\n' ? "" : "\n");
    if (ioerr) printf("INFO C6 %s couldn't be opened to read (IoErr %ld)\n", path, ioerr);
    if (over) {
        /* ended: its verdicts, from buf or the steps file (the Run file may have none of its stdout) */
        closewb_row("C6", buf, steps, WBGAP_FAIL, "CLOSEWB ended without a C6 line");
        closewb_row("C6a", buf, steps, WBGAP_FAIL, "CLOSEWB ended without a C6a line");
        DeleteFile((CONST_STRPTR)WBGAP_C6_STEPS);    /* a row that ended has no use for its steps; a hung one keeps them */
    } else {
        char why[192], step[96];
        /* the steps file first: closed after each line, it has every step; buf may be held or short */
        if (!wbgap_closewb_last_step(steps, step, sizeof step)) wbgap_closewb_last_step(buf, step, sizeof step);
        if (wbgap_closewb_verdict(buf, why, sizeof why) != WBGAP_NONE || wbgap_closewb_verdict(steps, why, sizeof why) != WBGAP_NONE) {
            /* C6 decided; its clean-up hung: a real fault, counted as C6a (Ackerman, #222) */
            snprintf(why, sizeof why, "%s didn't end in %d s (left running)", step[0] ? step : "the clean-up", secs);
            closewb_row("C6", buf, steps, WBGAP_FAIL, "");
            closewb_row("C6a", buf, steps, WBGAP_FAIL, why);
        } else {
            if (step[0])
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); last step: %s",
                         secs, step);
            else
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); no step written",
                         secs);
            wbt_fail("C6", why);
            wbt_skip("C6a", "C6 didn't decide");
        }
    }
    return wbt_fails() ? 5 : 0;
}

int wbt_closewbwait(int argc, char **argv)
{
    if (argc < 4) { printf("usage: WBGapTest CLOSEWBWAIT <file> <secs>\n"); return 20; }
    return closewb_wait(argv[2], atoi(argv[3]));
}
