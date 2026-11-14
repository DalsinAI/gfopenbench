/* W3: window and screen titles (tests/amiga/rows/TITLES_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "TITLES.h"

void titles_plan(void);
WBGAP_PLAN(titles_plan)

void titles_plan(void)
{
    char why[320];
    CHECK(wbgap_title_judge("WBGapW3", "WBGapW3", "Workbench Screen", why, sizeof why) == WBGAP_PASS && !why[0],
          "the drawer's name, a screen title: PASS");
    CHECK(wbgap_title_judge("WBGapW3  2 files", "WBGapW3", "x", why, sizeof why) == WBGAP_PASS, "the name, then a blank and more: PASS");
    CHECK(wbgap_title_judge("RAM:WBGapW3", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL && strstr(why, "RAM:WBGapW3"),
          "the path, not the name: FAIL, \"%s\"", why);
    CHECK(wbgap_title_judge("WBGapTx", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL, "a longer name: FAIL");
    CHECK(wbgap_title_judge("wbgapt", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL, "the name's case changed: FAIL");
    CHECK(wbgap_title_judge("", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL, "an empty title: FAIL");
    CHECK(wbgap_title_judge("WBGapW3", "WBGapW3", "", why, sizeof why) == WBGAP_FAIL && strstr(why, "screen has no title"),
          "no screen title: FAIL, \"%s\"", why);
    CHECK(wbgap_title_judge("WBGapW3", "WBGapW3", NULL, why, sizeof why) == WBGAP_FAIL, "a NULL screen title: FAIL");
    CHECK(wbgap_title_judge(NULL, "WBGapW3", "x", why, sizeof why) == WBGAP_SKIP && strstr(why, "not read"),
          "the window not read: SKIP, \"%s\"", why);
    CHECK(wbgap_title_judge("WBGapW3", "WBGapW3", "x", NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    CHECK(wbgap_title_judge("RAM:WBGapW3 - 0 items", "WBGapW3", "x", why, sizeof why) == WBGAP_PASS
          && !strncmp(why, "chosen difference", 17) && strstr(why, "DECISION-DRAWER-TITLE"),
          "GFOpenBench's \"path - N items\": a chosen difference, PASS with the decision named, \"%s\"", why);
    CHECK(wbgap_title_judge("Work:a/b/WBGapW3 - 1 item", "WBGapW3", "x", why, sizeof why) == WBGAP_PASS
          && !strncmp(why, "chosen difference", 17), "a deeper path, 1 item: the chosen difference");
    CHECK(wbgap_title_judge("RAM:WBGapW3 - 0 items", "WBGapW3", "", why, sizeof why) == WBGAP_FAIL
          && strstr(why, "screen has no title"), "the chosen title, no screen title: still FAIL");
    CHECK(wbgap_title_judge("RAM:XWBGapW3 - 0 items", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL,
          "another drawer's name ending in the name: FAIL");
    CHECK(wbgap_title_judge("RAM:WBGapW3 - items", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL
          && wbgap_title_judge("RAM:WBGapW3 - 2 files", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL
          && wbgap_title_judge("RAM:WBGapW3 - 0 items more", "WBGapW3", "x", why, sizeof why) == WBGAP_FAIL,
          "no count, other words, more after: FAIL");
    CHECK(wbgap_find("W3") >= 0 && wbgap_row(wbgap_find("W3"))->mode && !strcmp(wbgap_row(wbgap_find("W3"))->mode, "TITLES"),
          "W3's line comes from TITLES");
}
