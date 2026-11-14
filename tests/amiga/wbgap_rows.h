/* wbgap_rows: every gap id of amicradle-teams designs/OPENBENCH-GAPS-20261112.md, with the rows added for
 * the 13 Nov decisions in designs/WORKBENCH-REDESIGN-20261113.md, and the report that turns WBGapTest's
 * PASS, FAIL and SKIP lines into exactly one line per id. Plain C99, no Amiga headers: WBGapTest links it
 * on the Amiga (WBGapTest REPORT) and tests/host/test_wbgap_rows.c on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_ROWS_H
#define WBGAP_ROWS_H

/* how a row is checked: by WBGapTest itself, by eye from a screenshot, or by hand */
enum { WBGAP_AUTO, WBGAP_LOOK, WBGAP_HAND };

struct wbgap_row {
    const char *id;      /* "M19", "A8a", "I12b" */
    const char *parent;  /* "A8" for "A8a"; NULL for a row of its own */
    int how;             /* WBGAP_AUTO, WBGAP_LOOK or WBGAP_HAND */
    const char *mode;    /* the WBGapTest mode (or script step) that prints its line; NULL if none yet */
    const char *what;    /* the row in a few words, for a SKIP line */
};

int wbgap_row_count(void);
const struct wbgap_row *wbgap_row(int i);
int wbgap_find(const char *id);   /* its index, or -1 */
int wbgap_has_parts(int i);       /* other rows name this one as their parent */

/* a row's state in a report, worst wins: FAIL over PASS over SKIP over nothing */
enum { WBGAP_NONE, WBGAP_SKIP, WBGAP_PASS, WBGAP_FAIL };

#define WBGAP_MAXROWS 192
#define WBGAP_WHY 96

struct wbgap_report {
    unsigned char state[WBGAP_MAXROWS];
    char why[WBGAP_MAXROWS][WBGAP_WHY];
    int unknown;   /* PASS, FAIL or SKIP lines naming no row */
};

void wbgap_report_init(struct wbgap_report *r);
/* one line of WBGapTest's output: 1 if it set a row, 0 if it isn't a result (INFO, MENU, blank),
 * -1 if it names no row. "FAIL M* why" sets every M row with no line yet that the first M row's mode checks. */
int wbgap_report_line(struct wbgap_report *r, const char *line);

typedef void (*wbgap_out)(void *ctx, const char *line);
/* exactly one line per row, in the order of the list, then "INFO rows N: P pass, F fail, S skip (K not run)".
 * A row with no line: with no mode of its own but parts, its parts decide it (any FAIL, FAIL; all PASS, PASS;
 * else SKIP naming the parts not settled); else SKIP "look", "hand", "not run: <mode>" (automatic, with a mode)
 * or "not written" (automatic, no check yet).
 * Returns the number of FAIL lines. */
int wbgap_report_print(const struct wbgap_report *r, wbgap_out out, void *ctx);

/* a launch record, "launch stack S pri P args N name1 name2 ...", as LaunchT writes it (LAUNCHREC): 1 if it holds
 * exactly the names of want (blank-separated), in that order, and N is their count; 0 otherwise */
int wbgap_launch_args(const char *rec, const char *want);

/* LAUNCHDESK and RAM:.backdrop. SETUP's plan from what it read: n bytes (-1, no file) into a buffer of cap,
 * and whether the three lines are in it already */
enum { WBGAP_BD_TOOLONG, WBGAP_BD_MADE, WBGAP_BD_SAVED, WBGAP_BD_HAD };
int wbgap_backdrop_setup(long n, long cap, int has_lines);
/* DONE's from the markers SETUP left: wbgap-backdrop-was there, put it back; else wbgap-backdrop-made there,
 * delete .backdrop; else leave .backdrop as it is */
enum { WBGAP_BD_RESTORE, WBGAP_BD_DELETE, WBGAP_BD_LEAVE };
int wbgap_backdrop_done(int have_was, int have_made);

/* The system bar (gfopenprefs TitleBar, GFOpenTitle 0.9): its row window on the title bar holds, left to right, the
 * memory and time, the systray, then Prefs and Power, one cell each (Common/traygrid.h's TRAY_CELL). Power's is the
 * row's last cell: the point (x, y) to click it, from the row's box */
#define WBGAP_TRAY_CELL 30
void wbgap_power_point(long left, long top, long width, long height, long xy[2]);
/* whose list opened, from where it hangs: Power's ends at the row's right edge, Prefs' one cell in from it (a list
 * pushed to x 0 by a narrow screen can't be told: UNSURE) */
enum { WBGAP_BAR_POWER, WBGAP_BAR_PREFS, WBGAP_BAR_OTHER, WBGAP_BAR_UNSURE };
int wbgap_bar_menu_owner(long row_right, long menu_left, long menu_width);
/* Power's list (GFOpenTitle's menu_open, MK_POWER): 3 pixels, an "Eject <volume>" row for each USB stick mounted
 * (a DOS device on gfopenusbacfs.device or usbdisk.device with a handler running) and a 6-pixel line after them,
 * then Reboot, Restart the machine... and Power off..., then 3 pixels; a row is the bar font's height + 4 */
long wbgap_power_height(int sticks, long row_height);
/* B1 (the three power rows) and B2 (an Eject row for each stick) from the list's height: WBGAP_PASS, WBGAP_FAIL or
 * WBGAP_SKIP (B2 with no stick mounted) */
void wbgap_power_judge(long height, int sticks, long row_height, int *b1, int *b2);
/* A drawer window a row leaves over the desktop (BIG's RAM:WBGapBig, 1000 icons): the next rows click the desktop's
 * left-out icons, and a click on one of its file icons stops an unattended run on "has no default tool". So it is
 * closed and waited for; if it stays, it is parked at its smallest size in the screen's bottom-right corner.
 * The box to park it in (left, top, width, height) on a screen sw x sh, for a window at least minw x minh: */
void wbgap_park_box(long sw, long sh, long minw, long minh, long box[4]);
/* what happened: gone (closed), parked (still open, moved out of the way), or stuck (still open where it was) */
enum { WBGAP_CL_GONE, WBGAP_CL_PARKED, WBGAP_CL_STUCK };
int wbgap_clear_result(int gone, int at_park_box);
/* the desktop rows may click when nothing is stuck over it */
int wbgap_desk_clickable(int clear_result);
/* D5: the RAM Disk's icon on the desktop. WBGapRoot.rexx writes the desktop's icons, one "<type> <name>" line each
 * (the WORKBENCH port's window.icons.all.N.type and .name, NAME 'root'); WBGapTest DISKS reads them.
 * The volume name from NameFromLock's "Ram Disk:" (the colon and anything after it dropped) into out: */
void wbgap_volume_name(const char *path, char *out, unsigned cap);
/* 1 if a line of list (as WBGapRoot.rexx writes it) is a DISK icon whose name is vol, case not minded; the port's
 * trailing colon ("DISK RAM Disk:") doesn't count */
int wbgap_root_has_disk(const char *list, const char *vol);
/* D5r: RAM Disk first in the desktop's volume column, from RAM:wbgap-root-pos (wbgap_rows.c) */
int wbgap_ram_first(const char *pos, const char *vol, char *top, unsigned cap);
/* I14: 1 if a line of list (as WBGapRoot.rexx writes it) has the name, of any type, case not minded */
int wbgap_root_has_name(const char *list, const char *name);
/* I14: RAM:WBGap/noicon (no .info) among the drawer's icons with Only Icons (must not be) and with All Files (must);
 * each 0, 1, or -1 if not read (SKIP). WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the reason in why */
int wbgap_showall_judge(int only_icons, int all_files, char *why, unsigned cap);
/* D1: a disk's icon goes while its file system is inhibited (as for Format) and comes back after. The three answers
 * of wbgap_root_has_disk: before, while inhibited, after; -1 where the icons weren't read. WBGAP_SKIP if it had no
 * icon before (nothing to watch) or a read failed (nothing seen), else WBGAP_PASS or WBGAP_FAIL, the reason in why */
int wbgap_diskio_judge(int before, int inhibited, int after, char *why, unsigned cap);
/* 1 if word is anywhere in text, case not minded (D5's hint: the name there, but not as a DISK) */
int wbgap_has_nocase(const char *text, const char *word);

/* C6: CloseWorkBench() with programs running. Refused (closed 0: the Shell's window is on the screen) the same screen,
 * the desktop's port and RAM:WBGapC's drawer window must all stay; if it did close, OpenWorkBench() must bring the
 * screen back (reopened) and the desktop must open the drawer again. WBGAP_PASS or WBGAP_FAIL, the reason in why */
int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap);

/* A8c, WhichWorkbenchObjectA: an icon snapshotted at (cx, cy) in a drawer, its window's borders bl and bt. The point
 * on the icon to ask about, relative to the window (WhichWorkbenchObjectA's x, y): its image's top-left + 4 */
void wbgap_which_point(long bl, long bt, long cx, long cy, long xy[2]);
/* the row from the two answers: on the icon (type, name it gave) and on the window's empty corner (type there).
 * WBGAP_WBO_* below (workbench.h's WBO_ values). WBGAP_PASS or WBGAP_FAIL, the reason in why */
enum { WBGAP_WBO_NONE = 0, WBGAP_WBO_DRAWER = 1, WBGAP_WBO_ICON = 2 };
int wbgap_which_judge(unsigned long on_icon, const char *name, const char *want, unsigned long on_empty, char *why,
                      unsigned cap);

#endif
