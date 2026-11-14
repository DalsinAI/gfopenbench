/* I6: a file dragged onto a trashcan (tests/amiga/rows/TRASH_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "TRASH.h"

void trash_plan(void);
WBGAP_PLAN(trash_plan)

void trash_plan(void)
{
    char why[200];
    int i = wbgap_find("I6");
    CHECK(wbgap_trash_judge(0, 0, 1, 1, 1, "f", why, sizeof why) == WBGAP_PASS && !why[0], "f and f.info in the trash: PASS");
    CHECK(wbgap_trash_judge(1, 1, 0, 0, 1, "f", why, sizeof why) == WBGAP_FAIL && why[0], "nothing moved: FAIL, \"%s\"", why);
    CHECK(wbgap_trash_judge(1, 1, 1, 1, 1, "f", why, sizeof why) == WBGAP_FAIL, "copied, not moved: FAIL");
    CHECK(wbgap_trash_judge(0, 1, 1, 0, 1, "f", why, sizeof why) == WBGAP_FAIL, "the icon left behind: FAIL");
    CHECK(wbgap_trash_judge(0, 0, 0, 0, 1, "f", why, sizeof why) == WBGAP_FAIL, "gone altogether (deleted, not trashed): FAIL");
    CHECK(wbgap_trash_judge(0, 0, 1, 1, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "trashcan's own icon"),
          "the trashcan's icon gone: FAIL, \"%s\"", why);
    CHECK(wbgap_trash_judge(0, 0, 1, 1, -1, "f", why, sizeof why) == WBGAP_SKIP && strstr(why, "looked at"),
          "not looked at: SKIP, \"%s\"", why);
    CHECK(wbgap_trash_judge(0, 0, 1, 1, 1, NULL, NULL, 0) == WBGAP_PASS, "no name, no room for a reason: still judged");
    CHECK(i >= 0 && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "TRASH"), "I6's line comes from TRASH");
}
