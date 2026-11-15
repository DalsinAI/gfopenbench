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
    int n[4] = {120, 120, 120, 120}, all[4] = {100, 100, 100, 100}, none[4] = {-1, 0, 0, 0};
    int quarter[4] = {30, 30, 30, 30}, under[4] = {29, 100, 100, 100};
    int no_new[4] = {100, 0, 100, 2}, fb[4] = {-1, 110, 0, 0}, few[4] = {120, 120, 9, 120};
    int i = wbgap_find("I13");
    CHECK(i >= 0 && wbgap_row(i)->how == WBGAP_AUTO && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "ICONFMT"),
          "I13 automatic, from ICONFMT");
    CHECK(wbgap_iconfmt_judge(n, all, none, why, sizeof why) == WBGAP_PASS && !why[0], "all four shown: PASS");
    CHECK(wbgap_iconfmt_judge(n, quarter, none, why, sizeof why) == WBGAP_PASS, "a quarter each (label, frame): PASS");
    CHECK(wbgap_iconfmt_judge(n, under, none, why, sizeof why) == WBGAP_FAIL && strstr(why, "b_classic: 29 of 120"),
          "the classic one under a quarter: FAIL, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(n, no_new, fb, why, sizeof why) == WBGAP_FAIL &&
              strstr(why, "c_newicons: 0 of 120 points its colour, the classic picture (pen 1) shown instead") &&
              strstr(why, "; e_png: 2 of 120 points its colour") && !strstr(why, "e_png: 2 of 120 points its colour,") &&
              !strstr(why, "glowicons"),
          "NewIcons on its classic picture, PNG not drawn: FAIL naming both, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(few, all, none, why, sizeof why) == WBGAP_SKIP && strstr(why, "glowicons"),
          "too few points read: SKIP, \"%s\"", why);
    CHECK(wbgap_iconfmt_judge(n, no_new, fb, why, 8) == WBGAP_FAIL && strlen(why) < 8, "a short buffer: cut, still judged");
    CHECK(wbgap_iconfmt_judge(n, all, none, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    CHECK(wbgap_iconfmt_near(0x30D090, WBGAP_ICONFMT_NEWICONS_RGB, 0) && wbgap_iconfmt_near(0x58B8B8, 0x3090D0, 40) &&
              !wbgap_iconfmt_near(0x3090D0, 0x3090FA, 40) && !wbgap_iconfmt_near(0x000000, 0x290000, 40),
          "near: each part within tol, none beyond");
    CHECK(!strcmp(wbgap_iconfmt_name(3), "e_png") && !strcmp(wbgap_iconfmt_name(9), "?"), "the names");
}
