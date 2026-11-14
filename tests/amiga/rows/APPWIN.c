/* ---- A1, A2: AddAppWindowA and its messages, AddAppWindowDropZoneA ----
 * APPDROP's drawer (RAM:WBGapD, its project d found in its window) and a window of our own clear of it. d is dragged
 * with DragTest onto our window:
 *   A1: made an AppWindow (id 1, userdata $A1): one AppMessage, AMTYPE_APPWINDOW with that id and userdata, and
 *       am_MouseX/Y where d was let go; then RemoveAppWindow, and the same drag sends nothing.
 *   A2: made an AppWindow (id 2) with one drop zone, the window's right half (id 22, userdata $A22): a drop in the
 *       zone is AMTYPE_APPWINDOWZONE with the zone's id and userdata; a drop in the left half sends nothing (with
 *       drop zones, only drops in a zone count: OS 3.2.3's autodoc, and GFOpenBench's OWBCTRLA_AppWindowAt).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "APPWIN.h"

WBGAP_MODE(APPWIN, wbt_appwin, NEWROWS)
WBGAP_ROWMODE(A1, APPWIN)
WBGAP_ROWMODE(A2, APPWIN)

struct appwin_got {
    int got;
    long type, mx, my;
    unsigned long id, userdata;
};

/* the first AppMessage at port within ticks/50 s (each one replied), into *g */
static void appwin_wait(struct MsgPort *port, int ticks, struct appwin_got *g, const char *row, const char *what)
{
    struct AppMessage *am;
    int t;
    memset(g, 0, sizeof *g);
    for (t = 0; t < ticks; t += 5) {
        while ((am = (struct AppMessage *)GetMsg(port))) {
            if (!g->got) {
                g->got = 1;
                g->type = am->am_Type;
                g->id = am->am_ID;
                g->userdata = am->am_UserData;
                g->mx = am->am_MouseX;
                g->my = am->am_MouseY;
            }
            ReplyMsg((struct Message *)am);
        }
        if (g->got) break;
        Delay(5);
    }
    if (g->got)
        printf("INFO %s %s: type %ld id %lu userdata $%lx at %ld,%ld\n", row, what, g->type, g->id, g->userdata, g->mx, g->my);
    else printf("INFO %s %s: no AppMessage\n", row, what);
}

/* every message still at port replied before a drop that must send nothing, so one that came late from the drop
 * before isn't counted against it (Budgie, #168); how many there were is said */
static void appwin_drain(struct MsgPort *port, const char *row)
{
    struct Message *m;
    int n = 0;
    Delay(25);
    while ((m = GetMsg(port))) { ReplyMsg(m); n++; }
    if (n) printf("INFO %s %d late AppMessage(s) from the drop before, replied\n", row, n);
}

/* d dragged from RAM:WBGapD's window, looked up again for each drag (the desktop may have moved or reopened it), to
 * (rx, ry) inside our window w; the point let go at, relative to w, in rel. 0, and no drag, if the window is gone */
static int appwin_drop(struct Window *w, long rx, long ry, long rel[2], const char *row)
{
    long from[2], to[2];
    struct Window *dw = wbt_find_window("WBGapD");
    if (!dw) {
        printf("INFO %s RAM:WBGapD's window gone before a drag\n", row);
        return 0;
    }
    wbt_drop_from(dw, from);
    to[0] = w->LeftEdge + rx;
    to[1] = w->TopEdge + ry;
    rel[0] = rx;
    rel[1] = ry;
    wbt_drop_drag(from, to, row);
    return 1;
}

int wbt_appwin(int argc, char **argv)
{
    struct MsgPort *port = CreateMsgPort();
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
    struct Window *dw, *mine = NULL;
    struct AppWindow *aw;
    struct AppWindowDropZone *zone;
    struct appwin_got g;
    long box[4], dbox[4], rel[2], w, h;
    char why1[160], why2[160], why[160];
    int stage, v1, v2, ok;
    (void)argc, (void)argv;
    if (!port || !wb) {
        wbt_fail("A1", "setup (no port or no Workbench screen)");
        wbt_fail("A2", "setup (no port or no Workbench screen)");
        goto out;
    }
    if (!(dw = wbt_drop_ready(&stage))) {
        snprintf(why, sizeof why, "%s", stage == 1 ? "RAM:WBGapD's window didn't open" : "no icon d to drag in RAM:WBGapD");
        wbt_fail("A1", why);
        wbt_fail("A2", why);
        goto out;
    }
    dbox[0] = dw->LeftEdge; dbox[1] = dw->TopEdge; dbox[2] = dw->Width; dbox[3] = dw->Height;
    if (wbgap_drop_place(wb->Width, wb->Height, wb->BarHeight + 1, 240, 120, dbox, box))
        mine = OpenWindowTags(NULL, WA_PubScreen, (ULONG)wb, WA_Left, box[0], WA_Top, box[1], WA_Width, box[2],
                              WA_Height, box[3], WA_Title, (ULONG)"Drop target A1 A2", WA_DragBar, TRUE,
                              WA_DepthGadget, TRUE, WA_Activate, FALSE, TAG_DONE);
    if (!mine) {
        wbt_fail("A1", "no room for a window of our own clear of RAM:WBGapD's, or it didn't open");
        wbt_fail("A2", "no room for a window of our own clear of RAM:WBGapD's, or it didn't open");
        goto out;
    }
    w = mine->Width;
    h = mine->Height;

    /* A1: the AppWindow's own message, then none once it is removed */
    if (!(aw = AddAppWindowA(1, 0xA1, mine, port, NULL))) {
        snprintf(why, sizeof why, "AddAppWindowA NULL (IoErr %ld)", (long)IoErr());
        wbt_fail("A1", why);
    } else {
        Delay(25);
        ok = appwin_drop(mine, w / 2, h / 2 + 10, rel, "A1");
        appwin_wait(port, 150, &g, "A1", "onto the AppWindow");
        v1 = wbgap_appwin_msg_judge(g.got, g.type, AMTYPE_APPWINDOW, g.id, 1, g.userdata, 0xA1, g.mx, g.my, rel, w, h,
                                    why1, sizeof why1);
        RemoveAppWindow(aw);
        appwin_drain(port, "A1");
        ok = appwin_drop(mine, w / 2, h / 2 + 10, rel, "A1") && ok;
        appwin_wait(port, 100, &g, "A1", "after RemoveAppWindow");
        v2 = wbgap_appwin_none_judge(g.got, g.type, g.id, "after RemoveAppWindow", why2, sizeof why2);
        wbt_say("A1", wbgap_appwin_row(ok, v1, why1, v2, why2, why, sizeof why), why);
    }

    /* A2: a drop zone, the window's right half; a drop there, then one in the left half */
    if (!(aw = AddAppWindowA(2, 0xA2, mine, port, NULL))) {
        snprintf(why, sizeof why, "AddAppWindowA NULL (IoErr %ld)", (long)IoErr());
        wbt_fail("A2", why);
    } else {
        struct TagItem zt[5];
        zt[0].ti_Tag = WBDZA_Left;   zt[0].ti_Data = (ULONG)(w / 2);
        zt[1].ti_Tag = WBDZA_Top;    zt[1].ti_Data = 0;
        zt[2].ti_Tag = WBDZA_Width;  zt[2].ti_Data = (ULONG)(w - w / 2);
        zt[3].ti_Tag = WBDZA_Height; zt[3].ti_Data = (ULONG)h;
        zt[4].ti_Tag = TAG_END;      zt[4].ti_Data = 0;
        zone = AddAppWindowDropZoneA(aw, 22, 0xA22, zt);
        if (!zone) {
            snprintf(why, sizeof why, "AddAppWindowDropZoneA NULL (IoErr %ld)", (long)IoErr());
            wbt_fail("A2", why);
        } else {
            Delay(25);
            ok = appwin_drop(mine, (w * 3) / 4, h / 2 + 10, rel, "A2");
            appwin_wait(port, 150, &g, "A2", "into the drop zone");
            v1 = wbgap_appwin_msg_judge(g.got, g.type, AMTYPE_APPWINDOWZONE, g.id, 22, g.userdata, 0xA22, g.mx, g.my,
                                        rel, w, h, why1, sizeof why1);
            appwin_drain(port, "A2");
            ok = appwin_drop(mine, w / 4, h / 2 + 10, rel, "A2") && ok;
            appwin_wait(port, 100, &g, "A2", "outside the drop zone");
            v2 = wbgap_appwin_none_judge(g.got, g.type, g.id, "a drop outside the drop zone", why2, sizeof why2);
            wbt_say("A2", wbgap_appwin_row(ok, v1, why1, v2, why2, why, sizeof why), why);
            RemoveAppWindowDropZone(aw, zone);
        }
        RemoveAppWindow(aw);
    }
out:
    if (mine) CloseWindow(mine);
    wbt_drop_clean();
    if (wb) UnlockPubScreen(NULL, wb);
    if (port) {
        struct Message *m;
        while ((m = GetMsg(port))) ReplyMsg(m);
        DeleteMsgPort(port);
    }
    return wbt_fails() ? 5 : 0;
}
