/* S1-S3, S2d: the start order; A10a: LoadWB's switches and the desktop's line (tests/amiga/rows/STARTUP_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "STARTUP.h"

void startup_plan(void);
WBGAP_PLAN(startup_plan)

void startup_plan(void)
{
    char why[200], out[120];
    const char *r;
    /* B (DONOTWAIT, 4 s) first, C waited for, then A; E, C, D by date added */
    const char *good = "start WBGapB\nstart WBGapE\nend WBGapE\nstart WBGapC\nend WBGapC\nstart WBGapD\nend WBGapD\n"
                       "start WBGapA\nend WBGapB\nend WBGapA\n";
    const char *by_name = "start WBGapB\nstart WBGapC\nend WBGapC\nstart WBGapD\nend WBGapD\nstart WBGapE\nend WBGapE\n"
                          "start WBGapA\nend WBGapB\nend WBGapA\n";
    const char *b_held = "start WBGapB\nend WBGapB\nstart WBGapC\nend WBGapC\nstart WBGapA\nend WBGapA\n";
    const char *a_early = "start WBGapB\nstart WBGapC\nstart WBGapA\nend WBGapC\nend WBGapB\nend WBGapA\n";

    /* S1, S2, S3 */
    CHECK(wbgap_startup_order(good, "S1", 1, why, sizeof why) == WBGAP_PASS && !why[0], "all three started: S1 PASS");
    CHECK(wbgap_startup_order("start WBGapB\nstart WBGapC\n", "S1", 1, why, sizeof why) == WBGAP_FAIL &&
          !strcmp(why, "not all three started"), "A missing: S1 FAIL, \"%s\"", why);
    CHECK(wbgap_startup_order(good, "S2", 1, why, sizeof why) == WBGAP_PASS, "B, C, A: S2 PASS");
    CHECK(wbgap_startup_order("start WBGapC\nstart WBGapB\nstart WBGapA\n", "S2", 1, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "STARTPRI"), "C before B: S2 FAIL, \"%s\"", why);
    CHECK(wbgap_startup_order(good, "S3", 1, why, sizeof why) == WBGAP_PASS, "B not waited for, C waited for: S3 PASS");
    CHECK(wbgap_startup_order(b_held, "S3", 1, why, sizeof why) == WBGAP_FAIL, "B (DONOTWAIT) held C up: S3 FAIL");
    CHECK(wbgap_startup_order(a_early, "S3", 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "DONOTWAIT"),
          "A started before C ended: S3 FAIL, \"%s\"", why);
    CHECK(wbgap_startup_order("start WBGapB\nstart WBGapC\nstart WBGapA\n", "S3", 1, why, sizeof why) == WBGAP_FAIL,
          "no end lines: S3 FAIL");
    CHECK(wbgap_startup_order(NULL, "S1", 1, why, sizeof why) == WBGAP_FAIL, "no text: S1 FAIL");

    /* S2d */
    CHECK(wbgap_startup_order(good, "S2d", 1, why, sizeof why) == WBGAP_PASS, "E, C, D on GFOS: S2d PASS");
    CHECK(wbgap_startup_order(by_name, "S2d", 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "E, C, D"),
          "C, D, E (by name) on GFOS: S2d FAIL, \"%s\"", why);
    CHECK(wbgap_startup_order(by_name, "S2d", 0, why, sizeof why) == WBGAP_SKIP && strstr(why, "3.2.3"),
          "3.2.3: S2d SKIP, \"%s\"", why);
    CHECK(wbgap_startup_order(b_held, "S2d", 1, why, sizeof why) == WBGAP_SKIP && strstr(why, "WBGapD"),
          "D and E not set up: S2d SKIP, \"%s\"", why);
    CHECK(wbgap_startup_order("start WBGapE\nstart WBGapD\n", "S2d", 1, why, sizeof why) == WBGAP_FAIL,
          "C never started: S2d FAIL");
    CHECK(wbgap_startup_order(good, "S9", 1, NULL, 0) == WBGAP_SKIP, "another row: SKIP, no room for a reason");

    /* A10a: the switches */
    CHECK(wbgap_startup_switches("", out, sizeof out) == 0 && !out[0], "none: empty");
    CHECK(wbgap_startup_switches(" delay -debug\n", out, sizeof out) == 2 && !strcmp(out, " DEBUG DELAY"),
          "-DEBUG as DEBUG, the template's order, case not minded: \"%s\"", out);
    CHECK(wbgap_startup_switches("SKIP DEBUG -DEBUG", out, sizeof out) == 2 && !strcmp(out, " DEBUG SKIPWBSTARTUP"),
          "SKIP as SKIPWBSTARTUP, DEBUG once: \"%s\"", out);
    CHECK(wbgap_startup_switches("CLEANUP NEWPATH SIMPLEGELS LEGACY\tDELAY", out, sizeof out) == 5 &&
          !strcmp(out, " DELAY CLEANUP NEWPATH SIMPLEGELS LEGACY"), "the rest: \"%s\"", out);
    CHECK(wbgap_startup_switches(">NIL: DELAYS XDEBUG", out, sizeof out) == 0, "only whole words count");
    CHECK(wbgap_startup_switches("DEBUG DELAY CLEANUP", out, 14) == 2 && !strcmp(out, " DEBUG DELAY"),
          "no room for the third: two, \"%s\"", out);
    {
        char text[] = "delay";
        wbgap_startup_switches(text, out, sizeof out);
        CHECK(!strcmp(text, "delay"), "the text is left as it was");
    }

    /* A10a: the desktop's line */
    r = wbgap_startup_desk_rest("LoadWB DELAY\n", 0);
    CHECK(r && !strcmp(r, " DELAY\n"), "LoadWB: the rest");
    r = wbgap_startup_desk_rest("  run >NIL: c:loadwb -debug", 0);
    CHECK(r && !strcmp(r, " -debug"), "blanks, Run, a redirection and C:, case not minded");
    r = wbgap_startup_desk_rest("LoadWB", 0);
    CHECK(r && !*r, "LoadWB alone: an empty rest");
    CHECK(!wbgap_startup_desk_rest("LoadWBX DELAY", 0), "another command that begins alike: no");
    CHECK(!wbgap_startup_desk_rest("; LoadWB DELAY", 0), "a comment: no");
    CHECK(!wbgap_startup_desk_rest("LoadWB DELAY", 1), "OpenBench there: LoadWB's line isn't the desktop's");
    r = wbgap_startup_desk_rest("Run <NIL: SYS:System/OpenBench CLEANUP\n", 1);
    CHECK(r && !strcmp(r, " CLEANUP\n"), "SYS:System/OpenBench's line");
    CHECK(!wbgap_startup_desk_rest("System/OpenBench CLEANUP", 1), "not by its full name: no");
    CHECK(!wbgap_startup_desk_rest(NULL, 0), "no line: no");
}
