/* BAR: the system bar (tests/amiga/rows/BAR.c), its judges.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "BAR.h"

void bar_plan(void);
WBGAP_PLAN(bar_plan)

/* B1, B2: GFOpenTitle's Power cell, whose list a click opened, and its rows */
static void bar_point_plan(void)
{
    CHECK(wbgap_point_in_box(1880, 30, 1756, 24, 139, 57), "a point inside the list: under it");
    CHECK(!wbgap_point_in_box(1880, 11, 1756, 24, 139, 57), "Power's cell on the bar, above the list: not under it");
    CHECK(wbgap_point_in_box(1756, 24, 1756, 24, 139, 57) && !wbgap_point_in_box(1895, 24, 1756, 24, 139, 57) &&
          !wbgap_point_in_box(1756, 81, 1756, 24, 139, 57), "the left and top edges are in, the right and bottom out");
    CHECK(!wbgap_point_in_box(10, 10, 0, 0, 0, 0), "an empty box holds nothing");
}

void bar_plan(void)
{
    long xy[2];
    bar_point_plan();
    wbgap_power_point(400, 0, 240, 11, xy);
    CHECK(xy[0] == 625 && xy[1] == 5, "Power's cell: the middle of the row's last 30 pixels (%ld,%ld)", xy[0], xy[1]);
    CHECK(xy[0] >= 640 - WBGAP_TRAY_CELL && xy[0] < 640, "inside the last cell");
    wbgap_power_point(0, 0, 20, 10, xy);
    CHECK(xy[0] == 10 && xy[1] == 5, "a row narrower than a cell: its middle");
    CHECK(wbgap_bar_menu_owner(640, 500, 140) == WBGAP_BAR_POWER, "a list ending at the row's right edge: Power's");
    CHECK(wbgap_bar_menu_owner(640, 470, 140) == WBGAP_BAR_PREFS, "one cell in: Prefs'");
    CHECK(wbgap_bar_menu_owner(640, 200, 140) == WBGAP_BAR_OTHER, "anywhere else: another window");
    CHECK(wbgap_bar_menu_owner(640, 0, 600) == WBGAP_BAR_UNSURE, "pushed to x 0: can't tell");
    CHECK(wbgap_bar_menu_owner(100, 0, 100) == WBGAP_BAR_POWER, "at x 0 but ending at the row's edge: Power's");
    {
        int b1, b2;
        CHECK(wbgap_power_height(0, 12) == 42, "no stick: 3 + 3 rows of 12 + 3");
        CHECK(wbgap_power_height(2, 12) == 72, "two sticks: their rows and the line too");
        CHECK(wbgap_power_height(-1, 12) == 42, "a negative count is none");
        wbgap_power_judge(42, 0, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_SKIP, "no stick, the power rows: B1 PASS, B2 SKIP");
        wbgap_power_judge(72, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_PASS, "two sticks, their Eject rows: both PASS");
        wbgap_power_judge(42, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_FAIL, "two sticks but no Eject rows: B2 FAIL");
        wbgap_power_judge(54, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL && b2 == WBGAP_FAIL, "one Eject row for two sticks: neither");
        wbgap_power_judge(30, 0, 12, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL && b2 == WBGAP_SKIP, "two rows only, no stick: B1 FAIL, B2 SKIP");
        wbgap_power_judge(42, 0, 0, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL, "no row height: no PASS");
    }
    CHECK(wbgap_find("B1") >= 0 && wbgap_find("B2") >= 0 && wbgap_find("B3") >= 0, "B1-B3 are rows");
    CHECK(!strcmp(wbgap_row(wbgap_find("B3"))->mode, "BAR"), "B3's line comes from BAR");
}
