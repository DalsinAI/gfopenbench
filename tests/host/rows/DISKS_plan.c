/* D5r: RAM Disk first in the desktop's volume column (tests/amiga/rows/DISKS.c), wbgap_ram_first on
 * "<left> <top> <width> <name>" lines. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "DISKS.h"

void disks_plan(void);
WBGAP_PLAN(disks_plan)

void disks_plan(void)
{
    char top[40];
    CHECK(wbgap_ram_first("8 20 64 Ram Disk\n8 100 64 System\n8 180 64 Work\n", "Ram Disk", top, sizeof top) == 1
          && !strcmp(top, "Ram Disk"), "RAM Disk at the column's top: PASS");
    CHECK(wbgap_ram_first("8 100 64 Ram Disk\n8 20 64 System\n", "Ram Disk", top, sizeof top) == 0
          && !strcmp(top, "System"), "System above it: FAIL, naming System");
    CHECK(wbgap_ram_first("8 100 64 Ram Disk\n300 10 64 Games\n", "Ram Disk", top, sizeof top) == 1,
          "a disk further right, higher up, is another column");
    CHECK(wbgap_ram_first("12 100 64 Ram Disk\n40 10 64 USB0\n", "Ram Disk", top, sizeof top) == 0,
          "a disk less than an icon's width right is in the column");
    CHECK(wbgap_ram_first("8 20 64 RAM DISK\r\n8 90 64 Work\n", "Ram Disk", top, sizeof top) == 1,
          "case not minded, CR and blanks ignored");
    CHECK(wbgap_ram_first("8 20 64 Ram\n", "Ram Disk", top, sizeof top) == 0, "a shorter name isn't it");
    CHECK(wbgap_ram_first("8 20 64 RAM Disk:\n8 100 64 System:\n", "Ram Disk", top, sizeof top) == 1
          && !strcmp(top, "RAM Disk"), "a port that names disks with a trailing colon (Marcus, 833f230d)");
    CHECK(wbgap_ram_first("8 20 64 System:\n8 100 64 RAM Disk:\n", "Ram Disk", top, sizeof top) == 0
          && !strcmp(top, "System"), "with colons, System above it: FAIL, naming System");
    CHECK(wbgap_ram_first("8 20 64 Ram Disk::\n", "Ram Disk", top, sizeof top) == 0, "only one colon is dropped");
    CHECK(wbgap_ram_first("8 20 64 Ram Disk\n", "Ram Disk", top, 0) == -1, "cap 0: -1, nothing written");
    CHECK(wbgap_ram_first("8 20 64 Work\n8 20 64 Ram Disk\n", "Ram Disk", top, sizeof top) == 0,
          "a tie on the top edge: the first read wins");
    CHECK(wbgap_ram_first("", "Ram Disk", top, sizeof top) == -1 && wbgap_ram_first(NULL, "Ram Disk", top, sizeof top) == -1,
          "no disks read: -1");
    CHECK(wbgap_ram_first("junk\n8 20 64 Ram Disk\n", "Ram Disk", top, sizeof top) == 1, "a line it can't read is passed over");
    CHECK(wbgap_ram_first("8 20 4 Ram Disk\n20 10 4 Work\n", "Ram Disk", top, sizeof top) == 0,
          "a width under 16 counts as 16");
    CHECK(wbgap_ram_first("10 20 40 \n", "Ram Disk", top, sizeof top) == -1 && !top[0],
          "a line with no name after the width: passed over, nothing read past it (Mr Flibble, 7456fde7)");
    CHECK(wbgap_ram_first("8 5 64 \n8 20 64 Ram Disk\n8 100 64 Work\n", "Ram Disk", top, sizeof top) == 1
          && !strcmp(top, "Ram Disk"), "a nameless line above RAM Disk is passed over");
    CHECK(wbgap_ram_first("8 5 64 :\r\n8 20 64 Ram Disk", "Ram Disk", top, sizeof top) == 1,
          "a name of just a colon is no name; the last line needs no newline");
    CHECK(wbgap_ram_first("8 20\n64 Ram Disk\n", "Ram Disk", top, sizeof top) == -1,
          "numbers split over two lines aren't joined");
    CHECK(wbgap_ram_first("8 20 64 Ram Disk\n", "Ram Disk", top, 4) == 0 && !strcmp(top, "Ram"),
          "a name longer than cap is cut to cap - 1");
    CHECK(wbgap_ram_first("8 20 64 Ram Disk\t\n8 100 64 Work\n", "Ram Disk", top, sizeof top) == 1
          && !strcmp(top, "Ram Disk"), "a trailing tab is dropped like a blank (Mr Flibble's nit, dabc05f0)");
}
