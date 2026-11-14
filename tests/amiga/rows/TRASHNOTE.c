/* ---- I6t, I6p: GFOS's own trash, its note, and putting back ----
 * The engine's trash is a volume's own Trashcan (<volume>:Trashcan; desktop/ob_drop.c is_engine_trash): a file dropped
 * there goes in through GFOpenFiles' engine (of_delete), with a line for it and its icon in the Trashcan's note
 * (.GFOpenFiles: name TAB where it came from), and GFOpenFiles puts it back from that note (of_restore).
 * RAM:AWBGapR (a snapshotted at 20,10) is opened at the top-left, half the screen wide at most 400, and the RAM Disk's
 * own window beside it, clear of it (found by "RAM: - ", GFOpenDrawer's title, or by the volume's name). RAM:Trashcan
 * is made, with a trashcan's icon, at 200,150 in that window, and f at 120,10 in RAM:AWBGapR, each found where its
 * window's inside changed. The Trashcan is also read by its name, through the Workbench's ARexx port (GETATTR
 * window.icons.all.N, as DESKDROP's list): its size is its own icon's, so when what changed is wider than two of it,
 * more than the Trashcan moved (the window lays its icons out again by name) and the drop could miss: I6t fails with
 * that reason. Its place is the drawer's layout frame, not the window's (14 Nov lab: 130,78 by name, drawn at
 * 146,58), so the drop stays on what changed. With no ARexx, f's size is the yardstick, as before. f is dragged onto
 * the Trashcan with DragTest.
 * I6t: f and f.info must leave RAM:AWBGapR for RAM:Trashcan, and the note must have a line for each, from
 * ...AWBGapR/f and ...AWBGapR/f.info. I6p: gfopenfiles.library's OFRestore of RAM:Trashcan/f must put f and its icon
 * back in RAM:AWBGapR, out of the Trashcan, and take their lines out of the note.
 * Only on GFOpenBench: 3.2.3's trashcan keeps no note. Only when RAM: has no Trashcan already (the user's trash is
 * not ours to empty: both SKIP). Everything this made is cleared afterwards, the Trashcan and its note with it, and
 * the RAM Disk's window closed again when it wasn't open before (it is moved and sized here, not put back).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "TRASHNOTE.h"

WBGAP_MODE(TRASHNOTE, wbt_trashnote, NEWROWS)
WBGAP_ROWMODE(I6t, TRASHNOTE)
WBGAP_ROWMODE(I6p, TRASHNOTE)

/* RAM:AWBGapR, not RAM:WBGapR: GFOpenDrawer arranges a drawer with no snapshots by name, and a drawer named after
 * "Trashcan" moved a place when the Trashcan was added, so the Trashcan's box took in both icons and the drag let go
 * between them, in RAM: itself (14 Nov lab). Named before it, nothing else in RAM:'s window moves */
#define TN_DIR "RAM:AWBGapR"
#define TN_CAN "RAM:Trashcan"
#define TN_NOTE TN_CAN "/.GFOpenFiles"
#define TN_REXX "RAM:wbgap-tn.rexx"
#define TN_POS "RAM:wbgap-tnpos"
#define TN_W 400
#define TN_H 300
#define TN_MINW 320
/* the RAM Disk's window, found by its title: GFOpenDrawer's is the path it was opened by, then " - n items" ("RAM: -
 * 3 items", 14 Nov lab), never "RAM:AWBGapR - ..."'s; a desktop that titles it with the volume's name ("RAM Disk") is
 * found by that */
#define TN_RAMWIN "RAM: - "

static const char tn_rexx[] =
    "/* WBGapTest TRASHNOTE: out win name: the box of icon name in window win, \"left top width height\", into out.\n"
    "   MIT, Copyright (c) 2026 AmiCradle Team. */\n"
    "OPTIONS RESULTS\n"
    "PARSE ARG out win icon\n"
    "win = STRIP(win); want = UPPER(STRIP(icon))\n"
    "IF ~SHOW('P', 'WORKBENCH') THEN EXIT 5\n"
    "ADDRESS WORKBENCH\n"
    "GETATTR 'window.icons.all.count' 'NAME' win\n"
    "IF RC ~= 0 THEN EXIT 5\n"
    "n = RESULT\n"
    "IF n = '' | n = 'RESULT' THEN n = 0\n"
    "DO i = 0 TO n - 1\n"
    "  GETATTR 'window.icons.all.'i'.name' 'NAME' win\n"
    "  IF UPPER(RESULT) ~= want THEN ITERATE\n"
    "  GETATTR 'window.icons.all.'i'.left' 'NAME' win; x = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.top' 'NAME' win; y = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.width' 'NAME' win; w = RESULT\n"
    "  GETATTR 'window.icons.all.'i'.height' 'NAME' win; h = RESULT\n"
    "  IF ~OPEN('o', out, 'W') THEN EXIT 5\n"
    "  CALL WRITELN('o', x y w h)\n"
    "  CALL CLOSE('o')\n"
    "  EXIT 0\n"
    "END\n"
    "EXIT 5\n";

/* RAM:Trashcan's icon in the RAM Disk's window by its name (its size, and its place in the drawer's layout),
 * checked against changed: wbgap_trashnote_iconbox's 1, 0 (no rx, no port, not listed: tried 5 times half a second
 * apart) or -1 */
static int tn_byname(const long *changed, long box[4])
{
    char cmd[240], line[96];
    BPTR l, f;
    LONG n;
    int up, t, v = 0;
    if (!(l = Lock((CONST_STRPTR)"SYS:Rexxc/rx", ACCESS_READ))) return 0;
    UnLock(l);
    Forbid();
    up = FindPort((CONST_STRPTR)"REXX") != NULL;
    Permit();
    if (!up) {
        wbt_run("Run >NIL: SYS:System/RexxMast >NIL:");
        Delay(50);
    }
    if (!(f = Open((CONST_STRPTR)TN_REXX, MODE_NEWFILE))) return 0;
    n = (LONG)strlen(tn_rexx);
    up = Write(f, (APTR)tn_rexx, n) == n;
    Close(f);
    if (!up || !wbgap_rx_line(cmd, sizeof cmd, TN_REXX, TN_POS " RAM: Trashcan")) return 0;
    for (t = 0; t < 5 && v != 1; t++) {
        if (t) Delay(25);
        DeleteFile((CONST_STRPTR)TN_POS);
        wbt_run(cmd);
        if (wbt_read_all(TN_POS, line, sizeof line) < 0) line[0] = 0;
        v = wbgap_trashnote_iconbox(line, changed, box);
    }
    DeleteFile((CONST_STRPTR)TN_POS);
    return v;
}

/* wbt_place, tried 3 times a second apart: a drawer window GFOpenDrawer has just opened may still size itself to fit
 * (14 Nov lab: the drawer's window left at 316x210 when the first ChangeWindowBox raced it) */
static struct Window *tn_place(const char *title, const long *avoid, long w, long h)
{
    struct Window *x = NULL;
    int i;
    for (i = 0; i < 3 && !x; i++) {
        if (i) Delay(50);
        x = wbt_place(title, avoid, w, h);
    }
    return x;
}

static int tn_there(const char *path)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

static int tn_gfos(void)   /* GFOpenBench's ARexx port, as WBGapTest's is_gfos */
{
    struct MsgPort *p;
    Forbid();
    p = FindPort((CONST_STRPTR)"OPENBENCH");
    Permit();
    return p != NULL;
}

/* RAM:'s volume name ("RAM Disk"), or "" */
static void tn_ram_title(char *out, long n)
{
    BPTR l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ);
    char *c;
    out[0] = 0;
    if (!l) return;
    if (!NameFromLock(l, (STRPTR)out, n)) out[0] = 0;
    UnLock(l);
    if ((c = strchr(out, ':'))) *c = 0;
}

static void tn_clean(int can_ours, int ram_ours, const char *title)
{
    static const char *const mine[] = {TN_DIR "/f.info", TN_DIR "/f", TN_DIR "/a.info", TN_DIR "/a", TN_DIR ".info",
                                       TN_DIR};
    static const char *const can[] = {TN_CAN "/f.info", TN_CAN "/f", TN_NOTE, TN_CAN ".info", TN_CAN};
    unsigned i, pass;
    int t;
    DeleteFile((CONST_STRPTR)TN_REXX);
    DeleteFile((CONST_STRPTR)TN_POS);
    CloseWorkbenchObjectA((STRPTR)TN_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window("AWBGapR"); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write a drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof mine / sizeof mine[0]; i++) DeleteFile((CONST_STRPTR)mine[i]);
        if (can_ours)
            for (i = 0; i < sizeof can / sizeof can[0]; i++) DeleteFile((CONST_STRPTR)can[i]);
    }
    if (tn_there(TN_DIR)) {      /* something the list doesn't name: all of it, as TRASH's RAM:WBGapT */
        printf("INFO I6t %s was left after the clean (a file not made here); deleted with all in it\n", TN_DIR);
        wbt_run("C:Delete >NIL: " TN_DIR " ALL QUIET FORCE");
        DeleteFile((CONST_STRPTR)TN_DIR ".info");
    }
    if (can_ours && tn_there(TN_CAN)) {   /* made here (RAM: had none): ours to clear, a desktop's own files too */
        printf("INFO I6t %s was left after the clean; deleted with all in it\n", TN_CAN);
        wbt_run("C:Delete >NIL: " TN_CAN " ALL QUIET FORCE");
        DeleteFile((CONST_STRPTR)TN_CAN ".info");
    }
    if (ram_ours && title && title[0]) {  /* the RAM Disk's window wasn't open before: closed again */
        CloseWorkbenchObjectA((STRPTR)"RAM:", NULL);
        for (t = 0; t < 40 && wbt_find_window(title); t++) Delay(5);
    }
}

/* gfopenfiles.library 1's OFNewContext(tags)(a0), OFRestore(context,trashed)(a0/a1) and OFDisposeContext(context)(a0)
 * (gfopenfilemanager include/fd/gfopenfiles_lib.fd: bias 30, so -30, -78, -36), called by hand: WBGapTest is built
 * without GFOpenFiles' headers */
static APTR ofl_new(struct Library *base)
{
    register struct Library *a6 __asm("a6") = base;
    register APTR a0 __asm("a0") = NULL;
    register APTR d0 __asm("d0");
    __asm__ volatile ("jsr -30(%%a6)" : "=r"(d0), "+r"(a0), "+r"(a6) : : "d1", "a1", "cc", "memory");
    return d0;
}

static LONG ofl_restore(struct Library *base, APTR ctx, CONST_STRPTR trashed)
{
    register struct Library *a6 __asm("a6") = base;
    register APTR a0 __asm("a0") = ctx;
    register CONST_STRPTR a1 __asm("a1") = trashed;
    register LONG d0 __asm("d0");
    __asm__ volatile ("jsr -78(%%a6)" : "=r"(d0), "+r"(a0), "+r"(a1), "+r"(a6) : : "d1", "cc", "memory");
    return d0;
}

static void ofl_dispose(struct Library *base, APTR ctx)
{
    register struct Library *a6 __asm("a6") = base;
    register APTR a0 __asm("a0") = ctx;
    __asm__ volatile ("jsr -36(%%a6)" : "+r"(a0), "+r"(a6) : : "d0", "d1", "a1", "cc", "memory");
}

int wbt_trashnote(int argc, char **argv)
{
    static char note[4096];
    struct Window *w, *rw;
    struct Screen *wb;
    struct Library *ofl;
    APTR ctx;
    long box_f[4], box_t[4], byname[4], rbox[4], from[2], to[2], ww, rc = WBGAP_PUTBACK_NOTRIED;
    char cmd[320], why[200], title[64], vol[64];
    int how, t, sf, si, df, di, v, can_ours = 0, ram_ours = 0;
    BPTR l;
    (void)argc, (void)argv;
    if (!tn_gfos()) {
        wbt_skip("I6t", "GFOS only: 3.2.3's trashcan keeps no note of where a file came from");
        wbt_skip("I6p", "GFOS only: 3.2.3 has no put back from the trashcan");
        return wbt_fails() ? 5 : 0;
    }
    if (tn_there(TN_CAN)) {
        wbt_skip("I6t", "RAM:Trashcan is there already: the user's trash, not ours to empty");
        wbt_skip("I6p", "RAM:Trashcan is there already: the user's trash, not ours to empty");
        return wbt_fails() ? 5 : 0;
    }
    wb = LockPubScreen((CONST_STRPTR)"Workbench");
    ww = wb ? wb->Width / 2 : 0;          /* two windows side by side, as DRAGFILE's */
    if (ww > TN_W) ww = TN_W;
    if (wb) UnlockPubScreen(NULL, wb);
    tn_ram_title(vol, sizeof vol);
    snprintf(title, sizeof title, "%s", TN_RAMWIN);
    if (ww < TN_MINW || !vol[0]) {
        wbt_skip("I6t", !vol[0] ? "the RAM Disk's name not read" : "the Workbench screen is narrower than two 320-wide windows");
        wbt_skip("I6p", "not tried (I6t skipped)");
        return wbt_fails() ? 5 : 0;
    }
    tn_clean(0, 0, NULL);
    ram_ours = !wbt_find_window(TN_RAMWIN) && !wbt_find_window(vol);   /* closed again after only if it wasn't open */
    can_ours = 1;                         /* from here RAM:Trashcan, if any, is ours */
    wbt_drawer(TN_DIR, "a");
    if (!wbt_open_drawer(TN_DIR, "AWBGapR", 10, &how) || !(w = tn_place("AWBGapR", NULL, ww, TN_H))) {
        wbt_fail("I6t", TN_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    rbox[0] = w->LeftEdge; rbox[1] = w->TopEdge; rbox[2] = w->Width; rbox[3] = w->Height;
    if (!wbt_open_drawer("RAM:", TN_RAMWIN, 10, &how) && wbt_find_window(vol))
        snprintf(title, sizeof title, "%s", vol);   /* a desktop that titles it with the volume's name */
    printf("INFO I6t the RAM Disk's window is found by \"%s\"\n", title);
    if (!wbt_find_window(title) || !(rw = tn_place(title, rbox, ww, TN_H))) {
        wbt_fail("I6t", "the RAM Disk's window didn't open, or couldn't be put clear of " TN_DIR "'s");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    printf("INFO I6t %s's window at %d,%d %dx%d, the RAM Disk's at %d,%d %dx%d\n", TN_DIR, w->LeftEdge, w->TopEdge,
           w->Width, w->Height, rw->LeftEdge, rw->TopEdge, rw->Width, rw->Height);
    Delay(50);
    if (!wbt_make_icon(rw, "RAM:", "Trashcan", WBGARBAGE, 200, 150, box_t) || !(rw = wbt_find_window(title)) ||
        !(w = wbt_find_window("AWBGapR")) || !wbt_make_icon(w, TN_DIR, "f", WBPROJECT, 120, 10, box_f) ||
        !(w = wbt_find_window("AWBGapR")) || !(rw = wbt_find_window(title))) {
        wbt_fail("I6t", "RAM:Trashcan or f never showed in its window (5 s after UpdateWorkbench)");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    /* both boxes shown: the name's place is the drawer's layout frame, not the window's inside, so the drop stays on
     * what changed, and the name's own width tells whether more than its icon moved */
    if ((v = tn_byname(box_t, byname)) == 1)
        printf("INFO I6t RAM:Trashcan by its name: %ldx%ld (at %ld,%ld in the drawer's layout); its window's inside "
               "changed in %ld,%ld %ldx%ld\n", byname[2], byname[3], byname[0], byname[1], box_t[0], box_t[1],
               box_t[2], box_t[3]);
    else if (v < 0)
        printf("INFO I6t RAM:Trashcan by its name is %ldx%ld, bigger than where its window changed (%ld,%ld %ldx%ld): "
               "not used\n", byname[2], byname[3], box_t[0], box_t[1], box_t[2], box_t[3]);
    else
        printf("INFO I6t RAM:Trashcan not found by its name (no rx, no WORKBENCH port, or not listed)\n");
    if (box_t[2] > (v == 1 ? byname[2] : box_f[2]) * 2) {   /* more than one icon changed: the drop could miss it */
        printf("INFO I6t RAM:Trashcan's box is %ldx%ld, wider than two of %s (%ldx%ld): more than its icon moved\n",
               box_t[2], box_t[3], v == 1 ? "its own icon" : "f's", v == 1 ? byname[2] : box_f[2],
               v == 1 ? byname[3] : box_f[3]);
        wbt_fail("I6t", "RAM:Trashcan's icon not told apart: other icons moved in its window when it was added");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    printf("INFO I6t f at %ld,%ld %ldx%ld in %s's window; RAM:Trashcan at %ld,%ld %ldx%ld in %s's\n", box_f[0], box_f[1],
           box_f[2], box_f[3], TN_DIR, box_t[0], box_t[1], box_t[2], box_t[3], title);
    wbgap_grab_point(box_f, from);
    wbgap_grab_point(box_t, to);
    from[0] += w->LeftEdge + w->BorderLeft;
    from[1] += w->TopEdge + w->BorderTop;
    to[0] += rw->LeftEdge + rw->BorderLeft;
    to[1] += rw->TopEdge + rw->BorderTop;
    snprintf(cmd, sizeof cmd, "%s DRAG %ld %ld %ld %ld STEPS 16", wbt_dragtest(), from[0], from[1], to[0], to[1]);
    printf("INFO I6t drag %ld,%ld to %ld,%ld\n", from[0], from[1], to[0], to[1]);
    wbt_run(cmd);
    for (t = 0; t < 50; t++) {            /* up to 5 s: the move is a job of its own, its note written at the end */
        if (tn_there(TN_CAN "/f") && tn_there(TN_CAN "/f.info") && tn_there(TN_NOTE)) break;
        Delay(5);
    }
    Delay(10);
    sf = tn_there(TN_DIR "/f");
    si = tn_there(TN_DIR "/f.info");
    df = tn_there(TN_CAN "/f");
    di = tn_there(TN_CAN "/f.info");
    if (wbt_read_all(TN_NOTE, note, sizeof note) < 0) note[0] = 0;
    printf("INFO I6t f: in %s %d/%d (file/icon), in RAM:Trashcan %d/%d; the note %ld bytes\n", TN_DIR, sf, si, df, di,
           (long)strlen(note));
    v = wbgap_trashnote_judge(sf, si, df, di, tn_there(TN_NOTE) ? note : NULL, "f", "AWBGapR/f", why, sizeof why);
    wbt_say("I6t", v, why);

    /* I6p: put back, as GFOpenFiles does */
    if (df) {
        if (!(ofl = OpenLibrary((CONST_STRPTR)"gfopenfiles.library", 1))) rc = WBGAP_PUTBACK_NOLIB;
        else {
            if ((ctx = ofl_new(ofl))) {
                rc = ofl_restore(ofl, ctx, (CONST_STRPTR)TN_CAN "/f");
                ofl_dispose(ofl, ctx);
            } else
                rc = WBGAP_PUTBACK_NOCTX;
            CloseLibrary(ofl);
        }
    }
    if (rc != WBGAP_PUTBACK_NOLIB && rc != WBGAP_PUTBACK_NOTRIED) Delay(10);
    if (wbt_read_all(TN_NOTE, note, sizeof note) < 0) note[0] = 0;
    printf("INFO I6p OFRestore %ld; f in %s %d/%d, in RAM:Trashcan %d/%d\n", rc, TN_DIR, tn_there(TN_DIR "/f"),
           tn_there(TN_DIR "/f.info"), tn_there(TN_CAN "/f"), tn_there(TN_CAN "/f.info"));
    v = wbgap_putback_judge(rc, tn_there(TN_DIR "/f"), tn_there(TN_DIR "/f.info"),
                            tn_there(TN_CAN "/f") || tn_there(TN_CAN "/f.info"),
                            wbgap_trashnote_line(note, "f", "AWBGapR/f") ||
                                wbgap_trashnote_line(note, "f.info", "AWBGapR/f.info"),
                            "f", why, sizeof why);
    wbt_say("I6p", v, why);
out:
    if ((l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ))) {   /* the Trashcan's icon off the RAM Disk's window */
        tn_clean(can_ours, ram_ours, title);
        UpdateWorkbench((CONST_STRPTR)"Trashcan", l, UPDATEWB_ObjectRemoved);
        UnLock(l);
    } else
        tn_clean(can_ours, ram_ours, title);
    return wbt_fails() ? 5 : 0;
}
