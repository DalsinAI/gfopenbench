/* ---- I1, I2: a click selects an icon, a click elsewhere deselects it, Shift-click adds to the selection ----
 * RAM:WBGapS (a snapshotted at 20,10) opened at the top-left, 400x300 or the screen if smaller; b made in it at
 * 120,10 and c at 220,10, each found where the window's inside changed. The names sort after a, so a desktop that
 * lays a drawer out by name (GFOpenBench) adds each at the end and moves nothing else. DragTest then clicks, a second and a half
 * apart (never a double-click), and after each click the window's selected icons are read through the WORKBENCH
 * ARexx port (window.icons.selected, as GFOpenBench and 3.2.3 both answer it):
 *   I1: a click on b: b alone; on c: c alone; on the window's empty corner: nothing.
 *   I2: a click on b, then a Shift-click on c: b and c.
 * The ARexx reader is written to RAM: by this file, so no other file is needed. Without rx or a WORKBENCH port the
 * rows SKIP. The drawer is cleared afterwards.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "SELECT.h"

WBGAP_MODE(SELECT, wbt_select, NEWROWS)
WBGAP_ROWMODE(I1, SELECT)
WBGAP_ROWMODE(I2, SELECT)

#define SEL_DIR "RAM:WBGapS"
#define SEL_REXX "RAM:wbgap-sel.rexx"
#define SEL_OUT "RAM:wbgap-sel"
#define SEL_W 400
#define SEL_H 300

static const char sel_rexx[] =
    "/* WBGapTest SELECT: the selected icons of window win, a name a line, into out */\n"
    "OPTIONS RESULTS\n"
    "PARSE ARG out win\n"
    "win = STRIP(win)\n"
    "IF ~SHOW('P', 'WORKBENCH') THEN EXIT 5\n"
    "ADDRESS WORKBENCH\n"
    "GETATTR 'window.icons.selected.count' NAME win\n"
    "IF RC ~= 0 THEN EXIT 5\n"
    "n = RESULT\n"
    "IF n = '' | n = 'RESULT' THEN n = 0\n"
    "IF ~OPEN('o', out, 'W') THEN EXIT 5\n"
    "DO i = 0 TO n - 1\n"
    "  GETATTR 'window.icons.selected.'i'.name' NAME win\n"
    "  CALL WRITELN('o', RESULT)\n"
    "END\n"
    "CALL CLOSE('o')\n"
    "EXIT 0\n";

static int sel_write(const char *path, const char *text)
{
    BPTR f = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG n = (LONG)strlen(text);
    int ok;
    if (!f) return 0;
    ok = Write(f, (APTR)text, n) == n;
    Close(f);
    return ok;
}

/* rx there and the REXX port up (started if not): 1 */
static int sel_rexx_up(void)
{
    BPTR l = Lock((CONST_STRPTR)"SYS:Rexxc/rx", ACCESS_READ);
    int up;
    if (!l) return 0;
    UnLock(l);
    Forbid();
    up = FindPort((CONST_STRPTR)"REXX") != NULL;
    Permit();
    if (!up) {
        wbt_run("Run >NIL: SYS:System/RexxMast >NIL:");
        Delay(50);
    }
    return sel_write(SEL_REXX, sel_rexx);
}

/* the drawer's selected icons into list (a name a line); list, or NULL if not read */
static const char *sel_read(const char *row, char *list, long cap)
{
    char cmd[200];
    long n;
    DeleteFile((CONST_STRPTR)SEL_OUT);
    if (!wbgap_rx_line(cmd, sizeof cmd, SEL_REXX, SEL_OUT " " SEL_DIR)) return NULL;
    wbt_run(cmd);
    n = wbt_read_all(SEL_OUT, list, cap);
    DeleteFile((CONST_STRPTR)SEL_OUT);
    if (n < 0) { printf("INFO %s the selected icons not read (no WORKBENCH port, or GETATTR refused)\n", row); return NULL; }
    return list;
}

/* a click (Shift-click if shift) at the window's inside point x,y, then a second and a half: no double-click */
static void sel_click(struct Window *w, long x, long y, int shift, const char *row, const char *what)
{
    char cmd[320];
    long sx = w->LeftEdge + w->BorderLeft + x, sy = w->TopEdge + w->BorderTop + y;
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld%s", wbt_dragtest(), sx, sy, shift ? " QUAL LSHIFT" : "");
    printf("INFO %s %s at %ld,%ld\n", row, what, sx, sy);
    wbt_run(cmd);
    Delay(75);
}

/* the selection after a step, judged; 1 if PASS. A FAIL or SKIP is printed by the caller with *why */
static int sel_step(struct Window *w, const long *at, int shift, const char *row, const char *what, const char *want,
                    char *why, unsigned cap, int *skipped)
{
    static char list[2048];
    const char *got;
    int v;
    sel_click(w, at[0], at[1], shift, row, what);
    got = sel_read(row, list, sizeof list - 1);
    if (got) {
        list[sizeof list - 1] = 0;
        printf("INFO %s after %s: \"%s\" selected\n", row, what, got);
    }
    v = wbgap_select_judge(got, want, what, why, cap);
    if (v == WBGAP_SKIP) *skipped = 1;
    return v == WBGAP_PASS;
}

static void sel_clean(void)
{
    static const char *const files[] = {SEL_DIR "/a.info", SEL_DIR "/a", SEL_DIR "/b.info", SEL_DIR "/b",
                                        SEL_DIR "/c.info", SEL_DIR "/c", SEL_DIR ".info", SEL_DIR};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)SEL_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window("WBGapS"); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write the drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
    DeleteFile((CONST_STRPTR)SEL_REXX);
    DeleteFile((CONST_STRPTR)SEL_OUT);
}

int wbt_select(int argc, char **argv)
{
    struct Window *w;
    long box_b[4], box_c[4], pb[2], pc[2], empty[2];
    char why[300];
    int how, skipped = 0, ok;
    (void)argc, (void)argv;
    sel_clean();
    if (!sel_rexx_up()) {
        wbt_skip("I1", "no rx (SYS:Rexxc/rx), or " SEL_REXX " not written");
        wbt_skip("I2", "no rx (SYS:Rexxc/rx), or " SEL_REXX " not written");
        return wbt_fails() ? 5 : 0;
    }
    wbt_drawer(SEL_DIR, "a");
    if (!wbt_open_drawer(SEL_DIR, "WBGapS", 10, &how) || !(w = wbt_place("WBGapS", NULL, SEL_W, SEL_H))) {
        wbt_fail("I1", SEL_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        wbt_fail("I2", SEL_DIR "'s window didn't open");
        goto out;
    }
    Delay(50);
    if (!wbt_make_icon(w, SEL_DIR, "b", WBPROJECT, 120, 10, box_b) || !(w = wbt_find_window("WBGapS")) ||
        !wbt_make_icon(w, SEL_DIR, "c", WBPROJECT, 220, 10, box_c) || !(w = wbt_find_window("WBGapS"))) {
        wbt_fail("I1", "b or c never showed in " SEL_DIR "'s window (5 s after UpdateWorkbench)");
        wbt_fail("I2", "no icons to click");
        goto out;
    }
    wbgap_grab_point(box_b, pb);
    wbgap_grab_point(box_c, pc);
    empty[0] = w->Width - w->BorderLeft - w->BorderRight - 30;    /* the inside's bottom-right corner: no icon there */
    empty[1] = w->Height - w->BorderTop - w->BorderBottom - 30;
    printf("INFO I1 in %s's window: b at %ld,%ld %ldx%ld, c at %ld,%ld %ldx%ld, empty at %ld,%ld\n", SEL_DIR, box_b[0],
           box_b[1], box_b[2], box_b[3], box_c[0], box_c[1], box_c[2], box_c[3], empty[0], empty[1]);

    /* I1 */
    ok = sel_step(w, pb, 0, "I1", "a click on b", "b", why, sizeof why, &skipped) &&
         (w = wbt_find_window("WBGapS")) && sel_step(w, pc, 0, "I1", "a click on c", "c", why, sizeof why, &skipped) &&
         (w = wbt_find_window("WBGapS")) &&
         sel_step(w, empty, 0, "I1", "a click on the window's empty corner", "", why, sizeof why, &skipped);
    if (ok) wbt_pass("I1");
    else if (!w) wbt_fail("I1", SEL_DIR "'s window went");
    else if (skipped) wbt_skip("I1", why);
    else wbt_fail("I1", why);

    /* I2 */
    skipped = 0;
    ok = (w = wbt_find_window("WBGapS")) && sel_step(w, pb, 0, "I2", "a click on b", "b", why, sizeof why, &skipped) &&
         (w = wbt_find_window("WBGapS")) && sel_step(w, pc, 1, "I2", "a Shift-click on c", "b c", why, sizeof why, &skipped);
    if (ok) wbt_pass("I2");
    else if (!w) wbt_fail("I2", SEL_DIR "'s window went");
    else if (skipped) wbt_skip("I2", why);
    else wbt_fail("I2", why);
    if ((w = wbt_find_window("WBGapS"))) sel_click(w, empty[0], empty[1], 0, "I2", "a click on the empty corner (clean up)");
out:
    sel_clean();
    return wbt_fails() ? 5 : 0;
}
