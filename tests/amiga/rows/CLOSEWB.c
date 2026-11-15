/* ---- C6 (CLOSEWB): CloseWorkBench / OpenWorkBench with programs running ----
 * RAM:WBGapC's drawer window open, CloseWorkBench() is called; if it closed the screen, OpenWorkBench(). Then up to
 * 10 s for the desktop's port and the drawer's window to be back (wbgap_closewb_judge). The script starts this with
 * Run and waits for it with CLOSEWBWAIT (rows/CLOSEWBWAIT.c), since a CloseWorkBench() may never return.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "CLOSEWB.h"

WBGAP_MODE(CLOSEWB, wbt_closewb, SCRIPT)
WBGAP_ROWMODE(C6, CLOSEWB)
WBGAP_ROWMODE(C6a, CLOSEWB)

/* CLOSEWB's stdout is a file (Run >RAM:wbgap-c6), which stdio holds until
 * exit; each step is flushed, and also appended to WBGAP_C6_STEPS (closed each
 * time, so it can be read while CLOSEWB runs), so CLOSEWBWAIT can name the
 * last one reached when the process doesn't end in time (wbgap_closewb_last_step). */
/* a line appended to WBGAP_C6_STEPS, opened and closed for it: the Run >file output may be held while CLOSEWB runs */
static void steps_add(const char *line, int n)
{
    BPTR fh;
    if (n > 0 && (fh = Open((CONST_STRPTR)WBGAP_C6_STEPS, MODE_READWRITE))) {
        Seek(fh, 0, OFFSET_END);
        Write(fh, (APTR)line, n);
        Close(fh);
    }
}

static void step(const char *what)
{
    char line[96];
    int n = snprintf(line, sizeof line, "INFO C6 step: %s\n", what);
    printf("%s", line);
    fflush(stdout);
    steps_add(line, n < (int)sizeof line ? n : (int)sizeof line - 1);
}

/* a row's verdict in the steps file too (the Run file may never get stdout): wbt_say and an INFO <row> verdict line */
static void say(const char *row, int v, const char *why)
{
    char line[160];
    int n;
    wbt_say(row, v, why);
    fflush(stdout);
    n = snprintf(line, sizeof line, "INFO %s verdict: %s%s%s\n", row, v == WBGAP_PASS ? "PASS" : v == WBGAP_FAIL ? "FAIL" : "SKIP",
                 v == WBGAP_PASS || !why || !why[0] ? "" : " ", v == WBGAP_PASS || !why ? "" : why);
    steps_add(line, n < (int)sizeof line ? n : (int)sizeof line - 1);
}

static int desk_port(void)
{
    int up;
    Forbid();
    up = FindPort((CONST_STRPTR)"GFOPENBENCH") || FindPort((CONST_STRPTR)"WORKBENCH");
    Permit();
    return up;
}

static struct Screen *wb_screen_now(void)
{
    struct Screen *s = LockPubScreen((CONST_STRPTR)"Workbench");
    if (s) UnlockPubScreen(NULL, s);              /* not held: a lock would stop CloseWorkBench() */
    return s;
}

static int closewb_row(void)
{
    struct Screen *before, *after;
    struct Window *w;
    LONG closed;
    int how, reopened = 0, t, port = 0, drawer = 0, v, cleared;
    char why[96];
    BPTR l;
    DeleteFile((CONST_STRPTR)WBGAP_C6_STEPS);
    step("making RAM:WBGapC");
    l = CreateDir((CONST_STRPTR)"RAM:WBGapC");
    if (l) UnLock(l);
    wbt_put_icon("RAM:WBGapC", WBDRAWER, NULL);
    step("opening RAM:WBGapC's window");
    if (!(w = wbt_open_drawer("RAM:WBGapC", "WBGapC", 10, &how))) {
        say("C6", WBGAP_SKIP, "RAM:WBGapC's window didn't open");
        say("C6a", WBGAP_SKIP, "RAM:WBGapC's window didn't open");
        return 0;
    }
    if (!(before = wb_screen_now())) {
        say("C6", WBGAP_SKIP, "no Workbench screen");
        say("C6a", WBGAP_SKIP, "no Workbench screen");
        return 0;
    }
    step("calling CloseWorkBench()");
    closed = CloseWorkBench();
    printf("INFO C6 CloseWorkBench() gave %ld\n", (long)closed);
    if (closed) {
        step("calling OpenWorkBench()");
        reopened = OpenWorkBench() != 0;
        printf("INFO C6 OpenWorkBench() %s\n", reopened ? "opened the screen again" : "failed");
    }
    step("waiting up to 10 s for the desktop and RAM:WBGapC's window");
    for (t = 0; t < 100; t++) {                     /* up to 10 s for the desktop and its drawer to be back */
        port = desk_port();
        drawer = wbt_find_window("WBGapC") != NULL;
        if (port && drawer) break;
        Delay(5);
    }
    after = wb_screen_now();
    v = wbgap_closewb_judge(closed != 0, reopened, after == before, port, drawer, why, sizeof why);
    say("C6", v, why);
    step("closing RAM:WBGapC's window");
    cleared = wbt_clear_drawer("RAM:WBGapC", "WBGapC", "C6");
    step("deleting RAM:WBGapC");
    DeleteFile((CONST_STRPTR)"RAM:WBGapC.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapC");
    step("ending");
    /* C6a: the clean-up ended, and the window closed (CLOSEWBWAIT gives C6a its FAIL when the clean-up hangs) */
    say("C6a", cleared == WBGAP_CL_GONE ? WBGAP_PASS : WBGAP_FAIL,
        cleared == WBGAP_CL_GONE ? "" : cleared == WBGAP_CL_PARKED ? "RAM:WBGapC's window wouldn't close (parked)"
                                                                    : "RAM:WBGapC's window wouldn't close or move");
    return wbt_fails() ? 5 : 0;
}

int wbt_closewb(int argc, char **argv)
{
    int rc;
    (void)argc;
    (void)argv;
    rc = closewb_row();
    printf("INFO C6 done\n");                    /* the last line, on every way out: CLOSEWBWAIT waits for it */
    fflush(stdout);
    steps_add("INFO C6 done\n", 13);              /* and in the steps file, which CLOSEWBWAIT can always read */
    return rc;
}
