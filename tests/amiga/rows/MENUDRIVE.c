/* ---- the M rows driven: each item picked with the mouse, as a hand picks it, and what it did looked at ----
 * In RAM:WBGap's drawer window (the script opens it). The pointer goes title, item (and sub-item) with the right
 * button held (wbgap_menu_path, DragTest DRAG RIGHT); what changed is read: the window's pixels, a window that
 * comes or goes, a drawer made. Only reads and input events: nothing is drawn here.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include "wbgap_menu.h"
#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "MENUDRIVE.h"

extern struct IntuitionBase *IntuitionBase;

/* the M rows' table mode is MENUS (MENUS and MENUDRIVE both give their lines), so no WBGAP_ROWMODE here */
WBGAP_MODE(MENUDRIVE, wbt_menudrive, SCRIPT)

#define MD_MAXWINS 48

static struct Window *drawer_window(void)
{
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
    struct Window *w, *found = NULL;
    ULONG lock;
    if (!wb) return NULL;
    lock = LockIBase(0);
    for (w = wb->FirstWindow; w && !found; w = w->NextWindow)
        if ((w->Flags & WFLG_WBENCHWINDOW) && w->MenuStrip && w->Title && strstr((const char *)w->Title, "WBGap") &&
            !strstr((const char *)w->Title, "WBGapL") && !strstr((const char *)w->Title, "WBGapI"))
            found = w;
    UnlockIBase(lock);
    UnlockPubScreen(NULL, wb);
    return found;
}


/* a window on s that wasn't in was[0..n), within secs */
static struct Window *new_window(struct Screen *s, struct Window **was, int n, int secs)
{
    struct Window *now[MD_MAXWINS];
    int t, k, j;
    for (t = 0; t < secs * 10 || t == 0; t++) {
        int m = wbt_windows_now(s, now, MD_MAXWINS);
        for (k = 0; k < m; k++) {
            for (j = 0; j < n && was[j] != now[k]; j++) {}
            if (j == n) return now[k];
        }
        if (secs) Delay(5);
    }
    return NULL;
}


/* the item spec names ("menu1|menu2:item|item/sub") in the strip: its boxes; 0 if not there, -1 if disabled */
static int find_item(struct Menu *strip, const char *spec, struct wbgap_box *mb, struct wbgap_box *ib, struct wbgap_box *sb, int *has_sub)
{
    char menus[80], items[160], *c, tmp[160], *alt;
    struct Menu *m;
    c = strchr(spec, ':');
    snprintf(menus, sizeof menus, "%.*s", (int)(c - spec), spec);
    snprintf(items, sizeof items, "%s", c + 1);
    for (m = strip; m; m = m->NextMenu) {
        char mn[24];
        struct MenuItem *it, *sub;
        wbt_norm(mn, (const char *)m->MenuName, sizeof mn);
        if (menus[0] != '*') {
            char mt[80], *a;
            int ok = 0;
            strcpy(mt, menus);
            for (a = strtok(mt, "|"); a && !ok; a = strtok(NULL, "|")) ok = !strcmp(a, mn);
            if (!ok) continue;
        }
        strcpy(tmp, items);
        for (alt = tmp; alt; ) {
            char *bar = strchr(alt, '|'), *slash, want[40], wsub[40];
            if (bar) *bar = 0;
            slash = strchr(alt, '/');
            snprintf(want, sizeof want, "%.*s", slash ? (int)(slash - alt) : (int)strlen(alt), alt);
            snprintf(wsub, sizeof wsub, "%s", slash ? slash + 1 : "");
            for (it = m->FirstItem; it; it = it->NextItem) {
                char t[40];
                if (!wbt_item_text(it)) continue;
                wbt_norm(t, wbt_item_text(it), sizeof t);
                if (strcmp(t, want)) continue;
                mb->left = m->LeftEdge, mb->top = 0, mb->width = m->Width, mb->height = m->Height;
                ib->left = it->LeftEdge, ib->top = it->TopEdge, ib->width = it->Width, ib->height = it->Height;
                *has_sub = 0;
                if (!wsub[0]) return (it->Flags & ITEMENABLED) && (m->Flags & MENUENABLED) ? 1 : -1;
                for (sub = it->SubItem; sub; sub = sub->NextItem) {
                    char st[40];
                    if (!wbt_item_text(sub)) continue;
                    wbt_norm(st, wbt_item_text(sub), sizeof st);
                    if (strcmp(st, wsub)) continue;
                    sb->left = sub->LeftEdge, sb->top = sub->TopEdge, sb->width = sub->Width, sb->height = sub->Height;
                    *has_sub = 1;
                    return (sub->Flags & ITEMENABLED) && (it->Flags & ITEMENABLED) ? 1 : -1;
                }
            }
            alt = bar ? bar + 1 : NULL;
        }
    }
    return 0;
}

/* the item picked in w's menus with the mouse: 1 when picked, 0 not there, -1 disabled */
static int pick(struct Window *w, const char *spec)
{
    struct wbgap_box mb, ib, sb;
    int xy[2 * WBGAP_MENU_MAXPTS], n, k, has_sub = 0, found;
    char cmd[400];
    ULONG lock;
    ActivateWindow(w);
    WindowToFront(w);
    Delay(25);
    lock = LockIBase(0);
    found = w->MenuStrip ? find_item(w->MenuStrip, spec, &mb, &ib, &sb, &has_sub) : 0;
    UnlockIBase(lock);
    if (found <= 0) return found;
    if (!(n = wbgap_menu_path(w->WScreen->BarHeight, &mb, &ib, has_sub ? &sb : NULL, xy))) return 0;
    k = snprintf(cmd, sizeof cmd, "%s DRAG", wbt_dragtest());
    for (found = 0; found < n; found++) k += snprintf(cmd + k, sizeof cmd - k, " %d %d", xy[2 * found], xy[2 * found + 1]);
    snprintf(cmd + k, sizeof cmd - k, " RIGHT STEPS 6 HOLD 10");
    printf("INFO picked %s: %s\n", spec, cmd + strlen(wbt_dragtest()) + 1);
    wbt_run(cmd);
    Delay(25);
    return 1;
}

/* picked in w's menus, or where w's haven't got it, in another Workbench window's (GFOpenBench keeps its own
 * menu on the desktop's window) */
static int pick_any(struct Window *w, const char *spec)
{
    struct Window *o, *other = NULL;
    struct wbgap_box mb, ib, sb;
    int r = pick(w, spec), hs, f = 0;
    ULONG lock;
    if (r != 0) return r;
    lock = LockIBase(0);
    for (o = w->WScreen->FirstWindow; o && !other; o = o->NextWindow)
        if (o != w && (o->Flags & WFLG_WBENCHWINDOW) && o->MenuStrip && (f = find_item(o->MenuStrip, spec, &mb, &ib, &sb, &hs)) != 0)
            other = o;
    UnlockIBase(lock);
    if (!other) return 0;
    if (f < 0) return -1;
    printf("INFO %s from the menus of \"%s\"\n", spec, other->Title ? (const char *)other->Title : "(no title)");
    return pick(other, spec);
}

/* a requester's button clicked: the leftmost (0) or rightmost (1) of the gadgets lowest in it */
static int click_button(struct Window *w, int right)
{
    struct Gadget *g, *best = NULL;
    LONG bx = 0, by = -1, x, y;
    char cmd[300];
    ULONG lock = LockIBase(0);
    for (g = w->FirstGadget; g; g = g->NextGadget) {
        if ((g->GadgetType & GTYP_SYSGADGET) || (g->GadgetType & GTYP_GTYPEMASK) != GTYP_BOOLGADGET) continue;
        x = ((g->Flags & GFLG_RELRIGHT) ? w->Width - 1 + g->LeftEdge : g->LeftEdge) + g->Width / 2;
        y = ((g->Flags & GFLG_RELBOTTOM) ? w->Height - 1 + g->TopEdge : g->TopEdge) + g->Height / 2;
        if (y > by + 2 || (y >= by - 2 && (right ? x > bx : x < bx))) best = g, bx = x, by = y;
    }
    if (best) bx += w->LeftEdge, by += w->TopEdge;
    UnlockIBase(lock);
    if (!best) return 0;
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), (long)bx, (long)by);
    wbt_run(cmd);
    Delay(25);
    return 1;
}


/* waits up to secs for w's inside to read as sum again */
static int back_to(struct Window *w, ULONG sum, int secs)
{
    int t;
    for (t = 0; t < secs * 5; t++) {
        if (wbt_inside_sum(w) == sum) return 1;
        Delay(10);
    }
    return 0;
}

static int dirs_in(const char *path, char names_out[][32], int n)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
    int k = 0;
    if (l && fib && Examine(l, fib))
        while (ExNext(l, fib))
            if (fib->fib_DirEntryType > 0 && k < n) snprintf(names_out[k++], 32, "%.31s", fib->fib_FileName);
    if (fib) FreeDosObject(DOS_FIB, fib);
    if (l) UnLock(l);
    return k;
}

static void not_picked(const char *row, int r)
{
    wbt_skip(row, r < 0 ? "its item is disabled here" : "its item isn't in the Workbench windows' menus (MENUS says)");
}

int wbt_menudrive(int argc, char **argv)
{
    struct Window *w = drawer_window(), *was[MD_MAXWINS], *nw;
    struct Screen *s;
    ULONG before;
    int r, nwas, k, n0, n1, i14_only;
    static char d0[32][32], d1[32][32];
    (void)argc;
    (void)argv;
    if (!w) {
        int how;
        wbt_open_drawer("RAM:WBGap", "WBGap", 5, &how);
        w = drawer_window();
    }
    if (!w) { wbt_fail("M*", "RAM:WBGap's drawer window isn't open (WBGapTest SETUP, DragTest OPEN RAM:WBGap)"); return 20; }
    s = w->WScreen;
    printf("INFO desktop: %s\n", wbt_is_gfos() ? "GFOpenBench (GFOS rows)" : "not GFOpenBench (3.2.3 rows)");

    /* M12 Select Contents, then M30 Clear Selection: the window back as it was */
    before = wbt_inside_sum(w);
    if ((r = pick(w, "*:select contents|select all")) <= 0) not_picked("M12", r);
    else if (wbt_changes_within(w, before, 3)) {
        wbt_pass("M12");
        if ((r = pick(w, "window:clear selection")) <= 0) not_picked("M30", r);
        else if (back_to(w, before, 3)) wbt_pass("M30");
        else wbt_fail("M30", "Clear Selection: the window didn't go back to nothing selected");
    } else {
        wbt_fail("M12", "Select Contents: nothing in the window changed");
        wbt_skip("M30", "nothing selected to clear (M12)");
    }
    pick(w, "window:clear selection");

    /* M15 Show > All Files: RAM:WBGap/noicon, a file with no icon, comes; then Only Icons */
    wbt_write_file("RAM:WBGap/noicon", "WBGapTest M15\n");
    Delay(50);
    i14_only = wbt_icons_have("I14", "RAM:WBGap", "noicon");
    before = wbt_inside_sum(w);
    if ((r = pick(w, "*:show/all files|show all files")) <= 0) { not_picked("M15", r); wbt_skip("I14", "Show All Files not picked (M15)"); }
    else {
        char why[64];
        int v;
        if (wbt_changes_within(w, before, 3)) wbt_pass("M15");
        else wbt_fail("M15", "Show All Files: nothing in the window changed (RAM:WBGap/noicon has no icon)");
        v = wbgap_showall_judge(i14_only, wbt_icons_have("I14", "RAM:WBGap", "noicon"), why, sizeof why);
        if (v == WBGAP_PASS) wbt_pass("I14");
        else if (v == WBGAP_SKIP) wbt_skip("I14", why);
        else wbt_fail("I14", why);
    }
    pick(w, "*:show/only icons|show only icons");
    Delay(25);

    /* M16 View By > Name: the window as a list; then by icon again */
    before = wbt_inside_sum(w);
    if ((r = pick(w, "*:view by/name|as a list")) <= 0) not_picked("M16", r);
    else if (wbt_changes_within(w, before, 3)) wbt_pass("M16");
    else wbt_fail("M16", "View By Name: nothing in the window changed");
    pick(w, "*:view by/icon|as icons");
    Delay(25);

    /* M6 About: a window comes; its button closes it */
    nwas = wbt_windows_now(s, was, MD_MAXWINS);
    if ((r = pick_any(w, "workbench|gfopenbench:about")) <= 0) not_picked("M6", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        wbt_pass("M6");
        click_button(nw, 0);
        if (wbt_still_there(s, nw)) wbt_key(0x45);
    } else wbt_fail("M6", "About: no window came");

    /* M2 Execute Command: its requester comes; Cancel */
    nwas = wbt_windows_now(s, was, MD_MAXWINS);
    if ((r = pick_any(w, "workbench|gfopenbench:execute command")) <= 0) not_picked("M2", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        wbt_pass("M2");
        click_button(nw, 1);
        if (wbt_still_there(s, nw)) wbt_key(0x45);
    } else wbt_fail("M2", "Execute Command: no requester came");

    /* M8 New Drawer: its requester, Return takes the name it offers, a drawer is made */
    n0 = dirs_in("RAM:WBGap", d0, 32);
    nwas = wbt_windows_now(s, was, MD_MAXWINS);
    if ((r = pick(w, "window:new drawer")) <= 0) not_picked("M8", r);
    else {
        int t, made = -1;
        nw = NULL;
        for (t = 0; t < 30 && made < 0; t++) {   /* a requester for its name, or (no requester) the drawer at once */
            if (!nw && (nw = new_window(s, was, nwas, 0))) {
                printf("INFO M8 a requester came: Return takes the name it offers\n");
                wbt_key(0x44);   /* Return */
            }
            Delay(5);
            n1 = dirs_in("RAM:WBGap", d1, 32);
            made = wbgap_menudrive_new_dir((const char (*)[32])d0, n0, (const char (*)[32])d1, n1);
        }
        if (made >= 0) {
            char p[64];
            wbt_pass("M8");
            printf("INFO M8 made RAM:WBGap/%s%s\n", d1[made], nw ? "" : " (no requester)");
            snprintf(p, sizeof p, "RAM:WBGap/%s.info", d1[made]);
            DeleteFile((CONST_STRPTR)p);
            snprintf(p, sizeof p, "RAM:WBGap/%s", d1[made]);
            DeleteFile((CONST_STRPTR)p);
        } else wbt_fail("M8", nw ? "New Drawer: Return in its requester made no drawer" : "New Drawer: no requester, no drawer made");
        if (nw && wbt_still_there(s, nw)) click_button(nw, 1);   /* Cancel */
    }

    /* M9 Open Parent: RAM:'s window comes */
    nwas = wbt_windows_now(s, was, MD_MAXWINS);
    if ((r = pick(w, "window:open parent")) <= 0) not_picked("M9", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        wbt_pass("M9");
        printf("INFO M9 \"%s\" came\n", nw->Title ? (const char *)nw->Title : "(no title)");
    } else if (wbt_still_there(s, w) && w->Title && !strstr((const char *)w->Title, "WBGap")) {
        wbt_pass("M9");   /* GFOpenBench's windows go back and forward: the parent in the same window */
        printf("INFO M9 the same window now shows \"%s\"\n", (const char *)w->Title);
    } else {
        struct Window *act = IntuitionBase->ActiveWindow;   /* RAM:'s window open already: to the front, active */
        if (act && act != w && (act->Flags & WFLG_WBENCHWINDOW) && act->Title && !strstr((const char *)act->Title, "WBGap")) {
            wbt_pass("M9");
            printf("INFO M9 \"%s\", open already, made active\n", (const char *)act->Title);
        } else wbt_fail("M9", "Open Parent: no window came or became active, and the window still shows RAM:WBGap");
    }

    /* M10 Close, last: the drawer's window goes; opened again for what comes after */
    if ((r = pick(w, "window:close")) <= 0) not_picked("M10", r);
    else {
        for (k = 0; k < 30 && wbt_still_there(s, w); k++) Delay(5);
        if (!wbt_still_there(s, w)) wbt_pass("M10"); else wbt_fail("M10", "Close: the drawer's window stayed");
    }
    {
        int how;
        if (!drawer_window()) wbt_open_drawer("RAM:WBGap", "WBGap", 5, &how);
    }
    return wbt_fails() ? 5 : 0;
}

