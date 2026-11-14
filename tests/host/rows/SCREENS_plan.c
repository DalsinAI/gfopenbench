/* SCREENS: screens and Intuition's pointer (tests/amiga/rows/SCREENS.c), its judges.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "SCREENS.h"

void screens_plan(void);
WBGAP_PLAN(screens_plan)

void screens_plan(void)
{
    char why[96];
    long xy[2];
    wbgap_depth_point(1, -23, 0, 23, 11, 640, xy);
    CHECK(xy[0] == 627 && xy[1] == 5, "C1: a RELRIGHT gadget's middle from the right edge: %ld,%ld", xy[0], xy[1]);
    wbgap_depth_point(0, 600, 2, 20, 10, 640, xy);
    CHECK(xy[0] == 610 && xy[1] == 7, "C1: a gadget from the left edge: %ld,%ld", xy[0], xy[1]);
    CHECK(wbgap_pointer_at(316, 236, 316, 236), "P4a: right where it was moved");
    CHECK(wbgap_pointer_at(315, 237, 316, 236) && wbgap_pointer_at(317, 235, 316, 236), "P4a: a pixel off still there");
    CHECK(!wbgap_pointer_at(318, 236, 316, 236) && !wbgap_pointer_at(316, 234, 316, 236), "P4a: two pixels off is not");
    CHECK(wbgap_pointer_layer(0x50001000UL, 7, 7, 7, why, sizeof why) == WBGAP_PASS && !why[0], "P4b: RTG, square unchanged: PASS");
    CHECK(wbgap_pointer_layer(0x50001000UL, 7, 9, 7, why, sizeof why) == WBGAP_FAIL && strstr(why, "drawn into"),
          "P4b: RTG, the pointer in the pixels: FAIL, \"%s\"", why);
    CHECK(wbgap_pointer_layer(0x50001000UL, 7, 9, 8, why, sizeof why) == WBGAP_SKIP && strstr(why, "by itself"),
          "P4b: the square changed by itself: SKIP, \"%s\"", why);
    CHECK(wbgap_pointer_layer(0x00029004UL, 7, 9, 7, why, sizeof why) == WBGAP_SKIP && strstr(why, "not on an RTG"),
          "P4b: a PAL chipset mode: SKIP, never a FAIL, \"%s\"", why);
    CHECK(wbgap_pointer_layer(WBGAP_NO_MODE, 7, 9, 7, why, sizeof why) == WBGAP_SKIP, "P4b: no mode id: SKIP");
    CHECK(wbgap_pointer_layer(0x50001000UL, 7, 9, 7, NULL, 0) == WBGAP_FAIL, "P4b: no room for a reason: still judged");
}
