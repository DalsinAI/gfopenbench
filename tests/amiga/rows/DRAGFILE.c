/* ---- I4, I7: a file dragged between drawer windows, and onto a drawer's icon ----
 * RAM:WBGapD (a snapshotted at 20,10) at the top-left and RAM:WBGapE (b at 20,10) in a corner clear of it, each
 * 440x360, or half the screen wide if that is less (both SKIP when half is under 320). d is made in
 * RAM:WBGapD at 120,10 and the drawer sub at 20,100 (under a), each found where the window changed (wbgap_diff_box). I4: d dragged into
 * RAM:WBGapE's window, which must move it there, d.info with it (one volume: a move, as 3.2.3). I7: d, found again
 * where RAM:WBGapE's window changed, dragged onto sub's icon, which must move it into RAM:WBGapD/sub.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "DRAGFILE.h"

WBGAP_MODE(DRAGFILE, wbt_dragfile, SCRIPT)
WBGAP_ROWMODE(I4, DRAGFILE)
WBGAP_ROWMODE(I7, DRAGFILE)

static long d_box[4], sub_box[4];

static int dragfile_exists(const char *path)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

/* the file and its .info, each there or not, in dir */
static void dragfile_where(const char *dir, const char *name, int *file, int *info)
{
    char path[96];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    *file = dragfile_exists(path);
    snprintf(path, sizeof path, "%s/%s.info", dir, name);
    *info = dragfile_exists(path);
}

/* after a drag of name from src to dst: wait up to 3 s for it at dst, then judge */
static int dragfile_judge(const char *row, const char *src, const char *dst, const char *name)
{
    char why[160];
    int sf, si, df = 0, di = 0, t;
    for (t = 0; t < 30; t++) {
        dragfile_where(dst, name, &df, &di);
        if (df && di) break;
        Delay(5);
    }
    Delay(10);                                    /* the move's other half (the source side) done */
    dragfile_where(src, name, &sf, &si);
    dragfile_where(dst, name, &df, &di);
    printf("INFO %s %s: in %s %d/%d (file/icon), in %s %d/%d\n", row, name, src, sf, si, dst, df, di);
    if (wbgap_move_judge(sf, si, df, di, name, dst, why, sizeof why) == WBGAP_PASS) { wbt_pass(row); return 1; }
    wbt_fail(row, why);
    return 0;
}

static void dragfile_clean(void)
{
    static const char *const files[] = {
        "RAM:WBGapD/sub/d.info", "RAM:WBGapD/sub/d", "RAM:WBGapD/sub.info", "RAM:WBGapD/sub", "RAM:WBGapD/d.info",
        "RAM:WBGapD/d", "RAM:WBGapD/a.info", "RAM:WBGapD/a", "RAM:WBGapD.info", "RAM:WBGapD", "RAM:WBGapE/d.info",
        "RAM:WBGapE/d", "RAM:WBGapE/b.info", "RAM:WBGapE/b", "RAM:WBGapE.info", "RAM:WBGapE"};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapD/sub", NULL);
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapE", NULL);
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapD", NULL);
    for (t = 0; t < 40 && (wbt_find_window("WBGapD") || wbt_find_window("WBGapE")); t++) Delay(5);
    /* twice, 0.5 s apart: a desktop that keeps a closed drawer's window size writes its .info after the close */
    for (pass = 0; pass < 2; pass++) {
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
}

#define DRAG_WH 360   /* each drawer's window's height; its width is wbgap_dragfile_width's */

int wbt_dragfile(int argc, char **argv)
{
    int keep = argc > 2 && !stricmp(argv[2], "KEEP");
    static unsigned long before[WBT_GRAB_N];
    struct Window *dw, *ew;
    long from[2], to[2], dbox[4], ebox[4], ww;
    int how, gw, gh;
    dragfile_clean();
    {   /* two windows side by side: on a narrower screen no corner is clear of the first, through no fault of the
         * desktop's */
        struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
        ww = wbgap_dragfile_width(wb ? wb->Width : 0);   /* half the screen, at most 440 (3.2.3's 800x600: 400) */
        if (wb) UnlockPubScreen(NULL, wb);
        if (!ww) {
            wbt_skip("I4", wb ? "the Workbench screen is narrower than two 320-wide drawer windows side by side"
                          : "no Workbench screen");
            wbt_skip("I7", wb ? "the Workbench screen is narrower than two 320-wide drawer windows side by side"
                          : "no Workbench screen");
            return wbt_fails() ? 5 : 0;
        }
    }
    wbt_drawer("RAM:WBGapD", "a");
    wbt_drawer("RAM:WBGapE", "b");
    if (!(dw = wbt_open_drawer("RAM:WBGapD", "WBGapD", 10, &how))) {
        wbt_fail("I4", "RAM:WBGapD's window didn't open");
        wbt_fail("I7", "RAM:WBGapD's window didn't open");
        goto out;
    }
    if (!(dw = wbt_place("WBGapD", NULL, ww, DRAG_WH))) {
        wbt_fail("I4", "RAM:WBGapD's window couldn't be sized and put at the top-left");
        wbt_fail("I7", "RAM:WBGapD's window couldn't be sized");
        goto out;
    }
    Delay(50);
    if (!wbt_make_icon(dw, "RAM:WBGapD", "d", WBPROJECT, 120, 10, d_box) ||
        !(dw = wbt_find_window("WBGapD")) || !wbt_make_icon(dw, "RAM:WBGapD", "sub", WBDRAWER, 20, 100, sub_box)) {
        wbt_fail("I4", "d or sub never showed in RAM:WBGapD's window (5 s after UpdateWorkbench)");
        wbt_fail("I7", "no icons to drag");
        goto out;
    }
    printf("INFO I4 in RAM:WBGapD's window: d at %ld,%ld %ldx%ld, sub at %ld,%ld %ldx%ld\n", d_box[0], d_box[1],
           d_box[2], d_box[3], sub_box[0], sub_box[1], sub_box[2], sub_box[3]);
    if (!(dw = wbt_find_window("WBGapD"))) { wbt_fail("I4", "RAM:WBGapD's window went"); wbt_fail("I7", "RAM:WBGapD's window went"); goto out; }
    dbox[0] = dw->LeftEdge; dbox[1] = dw->TopEdge; dbox[2] = dw->Width; dbox[3] = dw->Height;
    if (!wbt_open_drawer("RAM:WBGapE", "WBGapE", 10, &how) || !(ew = wbt_place("WBGapE", dbox, ww, DRAG_WH))) {
        wbt_fail("I4", "RAM:WBGapE's window didn't open, or couldn't be put clear of RAM:WBGapD's");
        wbt_fail("I7", "no second window");
        goto out;
    }
    Delay(50);
    /* I4: d from RAM:WBGapD's window into RAM:WBGapE's, dropped across the middle, two-thirds down: clear of b and
     * of any toolbar along the top */
    wbt_grab(ew, before, &gw, &gh);
    wbgap_grab_point(d_box, from);
    from[0] += dw->LeftEdge + dw->BorderLeft;
    from[1] += dw->TopEdge + dw->BorderTop;
    to[0] = ew->LeftEdge + ew->Width / 2;
    to[1] = ew->TopEdge + ew->Height * 2 / 3;
    wbt_drop_drag(from, to, "I4");
    if (!dragfile_judge("I4", "RAM:WBGapD", "RAM:WBGapE", "d")) { wbt_fail("I7", "I4 failed: no d in RAM:WBGapE to drag"); goto out; }
    /* I7: d, where RAM:WBGapE's window changed, onto sub's icon in RAM:WBGapD's window */
    if (!(ew = wbt_find_window("WBGapE")) || !wbt_settle(ew, before, gw, gh, ebox)) {
        wbt_fail("I7", "d's icon not seen in RAM:WBGapE's window after I4");
        goto out;
    }
    printf("INFO I7 d at %ld,%ld %ldx%ld in RAM:WBGapE's window\n", ebox[0], ebox[1], ebox[2], ebox[3]);
    wbgap_grab_point(ebox, from);
    from[0] += ew->LeftEdge + ew->BorderLeft;
    from[1] += ew->TopEdge + ew->BorderTop;
    if (!(dw = wbt_find_window("WBGapD"))) { wbt_fail("I7", "RAM:WBGapD's window went"); goto out; }
    wbgap_grab_point(sub_box, to);
    to[0] += dw->LeftEdge + dw->BorderLeft;
    to[1] += dw->TopEdge + dw->BorderTop;
    wbt_drop_drag(from, to, "I7");
    dragfile_judge("I7", "RAM:WBGapE", "RAM:WBGapD/sub", "d");
out:
    if (!keep) dragfile_clean();   /* KEEP: the windows left open, to look at */
    return wbt_fails() ? 5 : 0;
}
