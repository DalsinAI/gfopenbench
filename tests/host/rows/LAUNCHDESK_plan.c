/* I11: the Shift-double-click's verdict (tests/amiga/rows/LAUNCHDESK_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "LAUNCHDESK.h"

void launchdesk_plan(void);
WBGAP_PLAN(launchdesk_plan)

void launchdesk_plan(void)
{
    char why[200];
    const char *good = "launch stack 4096 pri 0 args 3 LaunchT p1 p2\n";
    CHECK(wbgap_desk_i11(0, good, 1, why, sizeof why) == WBGAP_PASS && !why[0], "LaunchT, p1, p2: PASS");
    CHECK(wbgap_desk_i11(0, good, 0, why, sizeof why) == WBGAP_PASS, "started, no clicked marker: still judged");
    CHECK(wbgap_desk_i11(1, good, 1, why, sizeof why) == WBGAP_SKIP && strstr(why, "click failed"),
          "a click failed: SKIP whatever started, \"%s\"", why);
    CHECK(wbgap_desk_i11(0, NULL, 0, why, sizeof why) == WBGAP_SKIP && strstr(why, "no icons"),
          "not clicked: SKIP, \"%s\"", why);
    CHECK(wbgap_desk_i11(0, NULL, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "started nothing"),
          "clicked, nothing started: FAIL, \"%s\"", why);
    CHECK(wbgap_desk_i11(0, "launch stack 4096 pri 0 args 3 LaunchT p2 p1\n", 1, why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "in order"), "out of order: FAIL, \"%s\"", why);
    CHECK(wbgap_desk_i11(0, "launch stack 4096 pri 0 args 1 LaunchT\n", 1, why, sizeof why) == WBGAP_FAIL,
          "LaunchT alone (only the tool): FAIL");
    CHECK(wbgap_desk_i11(0, NULL, 1, NULL, 0) == WBGAP_FAIL, "no room for a reason: still judged");
}
