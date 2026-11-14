/* I5: a file dragged onto the desktop's own space (tests/amiga/rows/DESKDROP_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "DESKDROP.h"

void deskdrop_plan(void);
WBGAP_PLAN(deskdrop_plan)

void deskdrop_plan(void)
{
    static const char desk[] = "10 10 60 40 RAM Disk\n10 80 60 40 Work\r\n300 200 40 30 f\n";
    char why[200];
    int i = wbgap_find("I5");
    CHECK(wbgap_deskdrop_judge(1, 1, 1, 0, "f", why, sizeof why) == WBGAP_PASS && !why[0], "left out: PASS");
    CHECK(wbgap_deskdrop_judge(1, 1, 1, -1, "f", why, sizeof why) == WBGAP_PASS, "the window not read: still PASS");
    CHECK(wbgap_deskdrop_judge(0, 0, 1, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "moved or deleted"),
          "moved off its drawer: FAIL, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(0, 1, 1, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "icon left behind"),
          "the file gone, its icon left: FAIL, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(1, 0, 1, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, ".info"),
          "the icon gone: FAIL, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(1, 1, 0, 1, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "isn't on the desktop"),
          "nothing happened: FAIL, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(1, 1, 1, 1, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "still in its drawer"),
          "on the desktop and in the window: FAIL, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(1, 1, -1, 0, "f", why, sizeof why) == WBGAP_SKIP && strstr(why, "not read"),
          "the desktop not read: SKIP, \"%s\"", why);
    CHECK(wbgap_deskdrop_judge(0, 0, -1, -1, "f", why, sizeof why) == WBGAP_FAIL,
          "moved off its drawer, the desktop not read: still FAIL (the disk says so)");
    CHECK(wbgap_deskdrop_judge(1, 1, 1, 0, NULL, NULL, 0) == WBGAP_PASS, "no name, no room for a reason: still judged");
    CHECK(wbgap_deskdrop_has(desk, "f") && wbgap_deskdrop_has(desk, "F"), "f found, any case");
    CHECK(wbgap_deskdrop_has(desk, "RAM Disk") && wbgap_deskdrop_has(desk, "Work"), "a name with a space, a CR end");
    CHECK(!wbgap_deskdrop_has(desk, "RAM") && !wbgap_deskdrop_has(desk, "g") && !wbgap_deskdrop_has(NULL, "f") &&
          !wbgap_deskdrop_has(desk, ""), "part of a name, none, no list: not found");
    CHECK(!wbgap_deskdrop_has("f\n10 10 f\n", "f"), "a line without its four numbers: not read");
    CHECK(!wbgap_deskdrop_clear(desk, 40, 30, 48) && !wbgap_deskdrop_clear(desk, 110, 50, 48),
          "on an icon, or within the margin: not clear");
    CHECK(wbgap_deskdrop_clear(desk, 200, 30, 48) && wbgap_deskdrop_clear("", 5, 5, 48) &&
          wbgap_deskdrop_clear(NULL, 5, 5, 48), "away from every icon, or none: clear");
    CHECK(!wbgap_deskdrop_clear(desk, 320, 215, 0) && wbgap_deskdrop_clear(desk, 340, 230, 0),
          "the box's own edge: inside to the left/top, outside at left+width");
    CHECK(i >= 0 && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "DESKDROP"), "I5's line comes from DESKDROP");
}
