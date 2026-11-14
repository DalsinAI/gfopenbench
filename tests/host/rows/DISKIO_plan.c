/* D1: a disk's icon goes while inhibited and comes back (tests/amiga/rows/DISKIO.c), its judge.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "DISKIO.h"

void diskio_plan(void);
WBGAP_PLAN(diskio_plan)

void diskio_plan(void)
{
    char why[64];
    CHECK(wbgap_diskio_judge(1, 0, 1, why, sizeof why) == WBGAP_PASS && !why[0], "there, gone, back: PASS");
    CHECK(wbgap_diskio_judge(0, 0, 0, why, sizeof why) == WBGAP_SKIP && strstr(why, "no icon before"),
          "no icon before: SKIP, \"%s\"", why);
    CHECK(wbgap_diskio_judge(0, 1, 1, why, sizeof why) == WBGAP_SKIP, "no icon before, whatever after: SKIP");
    CHECK(wbgap_diskio_judge(1, 1, 1, why, sizeof why) == WBGAP_FAIL && !strcmp(why, "its icon stayed while inhibited"),
          "stayed: FAIL, \"%s\"", why);
    CHECK(wbgap_diskio_judge(1, 0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "did not come back"),
          "went, not back: FAIL, \"%s\"", why);
    CHECK(wbgap_diskio_judge(1, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "stayed") && strstr(why, "then went"),
          "stayed then went: FAIL, \"%s\"", why);
    CHECK(wbgap_diskio_judge(1, -1, 1, why, sizeof why) == WBGAP_SKIP && strstr(why, "not read"),
          "not read while inhibited: SKIP, never a PASS (Budgie), \"%s\"", why);
    CHECK(wbgap_diskio_judge(1, 0, -1, why, sizeof why) == WBGAP_SKIP, "not read after: SKIP, never a FAIL");
    CHECK(wbgap_diskio_judge(1, 1, -1, why, sizeof why) == WBGAP_SKIP && wbgap_diskio_judge(-1, 0, 1, why, sizeof why) == WBGAP_SKIP,
          "any read failed: SKIP");
    CHECK(wbgap_diskio_judge(1, 0, 1, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
}
