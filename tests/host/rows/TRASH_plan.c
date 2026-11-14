/* I6: a file dragged onto a trashcan (tests/amiga/rows/TRASH_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "TRASH.h"
#include "WINGAD.h"

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
    {   /* why RAM:WBGapT's window isn't ready: never found, someone else's window, or not put where asked */
        const long want[4] = {0, 11, 400, 300}, got[4] = {0, 11, 400, 300}, off[4] = {120, 40, 400, 300},
                   lim[4] = {80, 40, 640, 512};
        CHECK(wbgap_trash_title_ours("WBGapT") && wbgap_trash_title_ours("WBGapT (12 free)") &&
                  wbgap_trash_title_ours("RAM:WBGapT"), "RAM:WBGapT's own titles are its");
        CHECK(!wbgap_trash_title_ours("WBGapTest") && !wbgap_trash_title_ours("WBGapTx") && !wbgap_trash_title_ours("") &&
                  !wbgap_trash_title_ours(NULL), "WBGapTest's window (\"WBGapT\" inside a longer name) isn't");
        CHECK(wbgap_trash_open_why(0, NULL, want, got, lim, why, sizeof why) && strstr(why, "never showed"),
              "no window: \"%s\"", why);
        CHECK(wbgap_trash_open_why(1, "WBGapTest", want, got, lim, why, sizeof why) && strstr(why, "WBGapTest") &&
                  strstr(why, "not " "RAM:WBGapT's"), "another window found by the name: \"%s\"", why);
        CHECK(wbgap_trash_open_why(1, "WBGapT", want, off, lim, why, sizeof why) && strstr(why, "120,40") &&
                  strstr(why, "within its limits"), "not put where asked: \"%s\"", why);
        CHECK(!wbgap_trash_open_why(1, "WBGapT", want, got, lim, why, sizeof why) && strstr(why, "placed late"),
              "there, but late: 0, \"%s\"", why);
        CHECK(wbgap_trash_open_why(1, "WBGapT", want, off, lim, NULL, 0), "no room for a reason: still judged");
    }
    {   /* the window I6 takes: the first whose title is RAM:WBGapT's, not the first holding "WBGapT" */
        const char *const t1[] = {"WBGapTest", "Workbench", "WBGapT"}, *const t2[] = {"WBGapTest", "WBGapTx"},
                          *const t3[] = {"WBGapT", "WBGapTest"}, *const t4[] = {NULL, "WBGapT (2 free)"};
        CHECK(wbgap_trash_pick(t1, 3) == 2, "WBGapTest's window first: RAM:WBGapT's is taken");
        CHECK(wbgap_trash_pick(t2, 2) < 0, "only longer names: none");
        CHECK(wbgap_trash_pick(t3, 2) == 0, "RAM:WBGapT's first: that one");
        CHECK(wbgap_trash_pick(t4, 2) == 1 && wbgap_trash_pick(NULL, 0) < 0, "an untitled window passed over; no list: none");
    }
    CHECK(i >= 0 && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "TRASH"), "I6's line comes from TRASH");
}
