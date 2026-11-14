/* F1, F2: the drawer shows changes without Update (tests/amiga/rows/NOTIFY.c), its window's size.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "NOTIFY.h"

void notify_plan(void);
WBGAP_PLAN(notify_plan)

void notify_plan(void)
{
    long s[2];
    CHECK(!wbgap_notify_size(640, 400, s) && s[0] == 640 && s[1] == 400, "big enough: left as it is");
    CHECK(!wbgap_notify_size(480, 240, s), "exactly the least: left");
    CHECK(wbgap_notify_size(200, 100, s) && s[0] == 480 && s[1] == 240, "too small both ways: 480x240");
    CHECK(wbgap_notify_size(600, 100, s) && s[0] == 600 && s[1] == 240, "only too short: its width kept");
    CHECK(wbgap_notify_size(479, 300, s) && s[0] == 480 && s[1] == 300, "one pixel narrow: widened, height kept");
}
