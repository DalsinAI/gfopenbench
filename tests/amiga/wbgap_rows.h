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
 * -1 if it names no row. "FAIL M* why" sets every M row with a mode that has no line yet. */
int wbgap_report_line(struct wbgap_report *r, const char *line);

typedef void (*wbgap_out)(void *ctx, const char *line);
/* exactly one line per row, in the order of the list, then "INFO rows N: P pass, F fail, S skip (K not run)".
 * A row with no line: with no mode of its own but parts, its parts decide it (any FAIL, FAIL; all PASS, PASS;
 * else SKIP naming the parts not settled); else SKIP "look", "hand", "not run: <mode>" (automatic, with a mode)
 * or "not written" (automatic, no check yet).
 * Returns the number of FAIL lines. */
int wbgap_report_print(const struct wbgap_report *r, wbgap_out out, void *ctx);

#endif
