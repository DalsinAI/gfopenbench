/* A1, A2: AddAppWindowA's messages and a drop zone's (tests/amiga/rows/APPWIN_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "APPWIN.h"

void appwin_plan(void);
WBGAP_PLAN(appwin_plan)

void appwin_plan(void)
{
    char why[160];
    const long at[2] = { 120, 70 };
    /* A1: AMTYPE_APPWINDOW is 7, AMTYPE_APPWINDOWZONE 10 (workbench.h); the judge takes the wanted type */
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 121, 69, at, 240, 120, why, sizeof why) == WBGAP_PASS && !why[0],
          "the AppWindow's type, id, userdata, where it was let go: PASS");
    CHECK(wbgap_appwin_msg_judge(0, 0, 7, 0, 1, 0, 0xA1, 0, 0, at, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "no AppMessage"), "none came: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 10, 7, 1, 1, 0xA1, 0xA1, 120, 70, at, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "am_Type 10, not 7"), "the wrong type: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 2, 1, 0xA1, 0xA1, 120, 70, at, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "am_ID 2, not 1"), "the wrong id: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0, 0xA1, 120, 70, at, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "am_UserData $0, not $a1"), "the userdata not passed on: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 360, 70, at, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "outside the window"), "the place on the screen, not in the window: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, -1, 70, at, 240, 120, why, sizeof why) == WBGAP_FAIL,
          "a place left of the window: FAIL");
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 120 + WBGAP_APPWIN_SLACK, 70 - WBGAP_APPWIN_SLACK, at, 240, 120,
                                 why, sizeof why) == WBGAP_PASS, "off by the slack: still PASS");
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 120 + WBGAP_APPWIN_SLACK + 1, 70, at, 240, 120, why,
                                 sizeof why) == WBGAP_FAIL && strstr(why, "not where it was let go"),
          "off by more: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 10, 10, NULL, 240, 120, why, sizeof why) == WBGAP_PASS,
          "no drop point to compare: only inside the window");
    CHECK(wbgap_appwin_msg_judge(1, 7, 7, 1, 1, 0xA1, 0xA1, 120, 70, at, 240, 120, NULL, 0) == WBGAP_PASS,
          "no room for a reason: still judged");
    /* A2: the zone's id and userdata, AMTYPE_APPWINDOWZONE */
    CHECK(wbgap_appwin_msg_judge(1, 10, 10, 22, 22, 0xA22, 0xA22, 180, 70, NULL, 240, 120, why, sizeof why) == WBGAP_PASS,
          "a drop in the zone: the zone's type, id and userdata, PASS");
    CHECK(wbgap_appwin_msg_judge(1, 7, 10, 2, 22, 0xA2, 0xA22, 180, 70, NULL, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "am_Type 7, not 10"), "the window's message, not the zone's: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_msg_judge(1, 10, 10, 2, 22, 0xA2, 0xA22, 180, 70, NULL, 240, 120, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "am_ID 2, not 22"), "the zone's type but the window's id: FAIL, \"%s\"", why);
    /* nothing must come */
    CHECK(wbgap_appwin_none_judge(0, 0, 0, "after RemoveAppWindow", why, sizeof why) == WBGAP_PASS && !why[0], "none: PASS");
    CHECK(wbgap_appwin_none_judge(1, 7, 1, "after RemoveAppWindow", why, sizeof why) == WBGAP_FAIL &&
          !strcmp(why, "after RemoveAppWindow: an AppMessage came all the same (type 7, id 1)"), "one came: FAIL, \"%s\"", why);
    CHECK(wbgap_appwin_none_judge(1, 7, 2, NULL, why, sizeof why) == WBGAP_FAIL && !strncmp(why, "a drop:", 7),
          "no name for the drop: \"a drop\"");
    /* one row from two parts */
    CHECK(wbgap_appwin_both(WBGAP_PASS, "", WBGAP_PASS, "", why, sizeof why) == WBGAP_PASS && !why[0], "both PASS: PASS");
    CHECK(wbgap_appwin_both(WBGAP_PASS, "", WBGAP_FAIL, "b wrong", why, sizeof why) == WBGAP_FAIL && !strcmp(why, "b wrong"),
          "the second FAILs: FAIL, its reason");
    CHECK(wbgap_appwin_both(WBGAP_FAIL, "a wrong", WBGAP_FAIL, "b wrong", why, sizeof why) == WBGAP_FAIL &&
          !strcmp(why, "a wrong"), "both FAIL: the first's reason");
    CHECK(wbgap_appwin_both(WBGAP_SKIP, "a not run", WBGAP_FAIL, "b wrong", why, sizeof why) == WBGAP_FAIL &&
          !strcmp(why, "b wrong"), "a FAIL beats a SKIP");
    CHECK(wbgap_appwin_both(WBGAP_SKIP, "a not run", WBGAP_PASS, "", why, sizeof why) == WBGAP_SKIP &&
          !strcmp(why, "a not run"), "a SKIP and a PASS: SKIP");
    CHECK(wbgap_appwin_both(WBGAP_PASS, "", WBGAP_SKIP, "b not run", why, sizeof why) == WBGAP_SKIP &&
          !strcmp(why, "b not run"), "a PASS and a SKIP: SKIP");
    CHECK(wbgap_find("A1") >= 0 && wbgap_row(wbgap_find("A1"))->mode && !strcmp(wbgap_row(wbgap_find("A1"))->mode, "APPWIN") &&
          wbgap_find("A2") >= 0 && !strcmp(wbgap_row(wbgap_find("A2"))->mode, "APPWIN"), "A1 and A2's lines come from APPWIN");
}
