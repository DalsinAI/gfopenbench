/* ---- W2: keyboard window cycling (GFOpenWindows' switcher, gfopenprefs Windows) ----
 * The Workbench screen's windows front to back by their layers, as the switcher counts them (wbgap_switch_offers:
 * a window with a system gadget, not a backdrop, tiny, or Workbench's root; tray buttons and panels don't take part).
 * Two windows of our own make sure there are some to cycle.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layers.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "SWITCH.h"

WBGAP_MODE(SWITCH, wbt_switch, SCRIPT)
WBGAP_ROWMODE(W2, SWITCH)

extern struct IntuitionBase *IntuitionBase;
extern struct Library *LayersBase;   /* WBGapTest.c's; opened here for the row and closed again */

#define MAXWINS 48

static int switch_order(struct Screen *s, unsigned long *ids, int cap)
{
    struct Layer *l;
    int n = 0;
    LockLayerInfo(&s->LayerInfo);   /* the layer list whole: a depth change half done holds this lock */
    for (l = s->LayerInfo.top_layer; l && n < cap; l = l->back) {
        struct Window *w = (struct Window *)l->Window;
        if (!w || !wbgap_switch_offers((unsigned long)w->Flags, w->Width, w->Height)) continue;
        ids[n++] = (unsigned long)w;
    }
    UnlockLayerInfo(&s->LayerInfo);
    return n;
}

/* A window that came or went across the key: its title, so a FAIL says whose it was. The address is looked
 * for among the screen's windows under LockIBase, so one that went is only named as gone. */
static void switch_name_change(struct Screen *s, const unsigned long *before, int nb, const unsigned long *after,
                               int na, const char *row)
{
    int i, j;
    for (i = 0; i < na; i++) {
        struct Window *w;
        char title[64];
        int ww = 0, wh = 0, wx = 0, wy = 0;
        unsigned long fl = 0;
        ULONG lock;
        for (j = 0; j < nb && before[j] != after[i]; j++) ;
        if (j < nb) continue;
        title[0] = 0;
        lock = LockIBase(0);        /* copied out under the lock; printed after it (no DOS under LockIBase) */
        for (w = s->FirstWindow; w && (unsigned long)w != after[i]; w = w->NextWindow) ;
        if (w) {
            if (w->Title) snprintf(title, sizeof title, "%s", (const char *)w->Title);
            ww = w->Width; wh = w->Height; wx = w->LeftEdge; wy = w->TopEdge; fl = w->Flags;
        }
        UnlockIBase(lock);
        printf("INFO %s came: \"%s\" %dx%d at %d,%d, flags $%08lx\n", row, title, ww, wh, wx, wy, fl);
    }
    for (i = 0; i < nb; i++) {
        for (j = 0; j < na && after[j] != before[i]; j++) ;
        if (j == na) printf("INFO %s went: the window at %d from the front\n", row, i);
    }
}

/* The windows of an order by title, front first, for a FAIL: copied out under LockIBase, printed after it */
static void switch_name(struct Screen *s, const unsigned long *ids, int n, const char *row, const char *what)
{
    static char line[768];
    unsigned len = 0;
    int i;
    ULONG lock = LockIBase(0);
    line[0] = 0;
    for (i = 0; i < n && len + 64 < sizeof line; i++) {
        struct Window *w;
        for (w = s->FirstWindow; w && (unsigned long)w != ids[i]; w = w->NextWindow) ;
        if (w && !w->Title)       /* untitled: its size, flags and IDCMP say what it is (a dock, a panel) */
            len += snprintf(line + len, sizeof line - len, "%s(untitled %dx%d $%lx/$%lx)", i ? ", " : "", w->Width,
                            w->Height, (unsigned long)w->Flags, (unsigned long)w->IDCMPFlags);
        else
            len += snprintf(line + len, sizeof line - len, "%s\"%.24s\"", i ? ", " : "", w ? (const char *)w->Title : "(gone)");
    }
    UnlockIBase(lock);
    printf("INFO %s %s: %s\n", row, what, line);
}

static int switch_step(struct Screen *s, int back)
{
    unsigned long before[MAXWINS], after[MAXWINS], active;
    char cmd[320], why[128];
    const char *row = back ? "W2b" : "W2a";
    int nb, na, r;
    ULONG lock;
    nb = switch_order(s, before, MAXWINS);
    snprintf(cmd, sizeof cmd, "%s KEY 0x42 QUAL %s", wbt_dragtest(), back ? "LAMIGA+LSHIFT" : "LAMIGA");
    wbt_run(cmd);
    Delay(50);
    na = switch_order(s, after, MAXWINS);
    lock = LockIBase(0);
    active = (unsigned long)IntuitionBase->ActiveWindow;
    UnlockIBase(lock);
    r = wbgap_switch_judge(back, before, nb, after, na, active, why, sizeof why);
    printf("INFO %s %d windows; %s\n", row, nb, r == WBGAP_PASS ? "as wanted" : why);
    if (na != nb) switch_name_change(s, before, nb, after, na, row);
    if (r == WBGAP_FAIL) {
        switch_name(s, before, nb, row, "before");
        switch_name(s, after, na, row, "after");
        switch_name(s, &active, 1, row, "active");
    }
    return r;
}

int wbt_switch(int argc, char **argv)
{
    static char env[2048];
    char keyname[40], why[96];
    struct Screen *s;
    struct Window *a = NULL, *b = NULL;
    struct MsgPort *p;
    long n;
    int r1 = WBGAP_SKIP, r2 = WBGAP_SKIP;
    (void)argc, (void)argv;
    Forbid();
    p = FindPort((CONST_STRPTR)"GFOpenWindows");
    Permit();
    if (!p) {
        if (wbt_is_gfos()) wbt_fail("W2", "GFOpenWindows isn't running (no GFOpenWindows port): no Amiga+Tab");
        else wbt_skip("W2", "no window switcher on this desktop (GFOpenWindows isn't running)");
        return wbt_fails() ? 5 : 0;
    }
    n = wbt_read_all("ENV:GFOpenPrefs/Windows", env, sizeof env);
    if (!wbgap_switcher_on(n < 0 ? NULL : env, keyname, sizeof keyname)) {
        wbt_skip("W2", "the switcher is off (ENV:GFOpenPrefs/Windows: switcher off)");
        return 0;
    }
    if (stricmp(keyname, "lcommand tab")) {
        snprintf(why, sizeof why, "the switcher's key is \"%s\", not lcommand tab", keyname);
        wbt_skip("W2", why);
        return 0;
    }
    if (!(LayersBase = OpenLibrary((CONST_STRPTR)"layers.library", 39))) { wbt_skip("W2", "no layers.library"); return 0; }
    if (!(s = LockPubScreen((CONST_STRPTR)"Workbench"))) {
        wbt_skip("W2", "no Workbench screen");
        CloseLibrary(LayersBase);
        return 0;
    }
    ScreenToFront(s);
    a = OpenWindowTags(NULL, WA_PubScreen, (ULONG)s, WA_Left, 60, WA_Top, 60, WA_Width, 220, WA_Height, 90,
                       WA_Title, (ULONG)"W2 cycle A", WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE,
                       WA_Activate, TRUE, TAG_DONE);
    b = OpenWindowTags(NULL, WA_PubScreen, (ULONG)s, WA_Left, 120, WA_Top, 100, WA_Width, 220, WA_Height, 90,
                       WA_Title, (ULONG)"W2 cycle B", WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE,
                       WA_Activate, TRUE, TAG_DONE);
    if (!a || !b) wbt_fail("W2", "our two windows didn't open");
    else {
        Delay(50);
        r1 = switch_step(s, 0);
        r2 = switch_step(s, 1);
        if (r1 == WBGAP_PASS && r2 == WBGAP_PASS) wbt_pass("W2");
        else if (r1 == WBGAP_SKIP || r2 == WBGAP_SKIP) wbt_skip("W2", "fewer than two windows to cycle");
        else wbt_fail("W2", r1 != WBGAP_PASS ? "Amiga+Tab didn't bring the back window to the front, active (INFO W2a)"
                                         : "Amiga+Shift+Tab didn't send the front window back (INFO W2b)");
    }
    if (b) CloseWindow(b);
    if (a) CloseWindow(a);
    UnlockPubScreen(NULL, s);
    CloseLibrary(LayersBase);
    LayersBase = NULL;
    return wbt_fails() ? 5 : 0;
}
