/* I13: the icon formats' judge (tests/amiga/rows/ICONFMT_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "ICONFMT.h"

void iconfmt_plan(void);
WBGAP_PLAN(iconfmt_plan)

void iconfmt_plan(void)
{
    char why[300];
    int all[4] = {300, 300, 300, 300}, half[4] = {150, 150, 150, 150}, under[4] = {149, 300, 300, 300};
    int no_new[4] = {300, 0, 300, 2};
    int i = wbgap_find("I13");
    CHECK(i >= 0 && wbgap_row(i)->how == WBGAP_AUTO && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "ICONFMT"),
          "I13 automatic, from ICONFMT");
    CHECK(wbgap_iconfmt_judge(20000, all, 100, why, sizeof why) == WBGAP_PASS && !why[0], "all four shown: PASS");
    CHECK(wbgap_iconfmt_judge(20000, half, 100, why, sizeof why) == WBGAP_PASS, "half of each (partly covered): PASS");
    CHECK(wbgap_iconfmt_judge(20000, under, 100, why, sizeof why) == WBGAP_FAIL && strstr(why, "b_classic: 149 points") &&
              !strstr(why, "pen 1"),
          "the classic one under half: FAIL, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(20000, no_new, 450, why, sizeof why) == WBGAP_FAIL &&
              strstr(why, "c_newicons: 0 points its colour; e_png: 2 points its colour (pen 1 grew by 450 points") &&
              !strstr(why, "glowicons"),
          "NewIcons on its classic picture, PNG not drawn: FAIL naming both, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(20000, no_new, 299, why, sizeof why) == WBGAP_FAIL && !strstr(why, "pen 1"),
          "pen 1 grown by less than a picture's worth: not said");
    CHECK(wbgap_iconfmt_judge(1999, all, 0, why, sizeof why) == WBGAP_SKIP && strstr(why, "1999"), "too few points: SKIP, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(20000, no_new, 450, why, 8) == WBGAP_FAIL && strlen(why) < 8, "a short buffer: cut, still judged");
    CHECK(wbgap_iconfmt_judge(20000, all, 0, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    CHECK(wbgap_iconfmt_near(0x30D090, WBGAP_ICONFMT_NEWICONS_RGB, 0) && wbgap_iconfmt_near(0x58B8B8, 0x3090D0, 40) &&
              !wbgap_iconfmt_near(0x3090D0, 0x3090FA, 40) && !wbgap_iconfmt_near(0x000000, 0x290000, 40),
          "near: each part within tol, none beyond");
    CHECK(!strcmp(wbgap_iconfmt_name(3), "e_png") && !strcmp(wbgap_iconfmt_name(9), "?"), "the names");
}
