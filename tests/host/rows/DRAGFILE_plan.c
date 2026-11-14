/* I4, I7: the drawer windows' width (tests/amiga/rows/DRAGFILE_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "DRAGFILE.h"

void dragfile_plan(void);
WBGAP_PLAN(dragfile_plan)

void dragfile_plan(void)
{
    CHECK(wbgap_dragfile_width(1920) == 440 && wbgap_dragfile_width(880) == 440, "a wide screen: 440 each (%ld)",
          wbgap_dragfile_width(1920));
    CHECK(wbgap_dragfile_width(800) == 400, "3.2.3's 800x600: half, 400 (%ld)", wbgap_dragfile_width(800));
    CHECK(wbgap_dragfile_width(640) == 320 && wbgap_dragfile_width(641) == 320, "640 wide: 320, the least that fits");
    CHECK(wbgap_dragfile_width(639) == 0 && wbgap_dragfile_width(320) == 0, "under 640: 0, both rows SKIP");
    CHECK(wbgap_dragfile_width(0) == 0 && wbgap_dragfile_width(-1) == 0, "no screen: 0");
}
