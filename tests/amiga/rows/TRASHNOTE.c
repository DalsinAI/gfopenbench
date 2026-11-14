/* ---- I6t, I6p: GFOS's own trash, its note, and putting back ----
 * The engine's trash is a volume's own Trashcan (<volume>:Trashcan; desktop/ob_drop.c is_engine_trash): a file dropped
 * there goes in through GFOpenFiles' engine (of_delete), with a line for it and its icon in the Trashcan's note
 * (.GFOpenFiles: name TAB where it came from), and GFOpenFiles puts it back from that note (of_restore).
 * RAM:WBGapR (a snapshotted at 20,10) is opened at the top-left, half the screen wide at most 400, and the RAM Disk's
 * own window beside it, clear of it. RAM:Trashcan is made, with a trashcan's icon, at 200,150 in that window, and f
 * at 120,10 in RAM:WBGapR, each found where its window's inside changed. f is dragged onto the Trashcan with DragTest.
 * I6t: f and f.info must leave RAM:WBGapR for RAM:Trashcan, and the note must have a line for each, from
 * ...WBGapR/f and ...WBGapR/f.info. I6p: gfopenfiles.library's OFRestore of RAM:Trashcan/f must put f and its icon
 * back in RAM:WBGapR, out of the Trashcan, and take their lines out of the note.
 * Only on GFOpenBench: 3.2.3's trashcan keeps no note. Only when RAM: has no Trashcan already (the user's trash is
 * not ours to empty: both SKIP). Everything this made is cleared afterwards, the Trashcan and its note with it.
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

#define TN_DIR "RAM:WBGapR"
#define TN_CAN "RAM:Trashcan"
#define TN_NOTE TN_CAN "/.GFOpenFiles"
#define TN_W 400
#define TN_H 300
#define TN_MINW 320

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

/* the RAM Disk's window title: its volume's name ("Ram Disk"), or "" */
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

static void tn_clean(int can_ours)
{
    static const char *const mine[] = {TN_DIR "/f.info", TN_DIR "/f", TN_DIR "/a.info", TN_DIR "/a", TN_DIR ".info",
                                       TN_DIR};
    static const char *const can[] = {TN_CAN "/f.info", TN_CAN "/f", TN_NOTE, TN_CAN ".info", TN_CAN};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)TN_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window("WBGapR"); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write a drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof mine / sizeof mine[0]; i++) DeleteFile((CONST_STRPTR)mine[i]);
        if (can_ours)
            for (i = 0; i < sizeof can / sizeof can[0]; i++) DeleteFile((CONST_STRPTR)can[i]);
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
    long box_f[4], box_t[4], rbox[4], from[2], to[2], ww, rc = WBGAP_PUTBACK_NOTRIED;
    char cmd[320], why[200], title[64];
    int how, t, sf, si, df, di, v, can_ours = 0;
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
    tn_ram_title(title, sizeof title);
    if (ww < TN_MINW || !title[0]) {
        wbt_skip("I6t", !title[0] ? "the RAM Disk's name not read" : "the Workbench screen is narrower than two 320-wide windows");
        wbt_skip("I6p", "not tried (I6t skipped)");
        return wbt_fails() ? 5 : 0;
    }
    tn_clean(0);
    can_ours = 1;                         /* from here RAM:Trashcan, if any, is ours */
    wbt_drawer(TN_DIR, "a");
    if (!wbt_open_drawer(TN_DIR, "WBGapR", 10, &how) || !(w = wbt_place("WBGapR", NULL, ww, TN_H))) {
        wbt_fail("I6t", TN_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    rbox[0] = w->LeftEdge; rbox[1] = w->TopEdge; rbox[2] = w->Width; rbox[3] = w->Height;
    if (!wbt_open_drawer("RAM:", title, 10, &how) || !(rw = wbt_place(title, rbox, ww, TN_H))) {
        wbt_fail("I6t", "the RAM Disk's window didn't open, or couldn't be put clear of " TN_DIR "'s");
        wbt_skip("I6p", "not tried (I6t failed)");
        goto out;
    }
    Delay(50);
    if (!wbt_make_icon(rw, "RAM:", "Trashcan", WBGARBAGE, 200, 150, box_t) || !(rw = wbt_find_window(title)) ||
        !(w = wbt_find_window("WBGapR")) || !wbt_make_icon(w, TN_DIR, "f", WBPROJECT, 120, 10, box_f) ||
        !(w = wbt_find_window("WBGapR")) || !(rw = wbt_find_window(title))) {
        wbt_fail("I6t", "RAM:Trashcan or f never showed in its window (5 s after UpdateWorkbench)");
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
    v = wbgap_trashnote_judge(sf, si, df, di, tn_there(TN_NOTE) ? note : NULL, "f", "WBGapR/f", why, sizeof why);
    wbt_say("I6t", v, why);

    /* I6p: put back, as GFOpenFiles does */
    if (df) {
        if (!(ofl = OpenLibrary((CONST_STRPTR)"gfopenfiles.library", 1))) rc = WBGAP_PUTBACK_NOLIB;
        else {
            if ((ctx = ofl_new(ofl))) {
                rc = ofl_restore(ofl, ctx, (CONST_STRPTR)TN_CAN "/f");
                ofl_dispose(ofl, ctx);
            } else
                rc = -1;                  /* no context: a failure of its own */
            CloseLibrary(ofl);
        }
    }
    if (rc != WBGAP_PUTBACK_NOLIB && rc != WBGAP_PUTBACK_NOTRIED) Delay(10);
    if (wbt_read_all(TN_NOTE, note, sizeof note) < 0) note[0] = 0;
    printf("INFO I6p OFRestore %ld; f in %s %d/%d, in RAM:Trashcan %d/%d\n", rc, TN_DIR, tn_there(TN_DIR "/f"),
           tn_there(TN_DIR "/f.info"), tn_there(TN_CAN "/f"), tn_there(TN_CAN "/f.info"));
    v = wbgap_putback_judge(rc, tn_there(TN_DIR "/f"), tn_there(TN_DIR "/f.info"),
                            tn_there(TN_CAN "/f") || tn_there(TN_CAN "/f.info"),
                            wbgap_trashnote_line(note, "f", "WBGapR/f") || wbgap_trashnote_line(note, "f.info", "WBGapR/f.info"),
                            "f", why, sizeof why);
    wbt_say("I6p", v, why);
out:
    if ((l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ))) {   /* the Trashcan's icon off the RAM Disk's window */
        tn_clean(can_ours);
        UpdateWorkbench((CONST_STRPTR)"Trashcan", l, UPDATEWB_ObjectRemoved);
        UnLock(l);
    } else
        tn_clean(can_ours);
    return wbt_fails() ? 5 : 0;
}
