/* ---- I5: a file dragged onto the desktop's own space ----
 * RAM:WBGapB (a snapshotted at 20,10) opened at the top-left, 400x300; the file f made in it at 120,10, found where
 * the window's inside changed (f sorts after a, so a desktop that lays a drawer out by name moves nothing else). f is
 * dragged with DragTest to a point of the desktop's own space: the backdrop window's layer there, and 48 pixels or
 * more from every icon on the desktop (their boxes read through the WORKBENCH port). On 3.2.3 that leaves f out
 * there for the run: f and f.info stay in RAM:WBGapB, f's icon is on the desktop and gone from the drawer's window
 * (ob_drop.c's spec, lab t4-dnd). The icon is put away again (Icons > Put Away through ARexx, where the desktop has
 * MENU INVOKE) and everything is cleared afterwards.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/clip.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "DESKDROP.h"

WBGAP_MODE(DESKDROP, wbt_deskdrop, NEWROWS)
WBGAP_ROWMODE(I5, DESKDROP)

extern struct Library *LayersBase;   /* WBGapTest.c's; opened here for the row and closed again, as SWITCH does */

#define BD_DIR "RAM:WBGapB"
#define BD_REXX "RAM:wbgap-bd.rexx"
#define BD_OUT "RAM:wbgap-bd"
#define BD_W 400
#define BD_H 300
#define BD_MARGIN 48

static const char bd_rexx[] =
    "/* WBGapTest DESKDROP: LIST out win: the icons of window win, \"left top width height name\" a line, into\n"
    "   out; PUTAWAY name: that icon on the desktop selected and Icons > Put Away. MIT, Copyright (c) 2026 AmiCradle\n"
    "   Team. */\n"
    "OPTIONS RESULTS\n"
    "PARSE ARG cmd out win\n"
    "win = STRIP(win)\n"
    "IF ~SHOW('P', 'WORKBENCH') THEN EXIT 5\n"
    "ADDRESS WORKBENCH\n"
    "IF cmd = 'PUTAWAY' THEN DO\n"
    "  ICON WINDOW 'root' NAMES STRIP(out) SELECT\n"
    "  MENU WINDOW 'root' INVOKE 'ICONS.PUTAWAY'\n"
    "  EXIT RC\n"
    "END\n"
    "GETATTR 'window.icons.all.count' NAME win\n"
    "IF RC ~= 0 THEN EXIT 5\n"
    "n = RESULT\n"
    "IF n = '' | n = 'RESULT' THEN n = 0\n"
    "IF ~OPEN('o', out, 'W') THEN EXIT 5\n"
    "DO i = 0 TO n - 1\n"
    "  GETATTR 'window.icons.all.'i'.left' NAME win; x = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.top' NAME win; y = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.width' NAME win; w = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.height' NAME win; h = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.name' NAME win\n"
    "  CALL WRITELN('o', x y w h RESULT)\n"
    "END\n"
    "CALL CLOSE('o')\n"
    "EXIT 0\n";

static int bd_there(const char *path)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

static int bd_write(const char *path, const char *text)
{
    BPTR f = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG n = (LONG)strlen(text);
    int ok;
    if (!f) return 0;
    ok = Write(f, (APTR)text, n) == n;
    Close(f);
    return ok;
}

/* rx there, the REXX port up (started if not) and the script written: 1 */
static int bd_rexx_up(void)
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
    return bd_write(BD_REXX, bd_rexx);
}

/* window win's icons into list; list, or NULL if they weren't read */
static const char *bd_list(const char *win, char *list, long cap)
{
    char cmd[240], args[160];
    long n;
    DeleteFile((CONST_STRPTR)BD_OUT);
    snprintf(args, sizeof args, "LIST " BD_OUT " %s", win);
    if (!wbgap_rx_line(cmd, sizeof cmd, BD_REXX, args)) return NULL;
    wbt_run(cmd);
    n = wbt_read_all(BD_OUT, list, cap);
    DeleteFile((CONST_STRPTR)BD_OUT);
    return n < 0 ? NULL : list;
}

/* f on the desktop: 1, 0, or -1 if the desktop's icons weren't read */
static int bd_on_desk(char *list, long cap)
{
    return bd_list("root", list, cap) ? wbgap_deskdrop_has(list, "f") : -1;
}

/* a point of the desktop's own space on screen s, clear of every icon in desk (root's icons): 1 and xy, else 0 */
static int bd_spot(struct Screen *s, const char *desk, long xy[2])
{
    static const int fx[] = {7, 6, 4, 6, 7, 5}, fy[] = {7, 6, 7, 4, 4, 6};   /* eighths of the screen */
    unsigned i;
    for (i = 0; i < sizeof fx / sizeof fx[0]; i++) {
        long x = s->Width * fx[i] / 8, y = s->Height * fy[i] / 8;
        struct Layer *ly;
        struct Window *bw;
        LockLayerInfo(&s->LayerInfo);
        ly = WhichLayer(&s->LayerInfo, x, y);
        bw = ly ? (struct Window *)ly->Window : NULL;
        UnlockLayerInfo(&s->LayerInfo);
        if (!bw || !(bw->Flags & WFLG_BACKDROP)) {   /* a window there, or the screen's bar */
            printf("INFO I5 %ld,%ld: %s \"%s\" (flags $%lx)\n", x, y, bw ? "a window," : "no window", bw && bw->Title ?
                   (const char *)bw->Title : "", bw ? (unsigned long)bw->Flags : 0UL);
            continue;
        }
        if (!wbgap_deskdrop_clear(desk, x - bw->LeftEdge - bw->BorderLeft, y - bw->TopEdge - bw->BorderTop, BD_MARGIN)) {
            printf("INFO I5 %ld,%ld: an icon of the desktop within %d pixels\n", x, y, BD_MARGIN);
            continue;
        }
        xy[0] = x;
        xy[1] = y;
        return 1;
    }
    return 0;
}

static void bd_clean(int rexx)
{
    static const char *const files[] = {BD_DIR "/f.info", BD_DIR "/f", BD_DIR "/a.info", BD_DIR "/a", BD_DIR ".info",
                                        BD_DIR};
    static char list[8192];
    char cmd[200];
    unsigned i, pass;
    int t;
    if (rexx && bd_on_desk(list, sizeof list) > 0 && wbgap_rx_line(cmd, sizeof cmd, BD_REXX, "PUTAWAY f")) {
        printf("INFO I5 f put away from the desktop: Icons > Put Away through ARexx gave %d\n", wbt_run(cmd));
        Delay(50);
    }
    CloseWorkbenchObjectA((STRPTR)BD_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window("WBGapB"); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write a drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
    if (bd_there(BD_DIR)) {
        printf("INFO I5 %s was left after the clean (a file not made here); deleted with all in it\n", BD_DIR);
        wbt_run("C:Delete >NIL: " BD_DIR " ALL QUIET FORCE");
        DeleteFile((CONST_STRPTR)BD_DIR ".info");
    }
    if (rexx) {
        Delay(50);
        if (bd_on_desk(list, sizeof list) > 0) printf("INFO I5 f's icon is still on the desktop after the clean\n");
    }
    DeleteFile((CONST_STRPTR)BD_REXX);
}

int wbt_deskdrop(int argc, char **argv)
{
    static char desk[8192], win[8192];
    struct Window *w;
    long box_f[4], from[2], to[2];
    char cmd[320], why[200];
    int how, t, sf, si, on = -1, in = -1, rexx;
    (void)argc, (void)argv;
    if (!(rexx = bd_rexx_up())) { wbt_skip("I5", "no SYS:Rexxc/rx, or " BD_REXX " not written"); return 0; }
    if (!(LayersBase = OpenLibrary((CONST_STRPTR)"layers.library", 39))) { wbt_skip("I5", "no layers.library"); return 0; }
    bd_clean(rexx);
    rexx = bd_rexx_up();
    wbt_drawer(BD_DIR, "a");
    if (!wbt_open_drawer(BD_DIR, "WBGapB", 10, &how) || !(w = wbt_place("WBGapB", NULL, BD_W, BD_H))) {
        wbt_fail("I5", BD_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    if (!wbt_make_icon(w, BD_DIR, "f", WBPROJECT, 120, 10, box_f) || !(w = wbt_find_window("WBGapB"))) {
        wbt_fail("I5", "f never showed in " BD_DIR "'s window (5 s after UpdateWorkbench)");
        goto out;
    }
    if (!bd_list("root", desk, sizeof desk)) {
        wbt_skip("I5", "the desktop's icons not read (no WORKBENCH port, or GETATTR refused)");
        goto out;
    }
    {
        const char *p;
        int lines = 0;
        for (p = desk; *p; p++) lines += *p == '\n';
        printf("INFO I5 the desktop has %d icons\n", lines);
    }
    if (wbgap_deskdrop_has(desk, "f")) {
        wbt_skip("I5", "an icon named f was on the desktop before the drag");
        goto out;
    }
    if (!bd_spot(w->WScreen, desk, to)) {
        wbt_skip("I5", "no point of the desktop's own space found clear of windows and icons");
        goto out;
    }
    wbgap_grab_point(box_f, from);
    from[0] += w->LeftEdge + w->BorderLeft;
    from[1] += w->TopEdge + w->BorderTop;
    printf("INFO I5 f at %ld,%ld %ldx%ld in %s's window; drag %ld,%ld to the desktop at %ld,%ld\n", box_f[0], box_f[1],
           box_f[2], box_f[3], BD_DIR, from[0], from[1], to[0], to[1]);
    snprintf(cmd, sizeof cmd, "%s DRAG %ld %ld %ld %ld STEPS 16", wbt_dragtest(), from[0], from[1], to[0], to[1]);
    wbt_run(cmd);
    for (t = 0; t < 6; t++) {    /* up to 3 s for it on the desktop */
        Delay(25);
        if ((on = bd_on_desk(desk, sizeof desk)) != 0) break;
    }
    Delay(25);                   /* the drawer's window shown again without it */
    in = bd_list(BD_DIR, win, sizeof win) ? wbgap_deskdrop_has(win, "f") : -1;
    sf = bd_there(BD_DIR "/f");
    si = bd_there(BD_DIR "/f.info");
    printf("INFO I5 f: in %s %d/%d (file/icon); on the desktop %d; in the drawer's window %d\n", BD_DIR, sf, si, on, in);
    if (bd_there("RAM:.backdrop")) printf("INFO I5 RAM:.backdrop is there (3.2.3 writes it only for Icons > Leave Out)\n");
    wbt_say("I5", wbgap_deskdrop_judge(sf, si, on, in, "f", why, sizeof why), why);
out:
    bd_clean(rexx);
    CloseLibrary(LayersBase);
    LayersBase = NULL;
    return wbt_fails() ? 5 : 0;
}
