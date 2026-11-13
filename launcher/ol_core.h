/* ol_core.h: the launcher's rules, plain C with no Amiga headers, tested on the host (tests/host/test_ol_core.c).
 * Workbench build plan row 6 (amicradle-teams designs/WORKBENCH-BUILD-PLAN-20261113.md), gap ids S2, S3, I11, I12
 * and A10, as OS 3.2.3 does them (designs/WORKBENCH-REDESIGN-20261113.md):
 *
 *   - S2: SYS:WBStartup's order. STARTPRI first, highest first. With the same STARTPRI, the entry added to the
 *     drawer first goes first (Dale, 13 Nov: "the default order is the date added"), then by name.
 *   - S3: one at a time. The next starts when this one has ended, unless its icon says DONOTWAIT; WAIT=n waits
 *     at most n seconds.
 *   - I11: Shift-double-click. Every selected icon is passed as an argument, in the order selected.
 *   - I12: STACK, TOOLPRI, DONOTWAIT and WAIT, read from the icon at launch.
 *   - A10: LoadWB's switches, as LoadWB passes them to StartWorkbench in d0.
 *
 * The Amiga side is launcher/ollib.c (gfopenlaunch.library) and GFOpenBench's desktop/openbench.c.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OL_CORE_H
#define OL_CORE_H

#define OL_WAIT_MAX 60                /* seconds a program with neither DONOTWAIT nor WAIT=n holds up the next */
#define OL_STACK_MAX (1024L * 1024L)  /* the most stack a STACK tool type gives */
#define OL_NAME_LEN 108

/* ---- tool types ---- */

/* The value of tool type key ("KEY=value", or "" for "KEY" alone; the key's case aside), or NULL. A tool type in
 * brackets, "(KEY=value)", is switched off and never matches. */
const char *ol_tt(const char *const *tt, const char *key);
/* A whole number at s (an optional sign, then digits); def when there is none. */
long ol_num(const char *s, long def);

/* ---- S2, S3: SYS:WBStartup ---- */

typedef struct ol_start {
    char name[OL_NAME_LEN];           /* the entry's name in the drawer, without ".info" */
    int pri;                          /* STARTPRI, -128 to 127 */
    int wait;                         /* seconds to wait for it to end: 0 for DONOTWAIT */
    unsigned long added;              /* when it was added to the drawer, in seconds (0: not known) */
} ol_start;

/* The entry's STARTPRI and wait, from its icon's tool types (NULL: an icon with none). */
void ol_start_read(ol_start *e, const char *const *tt);
/* qsort's order: the higher STARTPRI first; then the one added first (an unknown date last); then by name. */
int ol_start_cmp(const void *x, const void *y);

/* An AmigaDOS DateStamp (days, minutes, ticks since 1 January 1978) in seconds. */
unsigned long ol_stamp_secs(long days, long mins, long ticks);
/* The date an entry was added to its drawer, until the ACFS index (build plan row 8) records it: the older of
 * its program's and its icon's dates, either 0 when that file isn't there. 0 when neither is. */
unsigned long ol_added(unsigned long prog, unsigned long info);

/* ---- I12: the tool types at launch ---- */

typedef struct ol_opts {
    long stack;                       /* bytes */
    int pri;                          /* the process's priority, -128 to 127 */
} ol_opts;

/* What a launch gets: the most of the icon's stack, its STACK tool type and Workbench's default stack (STACK
 * each kept to OL_STACK_MAX); TOOLPRI as the priority (kept to -128 to 127), 0 without one. */
void ol_opts_read(ol_opts *o, const char *const *tt, long icon_stack, long wb_stack);

/* ---- I11: Shift-double-click ---- */

enum { OL_K_OTHER = 0, OL_K_TOOL, OL_K_PROJECT, OL_K_DRAWER };

/* sel[0..n-1]: the kinds of the icons selected, in the order selected (OL_K_...); clicked: the one double-clicked.
 * The program run: the icon clicked when it is a tool, else the first tool selected, else the clicked project's
 * own default tool (*tool -1). Every other selected icon is an argument, in the order selected (with *tool -1, the
 * clicked project too, in its place): args[] gets their indexes (n at the most), and the count is returned. -1 when there is nothing to run: a drawer or disk clicked
 * opens as itself. */
int ol_shift_args(const int *sel, int n, int clicked, int *tool, int *args);

/* ---- A10: LoadWB's switches ---- */

enum {
    OL_SW_DEBUG = 0x01, OL_SW_DELAY = 0x02, OL_SW_CLEANUP = 0x04, OL_SW_NEWPATH = 0x08,
    OL_SW_SKIPSTARTUP = 0x10, OL_SW_SIMPLEGELS = 0x20
};
#define OL_SW_ALL 0x3FUL
/* LoadWB's template, which GFOpenBench takes as its own (LEGACY gives no flag) */
#define OL_LOADWB_TEMPLATE "-DEBUG/S,DELAY/S,CLEANUP/S,NEWPATH/S,SKIP=SKIPWBSTARTUP/S,SIMPLEGELS/S,LEGACY/S"

/* The switches as LoadWB's words, one space between ("DELAY CLEANUP"; "" for none), in buf (n bytes). The
 * length written. */
int ol_switch_words(unsigned long flags, char *buf, int n);
/* The words back to flags, case aside; SKIP is SKIPWBSTARTUP, LEGACY and any other word give none. */
unsigned long ol_switch_parse(const char *line);

#endif
