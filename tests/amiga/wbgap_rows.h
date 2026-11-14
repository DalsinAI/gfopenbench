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
/* the k'th row whose mode comes from a file of its own (WBGAP_ROWMODE in tests/amiga/rows), NULL past the last */
const char *wbgap_gen_rowmode(int k, const char **mode);
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

/* I10 (LAUNCHDESK): WBGapDesk.rexx double-clicked OpenT (a tool) and Sub (a drawer) on the desktop. From what it
 * left: skip (a reason, or NULL), whether a click failed (bad) or all were given (clicked), OpenT's launch record
 * (NULL: none), and whether a window of Sub opened. 1 PASS, 0 FAIL, -1 SKIP; why (cap bytes) says what failed */
int wbgap_i10(const char *skip, int bad, int clicked, const char *run_rec, int sub_opened, char *why, unsigned cap);

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
/* I14: 1 if a line of list (as WBGapRoot.rexx writes it) has the name, of any type, case not minded */
int wbgap_root_has_name(const char *list, const char *name);
/* I14: RAM:WBGap/noicon (no .info) among the drawer's icons with Only Icons (must not be) and with All Files (must);
 * each 0, 1, or -1 if not read (SKIP). WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the reason in why */
int wbgap_showall_judge(int only_icons, int all_files, char *why, unsigned cap);
/* A5 (WBCONTROL) and other rows made of several readings: names[k]'s reading got[k] must be want[k] for every k.
 * WBGAP_PASS, WBGAP_FAIL (the first wrong one in why: "<name>: <got>, not <want>"), WBGAP_SKIP for none */
int wbgap_pairs_judge(int n, const char *const names[], const long want[], const long got[], char *why, unsigned cap);
/* 1 if word is anywhere in text, case not minded (D5's hint: the name there, but not as a DISK) */
int wbgap_has_nocase(const char *text, const char *word);

/* C6: CloseWorkBench() with programs running. Refused (closed 0: the Shell's window is on the screen) the same screen,
 * the desktop's port and RAM:WBGapC's drawer window must all stay; if it did close, OpenWorkBench() must bring the
 * screen back (reopened) and the desktop must open the drawer again. WBGAP_PASS or WBGAP_FAIL, the reason in why */
int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap);
/* What the CLOSEWB process (started with Run) has written so far: 1 when it
 * is over ("INFO C6 done", its last line), 0 while it isn't (or text is
 * NULL). CLOSEWBWAIT gives the row a FAIL of its own when it isn't over in
 * time, so a CloseWorkBench() that never returns can't hang the run. */
int wbgap_closewb_over(const char *text);

/* A8c, WhichWorkbenchObjectA: an icon snapshotted at (cx, cy) in a drawer, its window's borders bl and bt. The point
 * on the icon to ask about, relative to the window (WhichWorkbenchObjectA's x, y): its image's top-left + 4 */
void wbgap_which_point(long bl, long bt, long cx, long cy, long xy[2]);
/* the row from the two answers: on the icon (type, name it gave) and on the window's empty corner (type there).
 * WBGAP_WBO_* below (workbench.h's WBO_ values). WBGAP_PASS or WBGAP_FAIL, the reason in why */
enum { WBGAP_WBO_NONE = 0, WBGAP_WBO_DRAWER = 1, WBGAP_WBO_ICON = 2 };
int wbgap_which_judge(unsigned long on_icon, const char *name, const char *want, unsigned long on_empty, char *why,
                      unsigned cap);
/* A8c's empty point, relative to the window: three quarters across and down its inside (window w x h, borders
 * bl bt br bb), past a drawer's bar at the top and short of a scroller or a bar at the bottom. When that is within
 * 128 x 108 of the icon's picture at (ix, iy) (also relative to the window), a quarter across and three quarters
 * down instead. 0 for a window too small to have an inside. */
int wbgap_which_empty(long w, long h, long bl, long bt, long br, long bb, long ix, long iy, long xy[2]);
/* What the tags show of where the answer stopped: each WBOBJA_Type starts at WBGAP_WHICH_UNSET, and one left
 * there means the desktop's answer never reached the caller's tags. t1, t2 the drawer's two asks, t3 the desktop
 * window's (-1: not asked). Into out, for the FAIL line. */
#define WBGAP_WHICH_UNSET 0xFFFFUL
void wbgap_which_seen(unsigned long t1, unsigned long t2, long t3, char *out, unsigned cap);

/* I8, I9: an icon dragged from a drawer onto an AppIcon (I8) and onto an AppWindow (I9); the desktop must send
 * the AppMessage. A box (left, top, width, height) of w x h for a window or a drawer, in a corner of a screen
 * sw x sh below its title bar (top), clear of avoid (left, top, width, height; NULL for none), tried in the order
 * top-right, bottom-right, bottom-left, top-left: 1 and the box, or 0 if no corner is clear */
int wbgap_drop_place(long sw, long sh, long top, long w, long h, const long *avoid, long box[4]);
/* the point a drag lets go at: the box's middle */
void wbgap_drop_point(const long box[4], long xy[2]);
/* Where d's icon is in its drawer's window, found the same way on any desktop (a drawer window's toolbar or margins
 * put it elsewhere than its snapshot says): the window's inside sampled every step pixels (gw x gh samples, row by
 * row) before d is made and after; the box (left, top, width, height, from the inside's top-left) round every
 * sample that changed. 1 and the box, or 0 if nothing changed */
int wbgap_diff_box(const unsigned long *before, const unsigned long *after, int gw, int gh, int step, long box[4]);
/* the point to press on to drag the icon in box: across its middle, two-fifths down (on the image, above the label) */
void wbgap_grab_point(const long box[4], long xy[2]);
/* I4, I7: a file dragged within one volume is moved, its icon with it (3.2.3). What is where after the drag: the file
 * and its .info at the place it came from (src, src_info) and where it was dropped (dst, dst_info), each 1 if there.
 * name and where for the reason. WBGAP_PASS when both are at dst and neither at src; else WBGAP_FAIL, why says which */
int wbgap_move_judge(int src, int src_info, int dst, int dst_info, const char *name, const char *where, char *why,
                     unsigned cap);
/* INFOSAVE types into the Information window with DragTest KEY: c's raw key on a US keyboard, or -1 if it has
 * none here (a-z A-Z 0-9 - = . / and space); *shift is set to 1 for an upper-case letter, else 0 */
int wbgap_rawkey(int c, int *shift);
/* the command line that runs the ARexx script at path with args through SYS:Rexxc/rx, into buf: 1, or 0 (buf "")
 * if path is empty, has a space or a quote, or the line doesn't fit. rx runs a quoted first argument as the
 * program's own text, not as a file (so "path" gives Error 47 in line 1), and an unquoted path can't hold a space */
int wbgap_rx_line(char *buf, unsigned cap, const char *path, const char *args);

#endif
