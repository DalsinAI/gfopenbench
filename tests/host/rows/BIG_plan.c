/* X1: a drawer of 1000 files opens in 10 s (tests/amiga/rows/BIG.c), its time and judge.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "BIG.h"

void big_plan(void);
WBGAP_PLAN(big_plan)

void big_plan(void)
{
    char why[64];
    CHECK(wbgap_big_ticks(5, 10, 100, 5, 10, 400) == 250, "6 s less the still second: 250 ticks, %ld",
          wbgap_big_ticks(5, 10, 100, 5, 10, 400));
    CHECK(wbgap_big_ticks(5, 10, 2900, 5, 11, 200) == 250, "over a minute's edge: the minute counted");
    CHECK(wbgap_big_ticks(5, 1439, 2950, 6, 0, 100) == 100, "over midnight: the day counted");
    CHECK(wbgap_big_ticks(5, 10, 100, 5, 10, 150) == 0, "only the still second: 0");
    CHECK(wbgap_big_judge(0, why, sizeof why) == WBGAP_PASS && !why[0], "at once: PASS");
    CHECK(wbgap_big_judge(WBGAP_BIG_LIMIT - 1, why, sizeof why) == WBGAP_PASS, "just under 10 s: PASS");
    CHECK(wbgap_big_judge(WBGAP_BIG_LIMIT, why, sizeof why) == WBGAP_FAIL && !strcmp(why, "over 10 s"),
          "10 s: FAIL, \"%s\"", why);
    CHECK(wbgap_big_judge(30000, NULL, 0) == WBGAP_FAIL, "no room for a reason: still judged");
}
