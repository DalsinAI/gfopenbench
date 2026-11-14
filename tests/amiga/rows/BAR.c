/* ---- BAR: the system bar, B1 and B2 (GFOpenTitle's Power list, gfopenprefs TitleBar) and B3 (GFOpenSpeaker) ----
 * The row window GFOpenTitle keeps at the title bar's right end: its task's widest-reaching borderless window at the
 * top, no taller than the bar. Power's cell is its last (wbgap_power_point). The list is clicked open, looked at and
 * closed with Esc: nothing in it is ever picked (Reboot and Power off are in it).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <graphics/text.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "BAR.h"

WBGAP_MODE(BAR, wbt_bar, SCRIPT)
WBGAP_ROWMODE(B1, BAR)
WBGAP_ROWMODE(B2, BAR)
WBGAP_ROWMODE(B3, BAR)

#define BAR_MAXWINS 48

extern struct IntuitionBase *IntuitionBase;

static struct Window *bar_row(struct Screen *s, struct Task *t)
{
    struct Window *w, *best = NULL;
    ULONG lock = LockIBase(0);
    for (w = s->FirstWindow; w; w = w->NextWindow)
        if (w->UserPort && w->UserPort->mp_SigTask == t && (w->Flags & WFLG_BORDERLESS) && w->TopEdge == 0 &&
            w->Height <= s->BarHeight + 1 && (!best || w->LeftEdge + w->Width > best->LeftEdge + best->Width))
            best = w;
    UnlockIBase(lock);
    return best;
}

/* the active window, if it is t's and wasn't in was[0..n), within secs (GFOpenTitle's list opens active) */
static struct Window *new_active(struct Window **was, int n, struct Task *t, int secs)
{
    int k, j;
    for (k = 0; k < secs * 10; k++) {
        struct Window *a;
        ULONG lock = LockIBase(0);
        a = IntuitionBase->ActiveWindow;
        if (a && !(a->UserPort && a->UserPort->mp_SigTask == t)) a = NULL;
        UnlockIBase(lock);
        if (a) {
            for (j = 0; j < n && was[j] != a; j++) {}
            if (j == n) return a;
        }
        Delay(5);
    }
    return NULL;
}

/* the USB volumes mounted, as GFOpenTitle counts them for Power's Eject rows: DOS devices on gfopenusbacfs.device or
 * usbdisk.device with a handler running */
static int usb_sticks_now(void)
{
    struct DosList *dl = LockDosList(LDF_DEVICES | LDF_READ), *e = dl;
    int n = 0;
    while ((e = NextDosEntry(e, LDF_DEVICES)) != NULL) {
        struct DeviceNode *dn = (struct DeviceNode *)e;
        struct FileSysStartupMsg *fssm;
        const UBYTE *b;
        char dev[40];
        int k, len;
        /* dn_Startup of 1024 or less is a number, not an FSSM (the usual test, as Mount and Info make it): never
         * read through it, since a wrong guess here reads garbage while the DOS list is locked */
        if ((ULONG)dn->dn_Startup <= 1024 || !dn->dn_Task) continue;
        fssm = (struct FileSysStartupMsg *)BADDR(dn->dn_Startup);
        b = (const UBYTE *)BADDR(fssm->fssm_Device);
        if (!b) continue;
        len = (int)b[0] < (int)sizeof dev - 1 ? (int)b[0] : (int)sizeof dev - 1;
        for (k = 0; k < len; k++) dev[k] = (char)b[k + 1];
        dev[len] = 0;
        if (!strcmp(dev, "gfopenusbacfs.device") || !strcmp(dev, "usbdisk.device")) n++;
    }
    UnLockDosList(LDF_DEVICES | LDF_READ);
    return n;
}

static int bar_rows(void)
{
    struct Screen *s;
    struct MsgPort *p;
    struct Task *t;
    struct Window *row, *was[BAR_MAXWINS], *m;
    long xy[2], rr, ml, mt, mw, mh, rh;
    int n, k, owner, sticks, b1, b2, under, esc_left;
    char why[96];
    struct DrawInfo *dri;
    char cmd[320];
    Forbid();
    p = FindPort((CONST_STRPTR)"GFOpenTitle");
    t = p ? (struct Task *)p->mp_SigTask : NULL;
    k = FindPort((CONST_STRPTR)"GFOpenSpeaker") != NULL;
    Permit();
    /* B3: the speaker commodity started at boot (the 11 Nov bar design: nothing started GFOpenSpeaker, so the speaker
     * went missing). A bar with GFOpenTitle on it is GFOS's; without one (3.2.3) there is no speaker to want */
    if (k) wbt_pass("B3");
    else if (t) wbt_fail("B3", "GFOpenSpeaker isn't running (no GFOpenSpeaker port) though the bar is GFOpenTitle's");
    else wbt_skip("B3", "no GFOpenTitle bar (not GFOS)");
    if (!t) {
        wbt_skip("B1", "GFOpenTitle isn't running (no GFOpenTitle port)");
        wbt_skip("B2", "GFOpenTitle isn't running (no GFOpenTitle port)");
        return 0;
    }
    if (!(s = LockPubScreen((CONST_STRPTR)"Workbench"))) { wbt_skip("B1", "no Workbench screen"); wbt_skip("B2", "no Workbench screen"); return 0; }
    ScreenToFront(s);
    Delay(25);
    if (!(row = bar_row(s, t))) {
        wbt_fail("B1", "GFOpenTitle's row on the title bar not found");
        wbt_skip("B2", "no Power list to read");
        UnlockPubScreen(NULL, s);
        return 5;
    }
    wbgap_power_point(row->LeftEdge, row->TopEdge, row->Width, row->Height, xy);
    rr = row->LeftEdge + row->Width;
    /* a list row: the bar font's height + 4, as menu_open measures it */
    rh = (dri = GetScreenDrawInfo(s)) && dri->dri_Font ? dri->dri_Font->tf_YSize + 4 : 0;
    if (dri) FreeScreenDrawInfo(s, dri);
    sticks = usb_sticks_now();
    n = wbt_windows_now(s, was, BAR_MAXWINS);
    snprintf(cmd, sizeof cmd, "\"%s\" CLICK %ld %ld", wbt_dragtest(), xy[0], xy[1]);
    wbt_run(cmd);
    if (!(m = new_active(was, n, t, 2))) {
        wbt_fail("B1", "a click on Power opened no list");
        wbt_skip("B2", "no Power list to read");
        UnlockPubScreen(NULL, s);
        return 5;
    }
    ml = m->LeftEdge; mt = m->TopEdge; mw = m->Width; mh = m->Height;
    under = wbgap_point_in_box(xy[0], xy[1], ml, mt, mw, mh);
    printf("INFO B1 the row %d,%d %dx%d; Power clicked at %ld,%ld; a list %ld,%ld %ldx%ld; rows %ld high, %d USB volumes\n",
           row->LeftEdge, row->TopEdge, row->Width, row->Height, xy[0], xy[1], ml, mt, mw, mh, rh, sticks);
    owner = wbgap_bar_menu_owner(rr, ml, mw);
    /* Esc closes it before anything else: nothing in it is picked */
    snprintf(cmd, sizeof cmd, "%s KEY 0x45", wbt_dragtest());
    wbt_run(cmd);
    for (k = 0; k < 20 && wbt_still_there(s, m); k++) Delay(5);
    if ((esc_left = wbt_still_there(s, m))) {           /* a click on the screen's title, far from the list, closes it */
        snprintf(cmd, sizeof cmd, "\"%s\" CLICK 4 %d", wbt_dragtest(), row->TopEdge + row->Height / 2);
        wbt_run(cmd);
        for (k = 0; k < 20 && wbt_still_there(s, m); k++) Delay(5);
        printf("INFO B1 Esc left the list open; a click on the screen's title %s\n",
               wbt_still_there(s, m) ? "didn't close it either" : "closed it");
    }
    wbgap_power_judge(mh, sticks, rh, &b1, &b2);
    if (under) { wbt_fail("B1", "the list opened under the pointer (a mouse-up there could pick Reboot)"); b2 = WBGAP_NONE; }
    else if (esc_left) wbt_fail("B1", "Esc left the list open");   /* B2 still read: the height was taken before */
    else if (owner == WBGAP_BAR_PREFS) { wbt_fail("B1", "a click on Power's cell opened Prefs' list"); b2 = WBGAP_NONE; }
    else if (owner == WBGAP_BAR_OTHER) { wbt_fail("B1", "the window that opened doesn't hang from Power"); b2 = WBGAP_NONE; }
    else if (owner == WBGAP_BAR_UNSURE) { wbt_skip("B1", "the list was pushed to the screen's left edge: Power's or Prefs' can't be told"); b2 = WBGAP_NONE; }
    else {
        snprintf(why, sizeof why, "the list is %ld high, not %ld (Reboot, Restart, Power off, %d Eject rows)", mh,
                 wbgap_power_height(sticks, rh), sticks);
        wbt_say("B1", b1, why);
    }
    if (b2 == WBGAP_NONE) wbt_skip("B2", "Power's list not read");
    else {
        if (b2 == WBGAP_SKIP) snprintf(why, sizeof why, "no USB volume mounted");
        else snprintf(why, sizeof why, "%d USB volumes: the list is %ld high, not %ld with their Eject rows", sticks, mh,
                      wbgap_power_height(sticks, rh));
        wbt_say("B2", b2, why);
    }
    UnlockPubScreen(NULL, s);
    return wbt_fails() ? 5 : 0;
}

/* BAR brings the Workbench screen to the front; the screen that was in front goes back after */
int wbt_bar(int argc, char **argv)
{
    struct Screen *front;
    ULONG lock = LockIBase(0);
    int rc;
    (void)argc;
    (void)argv;
    front = IntuitionBase->FirstScreen;
    UnlockIBase(lock);
    rc = bar_rows();
    lock = LockIBase(0);
    if (front && IntuitionBase->FirstScreen != front) {
        struct Screen *x;
        for (x = IntuitionBase->FirstScreen; x && x != front; x = x->NextScreen) ;
        UnlockIBase(lock);
        if (x) ScreenToFront(front);              /* only if it is still open */
    } else UnlockIBase(lock);
    return rc;
}

