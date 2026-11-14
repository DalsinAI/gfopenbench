/* ---- I8, I9: an icon dragged onto an AppIcon and onto an AppWindow ----
 * RAM:WBGapD's project d, made while its drawer's window is open (a already in it) and found where the window
 * changed, is dragged with DragTest onto a window of
 * our own made an AppWindow (I9, id 9), then onto an AppIcon on the desktop (I8, id 8; its place read through the
 * WORKBENCH port by WBGapAppPos.rexx). The desktop must send the AppMessage, naming d in RAM:WBGapD.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "APPDROP.h"

WBGAP_MODE(APPDROP, wbt_appdrop, SCRIPT)
WBGAP_ROWMODE(I8, APPDROP)
WBGAP_ROWMODE(I9, APPDROP)

/* the first AppMessage at port within 3 s (each one replied), judged for row: d in RAM:WBGapD, of want_type and want_id */
static int appdrop_wait(struct MsgPort *port, long want_type, unsigned long want_id, const char *row)
{
    char why[160], name[64] = "", dir[160] = "";
    long type = 0, nargs = 0;
    unsigned long id = 0;
    int got = 0, t;
    struct AppMessage *am;
    for (t = 0; t < 60 && !got; t++) {   /* 3 s */
        while ((am = (struct AppMessage *)GetMsg(port))) {
            if (!got) {
                got = 1;
                type = am->am_Type;
                id = am->am_ID;
                nargs = am->am_NumArgs;
                if (nargs > 0 && am->am_ArgList) {
                    if (am->am_ArgList[0].wa_Name) snprintf(name, sizeof name, "%s", (const char *)am->am_ArgList[0].wa_Name);
                    if (am->am_ArgList[0].wa_Lock) NameFromLock(am->am_ArgList[0].wa_Lock, (STRPTR)dir, sizeof dir);
                }
            }
            ReplyMsg((struct Message *)am);
        }
        if (!got) Delay(5);
    }
    printf("INFO %s AppMessage: %s type %ld id %lu, %ld args, \"%s\" in \"%s\"\n", row, got ? "came," : "none,", type, id, nargs,
           name, dir);
    if (wbgap_drop_judge(got, type, want_type, id, want_id, nargs, name, dir, "d", "WBGapD", why, sizeof why) == WBGAP_PASS)
        wbt_pass(row);
    else wbt_fail(row, why);
    return got;
}

/* RAM:WBGapD's window moved to a corner clear of avoid; 1 if it is there within 2 s */
static int appdrop_move(struct Window **w, const long *avoid)
{
    long box[4];
    int t;
    struct Window *x = *w;
    if (!wbgap_drop_place(x->WScreen->Width, x->WScreen->Height, x->WScreen->BarHeight + 1, x->Width, x->Height, avoid, box))
        return 0;
    ChangeWindowBox(x, box[0], box[1], box[2], box[3]);
    for (t = 0; t < 40; t++) {
        Delay(5);
        if (!(x = wbt_find_window("WBGapD"))) return 0;
        if (x->LeftEdge == box[0] && x->TopEdge == box[1]) { *w = x; return 1; }
    }
    return 0;
}

int wbt_appdrop(int argc, char **argv)
{
    struct MsgPort *port = CreateMsgPort();
    struct DiskObject *aio = GetDefDiskObject(WBTOOL);
    struct Window *dw, *mine = NULL;
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
    int stage;
    long from[2], to[2], box[4];
    char cmd[320], text[80];
    if (!port || !aio || !wb) {
        wbt_fail("I9", "setup");
        wbt_fail("I8", "setup");
        goto out;
    }
    aio->do_CurrentX = aio->do_CurrentY = NO_ICON_POSITION;   /* the desktop places the AppIcon */
    if (!(dw = wbt_drop_ready(&stage))) {
        wbt_fail("I9", stage == 1 ? "RAM:WBGapD's window didn't open"
                              : "d's icon never showed in RAM:WBGapD's window (5 s after UpdateWorkbench)");
        wbt_fail("I8", stage == 1 ? "RAM:WBGapD's window didn't open" : "no icon d to drag");
        goto out;
    }
    {   /* I9: our window, in a corner clear of the drawer's, made an AppWindow */
        long dbox[4];
        struct AppWindow *aw = NULL;
        dbox[0] = dw->LeftEdge; dbox[1] = dw->TopEdge; dbox[2] = dw->Width; dbox[3] = dw->Height;
        if (wbgap_drop_place(wb->Width, wb->Height, wb->BarHeight + 1, 240, 120, dbox, box))
            mine = OpenWindowTags(NULL, WA_PubScreen, (ULONG)wb, WA_Left, box[0], WA_Top, box[1], WA_Width, box[2],
                                  WA_Height, box[3], WA_Title, (ULONG)"Drop target I9", WA_DragBar, TRUE,
                                  WA_DepthGadget, TRUE, WA_Activate, FALSE, TAG_DONE);
        if (!mine) wbt_fail("I9", "no room for a window of our own clear of RAM:WBGapD's, or it didn't open");
        else if (!(aw = AddAppWindowA(9, 0, mine, port, NULL))) {
            snprintf(text, sizeof text, "AddAppWindowA NULL (IoErr %ld)", (long)IoErr());
            wbt_fail("I9", text);
        } else {
            Delay(25);
            wbt_drop_from(dw, from);
            wbgap_drop_point(box, to);
            wbt_drop_drag(from, to, "I9");
            appdrop_wait(port, AMTYPE_APPWINDOW, 9, "I9");
        }
        if (aw) RemoveAppWindow(aw);
        if (mine) CloseWindow(mine);
        mine = NULL;
    }
    {   /* I8: an AppIcon on the desktop, found through the WORKBENCH port, and the drawer's window moved off it */
        struct AppIcon *ai;
        if (!wbt_exists("SYS:Rexxc/rx") || !FindPort((CONST_STRPTR)"WORKBENCH")) {
            wbt_skip("I8", "no rx or no WORKBENCH port to find the AppIcon by");
            goto out;
        }
        if (!(ai = AddAppIconA(8, 0, (STRPTR)"WBGapDrop", port, 0, aio, NULL))) {
            snprintf(text, sizeof text, "AddAppIconA NULL (IoErr %ld)", (long)IoErr());
            wbt_fail("I8", text);
            goto out;
        }
        Delay(50);
        snprintf(cmd, sizeof cmd, "SYS:Rexxc/rx SYS:T/WBGap/WBGapAppPos.rexx WBGapDrop");
        wbt_run(cmd);
        if (wbt_read_all("RAM:wbgap-appicon", text, sizeof text) < 0 || !wbgap_parse_box(text, box))
            wbt_skip("I8", "the AppIcon's place not read (WBGapAppPos.rexx)");
        else if (!(dw = wbt_find_window("WBGapD")) || !appdrop_move(&dw, box))
            wbt_fail("I8", "RAM:WBGapD's window couldn't be moved clear of the AppIcon");
        else {
            Delay(25);
            wbt_drop_from(dw, from);
            wbgap_drop_point(box, to);
            wbt_drop_drag(from, to, "I8");
            appdrop_wait(port, AMTYPE_APPICON, 8, "I8");
        }
        RemoveAppIcon(ai);
    }
out:
    wbt_drop_clean();
    if (port) {
        struct Message *m;
        while ((m = GetMsg(port))) ReplyMsg(m);
        DeleteMsgPort(port);
    }
    if (aio) FreeDiskObject(aio);
    if (wb) UnlockPubScreen(NULL, wb);
    return wbt_fails() ? 5 : 0;
}
