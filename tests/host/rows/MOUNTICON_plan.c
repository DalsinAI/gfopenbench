/* D2: a volume mounted later gets an icon (tests/amiga/rows/MOUNTICON.c), its judge.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "MOUNTICON.h"

void mounticon_plan(void);
WBGAP_PLAN(mounticon_plan)

void mounticon_plan(void)
{
    char why[96];
    CHECK(wbgap_mount_judge(0, 0, 0, 1, 1, why, sizeof why) == WBGAP_PASS && !why[0], "mounted, up, an icon: PASS");
    CHECK(wbgap_mount_judge(0, 0, 0, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "no icon"),
          "up but no icon: FAIL, \"%s\"", why);
    CHECK(wbgap_mount_judge(1, 0, 0, 1, 1, why, sizeof why) == WBGAP_SKIP && strstr(why, "mounted before"),
          "mounted before the test: SKIP, never a PASS, \"%s\"", why);
    CHECK(wbgap_mount_judge(0, -1, 0, 1, 1, why, sizeof why) == WBGAP_SKIP && strstr(why, "before the mount"),
          "icons not read before: SKIP, \"%s\"", why);
    CHECK(wbgap_mount_judge(0, 0, 10, 0, -1, why, sizeof why) == WBGAP_SKIP && strstr(why, "Mount gave 10"),
          "Mount failed: SKIP with its code, \"%s\"", why);
    CHECK(wbgap_mount_judge(0, 0, 0, 0, 0, why, sizeof why) == WBGAP_SKIP && strstr(why, "no volume"),
          "no volume came up: SKIP, not a FAIL of the desktop, \"%s\"", why);
    CHECK(wbgap_mount_judge(0, 0, 0, 1, -1, why, sizeof why) == WBGAP_SKIP && strstr(why, "after the mount"),
          "icons not read after: SKIP, \"%s\"", why);
    CHECK(wbgap_mount_judge(0, 0, 0, 1, 0, NULL, 0) == WBGAP_FAIL, "no room for a reason: still judged");
}
