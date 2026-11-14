/* I8, I9: the AppMessage a drop sends, and the AppIcon's box (tests/amiga/rows/APPDROP_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "APPDROP.h"

void appdrop_plan(void);
WBGAP_PLAN(appdrop_plan)

void appdrop_plan(void)
{
    long b[4];
    char why[128];
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "Ram Disk:WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_PASS && !why[0],
          "an AppWindow message for d in RAM:WBGapD: PASS");
    CHECK(wbgap_drop_judge(1, 8, 8, 8, 8, 2, "d", "RAM:WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_PASS,
          "an AppIcon message, more than one argument: PASS");
    CHECK(wbgap_drop_judge(0, 0, 7, 0, 9, 0, NULL, NULL, "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "no AppMessage"),
          "nothing came: FAIL, \"%s\"", why);
    CHECK(wbgap_drop_judge(1, 8, 7, 9, 9, 1, "d", "RAM:WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "type 8"),
          "the wrong type: FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 3, 9, 1, "d", "RAM:WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "id 3"),
          "another program's id: FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 0, NULL, NULL, "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "no arguments"),
          "no arguments (a drop of nothing): FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 1, "e", "RAM:WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "\"e\""),
          "another icon: FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "RAM:", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL && strstr(why, "drawer"),
          "the right name in the wrong drawer: FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "RAM:WBGapDX", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL,
          "a drawer whose name only starts the same: FAIL");
    CHECK(wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "RAM:XWBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL &&
          wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_FAIL &&
          wbgap_drop_judge(1, 7, 7, 9, 9, 1, "d", "RAM:T/WBGapD", "d", "WBGapD", why, sizeof why) == WBGAP_PASS,
          "a drawer whose name only ends the same, or a path with no volume: FAIL; under another drawer: PASS");
    CHECK(wbgap_parse_box("1680 11 64 40\n", b) && b[0] == 1680 && b[1] == 11 && b[2] == 64 && b[3] == 40, "the AppIcon's box read");
    CHECK(wbgap_parse_box("-2 0 64 40", b) && b[0] == -2, "a box partly off the screen's left still reads");
    CHECK(!wbgap_parse_box("1680 11 64", b) && !wbgap_parse_box("", b) && !wbgap_parse_box(NULL, b), "fewer than four numbers: no box");
    CHECK(!wbgap_parse_box("1 2 0 40", b) && !wbgap_parse_box("1 2 64 40 x", b) && !wbgap_parse_box("RESULT RESULT 1 1", b),
          "no width, something after, or the port's unset RESULT: no box");
}
