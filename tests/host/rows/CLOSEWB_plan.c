/* C6: CloseWorkBench / OpenWorkBench (tests/amiga/rows/CLOSEWB.c, CLOSEWBWAIT.c), its judges.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "CLOSEWB.h"

void closewb_plan(void);
WBGAP_PLAN(closewb_plan)

void closewb_plan(void)
{
    char why[96];
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 1, why, sizeof why) == WBGAP_PASS && !why[0], "refused, all still there: PASS");
    CHECK(wbgap_closewb_judge(1, 1, 0, 1, 1, why, sizeof why) == WBGAP_PASS, "closed, opened again, drawer back: PASS (a new screen is fine)");
    CHECK(wbgap_closewb_judge(1, 0, 0, 1, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't bring"), "not reopened: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 0, 1, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "screen changed"), "refused, new screen: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 1, 0, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "desktop went"), "refused, port gone: FAIL");
    CHECK(wbgap_closewb_judge(1, 1, 0, 0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't come back"),
          "closed, desktop not back: FAIL, \"%s\"", why);
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "window went"), "refused, drawer gone: FAIL");
    CHECK(wbgap_closewb_judge(1, 1, 1, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "opened again"), "closed, drawer not back: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 1, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    CHECK(wbgap_find("C6") >= 0 && !strcmp(wbgap_row(wbgap_find("C6"))->mode, "CLOSEWB"), "C6's line comes from CLOSEWB");
    /* CLOSEWBWAIT: the Run process is over only at its last line */
    CHECK(!wbgap_closewb_over(NULL) && !wbgap_closewb_over(""), "nothing written: not over");
    CHECK(!wbgap_closewb_over("INFO workbench.library 47.42\n"), "started, CloseWorkBench() not back: not over");
    CHECK(!wbgap_closewb_over("INFO C6 CloseWorkBench() gave 0\nPASS C6\n"), "judged but not cleaned up: not over");
    CHECK(wbgap_closewb_over("INFO C6 CloseWorkBench() gave 0\nPASS C6\nINFO C6 done\n"), "its last line: over");
    CHECK(wbgap_closewb_over("SKIP C6 no Workbench screen\nINFO C6 done"), "no newline at the end: over");
    CHECK(wbgap_closewb_over("x\r\nINFO C6 done\r\n"), "CR LF: over");
    CHECK(!wbgap_closewb_over("XINFO C6 done\n") && !wbgap_closewb_over("INFO C6 doner\n"), "only the whole line counts");
    /* the last step CLOSEWB reached, for CLOSEWBWAIT's FAIL */
    {
        char step[64];
        CHECK(!wbgap_closewb_last_step(NULL, step, sizeof step) && !step[0], "no text: no step");
        CHECK(!wbgap_closewb_last_step("", step, sizeof step) && !step[0], "nothing written: no step");
        CHECK(!wbgap_closewb_last_step("[CLI 1]\nINFO workbench.library 47.42\n", step, sizeof step) && !step[0],
              "Run's line and the version only: no step");
        CHECK(wbgap_closewb_last_step("INFO C6 step: opening RAM:WBGapC's window\n", step, sizeof step)
              && !strcmp(step, "opening RAM:WBGapC's window"), "one step, newline dropped: \"%s\"", step);
        CHECK(wbgap_closewb_last_step("INFO C6 step: opening RAM:WBGapC's window\nINFO C6 step: calling CloseWorkBench()\n",
                                      step, sizeof step) && !strcmp(step, "calling CloseWorkBench()"), "the last of two: \"%s\"", step);
        CHECK(wbgap_closewb_last_step("INFO C6 step: calling CloseWorkBench()\r\nINFO C6 CloseWorkBench() gave 0\r\n", step, sizeof step)
              && !strcmp(step, "calling CloseWorkBench()"), "CR LF, other INFO lines after it: \"%s\"", step);
        CHECK(wbgap_closewb_last_step("INFO C6 step: waiting for the desktop", step, sizeof step)
              && !strcmp(step, "waiting for the desktop"), "no newline at the end: \"%s\"", step);
        CHECK(!wbgap_closewb_last_step("XINFO C6 step: calling CloseWorkBench()\n", step, sizeof step) && !step[0],
              "only a whole line counts");
        CHECK(wbgap_closewb_last_step("INFO C6 step: calling CloseWorkBench()\n", step, 8) && !strcmp(step, "calling"),
              "cut to the room given: \"%s\"", step);
        CHECK(!wbgap_closewb_last_step("INFO C6 step: x\n", NULL, 0), "no room: 0, nothing written");
    }
}
