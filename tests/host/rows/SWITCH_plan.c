/* W2: GFOpenWindows' Amiga+Tab and Amiga+Shift+Tab (tests/amiga/rows/SWITCH_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "SWITCH.h"

void switch_plan(void);
WBGAP_PLAN(switch_plan)

void switch_plan(void)
{
    char k[32], why[128];
    unsigned long b[3] = {11, 22, 33}, fwd[3] = {33, 11, 22}, bk[3] = {22, 33, 11}, same[3] = {11, 22, 33};
    CHECK(wbgap_switcher_on(NULL, k, sizeof k) && !strcmp(k, "lcommand tab"), "no prefs file: on, Amiga+Tab");
    CHECK(wbgap_switcher_on("snap on 12\nswitcher on\n", k, sizeof k), "switcher on");
    CHECK(!wbgap_switcher_on("snap on 12\nswitcher off\r\nwheel.scroll on\n", k, sizeof k), "switcher off (CR too): off");
    CHECK(wbgap_switcher_on("switcher.key lcommand f1\n", k, sizeof k) && !strcmp(k, "lcommand f1"),
          "another key read: \"%s\"", k);
    CHECK(wbgap_switcher_on("  switcher   off  \n", k, sizeof k) == 0, "blanks round the words");
    CHECK(wbgap_switcher_on("switchers off\nswitcher offx\n", k, sizeof k) == 1, "other words aren't the switch");
    CHECK(wbgap_switch_judge(0, b, 3, fwd, 3, 33, why, sizeof why) == WBGAP_PASS, "Amiga+Tab: the back one in front and active");
    CHECK(wbgap_switch_judge(1, b, 3, bk, 3, 22, why, sizeof why) == WBGAP_PASS, "Amiga+Shift+Tab: the front one at the back, the next active");
    CHECK(wbgap_switch_judge(0, b, 3, same, 3, 11, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't come"), "nothing moved: FAIL");
    CHECK(wbgap_switch_judge(0, b, 3, fwd, 3, 11, why, sizeof why) == WBGAP_FAIL && strstr(why, "active"), "moved but not active: FAIL");
    CHECK(wbgap_switch_judge(1, b, 3, same, 3, 22, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't go"), "Shift: nothing moved: FAIL");
    CHECK(wbgap_switch_judge(1, b, 3, bk, 3, 33, why, sizeof why) == WBGAP_FAIL, "Shift: the wrong one active: FAIL");
    CHECK(wbgap_switch_judge(0, b, 3, fwd, 2, 33, why, sizeof why) == WBGAP_FAIL && strstr(why, "came or went"), "a window gone: FAIL");
    CHECK(wbgap_switch_judge(0, b, 1, fwd, 1, 11, why, sizeof why) == WBGAP_SKIP && wbgap_switch_judge(0, NULL, 0, fwd, 0, 0, why, sizeof why) == WBGAP_SKIP,
          "one window or none: SKIP");
    CHECK(wbgap_switch_offers(0x2 | 0x4 | 0x8, 220, 90), "an application window takes part");
    CHECK(wbgap_switch_offers(0x4, 220, 90) && wbgap_switch_offers(0x1, 220, 90), "one system gadget is enough");
    CHECK(!wbgap_switch_offers(0x10800, 25, 23) && !wbgap_switch_offers(0x10800, 113, 23),
          "the lab's tray buttons (borderless, RMB trap) don't");
    CHECK(!wbgap_switch_offers(0x840, 200, 120), "a borderless panel doesn't");
    CHECK(!wbgap_switch_offers(0x100 | 0x2, 640, 480) && !wbgap_switch_offers(0x2, 7, 90) && wbgap_switch_offers(0xA, 8, 8),
          "backdrop and tiny out, 8x8 in");
    CHECK(!wbgap_switch_offers(0x2000000 | 0x6, 640, 480) && wbgap_switch_offers(0x2000000 | 0xA, 300, 200),
          "Workbench's root out, its drawers in");
    CHECK(wbgap_find("W2") >= 0 && !strcmp(wbgap_row(wbgap_find("W2"))->mode, "SWITCH"), "W2 comes from SWITCH");
}
