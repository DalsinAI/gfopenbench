/* ---- I6: a file dragged onto a trashcan ----
 * RAM:WBGapT (a snapshotted at 20,10) opened at the top-left, 400x300 or the screen if smaller; the file f made in it
 * at 120,10 and the drawer trash, with a trashcan's icon (WBGARBAGE), at 220,10, each found where the window's inside
 * changed (both sort after a, so a desktop that lays a drawer out by name moves nothing else). f is dragged onto
 * trash's icon with DragTest. On one volume that must move f into RAM:WBGapT/trash, f.info with it, as 3.2.3 does,
 * and leave the trashcan where it is. Everything is cleared afterwards.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "TRASH.h"

extern struct IntuitionBase *IntuitionBase;

WBGAP_MODE(TRASH, wbt_trash, NEWROWS)
WBGAP_ROWMODE(I6, TRASH)

#define TR_DIR "RAM:WBGapT"
#define TR_CAN TR_DIR "/trash"
#define TR_W 400
#define TR_H 300

static int tr_there(const char *path)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

/* RAM:WBGapT's own window: the first whose title passes wbgap_trash_title_ours, never WBGapTest's own (whose title
 * holds "WBGapT" too and which wbt_find_window can find first in a full run) */
static struct Window *tr_window(void)
{
    const char *titles[32];
    struct Window *wins[32];
    struct Screen *s;
    struct Window *w;
    int n = 0, i;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && n < 32; s = s->NextScreen)
        for (w = s->FirstWindow; w && n < 32; w = w->NextWindow) {
            titles[n] = w->Title ? (const char *)w->Title : NULL;
            wins[n++] = w;
        }
    i = wbgap_trash_pick(titles, n);
    UnlockIBase(lock);
    return i >= 0 ? wins[i] : NULL;
}

/* that window waited for up to secs, then put at the top-left under the screen's bar, w x h at most the screen's (as
 * wbt_place does, but on RAM:WBGapT's own window) and waited for 2 s to get there */
static struct Window *tr_open_place(int secs, long w, long h)
{
    struct Window *x = NULL;
    long box[4];
    int t;
    for (t = 0; t < secs * 10 && !(x = tr_window()); t++) Delay(5);
    if (!x) return NULL;
    box[1] = x->WScreen->BarHeight + 1;
    box[0] = 0;
    box[2] = w > x->WScreen->Width ? x->WScreen->Width : w;
    box[3] = h > x->WScreen->Height - box[1] ? x->WScreen->Height - box[1] : h;
    ChangeWindowBox(x, box[0], box[1], box[2], box[3]);
    for (t = 0; t < 40; t++) {
        Delay(5);
        if (!(x = tr_window())) return NULL;
        if (x->LeftEdge == box[0] && x->TopEdge == box[1] && x->Width == box[2] && x->Height == box[3]) return x;
    }
    return NULL;
}

static void tr_clean(void)
{
    static const char *const files[] = {TR_CAN "/f.info", TR_CAN "/f", TR_CAN ".info", TR_CAN, TR_DIR "/f.info",
                                        TR_DIR "/f", TR_DIR "/a.info", TR_DIR "/a", TR_DIR ".info", TR_DIR};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)TR_CAN, NULL);
    CloseWorkbenchObjectA((STRPTR)TR_DIR, NULL);
    for (t = 0; t < 40 && tr_window(); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write a drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
    if (tr_there(TR_DIR)) {      /* something the list doesn't name (a desktop's own note in the trash): all of it */
        printf("INFO I6 %s was left after the clean (a file not made here); deleted with all in it\n", TR_DIR);
        wbt_run("C:Delete >NIL: " TR_DIR " ALL QUIET FORCE");
        DeleteFile((CONST_STRPTR)TR_DIR ".info");
    }
}

int wbt_trash(int argc, char **argv)
{
    struct Window *w;
    long box_f[4], box_t[4], from[2], to[2];
    char cmd[320], why[200];
    int how, t, sf, si, df = 0, di = 0, ci;
    BPTR l;
    (void)argc, (void)argv;
    tr_clean();
    wbt_drawer(TR_DIR, "a");
    wbt_open_drawer(TR_DIR, "WBGapT", 10, &how);   /* its wait can end on WBGapTest's own window: tr_open_place waits for ours */
    if (!(w = tr_open_place(10, TR_W, TR_H))) {
        long want[4] = {0, 0, TR_W, TR_H}, got[4] = {0, 0, 0, 0}, lim[4] = {0, 0, 0, 0};
        char title[80] = "";
        int found = 0;
        ULONG ib = LockIBase(0);   /* what wbt_place asked for and what the window found by the name has, read as one */
        if ((w = tr_window()) || (w = wbt_find_window("WBGapT"))) {
            long top = w->WScreen->BarHeight + 1;
            want[1] = top;
            want[2] = TR_W > w->WScreen->Width ? w->WScreen->Width : TR_W;
            want[3] = TR_H > w->WScreen->Height - top ? w->WScreen->Height - top : TR_H;
            got[0] = w->LeftEdge, got[1] = w->TopEdge, got[2] = w->Width, got[3] = w->Height;
            lim[0] = w->MinWidth, lim[1] = w->MinHeight, lim[2] = w->MaxWidth, lim[3] = w->MaxHeight;
            snprintf(title, sizeof title, "%s", w->Title ? (const char *)w->Title : "");
            found = 1;
        }
        UnlockIBase(ib);
        if (found) printf("INFO I6 the window found by \"WBGapT\": \"%s\" (opened by %s)\n", title, how ? "DragTest" : "workbench.library");
        if (wbgap_trash_open_why(found, title, want, got, lim, why, sizeof why) || !found) {
            snprintf(cmd, sizeof cmd, TR_DIR "'s window: %s", why);
            wbt_fail("I6", cmd);
            goto out;
        }
        printf("INFO I6 %s's window %s\n", TR_DIR, why);
    }
    Delay(50);
    if ((l = CreateDir((CONST_STRPTR)TR_CAN))) UnLock(l);   /* wbt_make_icon gives it the trashcan's icon */
    if (!wbt_make_icon(w, TR_DIR, "f", WBPROJECT, 120, 10, box_f) || !(w = tr_window()) ||
        !wbt_make_icon(w, TR_DIR, "trash", WBGARBAGE, 220, 10, box_t) || !(w = tr_window())) {
        wbt_fail("I6", "f or the trashcan never showed in " TR_DIR "'s window (5 s after UpdateWorkbench)");
        goto out;
    }
    printf("INFO I6 in %s's window: f at %ld,%ld %ldx%ld, the trashcan at %ld,%ld %ldx%ld\n", TR_DIR, box_f[0],
           box_f[1], box_f[2], box_f[3], box_t[0], box_t[1], box_t[2], box_t[3]);
    wbgap_grab_point(box_f, from);
    wbgap_grab_point(box_t, to);
    from[0] += w->LeftEdge + w->BorderLeft;
    from[1] += w->TopEdge + w->BorderTop;
    to[0] += w->LeftEdge + w->BorderLeft;
    to[1] += w->TopEdge + w->BorderTop;
    snprintf(cmd, sizeof cmd, "%s DRAG %ld %ld %ld %ld STEPS 16", wbt_dragtest(), from[0], from[1], to[0], to[1]);
    printf("INFO I6 drag %ld,%ld to %ld,%ld\n", from[0], from[1], to[0], to[1]);
    wbt_run(cmd);
    for (t = 0; t < 30; t++) {   /* up to 3 s for it in the trash */
        df = tr_there(TR_CAN "/f");
        di = tr_there(TR_CAN "/f.info");
        if (df && di) break;
        Delay(5);
    }
    Delay(10);                   /* the move's other half (the source side) done */
    sf = tr_there(TR_DIR "/f");
    si = tr_there(TR_DIR "/f.info");
    df = tr_there(TR_CAN "/f");
    di = tr_there(TR_CAN "/f.info");
    ci = tr_there(TR_CAN ".info");
    printf("INFO I6 f: in %s %d/%d (file/icon), in the trashcan %d/%d; the trashcan's icon %d\n", TR_DIR, sf, si, df,
           di, ci);
    wbt_say("I6", wbgap_trash_judge(sf, si, df, di, ci, "f", why, sizeof why), why);
out:
    tr_clean();
    return wbt_fails() ? 5 : 0;
}
