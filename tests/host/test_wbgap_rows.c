/* wbgap_rows' host test: WBGapTest's list of gap ids (every row of amicradle-teams designs/OPENBENCH-GAPS-20261112.md
 * plus the 13 Nov decisions' rows) and its report, one PASS, FAIL or SKIP line per id whatever the modes printed.
 *   test_wbgap_rows [OPENBENCH-GAPS-20261112.md]   with the file, also every "| ID |" row in it is in the list
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wbgap_rows.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

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
    CHECK(wbgap_row(wbgap_find("I11"))->how == WBGAP_AUTO && wbgap_row(wbgap_find("I12a"))->how == WBGAP_AUTO,
          "I11 and I12a are checked by WBGapTest (row 6's launcher)");
    CHECK(!strcmp(wbgap_row(wbgap_find("I11"))->mode, "LAUNCHDESK"), "I11 is the desktop's Shift-double-click (LAUNCHDESK)");
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

/* A8c: WhichWorkbenchObjectA on a snapshotted icon and on the window's empty corner */
static void which_plan(void)
{
    long xy[2];
    char why[96];
    wbgap_which_point(4, 11, 20, 10, xy);
    CHECK(xy[0] == 28 && xy[1] == 25, "the icon's image's top-left + 4, from the window's corner (%ld,%ld)", xy[0], xy[1]);
    CHECK(wbgap_which_judge(2, "w", "w", 1, why, sizeof why) == WBGAP_PASS && !why[0], "WBO_ICON \"w\", WBO_DRAWER: PASS");
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
    CHECK(!strncmp(line_of("N2"), "SKIP N2 not written", 19), "an automatic row with no check yet: \"%s\"", line_of("N2"));
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
    backdrop_plan();
    clear_plan();
    i11_lines();
    bar_plan();
    ramdisk_plan();
    which_plan();
    diskio_plan();
    print_all();
    print_empty();
    printf("test_wbgap_rows: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
