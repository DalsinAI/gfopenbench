/* wbgap_rows' host test: WBGapTest's list of gap ids (every row of amicradle-teams designs/OPENBENCH-GAPS-20261112.md
 * plus the 13 Nov decisions' rows) and its report, one PASS, FAIL or SKIP line per id whatever the modes printed.
 *   test_wbgap_rows [OPENBENCH-GAPS-20261112.md]   with the file, also every "| ID |" row in it is in the list
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"

int failures, checks;

/* the per-row plans (tests/host/rows, listed by tests/amiga/gen-rows.sh) */
#define WBGAP_GEN_PLAN(fn) void fn(void);
#include "wbgap_gen_plans.inc"
#undef WBGAP_GEN_PLAN

/* the modes the row files register (WBGAP_MODE) */
static const char *const gen_modenames[] = {
#define WBGAP_GEN_MODENAME(name) name,
#include "wbgap_gen_modenames.inc"
#undef WBGAP_GEN_MODENAME
    NULL
};

/* every row a file of its own names (WBGAP_ROWMODE) is in the table, its mode the file's and a registered mode */
static void gen_rowmodes_plan(void)
{
    const char *id, *mode;
    int k, i, m;
    for (k = 0; (id = wbgap_gen_rowmode(k, &mode)); k++) {
        i = wbgap_find(id);
        CHECK(i >= 0, "%s (WBGAP_ROWMODE, mode %s) is in the table", id, mode);
        CHECK(i < 0 || (wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, mode)),
              "%s's mode is %s, its file's, not the table's %s", id, mode, i >= 0 && wbgap_row(i)->mode ? wbgap_row(i)->mode : "(none)");
        for (m = 0; gen_modenames[m] && strcmp(gen_modenames[m], mode); m++) {}
        CHECK(gen_modenames[m] != NULL, "%s's mode %s is one a row file registers (WBGAP_MODE)", id, mode);
    }
    CHECK(gen_modenames[0] != NULL, "the row files' modes are listed: %s", gen_modenames[0] ? gen_modenames[0] : "none");
    CHECK(k >= 1, "the row files are listed (gen-rows.sh): %d", k);
}

/* the ids in the gaps document's tables, 12 Nov 2026 */
static const char *const doc_ids[] = {
    "M1", "M2", "M3", "M4", "M5", "M6", "M7", "M8", "M9", "M10", "M11", "M12", "M13", "M14", "M15", "M16", "M17",
    "M18", "M19", "M20", "M21", "M22", "M23", "M24", "M25", "M26", "M27", "M28", "M29", "M30", "M31", "M32", "M33",
    "M34", "M35", "I1", "I2", "I3", "I4", "I5", "I6", "I7", "I8", "I9", "I10", "I11", "I12", "I13", "I14", "I15",
    "I16", "N1", "N2", "N3", "N4", "N5", "N6", "N7", "N8", "A1", "A2", "A3", "A4", "A5", "A6", "A7", "A8", "A9",
    "A10", "R1", "R2", "S1", "S2", "S3", "D1", "D2", "D3", "D4", "D5", "D6", "C1", "C2", "C3", "C4", "C5", "C6",
    "W1", "W2", "W3", "F1", "F2", "P1", "P2", "P3", "P4", "P5", "P6", "P7", "X1", "X2", "X3", "X4",
};

/* the lines WBGapTest and WBGapRexx printed before this list existed: each must be a row */
static const char *const printed_ids[] = {"A6b", "A8a", "A8b", "C2a", "C2b", "R1b", "R2b"};

/* rows for the 13 Nov decisions */
static const char *const decision_ids[] = {
    "M16t", "M18p", "M23r", "M28k", "M32t", "M33r", "M34t", "I11", "I12a", "I12b", "S2d", "A10a", "A10b", "L1", "L1a", "L1b", "L1c", "M15t", "M23s", "M32h", "S2i", "I12c", "X2a", "X2b", "P4a", "P4b", "P6r",
};

static void list_shape(void)
{
    int n = wbgap_row_count(), i, j;
    CHECK(n > 100 && n <= WBGAP_MAXROWS, "%d rows (room for %d)", n, WBGAP_MAXROWS);
    for (i = 0; i < n; i++) {
        const struct wbgap_row *r = wbgap_row(i);
        CHECK(r && r->id && r->id[0] && strlen(r->id) < 8 && !strchr(r->id, ' '), "row %d has an id", i);
        CHECK(r->what && r->what[0], "%s says what it is", r->id);
        CHECK(r->how == WBGAP_AUTO || r->how == WBGAP_LOOK || r->how == WBGAP_HAND, "%s: how", r->id);
        if (r->mode) CHECK(r->how == WBGAP_AUTO && r->mode[0], "%s names a mode, so it is automatic", r->id);
        for (j = i + 1; j < n; j++) CHECK(strcmp(r->id, wbgap_row(j)->id), "%s is in the list twice", r->id);
        if (r->parent) {
            int p = wbgap_find(r->parent);
            CHECK(p >= 0, "%s: its parent %s is a row", r->id, r->parent);
            CHECK(!strncmp(r->id, r->parent, strlen(r->parent)), "%s starts with its parent's id %s", r->id, r->parent);
            CHECK(p >= 0 && p < i, "%s comes after its parent", r->id);
            CHECK(p < 0 || !wbgap_row(p)->parent, "%s: one level of parts only", r->id);
        }
    }
    CHECK(wbgap_row(-1) == NULL && wbgap_row(n) == NULL, "out of range gives NULL");
    CHECK(wbgap_find("nope") == -1 && wbgap_find("") == -1 && wbgap_find(NULL) == -1, "an unknown id is -1");
    CHECK(wbgap_find("m19") == -1, "ids are matched as written");
}

static void every_id(void)
{
    size_t i;
    for (i = 0; i < sizeof doc_ids / sizeof doc_ids[0]; i++) CHECK(wbgap_find(doc_ids[i]) >= 0, "%s is a row", doc_ids[i]);
    for (i = 0; i < sizeof printed_ids / sizeof printed_ids[0]; i++)
        CHECK(wbgap_find(printed_ids[i]) >= 0, "%s (already printed) is a row", printed_ids[i]);
    for (i = 0; i < sizeof decision_ids / sizeof decision_ids[0]; i++)
        CHECK(wbgap_find(decision_ids[i]) >= 0, "%s (13 Nov decision) is a row", decision_ids[i]);
    CHECK(wbgap_has_parts(wbgap_find("A8")) && wbgap_has_parts(wbgap_find("I12")) && wbgap_has_parts(wbgap_find("C2")),
          "A8, C2 and I12 are made of parts");
    CHECK(!wbgap_has_parts(wbgap_find("M19")), "M19 isn't");
    CHECK(wbgap_find("D5r") == wbgap_find("D5") + 1 && wbgap_row(wbgap_find("D5r"))->how == WBGAP_AUTO
          && !strcmp(wbgap_row(wbgap_find("D5r"))->mode, "DISKS") && !wbgap_row(wbgap_find("D5r"))->parent
          && !wbgap_has_parts(wbgap_find("D5")),
          "D5r, RAM Disk first on the column, is a row of its own after D5, checked by DISKS");
    {   /* wbgap_ram_first: "<left> <top> <width> <name>" a line */
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
    CHECK(wbgap_row(wbgap_find("I11"))->how == WBGAP_AUTO && wbgap_row(wbgap_find("I12a"))->how == WBGAP_AUTO,
          "I11 and I12a are checked by WBGapTest (row 6's launcher)");
    CHECK(!strcmp(wbgap_row(wbgap_find("I11"))->mode, "LAUNCHDESK"), "I11 is the desktop's Shift-double-click (LAUNCHDESK)");
    CHECK(wbgap_row(wbgap_find("I10"))->how == WBGAP_AUTO && !strcmp(wbgap_row(wbgap_find("I10"))->mode, "LAUNCHDESK"),
          "I10, double-click opens or runs, is checked in LAUNCHDESK");
    CHECK(wbgap_row(wbgap_find("I3"))->how == WBGAP_AUTO && !strcmp(wbgap_row(wbgap_find("I3"))->mode, "LAUNCHDESK"),
          "I3, the lasso, is checked by WBGapDesk.rexx in LAUNCHDESK");
}

/* the launch record LaunchT writes, as LAUNCH and LAUNCHDESK read it for I11 */
static void launch_args(void)
{
    const char *want = "LaunchT p1 p2";
    CHECK(wbgap_launch_args("launch stack 2097152 pri -3 args 3 LaunchT p1 p2\n", want), "the three, in order");
    CHECK(wbgap_launch_args("launch stack 4096 pri 0 args 3 LaunchT p1 p2", want), "no line's end");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 3 LaunchT p2 p1\n", want), "out of order");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 2 LaunchT p1\n", want), "one missing");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 4 LaunchT p1 p2 p3\n", want), "one too many");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 2 LaunchT p1 p2\n", want), "a count that doesn't match");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 3 LaunchT p1 p22\n", want), "a longer name isn't the name");
    CHECK(!wbgap_launch_args("launch stack 4096 pri 0 args 3 LaunchT p1 p\n", want), "a shorter one neither");
    CHECK(!wbgap_launch_args("", want) && !wbgap_launch_args("start WBGapA\n", want), "not a launch record");
    CHECK(wbgap_launch_args("launch stack 4096 pri 0 args 1 LaunchT\n", "LaunchT"), "the tool alone");
}

/* I10: a double-click runs a tool (OpenT, its one argument itself) and opens a drawer (Sub) */
static void i10_plan(void)
{
    char why[96];
    const char *ok = "launch stack 4096 pri 0 args 1 OpenT\n";
    CHECK(wbgap_i10(NULL, 0, 1, ok, 1, why, sizeof why) == 1 && !why[0], "both: PASS, nothing to say");
    CHECK(wbgap_i10(NULL, 0, 1, NULL, 1, why, sizeof why) == 0 && strstr(why, "started nothing"), "no record: FAIL");
    CHECK(wbgap_i10(NULL, 0, 1, ok, 0, why, sizeof why) == 0 && strstr(why, "Sub opened no window"), "no window: FAIL");
    CHECK(wbgap_i10(NULL, 0, 1, NULL, 0, why, sizeof why) == 0 && strstr(why, "and Sub's"), "neither: FAIL, both said");
    CHECK(wbgap_i10(NULL, 0, 1, "launch stack 4096 pri 0 args 2 OpenT p1\n", 1, why, sizeof why) == 0 &&
          strstr(why, "not with itself"), "a selection carried along: FAIL");
    CHECK(wbgap_i10(NULL, 0, 1, "launch stack 4096 pri 0 args 1 LaunchT\n", 1, why, sizeof why) == 0,
          "another tool's record isn't OpenT's");
    CHECK(wbgap_i10(NULL, 1, 1, ok, 1, why, sizeof why) == -1 && strstr(why, "failed"), "a failed click: SKIP");
    CHECK(wbgap_i10(NULL, 0, 0, ok, 1, why, sizeof why) == -1, "not clicked: SKIP");
    CHECK(wbgap_i10("OpenT lies in the lasso's box", 0, 1, ok, 1, why, sizeof why) == -1 &&
          !strcmp(why, "OpenT lies in the lasso's box"), "the script's own SKIP, as it said it");
    CHECK(wbgap_i10("", 0, 1, ok, 1, why, sizeof why) == 1, "an empty skip file isn't a SKIP");
    CHECK(wbgap_i10(NULL, 0, 1, ok, 0, why, 8) == 0 && strlen(why) == 7, "a cut buffer");
    CHECK(wbgap_i10(NULL, 0, 1, ok, 1, NULL, 0) == 1, "no buffer");
}

/* LAUNCHDESK SETUP and DONE: DONE deletes RAM:.backdrop only when SETUP made it */
static void backdrop_plan(void)
{
    CHECK(wbgap_backdrop_setup(-1, 2048, 0) == WBGAP_BD_MADE, "no .backdrop: SETUP makes it");
    CHECK(wbgap_backdrop_setup(0, 2048, 0) == WBGAP_BD_SAVED, "an empty one is kept and put back");
    CHECK(wbgap_backdrop_setup(100, 2048, 0) == WBGAP_BD_SAVED, "a short one is kept and put back");
    CHECK(wbgap_backdrop_setup(100, 2048, 1) == WBGAP_BD_HAD, "the three lines there already: left as it is");
    CHECK(wbgap_backdrop_setup(2046, 2048, 0) == WBGAP_BD_SAVED, "2046 bytes still fit");
    CHECK(wbgap_backdrop_setup(2047, 2048, 0) == WBGAP_BD_TOOLONG, "2047 filled the buffer: too long");
    CHECK(wbgap_backdrop_setup(2047, 2048, 1) == WBGAP_BD_TOOLONG, "too long wins over the lines");
    CHECK(wbgap_backdrop_done(1, 0) == WBGAP_BD_RESTORE && wbgap_backdrop_done(1, 1) == WBGAP_BD_RESTORE,
          "a kept copy is put back");
    CHECK(wbgap_backdrop_done(0, 1) == WBGAP_BD_DELETE, "made by SETUP: deleted");
    CHECK(wbgap_backdrop_done(0, 0) == WBGAP_BD_LEAVE, "too long, or the lines there already: left alone");
}

/* B1, B2: GFOpenTitle's Power cell, whose list a click opened, and its rows */
static void bar_point_plan(void)
{
    CHECK(wbgap_point_in_box(1880, 30, 1756, 24, 139, 57), "a point inside the list: under it");
    CHECK(!wbgap_point_in_box(1880, 11, 1756, 24, 139, 57), "Power's cell on the bar, above the list: not under it");
    CHECK(wbgap_point_in_box(1756, 24, 1756, 24, 139, 57) && !wbgap_point_in_box(1895, 24, 1756, 24, 139, 57) &&
          !wbgap_point_in_box(1756, 81, 1756, 24, 139, 57), "the left and top edges are in, the right and bottom out");
    CHECK(!wbgap_point_in_box(10, 10, 0, 0, 0, 0), "an empty box holds nothing");
}

static void bar_plan(void)
{
    long xy[2];
    wbgap_power_point(400, 0, 240, 11, xy);
    CHECK(xy[0] == 625 && xy[1] == 5, "Power's cell: the middle of the row's last 30 pixels (%ld,%ld)", xy[0], xy[1]);
    CHECK(xy[0] >= 640 - WBGAP_TRAY_CELL && xy[0] < 640, "inside the last cell");
    wbgap_power_point(0, 0, 20, 10, xy);
    CHECK(xy[0] == 10 && xy[1] == 5, "a row narrower than a cell: its middle");
    CHECK(wbgap_bar_menu_owner(640, 500, 140) == WBGAP_BAR_POWER, "a list ending at the row's right edge: Power's");
    CHECK(wbgap_bar_menu_owner(640, 470, 140) == WBGAP_BAR_PREFS, "one cell in: Prefs'");
    CHECK(wbgap_bar_menu_owner(640, 200, 140) == WBGAP_BAR_OTHER, "anywhere else: another window");
    CHECK(wbgap_bar_menu_owner(640, 0, 600) == WBGAP_BAR_UNSURE, "pushed to x 0: can't tell");
    CHECK(wbgap_bar_menu_owner(100, 0, 100) == WBGAP_BAR_POWER, "at x 0 but ending at the row's edge: Power's");
    {
        int b1, b2;
        CHECK(wbgap_power_height(0, 12) == 42, "no stick: 3 + 3 rows of 12 + 3");
        CHECK(wbgap_power_height(2, 12) == 72, "two sticks: their rows and the line too");
        CHECK(wbgap_power_height(-1, 12) == 42, "a negative count is none");
        wbgap_power_judge(42, 0, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_SKIP, "no stick, the power rows: B1 PASS, B2 SKIP");
        wbgap_power_judge(72, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_PASS, "two sticks, their Eject rows: both PASS");
        wbgap_power_judge(42, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_PASS && b2 == WBGAP_FAIL, "two sticks but no Eject rows: B2 FAIL");
        wbgap_power_judge(54, 2, 12, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL && b2 == WBGAP_FAIL, "one Eject row for two sticks: neither");
        wbgap_power_judge(30, 0, 12, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL && b2 == WBGAP_SKIP, "two rows only, no stick: B1 FAIL, B2 SKIP");
        wbgap_power_judge(42, 0, 0, &b1, &b2);
        CHECK(b1 == WBGAP_FAIL, "no row height: no PASS");
    }
    CHECK(wbgap_find("B1") >= 0 && wbgap_find("B2") >= 0 && wbgap_find("B3") >= 0, "B1-B3 are rows");
    CHECK(!strcmp(wbgap_row(wbgap_find("B3"))->mode, "BAR"), "B3's line comes from BAR");
}

/* BIG's RAM:WBGapBig off the desktop before the rows that click it */
static void clear_plan(void)
{
    long b[4];
    wbgap_park_box(640, 512, 120, 60, b);
    CHECK(b[0] == 520 && b[1] == 452 && b[2] == 120 && b[3] == 60, "parked in the bottom-right corner at its smallest");
    CHECK(b[0] + b[2] == 640 && b[1] + b[3] == 512, "its edges on the screen's");
    wbgap_park_box(320, 200, 400, 300, b);
    CHECK(b[0] == 0 && b[1] == 0 && b[2] == 320 && b[3] == 200, "no bigger than the screen, never off it");
    wbgap_park_box(640, 480, 0, 0, b);
    CHECK(b[2] == 1 && b[3] == 1 && b[0] == 639 && b[1] == 479, "no minimum given: one pixel, still on the screen");
    wbgap_park_box(0, 0, 10, 10, b);
    CHECK(b[0] == 0 && b[1] == 0 && b[2] == 1 && b[3] == 1, "a screen of no size: nothing negative");
    CHECK(wbgap_clear_result(1, 0) == WBGAP_CL_GONE && wbgap_clear_result(1, 1) == WBGAP_CL_GONE, "closed: gone");
    CHECK(wbgap_clear_result(0, 1) == WBGAP_CL_PARKED, "still open but in the corner: parked");
    CHECK(wbgap_clear_result(0, 0) == WBGAP_CL_STUCK, "still open where it was: stuck");
    CHECK(wbgap_desk_clickable(WBGAP_CL_GONE) && wbgap_desk_clickable(WBGAP_CL_PARKED), "gone or parked: the desktop is clicked");
    CHECK(!wbgap_desk_clickable(WBGAP_CL_STUCK), "stuck: no clicks (they'd land on its icons)");
}

/* D5: the RAM Disk's icon among the desktop's, as WBGapRoot.rexx writes them */
static void ramdisk_plan(void)
{
    char v[32];
    const char *root = "DISK System\nDISK Ram Disk\nDRAWER WBGapI\nTOOL LaunchT\n";
    wbgap_volume_name("Ram Disk:", v, sizeof v);
    CHECK(!strcmp(v, "Ram Disk"), "NameFromLock's \"Ram Disk:\": the volume \"%s\"", v);
    wbgap_volume_name("Ram Disk:T/x", v, sizeof v);
    CHECK(!strcmp(v, "Ram Disk"), "anything after the colon dropped");
    wbgap_volume_name("A very long volume name:", v, 7);
    CHECK(!strcmp(v, "A very"), "cut to the room given: \"%s\"", v);
    wbgap_volume_name(NULL, v, sizeof v);
    CHECK(!v[0], "no path: empty");
    CHECK(wbgap_root_has_disk(root, "Ram Disk"), "the RAM Disk's icon on the desktop");
    CHECK(wbgap_root_has_disk(root, "RAM DISK") && wbgap_root_has_disk("disk ram disk", "Ram Disk"), "case not minded");
    CHECK(wbgap_root_has_disk("DISK Ram Disk\r\n", "Ram Disk") && wbgap_root_has_disk("DISK Ram Disk  \n", "Ram Disk"),
          "a line's CR and trailing blanks don't count");
    CHECK(wbgap_root_has_disk("DISK RAM Disk:\n", "Ram Disk") && wbgap_root_has_disk("DISK System:\nDISK RAM Disk: \r\n", "Ram Disk"),
          "the port's \"RAM Disk:\" (a trailing colon, as Ivanova's lab gave) is the disk");
    CHECK(!wbgap_root_has_disk("DISK RAM Disk::\n", "Ram Disk") && !wbgap_root_has_disk("DISK :\n", ""),
          "only one colon is the port's");
    CHECK(wbgap_has_nocase("DRAWER RAM DISK\n", "Ram Disk") && wbgap_has_nocase("x ram disk", "RAM Disk"),
          "the hint's search: case not minded");
    CHECK(!wbgap_has_nocase("DISK System\n", "Ram Disk") && !wbgap_has_nocase("Ram Dis", "Ram Disk") &&
          !wbgap_has_nocase(NULL, "x") && !wbgap_has_nocase("x", ""), "the hint's search: not there, or nothing to look for");
    CHECK(!wbgap_root_has_disk("DISK System\nDRAWER WBGapI\n", "Ram Disk"), "no RAM Disk icon");
    CHECK(!wbgap_root_has_disk("DRAWER Ram Disk\n", "Ram Disk"), "a drawer of that name isn't the disk");
    CHECK(!wbgap_root_has_disk("DISK Ram Disk 2\n", "Ram Disk") && !wbgap_root_has_disk("DISK Ram\n", "Ram Disk"),
          "a longer or shorter name isn't it");
    CHECK(!wbgap_root_has_disk("", "Ram Disk") && !wbgap_root_has_disk(root, "") && !wbgap_root_has_disk(NULL, "Ram Disk"),
          "nothing read, or no name: no");
}

/* C6: CloseWorkBench / OpenWorkBench with programs running */
static void closewb_plan(void)
{
    char why[96];
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 1, why, sizeof why) == WBGAP_PASS && !why[0], "refused, all still there: PASS");
    CHECK(wbgap_closewb_judge(1, 1, 0, 1, 1, why, sizeof why) == WBGAP_PASS, "closed, opened again, drawer back: PASS (a new screen is fine)");
    CHECK(wbgap_closewb_judge(1, 0, 0, 1, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't bring"), "not reopened: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 0, 1, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "screen changed"), "refused, new screen: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 1, 0, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "desktop went"), "refused, port gone: FAIL");
    CHECK(wbgap_closewb_judge(1, 1, 0, 0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't come back"),
          "closed, desktop not back: FAIL, \"%s\"", why);
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "window went"), "refused, drawer gone: FAIL");
    CHECK(wbgap_closewb_judge(1, 1, 1, 1, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "opened again"), "closed, drawer not back: FAIL");
    CHECK(wbgap_closewb_judge(0, 0, 1, 1, 1, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    CHECK(wbgap_find("C6") >= 0 && !strcmp(wbgap_row(wbgap_find("C6"))->mode, "CLOSEWB"), "C6's line comes from CLOSEWB");
    /* CLOSEWBWAIT: the Run process is over only at its last line */
    CHECK(!wbgap_closewb_over(NULL) && !wbgap_closewb_over(""), "nothing written: not over");
    CHECK(!wbgap_closewb_over("INFO workbench.library 47.42\n"), "started, CloseWorkBench() not back: not over");
    CHECK(!wbgap_closewb_over("INFO C6 CloseWorkBench() gave 0\nPASS C6\n"), "judged but not cleaned up: not over");
    CHECK(wbgap_closewb_over("INFO C6 CloseWorkBench() gave 0\nPASS C6\nINFO C6 done\n"), "its last line: over");
    CHECK(wbgap_closewb_over("SKIP C6 no Workbench screen\nINFO C6 done"), "no newline at the end: over");
    CHECK(wbgap_closewb_over("x\r\nINFO C6 done\r\n"), "CR LF: over");
    CHECK(!wbgap_closewb_over("XINFO C6 done\n") && !wbgap_closewb_over("INFO C6 doner\n"), "only the whole line counts");
}

/* I14: a file with no icon shown only with Show All Files */
static void showall_plan(void)
{
    char why[64];
    const char *drawer = "PROJECT noicon\nDRAWER Sub\r\nTOOL WBGapW\n";
    CHECK(wbgap_root_has_name(drawer, "noicon") && wbgap_root_has_name(drawer, "NOICON") && wbgap_root_has_name(drawer, "Sub"),
          "a name of any type, case not minded, CR dropped");
    CHECK(!wbgap_root_has_name(drawer, "noico") && !wbgap_root_has_name("PROJECT noicon2\n", "noicon") &&
          !wbgap_root_has_name("noicon\n", "noicon") && !wbgap_root_has_name("PROJECT \n", ""),
          "a shorter or longer name, a line with no type, no name: no");
    CHECK(!wbgap_root_has_name(NULL, "x") && !wbgap_root_has_name("", "x"), "nothing read: no");
    CHECK(wbgap_showall_judge(0, 1, why, sizeof why) == WBGAP_PASS && !why[0], "hidden, then shown: PASS");
    CHECK(wbgap_showall_judge(1, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "Only Icons"), "shown with Only Icons: FAIL");
    CHECK(wbgap_showall_judge(0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "not shown"), "never shown: FAIL");
    CHECK(wbgap_showall_judge(-1, 1, why, sizeof why) == WBGAP_SKIP && wbgap_showall_judge(0, -1, why, sizeof why) == WBGAP_SKIP,
          "a read failed: SKIP");
    CHECK(wbgap_find("I14") >= 0 && !strcmp(wbgap_row(wbgap_find("I14"))->mode, "MENUS"), "I14's line comes from MENUS");
}

/* D1: a disk's icon goes while inhibited and comes back */
static void diskio_plan(void)
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
    CHECK(wbgap_find("D1") >= 0 && wbgap_row(wbgap_find("D1"))->mode && !strcmp(wbgap_row(wbgap_find("D1"))->mode, "DISKIO"),
          "D1's line comes from DISKIO");
}

/* D2: a volume mounted later gets an icon */
static void mount_plan(void)
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
    CHECK(wbgap_find("D2") >= 0 && wbgap_row(wbgap_find("D2"))->mode && !strcmp(wbgap_row(wbgap_find("D2"))->mode, "MOUNTICON"),
          "D2's line comes from MOUNTICON");
}

/* W2: GFOpenWindows' Amiga+Tab and Amiga+Shift+Tab */
static void switch_plan(void)
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

/* A8c: WhichWorkbenchObjectA on a snapshotted icon and on the window's empty corner */
static void which_plan(void)
{
    long xy[2];
    char why[96];
    wbgap_which_point(4, 11, 20, 10, xy);
    CHECK(xy[0] == 28 && xy[1] == 25, "the icon's image's top-left + 4, from the window's corner (%ld,%ld)", xy[0], xy[1]);
    CHECK(WBGAP_WBO_NONE == 0 && WBGAP_WBO_DRAWER == 1 && WBGAP_WBO_ICON == 2, "workbench.h's WBO_ values");
    CHECK(wbgap_which_judge(WBGAP_WBO_ICON, "w", "w", WBGAP_WBO_DRAWER, why, sizeof why) == WBGAP_PASS && !why[0],
          "WBO_ICON \"w\", WBO_DRAWER: PASS");
    CHECK(wbgap_which_judge(0, "", "w", 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "type 0"),
          "WBO_NONE everywhere (no desktop's answer): FAIL, \"%s\"", why);
    CHECK(wbgap_which_judge(1, "", "w", 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "on the icon"), "the drawer, not the icon: FAIL");
    CHECK(wbgap_which_judge(2, "x", "w", 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "\"x\""), "another icon's name: FAIL");
    CHECK(wbgap_which_judge(2, NULL, "w", 1, why, sizeof why) == WBGAP_FAIL, "no name given back: FAIL");
    CHECK(wbgap_which_judge(2, "w", "w", 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "empty corner"),
          "the empty corner not the drawer: FAIL");
    CHECK(wbgap_find("A8c") >= 0 && wbgap_row(wbgap_find("A8c"))->mode && !strcmp(wbgap_row(wbgap_find("A8c"))->mode, "API"),
          "A8c's line comes from API");
}

/* I8, I9: an icon dragged onto an AppIcon and onto an AppWindow */
static void drop_plan(void)
{
    long b[4], xy[2], drawer[4] = {600, 11, 300, 200}, all[4] = {0, 0, 1920, 1080};
    char why[128];
    CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, NULL, b) && b[0] == 1680 && b[1] == 11 && b[2] == 240 && b[3] == 120,
          "nothing in the way: the top-right corner, below the title bar (%ld,%ld)", b[0], b[1]);
    drawer[0] = 1700;
    CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, drawer, b) && b[0] == 1680 && b[1] == 960,
          "the drawer in the top-right: the bottom-right (%ld,%ld)", b[0], b[1]);
    {
        long tall[4] = {1600, 0, 320, 1080};
        CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, tall, b) && b[0] == 0 && b[1] == 960,
              "the right side taken: the bottom-left (%ld,%ld)", b[0], b[1]);
    }
    {
        long low[4] = {0, 500, 1920, 580}, right[4] = {1000, 0, 920, 1080};
        CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, low, b) && b[0] == 1680 && b[1] == 11, "the bottom taken: the top-right");
        CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, right, b) && b[0] == 0 && b[1] == 960, "the right taken: the bottom-left first");
    }
    CHECK(!wbgap_drop_place(1920, 1080, 11, 240, 120, all, b), "the whole screen in the way: no place");
    CHECK(!wbgap_drop_place(200, 100, 11, 240, 120, NULL, b) && !wbgap_drop_place(640, 120, 11, 240, 120, NULL, b),
          "bigger than the screen, or than the room below its title bar: no place");
    CHECK(!wbgap_drop_place(640, 480, 11, 0, 120, NULL, b), "no size: no place");
    {
        long a[4] = {1680, 11, 240, 120}, edge[4] = {1440, 11, 240, 120};
        CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, edge, b) && b[0] == 1680 && b[1] == 11,
              "a box that only touches the corner's edge isn't in the way");
        CHECK(wbgap_drop_place(1920, 1080, 11, 240, 120, a, b) && b[1] == 960, "a box on the corner exactly is");
    }
    wbgap_drop_point(drawer, xy);
    CHECK(xy[0] == 1850 && xy[1] == 111, "let go at the box's middle (%ld,%ld)", xy[0], xy[1]);
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
    {
        unsigned long a[6 * 4], c[6 * 4];
        memset(a, 0, sizeof a);
        memcpy(c, a, sizeof c);
        CHECK(!wbgap_diff_box(a, c, 6, 4, 2, b), "nothing changed: no box");
        c[1 * 6 + 2] = 7;
        c[2 * 6 + 4] = 9;
        CHECK(wbgap_diff_box(a, c, 6, 4, 2, b) && b[0] == 4 && b[1] == 2 && b[2] == 6 && b[3] == 4,
              "two samples changed: the box round both, in pixels (%ld,%ld %ldx%ld)", b[0], b[1], b[2], b[3]);
        c[0] = 1;
        c[23] = 1;
        CHECK(wbgap_diff_box(a, c, 6, 4, 2, b) && b[0] == 0 && b[1] == 0 && b[2] == 12 && b[3] == 8, "corners: the whole inside");
        CHECK(!wbgap_diff_box(NULL, c, 6, 4, 2, b) && !wbgap_diff_box(a, c, 0, 4, 2, b) && !wbgap_diff_box(a, c, 6, 4, 0, b),
              "nothing to compare: no box");
    }
    {
        long ib[4] = {40, 30, 50, 60};
        wbgap_grab_point(ib, xy);
        CHECK(xy[0] == 65 && xy[1] == 54, "pressed on the image: across the middle, two-fifths down (%ld,%ld)", xy[0], xy[1]);
    }
    {   /* I4, I7: moved, icon and all */
        char why[160];
        CHECK(wbgap_move_judge(0, 0, 1, 1, "d", "RAM:WBGapE", why, sizeof why) == WBGAP_PASS && !why[0], "moved with its icon: PASS");
        CHECK(wbgap_move_judge(1, 1, 0, 0, "d", "RAM:WBGapE", why, sizeof why) == WBGAP_FAIL && strstr(why, "still where it was"),
              "nothing moved: FAIL (%s)", why);
        CHECK(wbgap_move_judge(0, 0, 0, 0, "d", "RAM:WBGapE", why, sizeof why) == WBGAP_FAIL && strstr(why, "gone from"),
              "gone from both: FAIL (%s)", why);
        CHECK(wbgap_move_judge(1, 1, 1, 1, "d", "RAM:WBGapE", why, sizeof why) == WBGAP_FAIL && strstr(why, "copied"),
              "in both: copied, FAIL (%s)", why);
        CHECK(wbgap_move_judge(0, 1, 1, 0, "d", "RAM:WBGapE", why, sizeof why) == WBGAP_FAIL && strstr(why, "stayed behind"),
              "the file without its icon: FAIL (%s)", why);
        CHECK(wbgap_move_judge(0, 0, 1, 0, "d", "x", why, sizeof why) == WBGAP_FAIL && strstr(why, "without its icon"),
              "the icon lost: FAIL (%s)", why);
        CHECK(wbgap_move_judge(0, 1, 1, 1, "d", "x", why, sizeof why) == WBGAP_FAIL && strstr(why, ".info"),
              "the icon in both: FAIL (%s)", why);
        CHECK(wbgap_move_judge(0, 0, 1, 1, NULL, NULL, why, 0) == WBGAP_PASS, "no names, no room for a reason: no write");
        CHECK(wbgap_find("I4") >= 0 && !strcmp(wbgap_row(wbgap_find("I4"))->mode, "DRAGFILE") &&
              wbgap_row(wbgap_find("I4"))->how == WBGAP_AUTO && !strcmp(wbgap_row(wbgap_find("I7"))->mode, "DRAGFILE") &&
              wbgap_row(wbgap_find("I7"))->how == WBGAP_AUTO, "I4 and I7 come from DRAGFILE, automatic");
    }
    CHECK(wbgap_find("I8") >= 0 && !strcmp(wbgap_row(wbgap_find("I8"))->mode, "APPDROP") &&
          !strcmp(wbgap_row(wbgap_find("I9"))->mode, "APPDROP"), "I8 and I9 come from APPDROP");
}

/* N2-N5: INFOSAVE drives the Information window; its typing's raw keys (US keyboard) */
static void infosave_rows(void)
{
    static const char *const n[] = { "N2", "N3", "N4", "N5" };
    int i, sh;
    for (i = 0; i < 4; i++)
        CHECK(wbgap_find(n[i]) >= 0 && wbgap_row(wbgap_find(n[i]))->how == WBGAP_AUTO && wbgap_row(wbgap_find(n[i]))->mode
              && !strcmp(wbgap_row(wbgap_find(n[i]))->mode, "INFOSAVE"), "%s's line comes from INFOSAVE", n[i]);
    CHECK(wbgap_rawkey('a', &sh) == 0x20 && !sh, "a is 0x20");
    CHECK(wbgap_rawkey('q', &sh) == 0x10 && wbgap_rawkey('p', &sh) == 0x19, "q 0x10, p 0x19");
    CHECK(wbgap_rawkey('z', &sh) == 0x31 && wbgap_rawkey('m', &sh) == 0x37, "z 0x31, m 0x37");
    CHECK(wbgap_rawkey('1', &sh) == 0x01 && wbgap_rawkey('0', &sh) == 0x0A, "1 0x01, 0 0x0A");
    CHECK(wbgap_rawkey('=', &sh) == 0x0C && wbgap_rawkey('-', &sh) == 0x0B, "= 0x0C, - 0x0B");
    CHECK(wbgap_rawkey(' ', &sh) == 0x40 && wbgap_rawkey('.', &sh) == 0x39 && wbgap_rawkey('/', &sh) == 0x3A, "space . /");
    CHECK(wbgap_rawkey('G', &sh) == 0x24 && sh, "G is Shift and g (0x24)");
    CHECK(wbgap_rawkey(':', &sh) == -1 && !sh && wbgap_rawkey('*', &sh) == -1, "no key for : or * here: -1");
    {   /* every character INFOSAVE types has a key */
        static const char *const typed[] = { "gapnote", "dir", "a=3", "b=2" };
        int k;
        const char *c;
        for (k = 0; k < 4; k++)
            for (c = typed[k]; *c; c++) CHECK(wbgap_rawkey(*c, &sh) > 0, "'%c' of \"%s\" has a key", *c, typed[k]);
    }
}

/* I11's line comes from LAUNCHDESK; LAUNCH's INFO line about OpenWorkbenchObjectA is no result */
static void i11_lines(void)
{
    static struct wbgap_report r;
    wbgap_report_init(&r);
    CHECK(wbgap_report_line(&r, "INFO I11 OpenWorkbenchObjectA on a tool FALSE (IoErr 103): A6's, see LAUNCHDESK") == 0,
          "LAUNCH's INFO line sets nothing");
    CHECK(r.state[wbgap_find("I11")] == WBGAP_NONE, "I11 still has no line");
    CHECK(wbgap_report_line(&r, "SKIP I11 not left out on the desktop (RAM:.backdrop): p1") == 1 &&
          r.state[wbgap_find("I11")] == WBGAP_SKIP, "WBGapDesk.rexx's SKIP");
    CHECK(wbgap_report_line(&r, "PASS I11") == 1 && r.state[wbgap_find("I11")] == WBGAP_PASS, "then LAUNCHDESK CHECK's PASS");
}

/* with the document: every "| ID |" at the start of a table row is in the list */
static void from_doc(const char *path)
{
    FILE *f = fopen(path, "r");
    char line[1024];
    int seen = 0;
    CHECK(f != NULL, "%s opens", path);
    if (!f) return;
    while (fgets(line, sizeof line, f)) {
        char id[16];
        int k = 0;
        const char *p = line;
        if (*p++ != '|') continue;
        while (*p == ' ') p++;
        if (*p < 'A' || *p > 'Z' || p[1] < '0' || p[1] > '9') continue;
        while ((*p >= 'A' && *p <= 'Z') || (*p >= '0' && *p <= '9') || (*p >= 'a' && *p <= 'z')) { if (k < 15) id[k++] = *p; p++; }
        id[k] = 0;
        if (*p != ' ' && *p != '|') continue;
        seen++;
        CHECK(wbgap_find(id) >= 0, "%s (in %s) is a row", id, path);
    }
    fclose(f);
    CHECK(seen >= 100, "%d rows read from %s", seen, path);
}

static void lines(void)
{
    struct wbgap_report r;
    int m19 = wbgap_find("M19"), m2 = wbgap_find("M2"), a3 = wbgap_find("A3");
    wbgap_report_init(&r);
    CHECK(wbgap_report_line(&r, "INFO workbench.library 45.1\n") == 0, "INFO is no result");
    CHECK(wbgap_report_line(&r, "MENU Workbench / Backdrop") == 0, "MENU is no result");
    CHECK(wbgap_report_line(&r, "") == 0 && wbgap_report_line(&r, "PASS") == 0 && wbgap_report_line(&r, "PASSM2") == 0,
          "a blank line, a bare word, no space");
    CHECK(wbgap_report_line(&r, "PASS M19\n") == 1 && r.state[m19] == WBGAP_PASS, "PASS M19");
    CHECK(wbgap_report_line(&r, "SKIP M19 no window\r\n") == 1 && r.state[m19] == WBGAP_PASS, "a later SKIP doesn't undo a PASS");
    CHECK(wbgap_report_line(&r, "FAIL M19 the requester didn't open") == 1 && r.state[m19] == WBGAP_FAIL, "FAIL wins over PASS");
    CHECK(!strcmp(r.why[m19], "the requester didn't open"), "its reason: \"%s\"", r.why[m19]);
    CHECK(wbgap_report_line(&r, "PASS M19") == 1 && r.state[m19] == WBGAP_FAIL, "a later PASS doesn't undo a FAIL");
    CHECK(wbgap_report_line(&r, "PASS Z99") == -1 && r.unknown == 1, "an id that isn't a row");
    CHECK(wbgap_report_line(&r, "  PASS M2") == 1 && r.state[m2] == WBGAP_PASS, "leading spaces");
    {
        char longl[400];
        memset(longl, 'x', sizeof longl);
        memcpy(longl, "FAIL A3 ", 8);
        longl[sizeof longl - 1] = 0;
        CHECK(wbgap_report_line(&r, longl) == 1 && strlen(r.why[a3]) == WBGAP_WHY - 1, "a long reason is cut, not overrun");
    }
}

static void wildcard(void)
{
    struct wbgap_report r;
    int m2 = wbgap_find("M2"), m19 = wbgap_find("M19"), a3 = wbgap_find("A3");
    wbgap_report_init(&r);
    wbgap_report_line(&r, "PASS M2");
    CHECK(wbgap_report_line(&r, "FAIL M* no Workbench screen") == 1, "FAIL M* is a result");
    CHECK(r.state[m2] == WBGAP_PASS, "M* leaves a row that has a line");
    CHECK(r.state[m19] == WBGAP_FAIL && !strcmp(r.why[m19], "no Workbench screen"), "M* fails M19, which had none");
    CHECK(r.state[a3] == WBGAP_NONE, "M* leaves the A rows");
    CHECK(wbgap_report_line(&r, "FAIL A* setup") == 1 && r.state[a3] == WBGAP_FAIL, "A* fails A3 (API)");
    CHECK(r.state[wbgap_find("A8b")] == WBGAP_FAIL, "and A8b, API's too");
    CHECK(r.state[wbgap_find("A10a")] == WBGAP_NONE, "A* leaves A10a, STARTUP CHECK's");
    CHECK(r.state[wbgap_find("M16t")] == WBGAP_NONE, "M* leaves M16t, which MENUS doesn't check");
    CHECK(wbgap_report_line(&r, "FAIL Q* nothing") == -1, "a wildcard that matches no row");
}

/* print, captured */
static char got[WBGAP_MAXROWS + 4][160];
static int ngot;
static void cap(void *ctx, const char *line)
{
    (void)ctx;
    if (ngot < (int)(sizeof got / sizeof got[0])) snprintf(got[ngot], sizeof got[0], "%s", line);
    ngot++;
}
static const char *line_of(const char *id)
{
    int i;
    size_t n = strlen(id);
    for (i = 0; i < ngot && i < (int)(sizeof got / sizeof got[0]); i++)
        if (!strncmp(got[i] + 5, id, n) && (got[i][5 + n] == ' ' || got[i][5 + n] == 0)) return got[i];
    return "";
}

static void print_all(void)
{
    struct wbgap_report r;
    int n = wbgap_row_count(), i, fails;
    wbgap_report_init(&r);
    wbgap_report_line(&r, "PASS A8a");
    wbgap_report_line(&r, "PASS A8b");
    wbgap_report_line(&r, "PASS C2a");
    wbgap_report_line(&r, "FAIL C2b Left-Amiga-N didn't bring Workbench to the front");
    wbgap_report_line(&r, "PASS I12a");
    wbgap_report_line(&r, "PASS I12b");
    wbgap_report_line(&r, "PASS I12c");
    wbgap_report_line(&r, "PASS M19");
    wbgap_report_line(&r, "FAIL M23 Delete missing");
    wbgap_report_line(&r, "SKIP R1 no rx");
    ngot = 0;
    fails = wbgap_report_print(&r, cap, NULL);
    CHECK(ngot == n + 1, "%d lines for %d rows and the count", ngot, n);
    for (i = 0; i < n && i < ngot; i++) {
        const char *id = wbgap_row(i)->id;
        size_t k = strlen(id);
        CHECK((!strncmp(got[i], "PASS ", 5) || !strncmp(got[i], "FAIL ", 5) || !strncmp(got[i], "SKIP ", 5)) &&
              !strncmp(got[i] + 5, id, k) && (got[i][5 + k] == 0 || got[i][5 + k] == ' '),
              "line %d is %s's: \"%s\"", i, id, got[i]);
    }
    CHECK(!strncmp(got[n], "INFO rows ", 10), "the count last: \"%s\"", got[n]);
    CHECK(!strcmp(line_of("M19"), "PASS M19"), "M19: \"%s\"", line_of("M19"));
    CHECK(!strcmp(line_of("M23"), "FAIL M23 Delete missing"), "M23: \"%s\"", line_of("M23"));
    CHECK(!strcmp(line_of("R1"), "SKIP R1 no rx"), "R1: \"%s\"", line_of("R1"));
    CHECK(!strncmp(line_of("A8"), "SKIP A8 ", 8) && strstr(line_of("A8"), "A8c"), "A8 waits on A8c: \"%s\"", line_of("A8"));
    CHECK(!strncmp(line_of("C2"), "FAIL C2 ", 8) && strstr(line_of("C2"), "C2b"), "C2 fails by C2b: \"%s\"", line_of("C2"));
    CHECK(!strcmp(line_of("I12"), "PASS I12"), "I12 passes by its parts: \"%s\"", line_of("I12"));
    CHECK(!strncmp(line_of("P6"), "SKIP P6 look: ", 14) && strstr(line_of("P6"), "P6r"), "a look row with parts: \"%s\"", line_of("P6"));
    CHECK(!strncmp(line_of("P4"), "SKIP P4 not settled: P4a P4b", 28), "an automatic row whose parts did not run: \"%s\"", line_of("P4"));
    CHECK(!strncmp(line_of("I1"), "SKIP I1 look", 12), "a look row with no line: \"%s\"", line_of("I1"));
    CHECK(!strncmp(line_of("X3"), "SKIP X3 hand", 12), "a hand row: \"%s\"", line_of("X3"));
    CHECK(!strncmp(line_of("F1"), "SKIP F1 not run", 15) && strstr(line_of("F1"), "NOTIFY"), "an automatic row not run: \"%s\"", line_of("F1"));
    CHECK(wbgap_find("M27") >= 0 && wbgap_row(wbgap_find("M27"))->mode && !strcmp(wbgap_row(wbgap_find("M27"))->mode, "API"),
          "M27's line comes from API (its AddAppMenuItemA check), so a run without API says \"not run\", never \"not written\"");
    CHECK(!strncmp(line_of("N6"), "SKIP N6 not written", 19), "an automatic row with no check yet: \"%s\"", line_of("N6"));
    CHECK(fails == 3, "three FAIL lines (M23, C2b, C2): %d", fails);
    CHECK(strstr(got[n], " 3 fail") != NULL, "the count says 3 fail: \"%s\"", got[n]);
}

static void print_empty(void)
{
    struct wbgap_report r;
    int i, skips = 0;
    wbgap_report_init(&r);
    ngot = 0;
    CHECK(wbgap_report_print(&r, cap, NULL) == 0, "nothing run: no FAIL");
    for (i = 0; i < ngot - 1; i++) skips += !strncmp(got[i], "SKIP ", 5);
    CHECK(!strcmp(line_of("S2"), "SKIP S2 not run: STARTUP CHECK"), "S2 has a check of its own, not run: \"%s\"", line_of("S2"));
    CHECK(skips == wbgap_row_count(), "nothing run: every row a SKIP (%d of %d)", skips, wbgap_row_count());
}

int main(int argc, char **argv)
{
    list_shape();
    every_id();
    if (argc > 1) from_doc(argv[1]);
    lines();
    wildcard();
    launch_args();
    i10_plan();
    backdrop_plan();
    switch_plan();
    clear_plan();
    i11_lines();
    infosave_rows();
    bar_plan();
    ramdisk_plan();
    which_plan();
    bar_point_plan();
    closewb_plan();
    diskio_plan();
    mount_plan();
    showall_plan();
    drop_plan();
#define WBGAP_GEN_PLAN(fn) fn();
#include "wbgap_gen_plans.inc"
#undef WBGAP_GEN_PLAN
    gen_rowmodes_plan();
    print_all();
    print_empty();
    printf("test_wbgap_rows: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
