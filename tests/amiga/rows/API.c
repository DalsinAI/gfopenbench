/* ---- the API mode: workbench.library's calls asked by a program (rows A3, A4, A6, A6b, A7, A8a-c, A9, M27, N8) ----
 * On RAM:WBGap (WBGapTest SETUP's): AddAppIconA (A3); AddAppMenuItemA (A4), its Tools item shown within 2 s and gone
 * within 2 s of RemoveAppMenuItem (M27); OpenWorkbenchObjectA (A6); UpdateWorkbench after a new file and icon, the
 * drawer's window changed 100 ms later and IoErr left at 0 (A7, wbgap_api_update_judge); WBInfo, its window come
 * (A9, N8); MakeWorkbenchObjectVisibleA (A8a); ChangeWorkbenchSelectionA, its hook called (A8b);
 * WhichWorkbenchObjectA on RAM:WBGapW's icon w and on an empty point (A8c); CloseWorkbenchObjectA (A6b).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <utility/hooks.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/icon.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "API.h"
#if defined(WBO_NONE) && defined(WBO_DRAWER) && defined(WBO_ICON)
typedef char wbgap_api_wbo_same[(WBO_NONE == WBGAP_WBO_NONE && WBO_DRAWER == WBGAP_WBO_DRAWER && WBO_ICON == WBGAP_WBO_ICON) ? 1 : -1];
#endif

extern struct Library *WorkbenchBase, *IconBase;   /* WBGapTest.c's */

WBGAP_MODE(API, wbt_api, SCRIPT)
WBGAP_ROWMODE(M27, API)
WBGAP_ROWMODE(N8, API)
WBGAP_ROWMODE(A3, API)
WBGAP_ROWMODE(A4, API)
WBGAP_ROWMODE(A6, API)
WBGAP_ROWMODE(A6b, API)
WBGAP_ROWMODE(A7, API)
WBGAP_ROWMODE(A8a, API)
WBGAP_ROWMODE(A8b, API)
WBGAP_ROWMODE(A8c, API)
WBGAP_ROWMODE(A9, API)

static LONG sel_hook_fn(struct Hook *h __asm("a0"), APTR reserved __asm("a2"), struct IconSelectMsg *ism __asm("a1"))
{
    (void)reserved; (void)ism;
    (*(LONG *)h->h_Data)++;
    return ISMACTION_Ignore;
}

/* A8c: where ChangeWorkbenchSelectionA says the icon named in want[] has its picture (relative to the window) */
struct which_find { const char *want; long x, y; int found; };
static LONG which_hook_fn(struct Hook *h __asm("a0"), APTR reserved __asm("a2"), struct IconSelectMsg *ism __asm("a1"))
{
    struct which_find *f = (struct which_find *)h->h_Data;
    (void)reserved;
    if (ism->ism_Name && !strcmp((const char *)ism->ism_Name, f->want)) {
        f->x = ism->ism_Left;
        f->y = ism->ism_Top;
        f->found = 1;
    }
    return ISMACTION_Ignore;
}

/* "FAIL <id> why" for each row of mode whose id starts with letter: the mode's checks didn't run. Named row by
 * row, not "FAIL A*", whose mode is the first A row's that has one, and A1 (APPWIN, rows/APPWIN.c) is that now */
static void fail_mode_rows(const char *mode, char letter, const char *why)
{
    int i;
    for (i = 0; i < wbgap_row_count(); i++) {
        const struct wbgap_row *r = wbgap_row(i);
        if (r->mode && !strcmp(r->mode, mode) && r->id[0] == letter) wbt_fail(r->id, why);
    }
}

int wbt_api(int argc, char **argv)
{
    struct MsgPort *port = CreateMsgPort();
    struct DiskObject *dob = GetDefDiskObject(WBTOOL);
    struct AppIcon *ai;
    struct AppMenuItem *mi;
    struct Hook hook;
    LONG seen = 0;
    BOOL ok;
    char msg[160];
    (void)argc;
    (void)argv;
    wbt_setup();
    if (!port || !dob) { fail_mode_rows("API", 'A', "setup"); return 20; }
    ai = AddAppIconA(1, 0, (STRPTR)"WBGapIcon", port, 0, dob, NULL);
    if (ai) { wbt_pass("A3"); RemoveAppIcon(ai); } else snprintf(msg, sizeof msg, "AddAppIconA NULL (IoErr %ld)", (long)IoErr()), wbt_fail("A3", msg);
    mi = AddAppMenuItemA(2, 0, (STRPTR)"WBGapItem", port, NULL);
    if (mi) {
        wbt_pass("A4");
        /* M27: the item shows in the Tools menu of the Workbench's windows within 2 s, and goes when removed */
        if (wbt_tools_item_within("wbgapitem", 1, 100)) {
            RemoveAppMenuItem(mi);
            if (wbt_tools_item_within("wbgapitem", 0, 100)) wbt_pass("M27"); else wbt_fail("M27", "Tools/WBGapItem still shown 2 s after RemoveAppMenuItem");
        } else { RemoveAppMenuItem(mi); wbt_fail("M27", "Tools/WBGapItem not in the Workbench windows' menus 2 s after AddAppMenuItemA"); }
    } else {
        snprintf(msg, sizeof msg, "AddAppMenuItemA NULL (IoErr %ld)", (long)IoErr());
        wbt_fail("A4", msg);
        wbt_fail("M27", "no AppMenuItem (A4)");
    }
    ok = OpenWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) wbt_pass("A6"); else snprintf(msg, sizeof msg, "OpenWorkbenchObjectA FALSE (IoErr %ld)", (long)IoErr()), wbt_fail("A6", msg);
    Delay(50);
    {   /* UpdateWorkbench (A7): a file and icon made, then UpdateWorkbench; the drawer's window must show it at once,
         * sooner than the 200 ms GFOpenBench's own notify waits, and IoErr stays as it was */
        BPTR dir;
        struct Window *w = NULL;
        ULONG before = 0;
        LONG e;
        int t, changed = 0;
        DeleteFile((CONST_STRPTR)"RAM:WBGap/new2");
        DeleteFile((CONST_STRPTR)"RAM:WBGap/new2.info");
        for (t = 0; t < 30 && !(w = wbt_find_window("WBGap")); t++) Delay(10);
        if (w) { Delay(100); before = wbt_inside_sum(w); }
        wbt_write_file("RAM:WBGap/new2", "new\n");
        wbt_put_icon("RAM:WBGap/new2", WBPROJECT, NULL);
        dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
        SetIoErr(0);
        UpdateWorkbench((CONST_STRPTR)"new2", dir, UPDATEWB_ObjectAdded);
        printf("INFO UpdateWorkbench IoErr %ld\n", (long)IoErr());
        e = IoErr();
        if (!e && w) {
            Delay(5);
            changed = wbt_inside_sum(w) != before;
        }
        wbt_say("A7", wbgap_api_update_judge(e, w != NULL, changed, msg, sizeof msg), msg);
        if (dir) UnLock(dir);
    }
    {   /* WBInfo (A9; N8: a program's WBInfo opens it): TRUE, and its window comes; then closed (Ctrl-C) */
        BPTR dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
        struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
        struct Window *iw = NULL;
        ULONG r;
        int t;
        r = dir ? WBInfo(dir, (CONST_STRPTR)"a", wb) : 0;
        for (t = 0; r && t < 30 && !(iw = wbt_find_window("Information: a")); t++) Delay(10);
        if (r && iw) { wbt_pass("A9"); wbt_pass("N8"); }
        else {
            snprintf(msg, sizeof msg, "WBInfo %s, %s (IoErr %ld)", r ? "TRUE" : "FALSE", iw ? "window open" : "no window", (long)IoErr()), wbt_fail("A9", msg);
            wbt_fail("N8", "WBInfo from a program opened no Information window");
        }
        if (iw && iw->UserPort && iw->UserPort->mp_SigTask) Signal((struct Task *)iw->UserPort->mp_SigTask, SIGBREAKF_CTRL_C);
        if (wb) UnlockPubScreen(NULL, wb);
        if (dir) UnLock(dir);
    }
    ok = MakeWorkbenchObjectVisibleA((STRPTR)"RAM:WBGap/a", NULL);
    if (ok) wbt_pass("A8a"); else snprintf(msg, sizeof msg, "MakeWorkbenchObjectVisibleA FALSE (IoErr %ld)", (long)IoErr()), wbt_fail("A8a", msg);
    memset(&hook, 0, sizeof hook);
    hook.h_Entry = (HOOKFUNC)sel_hook_fn;
    hook.h_Data = &seen;
    ok = ChangeWorkbenchSelectionA((STRPTR)"RAM:WBGap", &hook, NULL);
    if (ok && seen > 0) wbt_pass("A8b"); else snprintf(msg, sizeof msg, "ChangeWorkbenchSelectionA %s, %ld icons seen (IoErr %ld)", ok ? "TRUE" : "FALSE", (long)seen, (long)IoErr()), wbt_fail("A8b", msg);
    {   /* A8c, WhichWorkbenchObjectA: RAM:WBGapW's icon w, asked about on its picture and on an empty point of the
         * drawer's inside (WBO_ICON named "w", then WBO_DRAWER). Where the picture is comes from the Workbench itself
         * (ChangeWorkbenchSelectionA's ism_Left, ism_Top): a drawer with a bar along its top (GFOpenBench's) or a
         * fitted layout doesn't put it at the snapshot's 20,10 from the borders. With no answer there, 20,10. */
#ifdef WBOBJA_Name
        BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapW");
        struct DiskObject *d;
        struct Window *ww;
        int how;
        if (l) UnLock(l);
        wbt_put_icon("RAM:WBGapW", WBDRAWER, NULL);
        wbt_write_file("RAM:WBGapW/w", "w\n");
        if ((d = GetDefDiskObject(WBPROJECT))) {
            d->do_CurrentX = 20;
            d->do_CurrentY = 10;
            PutDiskObject((CONST_STRPTR)"RAM:WBGapW/w", d);
            FreeDiskObject(d);
        }
        if (!(ww = wbt_open_drawer("RAM:WBGapW", "WBGapW", 10, &how))) wbt_fail("A8c", "RAM:WBGapW's window didn't open");
        else {
            /* the tags start at WBGAP_WHICH_UNSET, so the INFO lines say whether the desktop's answer reached them */
            char name[64] = "?", why[160], seen[96];
            ULONG on_icon = WBO_NONE, on_empty = WBO_NONE, on_desk = WBO_NONE;
            ULONG t1 = WBGAP_WHICH_UNSET, t2 = WBGAP_WHICH_UNSET, t3 = WBGAP_WHICH_UNSET;
            LONG e1, e2, e3 = 0;
            long xy[2];
            struct Window *desk = NULL, *o;
            struct Task *wt = ww->UserPort ? ww->UserPort->mp_SigTask : NULL;
            struct TagItem tg1[] = {{WBOBJA_Type, (ULONG)&t1}, {WBOBJA_Name, (ULONG)name}, {WBOBJA_NameSize, sizeof name},
                                    {TAG_DONE, 0}};
            struct TagItem tg2[] = {{WBOBJA_Type, (ULONG)&t2}, {TAG_DONE, 0}};
            struct TagItem tg3[] = {{WBOBJA_Type, (ULONG)&t3}, {TAG_DONE, 0}};
            struct which_find wf;
            struct Hook wh;
            long ex[2];
            Delay(25);
            memset(&wf, 0, sizeof wf);
            wf.want = "w";
            memset(&wh, 0, sizeof wh);
            wh.h_Entry = (HOOKFUNC)which_hook_fn;
            wh.h_Data = &wf;
            ChangeWorkbenchSelectionA((STRPTR)"RAM:WBGapW", &wh, NULL);
            if (wf.found) { xy[0] = wf.x + 4; xy[1] = wf.y + 4; }
            else wbgap_which_point(ww->BorderLeft, ww->BorderTop, 20, 10, xy);
            if (!wbgap_which_empty(ww->Width, ww->Height, ww->BorderLeft, ww->BorderTop, ww->BorderRight, ww->BorderBottom,
                                   xy[0] - 4, xy[1] - 4, ex)) { ex[0] = ww->Width / 2; ex[1] = ww->Height / 2; }
            printf("INFO A8c w's picture %s at %ld,%ld; the empty point %ld,%ld\n", wf.found ? "(the Workbench says)" : "(not said: 20,10)",
                   xy[0] - 4, xy[1] - 4, ex[0], ex[1]);
            SetIoErr(0);
            on_icon = WhichWorkbenchObjectA(ww, xy[0], xy[1], tg1);
            e1 = IoErr();
            /* stock OS 3.x gives ism_Left/ism_Top in the drawer's view (from its inside's top-left, not scrolled in a
             * drawer just opened); GFOpenBench gives the window's. Not the icon at the first: the view's, asked once */
            if (wf.found && on_icon != WBO_ICON) {
                long vp[2], vx, vy;
                ULONG again, t1_first = t1;
                LONG e_again;
                char name_first[64];
                wbgap_which_view_point(ww->BorderLeft, ww->BorderTop, wf.x, wf.y, vp);
                vx = vp[0];
                vy = vp[1];
                memcpy(name_first, name, sizeof name_first);
                t1 = WBGAP_WHICH_UNSET;
                strcpy(name, "?");
                SetIoErr(0);
                again = WhichWorkbenchObjectA(ww, vx, vy, tg1);
                e_again = IoErr();
                printf("INFO A8c the window's place gave %lu; the view's (%ld,%ld) gives %lu (type %lu, IoErr %ld)\n",
                       (unsigned long)on_icon, vx, vy, (unsigned long)again, (unsigned long)t1, (long)e_again);
                if (again == WBO_ICON) { on_icon = again; xy[0] = vx; xy[1] = vy; e1 = e_again; }
                else {
                    /* missed again: the A8c line and wbgap_which_seen tell of the first ask, as on_icon does */
                    t1 = t1_first;
                    memcpy(name, name_first, sizeof name_first);
                }
            }
            SetIoErr(0);
            on_empty = WhichWorkbenchObjectA(ww, ex[0], ex[1], tg2);
            e2 = IoErr();
            printf("INFO A8c window \"%s\" at %d,%d %dx%d, borders %d,%d,%d,%d, task \"%s\"\n",
                   ww->Title ? (const char *)ww->Title : "", ww->LeftEdge, ww->TopEdge, ww->Width, ww->Height, ww->BorderLeft,
                   ww->BorderTop, ww->BorderRight, ww->BorderBottom, wt && wt->tc_Node.ln_Name ? wt->tc_Node.ln_Name : "?");
            printf("INFO A8c on the icon at %ld,%ld: %lu (type %lu, \"%s\", IoErr %ld); on the empty point: %lu (type %lu, IoErr %ld)\n",
                   xy[0], xy[1], (unsigned long)on_icon, (unsigned long)t1, name, (long)e1, (unsigned long)on_empty,
                   (unsigned long)t2, (long)e2);
            /* the desktop's own window (backdrop, a Workbench window), asked about its bottom-right corner: INFO
             * only, it tells a desktop that answers nothing from one that doesn't know this drawer's window */
            {   /* the largest backdrop Workbench window (wbgap_pick_desk): on GFOpenBench the bar strip comes first */
                struct Window *wl[32];
                long dw[32], dh[32];
                unsigned long df[32];
                int nw = 0, k;
                ULONG lk = LockIBase(0);
                for (o = ww->WScreen ? ww->WScreen->FirstWindow : NULL; o && nw < 32; o = o->NextWindow) {
                    wl[nw] = o;
                    dw[nw] = o->Width;
                    dh[nw] = o->Height;
                    df[nw++] = o->Flags;
                }
                if ((k = wbgap_pick_desk(dw, dh, df, nw)) >= 0) desk = wl[k];
                UnlockIBase(lk);
            }
            if (desk) {
                SetIoErr(0);
                on_desk = WhichWorkbenchObjectA(desk, desk->Width - 4, desk->Height - 4, tg3);
                e3 = IoErr();
                printf("INFO A8c the desktop's corner: %lu (type %lu, IoErr %ld)\n", (unsigned long)on_desk,
                       (unsigned long)t3, (long)e3);
            } else
                printf("INFO A8c no desktop window (backdrop, Workbench) on the drawer's screen\n");
            wbgap_which_seen(t1, t2, desk ? (long)t3 : -1L, seen, sizeof seen);
            if (wbgap_which_judge(on_icon, name, "w", on_empty, why, sizeof why) == WBGAP_PASS) wbt_pass("A8c");
            else {
                size_t k = strlen(why);
                snprintf(why + k, sizeof why - k, "; %s", seen);
                wbt_fail("A8c", why);
            }
            wbt_clear_drawer("RAM:WBGapW", "WBGapW", "A8c");   /* closed, else parked off the desktop rows */
        }
        DeleteFile((CONST_STRPTR)"RAM:WBGapW/w.info");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW/w");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW.info");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW");
#else
        wbt_skip("A8c", "built without workbench.h's WBOBJA_ tags");
#endif
    }
    ok = CloseWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) wbt_pass("A6b"); else snprintf(msg, sizeof msg, "CloseWorkbenchObjectA FALSE (IoErr %ld)", (long)IoErr()), wbt_fail("A6b", msg);
    FreeDiskObject(dob);
    DeleteMsgPort(port);
    return wbt_fails() ? 5 : 0;
}

