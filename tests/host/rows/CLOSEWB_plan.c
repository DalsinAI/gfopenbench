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
}
