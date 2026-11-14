/* A5: WorkbenchControlA's tags (tests/amiga/rows/WBCONTROL.c), as readings that must match.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "WBCONTROL.h"

void wbcontrol_plan(void);
WBGAP_PLAN(wbcontrol_plan)

void wbcontrol_plan(void)
{
    static const char *const names[] = { "GetDefaultStackSize", "stack after Set 12344", "hidden after Remove" };
    long want[] = { 1, 12344, 0 }, got[] = { 1, 12344, 0 };
    char why[128];
    CHECK(wbgap_pairs_judge(3, names, want, got, why, sizeof why) == WBGAP_PASS && !why[0], "all as wanted: PASS");
    got[1] = 4096;
    CHECK(wbgap_pairs_judge(3, names, want, got, why, sizeof why) == WBGAP_FAIL &&
          !strcmp(why, "stack after Set 12344: 4096, not 12344"), "one wrong: FAIL naming it, \"%s\"", why);
    got[2] = 1;
    CHECK(wbgap_pairs_judge(3, names, want, got, why, sizeof why) == WBGAP_FAIL && strstr(why, "4096") && strstr(why, "and more"),
          "two wrong: the first named, and more said, \"%s\"", why);
    got[0] = 0;
    CHECK(wbgap_pairs_judge(3, names, want, got, why, sizeof why) == WBGAP_FAIL && !strncmp(why, "GetDefaultStackSize", 19),
          "the first wrong one in the list is the one named, \"%s\"", why);
    CHECK(wbgap_pairs_judge(0, names, want, got, why, sizeof why) == WBGAP_SKIP, "no readings: SKIP");
    CHECK(wbgap_pairs_judge(3, names, want, got, NULL, 0) == WBGAP_FAIL, "no room for a reason: still judged");
    CHECK(!wbgap_ctl_may_set(0) && wbgap_ctl_may_set(1), "a value is set only if its Get worked (Kryten): nothing to put back otherwise");
    CHECK(wbgap_ctl_reading(1, 12344) == 12344 && wbgap_ctl_reading(0, 0) == WBGAP_CTL_NOT_READ,
          "with no Get, the reading after Set is \"not read\", never the 0 left in the variable");
    {
        static const char *const n2[] = { "GetDefaultStackSize", "the stack after SetDefaultStackSize 12344" };
        long w2[] = { 1, 12344 }, g2[2];
        g2[0] = 0; g2[1] = wbgap_ctl_reading(0, 0);
        CHECK(wbgap_pairs_judge(2, n2, w2, g2, why, sizeof why) == WBGAP_FAIL && !strncmp(why, "GetDefaultStackSize: 0, not 1", 29),
              "a Get that fails is the FAIL named, \"%s\"", why);
    }
    CHECK(wbgap_find("A5") >= 0 && wbgap_row(wbgap_find("A5"))->mode && !strcmp(wbgap_row(wbgap_find("A5"))->mode, "WBCONTROL"),
          "A5's line comes from WBCONTROL");
}
