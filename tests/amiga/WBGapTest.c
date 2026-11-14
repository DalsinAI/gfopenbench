/* WBGapTest: what a desktop does that OS 3.2.3's Workbench does, row by row of amicradle-teams
 * designs/OPENBENCH-GAPS-20261112.md, on GFOpenBench and on OS 3.2.3's own Workbench alike (the Team,
 * 12 November 2026). Each check prints one line: "PASS <row>", "FAIL <row> <why>" or "SKIP <row> <why>".
 * WBGapTest.script runs them all in a lab; DragTest (beside it) gives the mouse and the keys.
 *
 *   WBGapTest MENUS [DUMP]      the Workbench windows' menus (WFLG_WBENCHWINDOW, on "Workbench"), the items
 *                               of rows M1-M26 by name; DUMP lists every menu, item and sub-item with its key
 *   WBGapTest SETUP             makes RAM:WBGap (a drawer, its icon, a tool "a" with an icon)
 *   WBGapTest API               rows A3, A4, M27, A6-A9 (A8c: WhichWorkbenchObjectA on RAM:WBGapW), N8 on RAM:WBGap (AddAppIconA, AddAppMenuItemA,
 *                               OpenWorkbenchObjectA, UpdateWorkbench, MakeWorkbenchObjectVisibleA,
 *                               WBInfo, ChangeWorkbenchSelectionA, CloseWorkbenchObjectA)
 *   WBGapTest NOTIFY            rows F1, F2: RAM:WBGap's window opened (OpenWorkbenchObjectA, else DragTest OPEN),
 *                               then a file with an icon made and an icon changed from here: the window's
 *                               inside must change by itself within 5 seconds, with no Update
 *   WBGapTest MENUDRIVE         rows M2, M6, M8, M9, M10, M12, M15, M16, M30 driven: each item picked with the
 *                               mouse in RAM:WBGap's window (DragTest DRAG RIGHT), and what it did read back
 *   WBGapTest SCREENS           rows C1, C2, C4, P4a, P4b (the pointer: moved, and on RTG a sprite layer); a screen of our own in front; its depth gadget clicked;
 *                               Left-Amiga-M and Left-Amiga-N
 *   WBGapTest SWITCH            row W2 (rows/SWITCH.c): GFOpenWindows' Amiga+Tab and Amiga+Shift+Tab cycle the Workbench screen's
 *                               windows (two of our own opened for it); SKIP with no switcher (3.2.3)
 *   WBGapTest BIG               row X1: RAM:WBGapBig with 1000 files with icons, opened and timed
 *   WBGapTest CLOSEWB           row C6: CloseWorkBench() with this Shell's window on the screen (refused: the screen,
 *                               the desktop and RAM:WBGapC's window stay), else OpenWorkBench() and both come back;
 *                               the script starts it with Run >RAM:wbgap-c6, last before REPORT
 *   WBGapTest CLOSEWBWAIT f s   row C6's line: waits up to s seconds for that CLOSEWB run to be over and prints file f;
 *                               not over in time is FAIL C6, so a CloseWorkBench() that never returns can't hang the run
 *   WBGapTest BAR               rows B1-B3: GFOpenSpeaker's port there (B3); GFOpenTitle's Power icon clicked, its list looked at (where it hangs;
 *                               its height: Reboot, Restart, Power off, and an Eject row for each USB volume mounted)
 *                               and closed with Esc; nothing in it is picked
 *   WBGapTest DISKS             rows D5 and D5r: RAM:'s volume (NameFromLock) has a disk icon among the desktop's icons,
 *                               and is first in the volume column (RAM:wbgap-root-pos),
 *                               as WBGapRoot.rexx wrote them to RAM:wbgap-root
 *   WBGapTest WBCONTROL         row A5 (rows/WBCONTROL.c): WorkbenchControlA's stack, restart time, global flags and
 *                               hidden devices, each set, read back and put back
 *   WBGapTest MOUNTICON [drive] row D2: the drive (RAD: if none given), not mounted yet, mounted with C:Mount; its
 *                               volume must get a disk icon among the desktop's icons (WBGapRoot.rexx) within 10 s
 *   WBGapTest TITLES            row W3 (rows/TITLES.c): RAM:WBGapW3's window titled with the drawer's name, its screen with a title
 *   WBGapTest NEWROWS           every mode in tests/amiga/rows marked NEWROWS, one after another (a new row's file is its only
 *                               line: the script runs them all here); each of those modes also runs alone by its name
 *   WBGapTest DISKIO [drive]    row D1: the drive's (RAD: if none given) disk icon goes while DragTest INHIBIT has
 *                               its file system stopped, and comes back after; the desktop's icons read each time
 *                               by WBGapRoot.rexx (beside this program) into RAM:wbgap-root-d1
 *   WBGapTest APPDROP           rows I9, I8: RAM:WBGapD's icon d dragged (DragTest) onto a window of ours made an
 *                               AppWindow, then onto an AppIcon (its place from WBGapAppPos.rexx); the AppMessage read
 *   WBGapTest DRAGFILE          rows I4, I7: RAM:WBGapD's d dragged into RAM:WBGapE's window (moved, icon and all),
 *                               then onto the drawer sub's icon in RAM:WBGapD (moved into it); KEEP: left open
 *   WBGapTest STARTUP SETUP     row S1-S3: icons for SYS:WBStartup/WBGapA, B and C (copies of this program,
 *                               made by the script); after a reboot each writes to RAM:wbgap-order
 *   WBGapTest STARTUP CHECK     reads RAM:wbgap-order (S1-S3, S2d), RAM:wbgap-launchwb (I12a, I12b: WBGapF, started
 *                               from WBStartup, and I12c: WBGapG with STACK= alone, its stack field the default) with stack (the icon's field and STACK=, 1 MB over the default stack and 1 MB) and TOOLPRI=-3) and ENV:GFOpenBench/LoadWB (A10a)
 *   WBGapTest LAUNCH            rows I11, I12a, I12b: RAM:WBGap/LaunchT (a copy of this program) started by
 *                               OpenWorkbenchObjectA with two projects as arguments, its icon's stack (the icon's field and STACK=, 1 MB over the default stack and 1 MB) and
 *                               TOOLPRI=-3; it writes what it got to RAM:wbgap-launch
 *   WBGapTest LAUNCHDESK SETUP|CHECK|DONE
 *                               row I11 as a hand does it: LaunchT, p1 and p2 left out on the desktop (RAM:.backdrop),
 *                               WBGapDesk.rexx clicks p1, Shift-clicks p2, Shift-double-clicks LaunchT; CHECK reads
 *                               what LaunchT got (RAM:wbgap-launchdesk); DONE puts RAM:.backdrop back. Row I10 too:
 *                               OpenT (a tool) and Sub (a drawer) left out with them, each double-clicked alone; CHECK
 *                               reads what OpenT got (RAM:wbgap-launchopen) and whether Sub's window opened
 *   WBGapTest LAYOUT            row L1's drawer: RAM:WBGapL, no icon snapshotted, holding tools ztool and atool,
 *                               projects mproj and bproj, drawers ydrawer and cdrawer; WBGapLayout.rexx checks it
 *   WBGapTest INFOSAVE          rows N2-N5: RAM:WBGapN/n's Information window (WBInfo) driven with DragTest: Deletable
 *                               off, a comment, a default tool, tool types a=1 edited to a=3, d=4 deleted, b=2
 *                               added, Save; then the file's protection and comment and its icon are read back.
 *                               GFOpenBench only (it finds the gadgets by desktop/ob_info_ids.h); SKIP on 3.2.3
 *   WBGapTest REPORT <file>     the lines of the modes above, in <file>, as exactly one PASS, FAIL or SKIP line per
 *                               gap id (wbgap_rows.c, the gaps document plus the 13 Nov decisions), and a count
 *   WBGapTest LOWMEM secs path  row X3 (WBGapX3.rexx): takes nearly all memory, in pieces too small for a program,
 *                               asks GFOpenBench to open path (its GFOPENBENCH port, as DragTest OPEN, without
 *                               waiting: ARexx can't run with no memory) and waits secs seconds; then looks for the
 *                               desktop's low-memory requester, gives the memory back and clicks its Retry (DragTest)
 *
 * Rows the 13 Nov decisions (amicradle-teams designs/WORKBENCH-REDESIGN-20261113.md) changed for GFOS only (M1, M4,
 * M5, M24, M26, S2d, A10a) are checked the GFOS way when the desktop is GFOpenBench (an OPENBENCH port, or a menu
 * titled GFOpenBench), and the 3.2.3 way otherwise.
 *
 * Started from the Workbench (as SYS:WBStartup/WBGapX), it writes "start X" to RAM:wbgap-order, waits
 * the STAMPSECS seconds its icon names, and writes "end X".
 *
 * For lab instances only: it writes to RAM: and, with STARTUP SETUP, to SYS:WBStartup.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/modeid.h>
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dosextens.h>
#include <dos/filehandler.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/layers.h>
#include <proto/wb.h>

#include "wbgap_menu.h"
#include "../../desktop/ob_info_ids.h"   /* the Information window's gadget ids (INFOSAVE) */
#include "wbgap_rows.h"
#include "wbgap_test.h"
#if defined(WBO_NONE) && defined(WBO_DRAWER) && defined(WBO_ICON)
typedef char wbgap_wbo_same[(WBO_NONE == WBGAP_WBO_NONE && WBO_DRAWER == WBGAP_WBO_DRAWER && WBO_ICON == WBGAP_WBO_ICON) ? 1 : -1];
#endif

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct WBStartup *_WBenchMsg;   /* libnix: the Workbench's message, when started from it */
struct Library *WorkbenchBase, *IconBase, *CyberGfxBase, *LayersBase;   /* LayersBase: SWITCH (rows/SWITCH.c) opens it */

static int fails;
static char dragtest[256] = "DragTest";   /* the full path of the DragTest beside this program */

static void find_dragtest(void)
{
    BPTR dir = GetProgramDir();
    if (dir && NameFromLock(dir, (STRPTR)dragtest, sizeof dragtest - 10)) AddPart((STRPTR)dragtest, (CONST_STRPTR)"DragTest", sizeof dragtest);
}
static void pass(const char *row) { printf("PASS %s\n", row); }
static void fail(const char *row, const char *why) { printf("FAIL %s %s\n", row, why); fails++; }
static void skip(const char *row, const char *why) { printf("SKIP %s %s\n", row, why); }

/* the desktop is GFOpenBench: its OPENBENCH ARexx port (0.1.53 on), or (MENUS) a menu titled GFOpenBench */
static int check_launchrec(const char *file, int args_too, unsigned long want_stack);
static void check_stack_alone(const char *file, unsigned long want);
static unsigned long stack_wanted(void);
static unsigned long icon_stack(const char *path);
static int gfos_menu_seen;
static int is_gfos(void)
{
    struct MsgPort *p;
    Forbid();
    p = FindPort((CONST_STRPTR)"OPENBENCH");
    Permit();
    return p != NULL || gfos_menu_seen;
}

static int exists(const char *path)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ);
    if (l) UnLock(l);
    return l != 0;
}

/* a program's "$VER: " string, up to its end; 0 if the file can't be read or has none */
static int ver_of(const char *path, char *out, int n)
{
    static char buf[4096 + 128];
    BPTR fh = Open((CONST_STRPTR)path, MODE_OLDFILE);
    long got, keep = 0, i;
    out[0] = 0;
    if (!fh) return 0;
    while ((got = Read(fh, buf + keep, 4096)) > 0) {
        long len = keep + got;
        for (i = 0; i + 6 <= len; i++)
            if (!memcmp(buf + i, "$VER: ", 6)) {
                int k = 0;
                if (i + 6 + n > len) {   /* the string runs on: read the rest of it */
                    memmove(buf, buf + i, len - i);
                    len -= i;
                    i = 0;
                    got = Read(fh, buf + len, 64);
                    if (got > 0) len += got;
                }
                for (i += 6; i < len && k < n - 1 && buf[i] && buf[i] != '\n' && buf[i] != '\r'; i++) out[k++] = buf[i];
                out[k] = 0;
                Close(fh);
                return 1;
            }
        keep = len < 5 ? len : 5;   /* "$VER:" across two reads */
        memmove(buf, buf + len - keep, keep);
    }
    Close(fh);
    return 0;
}

/* GFOS (13 Nov): Format and DiskCopy are applications of our own (OpenFormat, GFOpen...), not 3.2.3's */
static void app_row(const char *row, const char *path, int gfos)
{
    char v[80], why[160];
    if (!exists(path)) { snprintf(why, sizeof why, "no %s", path); fail(row, why); return; }
    ver_of(path, v, sizeof v);
    printf("INFO %s %s: %s\n", row, path, v[0] ? v : "no $VER");
    if (gfos && !strstr(v, "Open") && !strstr(v, "GF")) {
        snprintf(why, sizeof why, "%s is 3.2.3's (%s), not a GFOS application", path, v);
        fail(row, why);
    } else pass(row);
}

/* cybergraphics.library ReadRGBPixel (LVO -108: a1 rp, d0 x, d1 y), for a window on a deep RTG screen,
 * where ReadPixel gives no pen */
static ULONG rgb_pixel(struct RastPort *rp, UWORD x, UWORD y)
{
    register ULONG d0 __asm("d0") = x;
    register ULONG d1 __asm("d1") = y;
    register struct RastPort *a1 __asm("a1") = rp;
    register struct Library *base __asm("a2") = CyberGfxBase;   /* a6 may be the frame pointer: saved by hand */
    __asm volatile ("move.l %%a6,-(%%sp)\n\tmove.l %3,%%a6\n\tjsr -108(%%a6)\n\tmove.l (%%sp)+,%%a6"
                    : "+d"(d0), "+d"(d1), "+a"(a1) : "a"(base) : "a0", "cc", "memory");
    return d0;
}

/* ---- menus ---- */
#define MAXNAMES 400
static char names[MAXNAMES][64];   /* "menu/item" and "menu/item/sub", lower case, without "..." */
static int nnames;
static char keys[MAXNAMES];          /* each name's Amiga-key, 0 for none */
static char leaves[MAXNAMES][40];    /* the item's (or sub-item's) own text, normalised */
static char menus_of[MAXNAMES][24];  /* its menu's title, normalised */

static void norm(char *d, const char *s, int n)
{
    int i = 0;
    while (*s == ' ') s++;
    for (; *s && i < n - 1; s++) d[i++] = (*s >= 'A' && *s <= 'Z') ? *s + 32 : *s;
    d[i] = 0;
    while (i > 0 && (d[i - 1] == '.' || d[i - 1] == ' ' || d[i - 1] == '?')) d[--i] = 0;   /* "Backdrop?" */
}

static const char *item_text(struct MenuItem *it)
{
    if (!(it->Flags & ITEMTEXT) || !it->ItemFill) return NULL;
    return (const char *)((struct IntuiText *)it->ItemFill)->IText;
}

static void add_name(const char *menu, const char *item, const char *sub, int key, int dump)
{
    char m[24], i[40], s[40];
    norm(m, menu ? menu : "", sizeof m);
    if (!strcmp(m, "gfopenbench")) gfos_menu_seen = 1;
    norm(i, item, sizeof i);
    if (sub) norm(s, sub, sizeof s);
    if (dump) printf("MENU %s / %s%s%s%s%c\n", menu ? menu : "?", item, sub ? " / " : "", sub ? sub : "",
                     key ? "  key Amiga-" : "", key ? key : ' ');
    if (nnames < MAXNAMES) {
        keys[nnames] = (char)key;
        strcpy(leaves[nnames], sub ? s : i);
        strcpy(menus_of[nnames], m);
        snprintf(names[nnames++], 64, sub ? "%s/%s/%s" : "%s/%s", m, i, sub ? s : "");
    }
}

static void walk_strip(struct Menu *menu, int dump)
{
    for (; menu; menu = menu->NextMenu) {
        struct MenuItem *it, *sub;
        const char *mn = (const char *)menu->MenuName;
        for (it = menu->FirstItem; it; it = it->NextItem) {
            const char *t = item_text(it);
            if (!t) continue;
            add_name(mn, t, NULL, (it->Flags & COMMSEQ) ? it->Command : 0, dump);
            for (sub = it->SubItem; sub; sub = sub->NextItem)
                if (item_text(sub)) add_name(mn, t, item_text(sub), (sub->Flags & COMMSEQ) ? sub->Command : 0, dump);
        }
    }
}

/* spec "menu1|menu2:item1|item2" or "*:..." ("item" may be "item/sub") */
static int has(const char *spec)
{
    char menus[80], items[160], *c, *a, *b;
    int n;
    c = strchr(spec, ':');
    snprintf(menus, sizeof menus, "%.*s", (int)(c - spec), spec);
    snprintf(items, sizeof items, "%s", c + 1);
    for (n = 0; n < nnames; n++) {
        char *slash = strchr(names[n], '/');
        char mname[24];
        int mok = 0;
        snprintf(mname, sizeof mname, "%.*s", (int)(slash - names[n]), names[n]);
        if (menus[0] == '*') mok = 1;
        else {
            char tmp[80];
            strcpy(tmp, menus);
            for (a = strtok(tmp, "|"); a && !mok; a = strtok(NULL, "|")) mok = !strcmp(a, mname);
        }
        if (!mok) continue;
        {
            char tmp[160];
            strcpy(tmp, items);
            for (b = strtok(tmp, "|"); b; b = strtok(NULL, "|"))
                if (!strcmp(slash + 1, b)) return 1;
        }
    }
    return 0;
}

static const struct { const char *row, *spec; } menu_rows[] = {
    {"M1", "workbench|gfopenbench:backdrop"},
    {"M2", "workbench|gfopenbench:execute command"},
    {"M3", "workbench|gfopenbench:redraw all"},
    {"M4", "workbench|gfopenbench:update all|update"},
    {"M5", "workbench|gfopenbench:last message"},
    {"M6", "workbench|gfopenbench:about"},
    {"M7", "workbench|gfopenbench:quit|quit gfopenbench"},
    {"M8", "window:new drawer"},
    {"M9", "window:open parent"},
    {"M10", "window:close"},
    {"M11", "window:update"},
    {"M12", "*:select contents|select all"},
    {"M13", "window:clean up by|clean up"},
    {"M14", "window:snapshot/window|snapshot window"},
    {"M15", "*:show/all files|show all files"},
    {"M16", "*:view by/name|as a list"},
    {"M17", "icons:open"},
    {"M18", "icons:copy|duplicate"},
    {"M19", "icons:rename"},
    {"M20", "icons:information"},
    {"M21", "icons:unsnapshot"},
    {"M22", "icons:put away"},
    {"M23", "icons:delete"},
    {"M24", "icons:format disk|format"},
    {"M25", "icons:empty trash"},
    {"M26", "*:resetwb|refresh openbench"},
    {"M29", "*:open volume"},   /* row 5 (#105) moves it to the Window menu */
    {"M30", "window:clear selection"},
    {"M31", "window:resize to fit"},
    {"M32", "window:sort order/reverse"},
    {"M33", "window:find"},
    {"M34", "tools:auto-arrange icons"},
    {"M35", "icons:eject disk|eject"},
};

/* M28: every Amiga-key the Workbench's windows give means what it means on OS 3.2.3 (the leaf texts it may have;
 * a key 3.2.3 doesn't use may mean anything). The Dock's own window and menus are left out. */
static const struct { char key; const char *leaves; } wb_keys[] = {
    {'O', "open"}, {'K', "close"}, {'M', "update"}, {'A', "select contents|select all"}, {'Z', "clear selection"},
    {'S', "snapshot"}, {'U', "unsnapshot"}, {'L', "leave out"}, {'P', "put away"}, {'C', "copy"}, {'R', "rename"},
    {'I', "information"}, {'D', "delete"}, {'B', "backdrop"}, {'E', "execute command"}, {'?', "about"},
    {'Q', "quit|quit gfopenbench"}, {'N', "new drawer"}, {'-', "only icons"}, {'+', "all files|show all files"},
    {'F', "find"}, {'Y', "auto-arrange icons"}, {'G', "forward"}, {'H', "reverse"}, {'1', "icon|as icons"},
    {'2', "name|as a list"}, {'3', "date"}, {'4', "size"}, {'5', "type"}, {'.', "column|clean up"},
    {'6', "name"}, {'7', "date"}, {'8', "size"}, {'9', "type"},
};

static int check_keys(void)
{
    int n, k, bad = 0;
    for (n = 0; n < nnames; n++) {
        char up = keys[n] >= 'a' && keys[n] <= 'z' ? keys[n] - 32 : keys[n];
        if (!up || !strcmp(menus_of[n], "gfopendock")) continue;
        for (k = 0; k < (int)(sizeof wb_keys / sizeof wb_keys[0]); k++)
            if (wb_keys[k].key == up) {
                char tmp[80], *t;
                int ok = 0;
                strcpy(tmp, wb_keys[k].leaves);
                for (t = strtok(tmp, "|"); t && !ok; t = strtok(NULL, "|")) ok = !strcmp(t, leaves[n]);
                if (!ok) { printf("INFO M28 Amiga-%c is \"%s\" (%s), on 3.2.3 \"%s\"\n", up, names[n], menus_of[n], wb_keys[k].leaves); bad++; }
                break;
            }
    }
    return bad;
}

/* a window's icons now, through WBGapRoot.rexx (beside this program) into RAM:wbgap-root-<row>; -1 if not read, else
 * whether name is among them (I14) */
static int run(const char *cmd);
static long read_all(const char *path, char *buf, long n);
static int icons_have(const char *row, const char *win, const char *name)
{
    static char list[8192];
    char cmd[400], rexx[256], out[48], args[200];
    long n;
    BPTR l;
    int rexx_up;
    snprintf(rexx, sizeof rexx, "%s", dragtest);
    *PathPart((STRPTR)rexx) = 0;
    AddPart((STRPTR)rexx, (CONST_STRPTR)"WBGapRoot.rexx", sizeof rexx);
    snprintf(out, sizeof out, "RAM:wbgap-root-%s", row);
    snprintf(args, sizeof args, "%s %s %s", row, out, win);
    if (!wbgap_rx_line(cmd, sizeof cmd, rexx, args)) {
        printf("INFO %s WBGapRoot.rexx not run: its path \"%s\" has a space (rx can't be given one)\n", row, rexx);
        return -1;
    }
    if (!(l = Lock((CONST_STRPTR)"SYS:Rexxc/rx", ACCESS_READ))) return -1;
    UnLock(l);
    Forbid();
    rexx_up = FindPort((CONST_STRPTR)"REXX") != NULL;
    Permit();
    if (!rexx_up) {                         /* MENUDRIVE runs before the script starts RexxMast */
        run("Run >NIL: SYS:System/RexxMast >NIL:");
        Delay(50);
    }
    run(cmd);
    n = read_all(out, list, sizeof list);
    DeleteFile((CONST_STRPTR)out);
    return n < 0 ? -1 : wbgap_root_has_name(list, name);
}

static int do_menus(int dump)
{
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
    struct Window *w;
    ULONG lock;
    int wins = 0, i;
    if (!wb) { fail("M*", "no Workbench screen"); return 20; }
    lock = LockIBase(0);
    for (w = wb->FirstWindow; w; w = w->NextWindow)
        if ((w->Flags & WFLG_WBENCHWINDOW) && w->MenuStrip) { walk_strip(w->MenuStrip, dump); wins++; }
    UnlockIBase(lock);
    UnlockPubScreen(NULL, wb);
    printf("INFO menus from %d Workbench window(s), %d names\n", wins, nnames);
    if (dump) return 0;
    if (!wins) { fail("M*", "no Workbench window has a menu strip"); return 10; }
    printf("INFO desktop: %s\n", is_gfos() ? "GFOpenBench (GFOS rows)" : "not GFOpenBench (3.2.3 rows)");
    for (i = 0; i < (int)(sizeof menu_rows / sizeof menu_rows[0]); i++) {
        const char *row = menu_rows[i].row;
        if (is_gfos() && (!strcmp(row, "M1") || !strcmp(row, "M5"))) {   /* removed in GFOS (13 Nov) */
            if (has(menu_rows[i].spec)) fail(row, "removed by the 13 Nov decision, still in a menu");
            else pass(row);
        } else if (is_gfos() && !strcmp(row, "M4")) {   /* Refresh OpenBench covers Update All */
            if (has("*:refresh openbench")) pass(row); else fail(row, "no Refresh OpenBench item (covers Update All)");
        } else if (is_gfos() && !strcmp(row, "M24")) {   /* Format: a separate application */
            app_row(row, "SYS:System/Format", 1);
        } else if (has(menu_rows[i].spec)) pass(row);
        else fail(row, menu_rows[i].spec);
    }
    /* D3 Format and D4 DiskCopy: separate applications; on GFOS not 3.2.3's own */
    app_row("D3", "SYS:System/Format", is_gfos());
    app_row("D4", "SYS:System/DiskCopy", is_gfos());
    if (check_keys()) fail("M28", "Amiga-keys that mean something else than on 3.2.3 (INFO lines)"); else pass("M28");
    return fails ? 5 : 0;
}

/* ---- files ---- */
static int write_file(const char *path, const char *text)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    if (!fh) return 0;
    Write(fh, (APTR)text, strlen(text));
    Close(fh);
    return 1;
}

static int put_icon_stack(const char *path, LONG type, STRPTR *tooltypes, LONG stack)
{
    struct DiskObject *d = GetDefDiskObject(type);
    STRPTR *old;
    LONG oldstack;
    int ok;
    if (!d) return 0;
    old = d->do_ToolTypes;
    oldstack = d->do_StackSize;
    if (tooltypes) d->do_ToolTypes = tooltypes;
    if (stack) d->do_StackSize = stack;
    d->do_CurrentX = d->do_CurrentY = NO_ICON_POSITION;
    ok = PutDiskObject((CONST_STRPTR)path, d);
    d->do_ToolTypes = old;
    d->do_StackSize = oldstack;
    FreeDiskObject(d);
    return ok;
}
static int put_icon(const char *path, LONG type, STRPTR *tooltypes) { return put_icon_stack(path, type, tooltypes, 0); }

static int do_setup(void)
{
    BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGap");
    if (l) UnLock(l);
    put_icon("RAM:WBGap", WBDRAWER, NULL);
    write_file("RAM:WBGap/a", "WBGapTest\n");
    put_icon("RAM:WBGap/a", WBPROJECT, NULL);
    l = Lock((CONST_STRPTR)"RAM:WBGap/a.info", ACCESS_READ);
    printf("INFO RAM:WBGap %s\n", l ? "made" : "NOT made");
    if (l) UnLock(l);
    return l ? 0 : 20;
}

/* ---- the drawer's window ---- */
static struct Window *find_window(const char *part)
{
    struct Screen *s;
    struct Window *w, *found = NULL;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (w = s->FirstWindow; w && !found; w = w->NextWindow)
            if (w->Title && strstr((const char *)w->Title, part)) found = w;
    UnlockIBase(lock);
    return found;
}

static struct Window *open_drawer(const char *path, const char *title, int secs, int *how)
{
    struct Window *w;
    int t;
    *how = 0;
    if (!OpenWorkbenchObjectA((STRPTR)path, NULL)) {
        char cmd[320];
        snprintf(cmd, sizeof cmd, "%s OPEN %s", dragtest, path);
        SystemTags((CONST_STRPTR)cmd, TAG_DONE);
        *how = 1;
    }
    for (t = 0; t < secs * 10; t++) {
        if ((w = find_window(title))) return w;
        Delay(5);
    }
    return NULL;
}

/* A drawer's window off the desktop before rows that click it (wbgap_rows.h): closed through workbench.library and
 * waited for 5 s; still there, parked at its smallest in the screen's bottom-right corner and waited for 2 s more.
 * An INFO line for row says which. WBGAP_CL_* */
static int clear_drawer(const char *path, const char *title, const char *row)
{
    struct Window *w;
    long box[4], mw = 0, mh = 0, sw = 0, sh = 0;
    int t, at = 0, r;
    ULONG lock;
    if (!find_window(title)) return WBGAP_CL_GONE;
    CloseWorkbenchObjectA((STRPTR)path, NULL);
    for (t = 0; t < 50 && find_window(title); t++) Delay(5);
    lock = LockIBase(0);                            /* the window's fields read while Intuition can't change them */
    if ((w = find_window(title))) {
        mw = w->MinWidth ? w->MinWidth : w->BorderLeft + w->BorderRight + 40;
        mh = w->MinHeight ? w->MinHeight : w->BorderTop + w->BorderBottom + 20;
        sw = w->WScreen->Width;
        sh = w->WScreen->Height;
    }
    UnlockIBase(lock);
    if (w) {
        wbgap_park_box(sw, sh, mw, mh, box);
        ChangeWindowBox(w, box[0], box[1], box[2], box[3]);
        for (t = 0; t < 20 && !at; t++) {
            Delay(5);
            lock = LockIBase(0);
            if ((w = find_window(title)))
                at = w->LeftEdge == box[0] && w->TopEdge == box[1] && w->Width == box[2] && w->Height == box[3];
            UnlockIBase(lock);
            if (!w) break;
        }
    }
    r = wbgap_clear_result(!find_window(title), at);
    printf("INFO %s %s's window %s\n", row, path,
           r == WBGAP_CL_GONE ? "closed" : r == WBGAP_CL_PARKED ? "would not close: parked in the bottom-right corner"
                                                                 : "would not close or move: still over the desktop");
    return r;
}

/* the inside of a window, sampled every second pixel, as one number */
static ULONG inside_sum(struct Window *w)
{
    struct RastPort *rp = w->RPort;
    int deep = CyberGfxBase && GetBitMapAttr(rp->BitMap, BMA_DEPTH) > 8;
    ULONG sum = 0;
    int x, y;
    for (y = w->BorderTop; y < w->Height - w->BorderBottom; y += 2)
        for (x = w->BorderLeft; x < w->Width - w->BorderRight; x += 2)
            sum = sum * 31 + (deep ? rgb_pixel(rp, x, y) : (ULONG)ReadPixel(rp, x, y));
    return sum;
}

static int changes_within(struct Window *w, ULONG before, int secs)
{
    int t;
    for (t = 0; t < secs * 5; t++) {
        Delay(10);
        if (inside_sum(w) != before) return 1;
    }
    return 0;
}

static int do_notify(void)
{
    struct Window *w;
    ULONG before;
    int how;
    DeleteFile((CONST_STRPTR)"RAM:WBGap/new1");        /* a run before left them: the file must be new */
    DeleteFile((CONST_STRPTR)"RAM:WBGap/new1.info");
    do_setup();
    if (!(w = open_drawer("RAM:WBGap", "WBGap", 10, &how))) { fail("F1", "RAM:WBGap's window didn't open"); skip("F2", "no window"); return 10; }
    printf("INFO RAM:WBGap opened by %s\n", how ? "DragTest OPEN" : "OpenWorkbenchObjectA");
    if (w->Width < 480 || w->Height < 240) {   /* a new drawer may open too small to show its icons (L1 checks that) */
        ChangeWindowBox(w, w->LeftEdge, w->TopEdge, w->Width < 480 ? 480 : w->Width, w->Height < 240 ? 240 : w->Height);
        Delay(50);
        printf("INFO RAM:WBGap's window sized to %dx%d for F1 and F2\n", w->Width, w->Height);
    }
    Delay(100);   /* its icons drawn */
    before = inside_sum(w);
    write_file("RAM:WBGap/new1", "new\n");
    put_icon("RAM:WBGap/new1", WBPROJECT, NULL);
    if (changes_within(w, before, 5)) pass("F1"); else fail("F1", "a new file with an icon didn't show in 5 s");
    Delay(50);
    if (!(w = find_window("WBGap"))) { skip("F2", "window gone"); return 5; }
    before = inside_sum(w);
    put_icon("RAM:WBGap/a", WBTOOL, NULL);   /* "a"'s icon now a tool's image */
    if (changes_within(w, before, 5)) pass("F2"); else fail("F2", "a changed .info didn't show in 5 s");
    return fails ? 5 : 0;
}

/* ---- workbench.library ---- */
static LONG sel_hook_fn(struct Hook *h __asm("a0"), APTR reserved __asm("a2"), struct IconSelectMsg *ism __asm("a1"))
{
    (void)reserved; (void)ism;
    (*(LONG *)h->h_Data)++;
    return ISMACTION_Ignore;
}

/* 1 when, within ticks/50 s, the item named (normalised) is (want 1) or is not (want 0) in a Tools menu of the
 * Workbench's windows */
static int tools_item_within(const char *item, int want, int ticks)
{
    char spec[64];
    snprintf(spec, sizeof spec, "tools:%s", item);
    for (;;) {
        struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
        struct Window *w;
        ULONG lock;
        int found;
        nnames = 0;
        if (wb) {
            lock = LockIBase(0);
            for (w = wb->FirstWindow; w; w = w->NextWindow)
                if ((w->Flags & WFLG_WBENCHWINDOW) && w->MenuStrip) walk_strip(w->MenuStrip, 0);
            UnlockIBase(lock);
            UnlockPubScreen(NULL, wb);
        }
        found = has(spec);
        nnames = 0;
        if (found == want) return 1;
        if (ticks <= 0) return 0;
        Delay(5);
        ticks -= 5;
    }
}

/* "FAIL <id> why" for each row of mode whose id starts with letter: the mode's checks didn't run. Named row by
 * row, not "FAIL A*", whose mode is the first A row's that has one, and A1 (APPWIN, rows/APPWIN.c) is that now */
static void fail_mode_rows(const char *mode, char letter, const char *why)
{
    int i;
    for (i = 0; i < wbgap_row_count(); i++) {
        const struct wbgap_row *r = wbgap_row(i);
        if (r->mode && !strcmp(r->mode, mode) && r->id[0] == letter) fail(r->id, why);
    }
}

static int do_api(void)
{
    struct MsgPort *port = CreateMsgPort();
    struct DiskObject *dob = GetDefDiskObject(WBTOOL);
    struct AppIcon *ai;
    struct AppMenuItem *mi;
    struct Hook hook;
    LONG seen = 0;
    BOOL ok;
    do_setup();
    if (!port || !dob) { fail_mode_rows("API", 'A', "setup"); return 20; }
    ai = AddAppIconA(1, 0, (STRPTR)"WBGapIcon", port, 0, dob, NULL);
    if (ai) { pass("A3"); RemoveAppIcon(ai); } else printf("FAIL A3 AddAppIconA NULL (IoErr %ld)\n", (long)IoErr()), fails++;
    mi = AddAppMenuItemA(2, 0, (STRPTR)"WBGapItem", port, NULL);
    if (mi) {
        pass("A4");
        /* M27: the item shows in the Tools menu of the Workbench's windows within 2 s, and goes when removed */
        if (tools_item_within("wbgapitem", 1, 100)) {
            RemoveAppMenuItem(mi);
            if (tools_item_within("wbgapitem", 0, 100)) pass("M27"); else fail("M27", "Tools/WBGapItem still shown 2 s after RemoveAppMenuItem");
        } else { RemoveAppMenuItem(mi); fail("M27", "Tools/WBGapItem not in the Workbench windows' menus 2 s after AddAppMenuItemA"); }
    } else { printf("FAIL A4 AddAppMenuItemA NULL (IoErr %ld)\n", (long)IoErr()); fails++; fail("M27", "no AppMenuItem (A4)"); }
    ok = OpenWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) pass("A6"); else printf("FAIL A6 OpenWorkbenchObjectA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    Delay(50);
    {   /* UpdateWorkbench (A7): a file and icon made, then UpdateWorkbench; the drawer's window must show it at once,
         * sooner than the 200 ms GFOpenBench's own notify waits, and IoErr stays as it was */
        BPTR dir;
        struct Window *w = NULL;
        ULONG before = 0;
        int t;
        DeleteFile((CONST_STRPTR)"RAM:WBGap/new2");
        DeleteFile((CONST_STRPTR)"RAM:WBGap/new2.info");
        for (t = 0; t < 30 && !(w = find_window("WBGap")); t++) Delay(10);
        if (w) { Delay(100); before = inside_sum(w); }
        write_file("RAM:WBGap/new2", "new\n");
        put_icon("RAM:WBGap/new2", WBPROJECT, NULL);
        dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
        SetIoErr(0);
        UpdateWorkbench((CONST_STRPTR)"new2", dir, UPDATEWB_ObjectAdded);
        printf("INFO UpdateWorkbench IoErr %ld\n", (long)IoErr());
        if (IoErr() != 0) fail("A7", "UpdateWorkbench set IoErr");
        else if (!w) fail("A7", "RAM:WBGap's window isn't open");
        else {
            Delay(5);
            if (inside_sum(w) != before) pass("A7");
            else fail("A7", "the drawer didn't show the new icon 100 ms after UpdateWorkbench");
        }
        if (dir) UnLock(dir);
    }
    {   /* WBInfo (A9; N8: a program's WBInfo opens it): TRUE, and its window comes; then closed (Ctrl-C) */
        BPTR dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
        struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
        struct Window *iw = NULL;
        ULONG r;
        int t;
        r = dir ? WBInfo(dir, (CONST_STRPTR)"a", wb) : 0;
        for (t = 0; r && t < 30 && !(iw = find_window("Information: a")); t++) Delay(10);
        if (r && iw) { pass("A9"); pass("N8"); }
        else {
            printf("FAIL A9 WBInfo %s, %s (IoErr %ld)\n", r ? "TRUE" : "FALSE", iw ? "window open" : "no window", (long)IoErr()), fails++;
            printf("FAIL N8 WBInfo from a program opened no Information window\n"), fails++;
        }
        if (iw && iw->UserPort && iw->UserPort->mp_SigTask) Signal((struct Task *)iw->UserPort->mp_SigTask, SIGBREAKF_CTRL_C);
        if (wb) UnlockPubScreen(NULL, wb);
        if (dir) UnLock(dir);
    }
    ok = MakeWorkbenchObjectVisibleA((STRPTR)"RAM:WBGap/a", NULL);
    if (ok) pass("A8a"); else printf("FAIL A8a MakeWorkbenchObjectVisibleA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    memset(&hook, 0, sizeof hook);
    hook.h_Entry = (HOOKFUNC)sel_hook_fn;
    hook.h_Data = &seen;
    ok = ChangeWorkbenchSelectionA((STRPTR)"RAM:WBGap", &hook, NULL);
    if (ok && seen > 0) pass("A8b"); else printf("FAIL A8b ChangeWorkbenchSelectionA %s, %ld icons seen (IoErr %ld)\n", ok ? "TRUE" : "FALSE", (long)seen, (long)IoErr()), fails++;
    {   /* A8c, WhichWorkbenchObjectA: RAM:WBGapW's icon w, snapshotted at 20,10, asked about on its image and on the
         * window's empty bottom-right corner (WBO_ICON named "w", then WBO_DRAWER) */
#ifdef WBOBJA_Name
        BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapW");
        struct DiskObject *d;
        struct Window *ww;
        int how;
        if (l) UnLock(l);
        put_icon("RAM:WBGapW", WBDRAWER, NULL);
        write_file("RAM:WBGapW/w", "w\n");
        if ((d = GetDefDiskObject(WBPROJECT))) {
            d->do_CurrentX = 20;
            d->do_CurrentY = 10;
            PutDiskObject((CONST_STRPTR)"RAM:WBGapW/w", d);
            FreeDiskObject(d);
        }
        if (!(ww = open_drawer("RAM:WBGapW", "WBGapW", 10, &how))) fail("A8c", "RAM:WBGapW's window didn't open");
        else {
            char name[64] = "", why[96];
            ULONG on_icon = WBO_NONE, on_empty = WBO_NONE, t1 = WBO_NONE, t2 = WBO_NONE;
            long xy[2];
            struct TagItem tg1[] = {{WBOBJA_Type, (ULONG)&t1}, {WBOBJA_Name, (ULONG)name}, {WBOBJA_NameSize, sizeof name},
                                    {TAG_DONE, 0}};
            struct TagItem tg2[] = {{WBOBJA_Type, (ULONG)&t2}, {TAG_DONE, 0}};
            Delay(25);
            wbgap_which_point(ww->BorderLeft, ww->BorderTop, 20, 10, xy);
            on_icon = WhichWorkbenchObjectA(ww, xy[0], xy[1], tg1);
            on_empty = WhichWorkbenchObjectA(ww, ww->Width - ww->BorderRight - 6, ww->Height - ww->BorderBottom - 6, tg2);
            printf("INFO A8c on the icon at %ld,%ld: %lu (type %lu, \"%s\"); on the empty corner: %lu (type %lu)\n", xy[0], xy[1],
                   (unsigned long)on_icon, (unsigned long)t1, name, (unsigned long)on_empty, (unsigned long)t2);
            if (wbgap_which_judge(on_icon, name, "w", on_empty, why, sizeof why) == WBGAP_PASS) pass("A8c");
            else fail("A8c", why);
            clear_drawer("RAM:WBGapW", "WBGapW", "A8c");   /* closed, else parked off the desktop rows */
        }
        DeleteFile((CONST_STRPTR)"RAM:WBGapW/w.info");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW/w");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW.info");
        DeleteFile((CONST_STRPTR)"RAM:WBGapW");
#else
        skip("A8c", "built without workbench.h's WBOBJA_ tags");
#endif
    }
    ok = CloseWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) pass("A6b"); else printf("FAIL A6b CloseWorkbenchObjectA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    FreeDiskObject(dob);
    DeleteMsgPort(port);
    return fails ? 5 : 0;
}

/* ---- screens ---- */
static int run(const char *cmd) { return SystemTags((CONST_STRPTR)cmd, TAG_DONE); }

/* a square of a screen's own pixels, as one number: what the screen holds, never the pointer if that is a sprite */
static ULONG screen_box_sum(struct Screen *s, int cx, int cy, int half)
{
    struct RastPort *rp = &s->RastPort;
    int deep = CyberGfxBase && GetBitMapAttr(rp->BitMap, BMA_DEPTH) > 8;
    ULONG sum = 0;
    int x, y;
    for (y = cy - half; y < cy + half; y++)
        for (x = cx - half; x < cx + half; x++)
            sum = sum * 31 + (deep ? rgb_pixel(rp, x, y) : (ULONG)ReadPixel(rp, x, y));
    return sum;
}

/* P4: Intuition's pointer, as 3.2.3 (P4a: it goes where it is moved); on RTG a sprite layer (P4b): with the pointer
 * over a square of the Workbench screen, the screen's own pixels there are what they are with it away */
static void do_pointer(struct Screen *wb)
{
    char cmd[320];
    int cx = wb->Width / 2, cy = wb->Height / 2, half = 12;
    ULONG id = GetVPModeID(&wb->ViewPort), away1, over, away2;
    ScreenToFront(wb);
    snprintf(cmd, sizeof cmd, "%s MOVE %d %d", dragtest, wb->Width - 30, wb->Height - 30);
    run(cmd);
    Delay(25);
    away1 = screen_box_sum(wb, cx, cy, half);
    snprintf(cmd, sizeof cmd, "%s MOVE %d %d", dragtest, cx - 4, cy - 4);
    run(cmd);
    Delay(25);
    if (wb->MouseX >= cx - 5 && wb->MouseX <= cx - 3 && wb->MouseY >= cy - 5 && wb->MouseY <= cy - 3) pass("P4a");
    else { char why[64]; snprintf(why, sizeof why, "moved to %d,%d, the screen says %d,%d", cx - 4, cy - 4, wb->MouseX, wb->MouseY); fail("P4a", why); }
    over = screen_box_sum(wb, cx, cy, half);
    snprintf(cmd, sizeof cmd, "%s MOVE %d %d", dragtest, wb->Width - 30, wb->Height - 30);
    run(cmd);
    Delay(25);
    away2 = screen_box_sum(wb, cx, cy, half);
    printf("INFO P4 mode $%08lx, box %08lx / %08lx / %08lx\n", (unsigned long)id, (unsigned long)away1, (unsigned long)over, (unsigned long)away2);
    if (id == INVALID_ID || (id & 0xFFFF1000UL) < 0x00100000UL) skip("P4b", "Workbench is not on an RTG mode (the chipset's sprites)");
    else if (away1 != away2) skip("P4b", "the screen changed under the square by itself; run again");
    else if (over != away1) fail("P4b", "the pointer is drawn into the screen's pixels, not a sprite layer");
    else pass("P4b");
}

static int do_screens(void)
{
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench"), *s;
    struct Gadget *g;
    char cmd[320];
    int x = -1, y = -1;
    if (wb) pass("C4"); else { fail("C4", "LockPubScreen(Workbench) NULL"); return 20; }
    do_pointer(wb);
    s = OpenScreenTags(NULL, SA_LikeWorkbench, TRUE, SA_Title, (ULONG)"WBGapScreen", SA_ShowTitle, TRUE, TAG_DONE);
    if (!s) { fail("C1", "no test screen"); skip("C2", "no test screen"); UnlockPubScreen(NULL, wb); return 10; }
    ScreenToFront(s);
    Delay(50);
    for (g = s->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_SDEPTH) {
            x = ((g->Flags & GFLG_RELRIGHT) ? s->Width - 1 + g->LeftEdge : g->LeftEdge) + g->Width / 2;
            y = g->TopEdge + g->Height / 2;
        }
    if (x < 0) fail("C1", "the screen has no depth gadget");
    else {
        snprintf(cmd, sizeof cmd, "%s CLICK %d %d", dragtest, x, y);
        run(cmd);
        Delay(50);
        if (IntuitionBase->FirstScreen != s) pass("C1"); else fail("C1", "the depth gadget left the screen in front");
    }
    ScreenToFront(s);
    Delay(50);
    snprintf(cmd, sizeof cmd, "%s KEY 0x37 QUAL LAMIGA", dragtest);
    run(cmd);   /* M: the front screen to the back */
    Delay(50);
    if (IntuitionBase->FirstScreen != s) pass("C2a"); else fail("C2a", "Left-Amiga-M left the front screen in front");
    ScreenToFront(s);
    Delay(50);
    snprintf(cmd, sizeof cmd, "%s KEY 0x36 QUAL LAMIGA", dragtest);
    run(cmd);   /* N: Workbench to the front */
    Delay(50);
    if (IntuitionBase->FirstScreen == wb) pass("C2b"); else fail("C2b", "Left-Amiga-N didn't bring Workbench to the front");
    CloseScreen(s);
    UnlockPubScreen(NULL, wb);
    return fails ? 5 : 0;
}

/* ---- the M rows driven: each item picked with the mouse, as a hand picks it, and what it did looked at ----
 * In RAM:WBGap's drawer window (the script opens it). The pointer goes title, item (and sub-item) with the right
 * button held (wbgap_menu_path, DragTest DRAG RIGHT); what changed is read: the window's pixels, a window that
 * comes or goes, a drawer made. Only reads and input events: nothing is drawn here. */
#define MAXWINS 48

static long read_all(const char *path, char *buf, long n);

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

static int windows_now(struct Screen *s, struct Window **out, int n)
{
    struct Window *w;
    int k = 0;
    ULONG lock = LockIBase(0);
    for (w = s->FirstWindow; w && k < n; w = w->NextWindow) out[k++] = w;
    UnlockIBase(lock);
    return k;
}

/* a window on s that wasn't in was[0..n), within secs */
static struct Window *new_window(struct Screen *s, struct Window **was, int n, int secs)
{
    struct Window *now[MAXWINS];
    int t, k, j;
    for (t = 0; t < secs * 10 || t == 0; t++) {
        int m = windows_now(s, now, MAXWINS);
        for (k = 0; k < m; k++) {
            for (j = 0; j < n && was[j] != now[k]; j++) {}
            if (j == n) return now[k];
        }
        if (secs) Delay(5);
    }
    return NULL;
}

static int still_there(struct Screen *s, struct Window *w)
{
    struct Window *now[MAXWINS];
    int m = windows_now(s, now, MAXWINS), k;
    for (k = 0; k < m; k++) if (now[k] == w) return 1;
    return 0;
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
        norm(mn, (const char *)m->MenuName, sizeof mn);
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
                if (!item_text(it)) continue;
                norm(t, item_text(it), sizeof t);
                if (strcmp(t, want)) continue;
                mb->left = m->LeftEdge, mb->top = 0, mb->width = m->Width, mb->height = m->Height;
                ib->left = it->LeftEdge, ib->top = it->TopEdge, ib->width = it->Width, ib->height = it->Height;
                *has_sub = 0;
                if (!wsub[0]) return (it->Flags & ITEMENABLED) && (m->Flags & MENUENABLED) ? 1 : -1;
                for (sub = it->SubItem; sub; sub = sub->NextItem) {
                    char st[40];
                    if (!item_text(sub)) continue;
                    norm(st, item_text(sub), sizeof st);
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
    k = snprintf(cmd, sizeof cmd, "%s DRAG", dragtest);
    for (found = 0; found < n; found++) k += snprintf(cmd + k, sizeof cmd - k, " %d %d", xy[2 * found], xy[2 * found + 1]);
    snprintf(cmd + k, sizeof cmd - k, " RIGHT STEPS 6 HOLD 10");
    printf("INFO picked %s: %s\n", spec, cmd + strlen(dragtest) + 1);
    run(cmd);
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
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", dragtest, (long)bx, (long)by);
    run(cmd);
    Delay(25);
    return 1;
}

static void key(int code)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "%s KEY 0x%x", dragtest, code);
    run(cmd);
    Delay(15);
}

/* ---- INFOSAVE: rows N2-N5, the Information window's Save, driven as a hand does ---- */
/* the box (window-relative) of every gadget of w with this id, joined: a GadTools listview is more than one */
static int gad_box(struct Window *w, UWORD id, LONG *x, LONG *y, LONG *gw, LONG *gh)
{
    struct Gadget *g;
    LONG x0 = 0x7fff, y0 = 0x7fff, x1 = -1, y1 = -1;
    ULONG lock = LockIBase(0);
    for (g = w->FirstGadget; g; g = g->NextGadget) {
        LONG l, t;
        if ((g->GadgetType & GTYP_SYSGADGET) || g->GadgetID != id || g->Width <= 0 || g->Height <= 0) continue;
        l = (g->Flags & GFLG_RELRIGHT) ? w->Width - 1 + g->LeftEdge : g->LeftEdge;
        t = (g->Flags & GFLG_RELBOTTOM) ? w->Height - 1 + g->TopEdge : g->TopEdge;
        if (l < x0) x0 = l;
        if (t < y0) y0 = t;
        if (l + g->Width > x1) x1 = l + g->Width;
        if (t + g->Height > y1) y1 = t + g->Height;
    }
    UnlockIBase(lock);
    if (x1 < 0) return 0;
    *x = x0, *y = y0, *gw = x1 - x0, *gh = y1 - y0;
    return 1;
}

/* a click on gadget id of w: its middle, or (row >= 0, a listview) the middle of that row, near its left */
static int click_gad(struct Window *w, UWORD id, int row)
{
    LONG x, y, gw, gh, fh = w->WScreen->Font ? w->WScreen->Font->ta_YSize : 8;
    char cmd[300];
    if (!gad_box(w, id, &x, &y, &gw, &gh)) return 0;
    if (row >= 0) x += 24, y += 2 + row * fh + fh / 2;
    else x += gw / 2, y += gh / 2;
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", dragtest, (long)(w->LeftEdge + x), (long)(w->TopEdge + y));
    run(cmd);
    Delay(20);
    return 1;
}

static void key_q(int code, const char *qual)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "%s KEY 0x%x QUAL %s", dragtest, code, qual);
    run(cmd);
    Delay(15);
}

/* text typed on a US keyboard (wbgap_rawkey: a-z, A-Z with Shift, 0-9 - = . / space); 0 at a character it has no key for */
static int type_text(const char *t)
{
    for (; *t; t++) {
        int shift, code = wbgap_rawkey((unsigned char)*t, &shift);
        if (code < 0) return 0;
        if (shift) key_q(code, "LSHIFT"); else key(code);
    }
    return 1;
}

/* the string gadget id of w emptied (Right-Amiga X) and t typed into it; Return after it when ret */
static int set_string(struct Window *w, UWORD id, const char *t, int ret)
{
    if (!click_gad(w, id, -1)) return 0;
    key_q(0x32, "RAMIGA");
    if (!type_text(t)) return 0;
    if (ret) key(0x44);
    return 1;
}

static int infosave_row(void)
{
    static const char *const rows[] = { "N2", "N3", "N4", "N5" };
    static STRPTR tt[] = { (STRPTR)"a=1", (STRPTR)"d=4", NULL };
    struct FileInfoBlock *fib;
    struct DiskObject *dob;
    struct Screen *wb;
    struct Window *iw = NULL;
    BPTR dir, l;
    int t, i, done;
    if (!is_gfos()) {
        for (i = 0; i < 4; i++) skip(rows[i], "drives GFOpenBench's Information window by its gadget ids; 3.2.3's is checked by hand");
        return 0;
    }
    /* RAM:WBGapN/n: a project, every protection bit as a new file has it, no comment, no default tool,
     * tool types a=1 and d=4 */
    if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapN"))) UnLock(l);
    /* a rerun: the last run's Save left it not deletable, so clear that before it is written again */
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);
    write_file("RAM:WBGapN/n", "WBGapTest INFOSAVE\n");
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);
    SetComment((CONST_STRPTR)"RAM:WBGapN/n", (CONST_STRPTR)"");
    if (!put_icon("RAM:WBGapN/n", WBPROJECT, tt)) {
        for (i = 0; i < 4; i++) fail(rows[i], "RAM:WBGapN/n.info could not be written");
        return 10;
    }
    if ((dob = GetDiskObject((CONST_STRPTR)"RAM:WBGapN/n"))) {   /* a default project icon may name a tool: none here */
        dob->do_DefaultTool = (STRPTR)"";
        PutDiskObject((CONST_STRPTR)"RAM:WBGapN/n", dob);
        FreeDiskObject(dob);
    }
    dir = Lock((CONST_STRPTR)"RAM:WBGapN", ACCESS_READ);
    wb = LockPubScreen((CONST_STRPTR)"Workbench");
    if (dir && wb && WBInfo(dir, (CONST_STRPTR)"n", wb))
        for (t = 0; t < 30 && !(iw = find_window("Information: n")); t++) Delay(10);
    if (wb) UnlockPubScreen(NULL, wb);
    if (dir) UnLock(dir);
    if (!iw) {
        for (i = 0; i < 4; i++) fail(rows[i], "WBInfo opened no Information window for RAM:WBGapN/n");
        return 10;
    }
    Delay(50);   /* drawn */
    printf("INFO INFOSAVE Information window %ldx%ld at %ld,%ld\n", (long)iw->Width, (long)iw->Height, (long)iw->LeftEdge, (long)iw->TopEdge);
    /* N2: Deletable off. N3: a comment. N5: a default tool. N4: a=1 made a=3, d=4 deleted, b=2 added. */
    done = click_gad(iw, G_PD, -1)
        && set_string(iw, G_COMMENT, "gapnote", 0)
        && set_string(iw, G_TOOL, "dir", 0)
        && click_gad(iw, G_TTLIST, 0) && (key_q(0x32, "RAMIGA"), type_text("a=3")) && (key(0x44), 1)
        && click_gad(iw, G_TTLIST, 1) && click_gad(iw, G_TTDEL, -1)
        && click_gad(iw, G_TTNEW, -1) && type_text("b=2") && (key(0x44), 1)
        && click_gad(iw, G_SAVE, -1);
    for (t = 0; t < 30 && find_window("Information: n"); t++) Delay(10);
    if ((iw = find_window("Information: n"))) {   /* not saved (a requester?): closed unsaved */
        printf("INFO INFOSAVE the window is still open after Save%s\n", done ? "" : " (a gadget wasn't found)");
        if (iw->UserPort && iw->UserPort->mp_SigTask) Signal((struct Task *)iw->UserPort->mp_SigTask, SIGBREAKF_CTRL_C);
        for (i = 0; i < 4; i++) fail(rows[i], "Save didn't close the Information window");
        return 10;
    }
    l = 0;
    if ((fib = AllocDosObject(DOS_FIB, NULL)) && (l = Lock((CONST_STRPTR)"RAM:WBGapN/n", ACCESS_READ)) && Examine(l, fib)) {
        LONG p = fib->fib_Protection;
        if ((p & FIBF_DELETE) && !(p & (FIBF_READ | FIBF_WRITE | FIBF_EXECUTE))) pass("N2");
        else printf("FAIL N2 protection 0x%lx after Save, not Deletable off with R, W and E on\n", (unsigned long)p), fails++;
        if (!strcmp((const char *)fib->fib_Comment, "gapnote")) pass("N3");
        else printf("FAIL N3 the comment is \"%s\" after Save, not \"gapnote\"\n", (const char *)fib->fib_Comment), fails++;
    } else {
        fail("N2", "RAM:WBGapN/n can't be examined");
        fail("N3", "RAM:WBGapN/n can't be examined");
    }
    if (l) UnLock(l);
    if (fib) FreeDosObject(DOS_FIB, fib);
    if ((dob = GetDiskObject((CONST_STRPTR)"RAM:WBGapN/n"))) {
        STRPTR *x = dob->do_ToolTypes;
        char got[160] = "";
        for (i = 0; x && x[i]; i++) {
            if (i) strncat(got, ",", sizeof got - strlen(got) - 1);
            strncat(got, (const char *)x[i], sizeof got - strlen(got) - 1);
        }
        if (!strcmp(got, "a=3,b=2")) pass("N4");
        else printf("FAIL N4 tool types \"%s\" after Save, not \"a=3,b=2\" (a=1 edited, d=4 deleted, b=2 added)\n", got), fails++;
        if (dob->do_DefaultTool && !strcmp((const char *)dob->do_DefaultTool, "dir")) pass("N5");
        else printf("FAIL N5 the default tool is \"%s\" after Save, not \"dir\"\n", dob->do_DefaultTool ? (const char *)dob->do_DefaultTool : "(none)"), fails++;
        FreeDiskObject(dob);
    } else {
        fail("N4", "RAM:WBGapN/n.info can't be read");
        fail("N5", "RAM:WBGapN/n.info can't be read");
    }
    return fails ? 10 : 0;
}

/* RAM:WBGapN is gone after every run, whichever way it ended (Lyta, #150) */
static int do_infosave(void)
{
    int rc = infosave_row();
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);   /* the Save left it not deletable */
    DeleteFile((CONST_STRPTR)"RAM:WBGapN/n.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapN/n");
    DeleteFile((CONST_STRPTR)"RAM:WBGapN");
    return rc;
}

/* waits up to secs for w's inside to read as sum again */
static int back_to(struct Window *w, ULONG sum, int secs)
{
    int t;
    for (t = 0; t < secs * 5; t++) {
        if (inside_sum(w) == sum) return 1;
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
    skip(row, r < 0 ? "its item is disabled here" : "its item isn't in the Workbench windows' menus (MENUS says)");
}

static int do_menudrive(void)
{
    struct Window *w = drawer_window(), *was[MAXWINS], *nw;
    struct Screen *s;
    ULONG before;
    int r, nwas, k, n0, n1, i14_only;
    static char d0[32][32], d1[32][32];
    if (!w) {
        int how;
        open_drawer("RAM:WBGap", "WBGap", 5, &how);
        w = drawer_window();
    }
    if (!w) { fail("M*", "RAM:WBGap's drawer window isn't open (WBGapTest SETUP, DragTest OPEN RAM:WBGap)"); return 20; }
    s = w->WScreen;
    printf("INFO desktop: %s\n", is_gfos() ? "GFOpenBench (GFOS rows)" : "not GFOpenBench (3.2.3 rows)");

    /* M12 Select Contents, then M30 Clear Selection: the window back as it was */
    before = inside_sum(w);
    if ((r = pick(w, "*:select contents|select all")) <= 0) not_picked("M12", r);
    else if (changes_within(w, before, 3)) {
        pass("M12");
        if ((r = pick(w, "window:clear selection")) <= 0) not_picked("M30", r);
        else if (back_to(w, before, 3)) pass("M30");
        else fail("M30", "Clear Selection: the window didn't go back to nothing selected");
    } else {
        fail("M12", "Select Contents: nothing in the window changed");
        skip("M30", "nothing selected to clear (M12)");
    }
    pick(w, "window:clear selection");

    /* M15 Show > All Files: RAM:WBGap/noicon, a file with no icon, comes; then Only Icons */
    write_file("RAM:WBGap/noicon", "WBGapTest M15\n");
    Delay(50);
    i14_only = icons_have("I14", "RAM:WBGap", "noicon");
    before = inside_sum(w);
    if ((r = pick(w, "*:show/all files|show all files")) <= 0) { not_picked("M15", r); skip("I14", "Show All Files not picked (M15)"); }
    else {
        char why[64];
        int v;
        if (changes_within(w, before, 3)) pass("M15");
        else fail("M15", "Show All Files: nothing in the window changed (RAM:WBGap/noicon has no icon)");
        v = wbgap_showall_judge(i14_only, icons_have("I14", "RAM:WBGap", "noicon"), why, sizeof why);
        if (v == WBGAP_PASS) pass("I14");
        else if (v == WBGAP_SKIP) skip("I14", why);
        else fail("I14", why);
    }
    pick(w, "*:show/only icons|show only icons");
    Delay(25);

    /* M16 View By > Name: the window as a list; then by icon again */
    before = inside_sum(w);
    if ((r = pick(w, "*:view by/name|as a list")) <= 0) not_picked("M16", r);
    else if (changes_within(w, before, 3)) pass("M16");
    else fail("M16", "View By Name: nothing in the window changed");
    pick(w, "*:view by/icon|as icons");
    Delay(25);

    /* M6 About: a window comes; its button closes it */
    nwas = windows_now(s, was, MAXWINS);
    if ((r = pick_any(w, "workbench|gfopenbench:about")) <= 0) not_picked("M6", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        pass("M6");
        click_button(nw, 0);
        if (still_there(s, nw)) key(0x45);
    } else fail("M6", "About: no window came");

    /* M2 Execute Command: its requester comes; Cancel */
    nwas = windows_now(s, was, MAXWINS);
    if ((r = pick_any(w, "workbench|gfopenbench:execute command")) <= 0) not_picked("M2", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        pass("M2");
        click_button(nw, 1);
        if (still_there(s, nw)) key(0x45);
    } else fail("M2", "Execute Command: no requester came");

    /* M8 New Drawer: its requester, Return takes the name it offers, a drawer is made */
    n0 = dirs_in("RAM:WBGap", d0, 32);
    nwas = windows_now(s, was, MAXWINS);
    if ((r = pick(w, "window:new drawer")) <= 0) not_picked("M8", r);
    else {
        int t, made = -1;
        nw = NULL;
        for (t = 0; t < 30 && made < 0; t++) {   /* a requester for its name, or (no requester) the drawer at once */
            int j, i;
            if (!nw && (nw = new_window(s, was, nwas, 0))) {
                printf("INFO M8 a requester came: Return takes the name it offers\n");
                key(0x44);   /* Return */
            }
            Delay(5);
            n1 = dirs_in("RAM:WBGap", d1, 32);
            for (i = 0; i < n1 && made < 0; i++) {
                for (j = 0; j < n0 && strcmp(d0[j], d1[i]); j++) {}
                if (j == n0) made = i;
            }
        }
        if (made >= 0) {
            char p[64];
            pass("M8");
            printf("INFO M8 made RAM:WBGap/%s%s\n", d1[made], nw ? "" : " (no requester)");
            snprintf(p, sizeof p, "RAM:WBGap/%s.info", d1[made]);
            DeleteFile((CONST_STRPTR)p);
            snprintf(p, sizeof p, "RAM:WBGap/%s", d1[made]);
            DeleteFile((CONST_STRPTR)p);
        } else fail("M8", nw ? "New Drawer: Return in its requester made no drawer" : "New Drawer: no requester, no drawer made");
        if (nw && still_there(s, nw)) click_button(nw, 1);   /* Cancel */
    }

    /* M9 Open Parent: RAM:'s window comes */
    nwas = windows_now(s, was, MAXWINS);
    if ((r = pick(w, "window:open parent")) <= 0) not_picked("M9", r);
    else if ((nw = new_window(s, was, nwas, 3))) {
        pass("M9");
        printf("INFO M9 \"%s\" came\n", nw->Title ? (const char *)nw->Title : "(no title)");
    } else if (still_there(s, w) && w->Title && !strstr((const char *)w->Title, "WBGap")) {
        pass("M9");   /* GFOpenBench's windows go back and forward: the parent in the same window */
        printf("INFO M9 the same window now shows \"%s\"\n", (const char *)w->Title);
    } else {
        struct Window *act = IntuitionBase->ActiveWindow;   /* RAM:'s window open already: to the front, active */
        if (act && act != w && (act->Flags & WFLG_WBENCHWINDOW) && act->Title && !strstr((const char *)act->Title, "WBGap")) {
            pass("M9");
            printf("INFO M9 \"%s\", open already, made active\n", (const char *)act->Title);
        } else fail("M9", "Open Parent: no window came or became active, and the window still shows RAM:WBGap");
    }

    /* M10 Close, last: the drawer's window goes; opened again for what comes after */
    if ((r = pick(w, "window:close")) <= 0) not_picked("M10", r);
    else {
        for (k = 0; k < 30 && still_there(s, w); k++) Delay(5);
        if (!still_there(s, w)) pass("M10"); else fail("M10", "Close: the drawer's window stayed");
    }
    {
        int how;
        if (!drawer_window()) open_drawer("RAM:WBGap", "WBGap", 5, &how);
    }
    return fails ? 5 : 0;
}

/* ---- 1000 files ---- */
static int do_big(void)
{
    char path[64];
    struct Window *w;
    struct DateStamp a, b;
    BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapBig");
    int i, how;
    long ticks;
    if (l) UnLock(l);
    put_icon("RAM:WBGapBig", WBDRAWER, NULL);
    for (i = 0; i < 1000; i++) {
        snprintf(path, sizeof path, "RAM:WBGapBig/file%04d", i);
        write_file(path, "x\n");
        put_icon(path, WBPROJECT, NULL);
    }
    DateStamp(&a);
    w = open_drawer("RAM:WBGapBig", "WBGapBig", 60, &how);
    if (!w) {
        fail("X1", "RAM:WBGapBig's window didn't open in 60 s");
        clear_drawer("RAM:WBGapBig", "WBGapBig", "X1");   /* in case it opens late, over the desktop */
        return 10;
    }
    {   /* drawn: the inside stops changing for a second */
        ULONG last = inside_sum(w), now;
        int still = 0, t;
        for (t = 0; t < 600 && still < 5; t++) { Delay(10); now = inside_sum(w); still = now == last ? still + 1 : 0; last = now; }
    }
    DateStamp(&b);
    ticks = (b.ds_Days - a.ds_Days) * 24L * 60 * 3000 + (b.ds_Minute - a.ds_Minute) * 3000L + (b.ds_Tick - a.ds_Tick) - 50;
    printf("INFO X1 1000 icons shown in %ld.%02ld s\n", ticks / 50, (ticks % 50) * 2);
    if (ticks < 10 * 50) pass("X1"); else fail("X1", "over 10 s");
    /* closed again, and waited for: left open over the desktop's left-out icons, the next rows' clicks landed on its
     * file icons and stopped an unattended run on "has no default tool" (Franklin's lab, 14 November 2026) */
    clear_drawer("RAM:WBGapBig", "WBGapBig", "X1");
    return fails ? 5 : 0;
}

static long read_all(const char *path, char *buf, long n);

/* ---- D5: the RAM Disk's icon on the desktop ----
 * WBGapRoot.rexx wrote the desktop's icons to RAM:wbgap-root ("<type> <name>" a line); RAM:'s volume name comes from
 * NameFromLock ("Ram Disk:"). */
static int do_disks(void)
{
    static char list[8192];
    char path[108], vol[40];
    BPTR l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ);
    long n;
    if (!l || !NameFromLock(l, (STRPTR)path, sizeof path)) {
        if (l) UnLock(l);
        DeleteFile((CONST_STRPTR)"RAM:wbgap-root");
        fail("D5", "RAM: has no volume name");
        fail("D5r", "RAM: has no volume name");
        return 5;
    }
    UnLock(l);
    wbgap_volume_name(path, vol, sizeof vol);
    if ((n = read_all("RAM:wbgap-root", list, sizeof list)) < 0) {
        skip("D5", "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
        skip("D5r", "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
        return 0;
    }
    printf("INFO D5 RAM: is \"%s\"; %ld bytes of the desktop's icons\n", vol, n);
    if (wbgap_root_has_disk(list, vol)) pass("D5");
    else {
        char why[128];
        if (n == 0) snprintf(why, sizeof why, "the port gave no icons");
        else if (wbgap_has_nocase(list, vol)) snprintf(why, sizeof why, "\"%s\" is there but not as a DISK (the port's .type)", vol);
        else snprintf(why, sizeof why, "no disk icon \"%s\" on the desktop", vol);
        fail("D5", why);
    }
    DeleteFile((CONST_STRPTR)"RAM:wbgap-root");
    /* D5r: RAM Disk first in the volume column (Dale, 11 Nov), from the disks' places WBGapRoot.rexx wrote */
    if ((n = read_all("RAM:wbgap-root-pos", list, sizeof list)) < 0)
        skip("D5r", "the disks' places not read (WBGapRoot.rexx)");
    else {
        char top[40], why[128];
        int r = wbgap_ram_first(list, vol, top, sizeof top);
        printf("INFO D5r the column's top disk is \"%s\"\n", top);
        if (r > 0) pass("D5r");
        else if (r < 0) fail("D5r", "no disk icon's place read");
        else {
            snprintf(why, sizeof why, "the column starts with \"%s\", not \"%s\"", top, vol);
            fail("D5r", why);
        }
        DeleteFile((CONST_STRPTR)"RAM:wbgap-root-pos");
    }
    return fails ? 5 : 0;
}

/* ---- C6: CloseWorkBench / OpenWorkBench with programs running ---- */
static int desk_port(void)
{
    int up;
    Forbid();
    up = FindPort((CONST_STRPTR)"GFOPENBENCH") || FindPort((CONST_STRPTR)"WORKBENCH");
    Permit();
    return up;
}

static struct Screen *wb_screen_now(void)
{
    struct Screen *s = LockPubScreen((CONST_STRPTR)"Workbench");
    if (s) UnlockPubScreen(NULL, s);              /* not held: a lock would stop CloseWorkBench() */
    return s;
}

static int closewb_row(void)
{
    struct Screen *before, *after;
    struct Window *w;
    LONG closed;
    int how, reopened = 0, t, port = 0, drawer = 0, v;
    char why[96];
    BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapC");
    if (l) UnLock(l);
    put_icon("RAM:WBGapC", WBDRAWER, NULL);
    if (!(w = open_drawer("RAM:WBGapC", "WBGapC", 10, &how))) { skip("C6", "RAM:WBGapC's window didn't open"); return 0; }
    if (!(before = wb_screen_now())) { skip("C6", "no Workbench screen"); return 0; }
    closed = CloseWorkBench();
    printf("INFO C6 CloseWorkBench() gave %ld\n", (long)closed);
    if (closed) {
        reopened = OpenWorkBench() != 0;
        printf("INFO C6 OpenWorkBench() %s\n", reopened ? "opened the screen again" : "failed");
    }
    for (t = 0; t < 100; t++) {                     /* up to 10 s for the desktop and its drawer to be back */
        port = desk_port();
        drawer = find_window("WBGapC") != NULL;
        if (port && drawer) break;
        Delay(5);
    }
    after = wb_screen_now();
    v = wbgap_closewb_judge(closed != 0, reopened, after == before, port, drawer, why, sizeof why);
    if (v == WBGAP_PASS) pass("C6");
    else fail("C6", why);
    clear_drawer("RAM:WBGapC", "WBGapC", "C6");
    DeleteFile((CONST_STRPTR)"RAM:WBGapC.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapC");
    return fails ? 5 : 0;
}

static int do_closewb(void)
{
    int rc = closewb_row();
    printf("INFO C6 done\n");                    /* the last line, on every way out: CLOSEWBWAIT waits for it */
    return rc;
}

/* CLOSEWBWAIT file secs: the script starts CLOSEWB with Run >file, then this
 * waits up to secs for it to be over (wbgap_closewb_over) and prints what it
 * wrote. A CloseWorkBench() that never returns (lab t2-killcrazy-wbgap, 14
 * Nov, before amicradle-guest #752) is then FAIL C6 here, and the run goes
 * on to REPORT; the CLOSEWB process is left as it is, with its own code. */
static int do_closewb_wait(const char *path, int secs)
{
    static char buf[4096];
    int t, over = 0;
    long n = 0;
    BPTR fh;
    for (t = 0; t <= secs * 2 && !over; t++) {
        if (t) Delay(25);
        buf[0] = 0;
        if ((fh = Open((CONST_STRPTR)path, MODE_OLDFILE))) {
            n = Read(fh, buf, sizeof buf - 1);
            buf[n > 0 ? n : 0] = 0;
            Close(fh);
        }
        over = wbgap_closewb_over(buf);
    }
    if (buf[0]) printf("%s%s", buf, buf[strlen(buf) - 1] == '\n' ? "" : "\n");
    if (!over) {
        char why[96];
        snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running)", secs);
        fail("C6", why);
    }
    return fails ? 5 : 0;
}

/* ---- the system bar: B1 and B2, GFOpenTitle's Power list (gfopenprefs TitleBar) ----
 * The row window GFOpenTitle keeps at the title bar's right end: its task's widest-reaching borderless window at the
 * top, no taller than the bar. Power's cell is its last (wbgap_power_point). The list is clicked open, looked at and
 * closed with Esc: nothing in it is ever picked (Reboot and Power off are in it). */
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

static void say_row(const char *row, int state, const char *why)
{
    if (state == WBGAP_PASS) pass(row);
    else if (state == WBGAP_FAIL) fail(row, why);
    else skip(row, why);
}

static int bar_rows(void)
{
    struct Screen *s;
    struct MsgPort *p;
    struct Task *t;
    struct Window *row, *was[MAXWINS], *m;
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
    if (k) pass("B3");
    else if (t) fail("B3", "GFOpenSpeaker isn't running (no GFOpenSpeaker port) though the bar is GFOpenTitle's");
    else skip("B3", "no GFOpenTitle bar (not GFOS)");
    if (!t) {
        skip("B1", "GFOpenTitle isn't running (no GFOpenTitle port)");
        skip("B2", "GFOpenTitle isn't running (no GFOpenTitle port)");
        return 0;
    }
    if (!(s = LockPubScreen((CONST_STRPTR)"Workbench"))) { skip("B1", "no Workbench screen"); skip("B2", "no Workbench screen"); return 0; }
    ScreenToFront(s);
    Delay(25);
    if (!(row = bar_row(s, t))) {
        fail("B1", "GFOpenTitle's row on the title bar not found");
        skip("B2", "no Power list to read");
        UnlockPubScreen(NULL, s);
        return 5;
    }
    wbgap_power_point(row->LeftEdge, row->TopEdge, row->Width, row->Height, xy);
    rr = row->LeftEdge + row->Width;
    /* a list row: the bar font's height + 4, as menu_open measures it */
    rh = (dri = GetScreenDrawInfo(s)) && dri->dri_Font ? dri->dri_Font->tf_YSize + 4 : 0;
    if (dri) FreeScreenDrawInfo(s, dri);
    sticks = usb_sticks_now();
    n = windows_now(s, was, MAXWINS);
    snprintf(cmd, sizeof cmd, "\"%s\" CLICK %ld %ld", dragtest, xy[0], xy[1]);
    run(cmd);
    if (!(m = new_active(was, n, t, 2))) {
        fail("B1", "a click on Power opened no list");
        skip("B2", "no Power list to read");
        UnlockPubScreen(NULL, s);
        return 5;
    }
    ml = m->LeftEdge; mt = m->TopEdge; mw = m->Width; mh = m->Height;
    under = wbgap_point_in_box(xy[0], xy[1], ml, mt, mw, mh);
    printf("INFO B1 the row %d,%d %dx%d; Power clicked at %ld,%ld; a list %ld,%ld %ldx%ld; rows %ld high, %d USB volumes\n",
           row->LeftEdge, row->TopEdge, row->Width, row->Height, xy[0], xy[1], ml, mt, mw, mh, rh, sticks);
    owner = wbgap_bar_menu_owner(rr, ml, mw);
    /* Esc closes it before anything else: nothing in it is picked */
    snprintf(cmd, sizeof cmd, "%s KEY 0x45", dragtest);
    run(cmd);
    for (k = 0; k < 20 && still_there(s, m); k++) Delay(5);
    if ((esc_left = still_there(s, m))) {           /* a click on the screen's title, far from the list, closes it */
        snprintf(cmd, sizeof cmd, "\"%s\" CLICK 4 %d", dragtest, row->TopEdge + row->Height / 2);
        run(cmd);
        for (k = 0; k < 20 && still_there(s, m); k++) Delay(5);
        printf("INFO B1 Esc left the list open; a click on the screen's title %s\n",
               still_there(s, m) ? "didn't close it either" : "closed it");
    }
    wbgap_power_judge(mh, sticks, rh, &b1, &b2);
    if (under) { fail("B1", "the list opened under the pointer (a mouse-up there could pick Reboot)"); b2 = WBGAP_NONE; }
    else if (esc_left) fail("B1", "Esc left the list open");   /* B2 still read: the height was taken before */
    else if (owner == WBGAP_BAR_PREFS) { fail("B1", "a click on Power's cell opened Prefs' list"); b2 = WBGAP_NONE; }
    else if (owner == WBGAP_BAR_OTHER) { fail("B1", "the window that opened doesn't hang from Power"); b2 = WBGAP_NONE; }
    else if (owner == WBGAP_BAR_UNSURE) { skip("B1", "the list was pushed to the screen's left edge: Power's or Prefs' can't be told"); b2 = WBGAP_NONE; }
    else {
        snprintf(why, sizeof why, "the list is %ld high, not %ld (Reboot, Restart, Power off, %d Eject rows)", mh,
                 wbgap_power_height(sticks, rh), sticks);
        say_row("B1", b1, why);
    }
    if (b2 == WBGAP_NONE) skip("B2", "Power's list not read");
    else {
        if (b2 == WBGAP_SKIP) snprintf(why, sizeof why, "no USB volume mounted");
        else snprintf(why, sizeof why, "%d USB volumes: the list is %ld high, not %ld with their Eject rows", sticks, mh,
                      wbgap_power_height(sticks, rh));
        say_row("B2", b2, why);
    }
    UnlockPubScreen(NULL, s);
    return fails ? 5 : 0;
}

/* BAR brings the Workbench screen to the front; the screen that was in front goes back after */
static int do_bar(void)
{
    struct Screen *front;
    ULONG lock = LockIBase(0);
    int rc;
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

/* the desktop's icons now (WBGapRoot.rexx D1), and whether vol is a disk among them; -1 if they weren't read */
static int diskio_has_row(const char *row, const char *vol, const char *when)
{
    static char list[8192];
    char cmd[400], rexx[256], args[64];
    long n;
    snprintf(rexx, sizeof rexx, "%s", dragtest);
    *PathPart((STRPTR)rexx) = 0;
    AddPart((STRPTR)rexx, (CONST_STRPTR)"WBGapRoot.rexx", sizeof rexx);
    snprintf(args, sizeof args, "%s RAM:wbgap-root-d1", row);
    if (!wbgap_rx_line(cmd, sizeof cmd, rexx, args)) {
        printf("INFO %s WBGapRoot.rexx not run: its path \"%s\" has a space (rx can't be given one)\n", row, rexx);
        return -1;
    }
    run(cmd);
    n = read_all("RAM:wbgap-root-d1", list, sizeof list);
    DeleteFile((CONST_STRPTR)"RAM:wbgap-root-d1");
    if (n < 0) return -1;
    if (!vol || !*vol) return 0;
    printf("INFO %s %s: \"%s\" %s\n", row, when, vol, wbgap_root_has_disk(list, vol) ? "has an icon" : "has no icon");
    return wbgap_root_has_disk(list, vol);
}

static long read_all(const char *path, char *buf, long n);

/* ---- I8, I9: an icon dragged onto an AppIcon and onto an AppWindow ----
 * RAM:WBGapD's project d, made while its drawer's window is open (a already in it) and found where the window
 * changed, is dragged with DragTest onto a window of
 * our own made an AppWindow (I9, id 9), then onto an AppIcon on the desktop (I8, id 8; its place read through the
 * WORKBENCH port by WBGapAppPos.rexx). The desktop must send the AppMessage, naming d in RAM:WBGapD. */
static int drop_wait(struct MsgPort *port, long want_type, unsigned long want_id, const char *row)
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
        pass(row);
    else fail(row, why);
    return got;
}

/* d's icon in RAM:WBGapD's window: where its inside changed when d was made (wbgap_diff_box), from the inside's
 * top-left; the window's toolbar or margins, which differ from desktop to desktop, don't matter */
#define DROP_STEP 2
#define DROP_GW 300
#define DROP_GH 220
/* the inside of w sampled every DROP_STEP pixels into buf, row by row, at most DROP_GW x DROP_GH; its size in *gw, *gh */
static void drop_grab(struct Window *w, unsigned long *buf, int *gw, int *gh)
{
    struct RastPort *rp = w->RPort;
    int deep = CyberGfxBase && GetBitMapAttr(rp->BitMap, BMA_DEPTH) > 8;
    int gx, gy;
    *gw = (w->Width - w->BorderLeft - w->BorderRight) / DROP_STEP;
    *gh = (w->Height - w->BorderTop - w->BorderBottom) / DROP_STEP;
    if (*gw > DROP_GW) *gw = DROP_GW;
    if (*gh > DROP_GH) *gh = DROP_GH;
    if (*gw < 0) *gw = 0;
    if (*gh < 0) *gh = 0;
    for (gy = 0; gy < *gh; gy++)
        for (gx = 0; gx < *gw; gx++) {
            UWORD x = w->BorderLeft + gx * DROP_STEP, y = w->BorderTop + gy * DROP_STEP;
            buf[gy * *gw + gx] = deep ? rgb_pixel(rp, x, y) : (ULONG)ReadPixel(rp, x, y);
        }
}

static long d_box[4];

/* the point on the screen to press on d in RAM:WBGapD's window w */
static void drop_from(struct Window *w, long xy[2])
{
    wbgap_grab_point(d_box, xy);
    xy[0] += w->LeftEdge + w->BorderLeft;
    xy[1] += w->TopEdge + w->BorderTop;
}

/* d made in RAM:WBGapD with its window open, and found: 1, or 0 if its window's inside never changed in 5 s */
static int drop_make_d(struct Window *w)
{
    static unsigned long before[DROP_GW * DROP_GH], after[DROP_GW * DROP_GH];
    struct DiskObject *d;
    BPTR dir;
    long last[4] = {0, 0, 0, 0};
    int t, same = 0, gw, gh, aw, ah;
    drop_grab(w, before, &gw, &gh);
    write_file("RAM:WBGapD/d", "d\n");
    if ((d = GetDefDiskObject(WBPROJECT))) {
        d->do_CurrentX = 120;
        d->do_CurrentY = 10;
        PutDiskObject((CONST_STRPTR)"RAM:WBGapD/d", d);
        FreeDiskObject(d);
    }
    if ((dir = Lock((CONST_STRPTR)"RAM:WBGapD", ACCESS_READ))) {
        UpdateWorkbench((CONST_STRPTR)"d", dir, UPDATEWB_ObjectAdded);
        UnLock(dir);
    }
    for (t = 0; t < 25; t++) {   /* the box the same twice running, 0.2 s apart */
        Delay(10);
        drop_grab(w, after, &aw, &ah);
        if (aw != gw || ah != gh) return 0;   /* the window changed size: no telling */
        if (!wbgap_diff_box(before, after, gw, gh, DROP_STEP, d_box)) continue;
        if (!memcmp(last, d_box, sizeof last) && ++same >= 2) break;
        if (memcmp(last, d_box, sizeof last)) same = 0;
        memcpy(last, d_box, sizeof last);
    }
    if (t == 25 && !wbgap_diff_box(before, after, gw, gh, DROP_STEP, d_box)) return 0;
    printf("INFO I9 d's icon at %ld,%ld %ldx%ld in RAM:WBGapD's window\n", d_box[0], d_box[1], d_box[2], d_box[3]);
    return 1;
}

static void drop_drag(const long from[2], const long to[2], const char *row)
{
    char cmd[320];
    snprintf(cmd, sizeof cmd, "%s DRAG %ld %ld %ld %ld STEPS 16", dragtest, from[0], from[1], to[0], to[1]);
    printf("INFO %s drag %ld,%ld to %ld,%ld\n", row, from[0], from[1], to[0], to[1]);
    run(cmd);
}

/* RAM:WBGapD's window moved to a corner clear of avoid; 1 if it is there within 2 s */
static int drop_move(struct Window **w, const long *avoid)
{
    long box[4];
    int t;
    struct Window *x = *w;
    if (!wbgap_drop_place(x->WScreen->Width, x->WScreen->Height, x->WScreen->BarHeight + 1, x->Width, x->Height, avoid, box))
        return 0;
    ChangeWindowBox(x, box[0], box[1], box[2], box[3]);
    for (t = 0; t < 40; t++) {
        Delay(5);
        if (!(x = find_window("WBGapD"))) return 0;
        if (x->LeftEdge == box[0] && x->TopEdge == box[1]) { *w = x; return 1; }
    }
    return 0;
}

static void drop_clean(void)
{
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapD", NULL);
    DeleteFile((CONST_STRPTR)"RAM:wbgap-appicon");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD/d.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD/d");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD/a.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD/a");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapD");
}

/* RAM:WBGapD made afresh with a, already there and snapshotted at 20,10, so the drawer isn't empty (an empty
 * drawer's window looks otherwise) and nothing moves when d comes at 120,10; its window opened, then d made in it
 * and found (drop_make_d). The window, or NULL with *stage 1 (the window didn't open) or 2 (d never showed) */
static struct Window *drop_ready(int *stage)
{
    struct DiskObject *d;
    struct Window *dw;
    BPTR l;
    int how;
    drop_clean();
    if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapD"))) UnLock(l);
    put_icon("RAM:WBGapD", WBDRAWER, NULL);
    write_file("RAM:WBGapD/a", "a\n");
    if ((d = GetDefDiskObject(WBPROJECT))) {
        d->do_CurrentX = 20;
        d->do_CurrentY = 10;
        PutDiskObject((CONST_STRPTR)"RAM:WBGapD/a", d);
        FreeDiskObject(d);
    }
    if (!(dw = open_drawer("RAM:WBGapD", "WBGapD", 10, &how))) { *stage = 1; return NULL; }
    Delay(50);
    if (!drop_make_d(dw)) { *stage = 2; return NULL; }
    *stage = 0;
    return dw;
}

static int do_appdrop(void)
{
    struct MsgPort *port = CreateMsgPort();
    struct DiskObject *aio = GetDefDiskObject(WBTOOL);
    struct Window *dw, *mine = NULL;
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
    int stage;
    long from[2], to[2], box[4];
    char cmd[320], text[80];
    if (!port || !aio || !wb) {
        fail("I9", "setup");
        fail("I8", "setup");
        goto out;
    }
    aio->do_CurrentX = aio->do_CurrentY = NO_ICON_POSITION;   /* the desktop places the AppIcon */
    if (!(dw = drop_ready(&stage))) {
        fail("I9", stage == 1 ? "RAM:WBGapD's window didn't open"
                              : "d's icon never showed in RAM:WBGapD's window (5 s after UpdateWorkbench)");
        fail("I8", stage == 1 ? "RAM:WBGapD's window didn't open" : "no icon d to drag");
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
        if (!mine) fail("I9", "no room for a window of our own clear of RAM:WBGapD's, or it didn't open");
        else if (!(aw = AddAppWindowA(9, 0, mine, port, NULL))) {
            snprintf(text, sizeof text, "AddAppWindowA NULL (IoErr %ld)", (long)IoErr());
            fail("I9", text);
        } else {
            Delay(25);
            drop_from(dw, from);
            wbgap_drop_point(box, to);
            drop_drag(from, to, "I9");
            drop_wait(port, AMTYPE_APPWINDOW, 9, "I9");
        }
        if (aw) RemoveAppWindow(aw);
        if (mine) CloseWindow(mine);
        mine = NULL;
    }
    {   /* I8: an AppIcon on the desktop, found through the WORKBENCH port, and the drawer's window moved off it */
        struct AppIcon *ai;
        if (!exists("SYS:Rexxc/rx") || !FindPort((CONST_STRPTR)"WORKBENCH")) {
            skip("I8", "no rx or no WORKBENCH port to find the AppIcon by");
            goto out;
        }
        if (!(ai = AddAppIconA(8, 0, (STRPTR)"WBGapDrop", port, 0, aio, NULL))) {
            snprintf(text, sizeof text, "AddAppIconA NULL (IoErr %ld)", (long)IoErr());
            fail("I8", text);
            goto out;
        }
        Delay(50);
        snprintf(cmd, sizeof cmd, "SYS:Rexxc/rx SYS:T/WBGap/WBGapAppPos.rexx WBGapDrop");
        run(cmd);
        if (read_all("RAM:wbgap-appicon", text, sizeof text) < 0 || !wbgap_parse_box(text, box))
            skip("I8", "the AppIcon's place not read (WBGapAppPos.rexx)");
        else if (!(dw = find_window("WBGapD")) || !drop_move(&dw, box))
            fail("I8", "RAM:WBGapD's window couldn't be moved clear of the AppIcon");
        else {
            Delay(25);
            drop_from(dw, from);
            wbgap_drop_point(box, to);
            drop_drag(from, to, "I8");
            drop_wait(port, AMTYPE_APPICON, 8, "I8");
        }
        RemoveAppIcon(ai);
    }
out:
    drop_clean();
    if (port) {
        struct Message *m;
        while ((m = GetMsg(port))) ReplyMsg(m);
        DeleteMsgPort(port);
    }
    if (aio) FreeDiskObject(aio);
    if (wb) UnlockPubScreen(NULL, wb);
    return fails ? 5 : 0;
}

/* ---- I4, I7: a file dragged between drawer windows, and onto a drawer's icon ----
 * RAM:WBGapD (a snapshotted at 20,10) at the top-left and RAM:WBGapE (b at 20,10) in a corner clear of it, each
 * 440x360, or half the screen wide if that is less (both SKIP when half is under 320). d is made in
 * RAM:WBGapD at 120,10 and the drawer sub at 20,100 (under a), each found where the window changed (wbgap_diff_box). I4: d dragged into
 * RAM:WBGapE's window, which must move it there, d.info with it (one volume: a move, as 3.2.3). I7: d, found again
 * where RAM:WBGapE's window changed, dragged onto sub's icon, which must move it into RAM:WBGapD/sub. */
static long sub_box[4];

/* box: what changed in w since before (gw x gh samples) once it has stayed the same 0.4 s; 1, or 0 in 5 s */
static int drag_settle(struct Window *w, const unsigned long *before, int gw, int gh, long box[4])
{
    static unsigned long after[DROP_GW * DROP_GH];
    long last[4] = {0, 0, 0, 0};
    int t, same = 0, aw, ah;
    for (t = 0; t < 25; t++) {
        Delay(10);
        drop_grab(w, after, &aw, &ah);
        if (aw != gw || ah != gh) return 0;
        if (!wbgap_diff_box(before, after, gw, gh, DROP_STEP, box)) continue;
        if (!memcmp(last, box, sizeof last) && ++same >= 2) return 1;
        if (memcmp(last, box, sizeof last)) same = 0;
        memcpy(last, box, sizeof last);
    }
    return wbgap_diff_box(before, after, gw, gh, DROP_STEP, box);
}

/* an object made in the drawer dir (its window w open) at x,y and told to the desktop; its box found as d's is */
static int drag_make(struct Window *w, const char *dir, const char *name, LONG type, int x, int y, long box[4])
{
    static unsigned long before[DROP_GW * DROP_GH];
    char path[96];
    struct DiskObject *o;
    BPTR l;
    int gw, gh;
    drop_grab(w, before, &gw, &gh);
    snprintf(path, sizeof path, "%s/%s", dir, name);
    if (type == WBDRAWER) { if ((l = CreateDir((CONST_STRPTR)path))) UnLock(l); }
    else write_file(path, "x\n");
    if ((o = GetDefDiskObject(type))) {
        o->do_CurrentX = x;
        o->do_CurrentY = y;
        PutDiskObject((CONST_STRPTR)path, o);
        FreeDiskObject(o);
    }
    if ((l = Lock((CONST_STRPTR)dir, ACCESS_READ))) {
        UpdateWorkbench((CONST_STRPTR)name, l, UPDATEWB_ObjectAdded);
        UnLock(l);
    }
    return drag_settle(w, before, gw, gh, box);
}

/* RAM:WBGapX with x snapshotted at 20,10 in it (a drawer that isn't empty: an empty one's window draws otherwise) */
static void drag_drawer(const char *dir, const char *first)
{
    char path[96];
    struct DiskObject *o;
    BPTR l;
    if ((l = CreateDir((CONST_STRPTR)dir))) UnLock(l);
    put_icon(dir, WBDRAWER, NULL);
    snprintf(path, sizeof path, "%s/%s", dir, first);
    write_file(path, "x\n");
    if ((o = GetDefDiskObject(WBPROJECT))) {
        o->do_CurrentX = 20;
        o->do_CurrentY = 10;
        PutDiskObject((CONST_STRPTR)path, o);
        FreeDiskObject(o);
    }
}

/* the file and its .info, each there or not, in dir */
static void drag_where(const char *dir, const char *name, int *file, int *info)
{
    char path[96];
    snprintf(path, sizeof path, "%s/%s", dir, name);
    *file = exists(path);
    snprintf(path, sizeof path, "%s/%s.info", dir, name);
    *info = exists(path);
}

/* after a drag of name from src to dst: wait up to 3 s for it at dst, then judge */
static int drag_judge(const char *row, const char *src, const char *dst, const char *name)
{
    char why[160];
    int sf, si, df = 0, di = 0, t;
    for (t = 0; t < 30; t++) {
        drag_where(dst, name, &df, &di);
        if (df && di) break;
        Delay(5);
    }
    Delay(10);                                    /* the move's other half (the source side) done */
    drag_where(src, name, &sf, &si);
    drag_where(dst, name, &df, &di);
    printf("INFO %s %s: in %s %d/%d (file/icon), in %s %d/%d\n", row, name, src, sf, si, dst, df, di);
    if (wbgap_move_judge(sf, si, df, di, name, dst, why, sizeof why) == WBGAP_PASS) { pass(row); return 1; }
    fail(row, why);
    return 0;
}

static void drag_clean(void)
{
    static const char *const files[] = {
        "RAM:WBGapD/sub/d.info", "RAM:WBGapD/sub/d", "RAM:WBGapD/sub.info", "RAM:WBGapD/sub", "RAM:WBGapD/d.info",
        "RAM:WBGapD/d", "RAM:WBGapD/a.info", "RAM:WBGapD/a", "RAM:WBGapD.info", "RAM:WBGapD", "RAM:WBGapE/d.info",
        "RAM:WBGapE/d", "RAM:WBGapE/b.info", "RAM:WBGapE/b", "RAM:WBGapE.info", "RAM:WBGapE"};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapD/sub", NULL);
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapE", NULL);
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapD", NULL);
    for (t = 0; t < 40 && (find_window("WBGapD") || find_window("WBGapE")); t++) Delay(5);
    /* twice, 0.5 s apart: a desktop that keeps a closed drawer's window size writes its .info after the close */
    for (pass = 0; pass < 2; pass++) {
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
}

/* the window titled title made w x h (at most the screen's) and moved to a corner clear of avoid (avoid NULL: to the
 * top-left, under the screen's bar); it, or NULL if it isn't there within 2 s */
static struct Window *drag_place(const char *title, const long *avoid, long w, long h)
{
    struct Window *x = find_window(title);
    long box[4], top;
    int t;
    if (!x) return NULL;
    top = x->WScreen->BarHeight + 1;
    if (w > x->WScreen->Width) w = x->WScreen->Width;
    if (h > x->WScreen->Height - top) h = x->WScreen->Height - top;
    if (!avoid) {
        box[0] = 0;
        box[1] = top;
        box[2] = w;
        box[3] = h;
    } else if (!wbgap_drop_place(x->WScreen->Width, x->WScreen->Height, top, w, h, avoid, box))
        return NULL;
    ChangeWindowBox(x, box[0], box[1], box[2], box[3]);
    for (t = 0; t < 40; t++) {
        Delay(5);
        if (!(x = find_window(title))) return NULL;
        if (x->LeftEdge == box[0] && x->TopEdge == box[1] && x->Width == box[2] && x->Height == box[3]) return x;
    }
    return NULL;
}

#define DRAG_WW 440   /* each drawer's window: room for three icons in a row and no scroller */
#define DRAG_WH 360
#define DRAG_MINW 320 /* half the screen narrower than this: no room for both windows */

static int do_dragfile(int keep)
{
    static unsigned long before[DROP_GW * DROP_GH];
    struct Window *dw, *ew;
    long from[2], to[2], dbox[4], ebox[4], ww;
    int how, gw, gh;
    drag_clean();
    {   /* two windows side by side: on a narrower screen no corner is clear of the first, through no fault of the
         * desktop's */
        struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench");
        ww = wb ? wb->Width / 2 : 0;          /* each window at most half the screen (3.2.3's 800x600: 400) */
        if (ww > DRAG_WW) ww = DRAG_WW;
        if (wb) UnlockPubScreen(NULL, wb);
        if (ww < DRAG_MINW) {
            skip("I4", wb ? "the Workbench screen is narrower than two 320-wide drawer windows side by side"
                          : "no Workbench screen");
            skip("I7", wb ? "the Workbench screen is narrower than two 320-wide drawer windows side by side"
                          : "no Workbench screen");
            return fails ? 5 : 0;
        }
    }
    drag_drawer("RAM:WBGapD", "a");
    drag_drawer("RAM:WBGapE", "b");
    if (!(dw = open_drawer("RAM:WBGapD", "WBGapD", 10, &how))) {
        fail("I4", "RAM:WBGapD's window didn't open");
        fail("I7", "RAM:WBGapD's window didn't open");
        goto out;
    }
    if (!(dw = drag_place("WBGapD", NULL, ww, DRAG_WH))) {
        fail("I4", "RAM:WBGapD's window couldn't be sized and put at the top-left");
        fail("I7", "RAM:WBGapD's window couldn't be sized");
        goto out;
    }
    Delay(50);
    if (!drag_make(dw, "RAM:WBGapD", "d", WBPROJECT, 120, 10, d_box) ||
        !(dw = find_window("WBGapD")) || !drag_make(dw, "RAM:WBGapD", "sub", WBDRAWER, 20, 100, sub_box)) {
        fail("I4", "d or sub never showed in RAM:WBGapD's window (5 s after UpdateWorkbench)");
        fail("I7", "no icons to drag");
        goto out;
    }
    printf("INFO I4 in RAM:WBGapD's window: d at %ld,%ld %ldx%ld, sub at %ld,%ld %ldx%ld\n", d_box[0], d_box[1],
           d_box[2], d_box[3], sub_box[0], sub_box[1], sub_box[2], sub_box[3]);
    if (!(dw = find_window("WBGapD"))) { fail("I4", "RAM:WBGapD's window went"); fail("I7", "RAM:WBGapD's window went"); goto out; }
    dbox[0] = dw->LeftEdge; dbox[1] = dw->TopEdge; dbox[2] = dw->Width; dbox[3] = dw->Height;
    if (!open_drawer("RAM:WBGapE", "WBGapE", 10, &how) || !(ew = drag_place("WBGapE", dbox, ww, DRAG_WH))) {
        fail("I4", "RAM:WBGapE's window didn't open, or couldn't be put clear of RAM:WBGapD's");
        fail("I7", "no second window");
        goto out;
    }
    Delay(50);
    /* I4: d from RAM:WBGapD's window into RAM:WBGapE's, dropped across the middle, two-thirds down: clear of b and
     * of any toolbar along the top */
    drop_grab(ew, before, &gw, &gh);
    drop_from(dw, from);
    to[0] = ew->LeftEdge + ew->Width / 2;
    to[1] = ew->TopEdge + ew->Height * 2 / 3;
    drop_drag(from, to, "I4");
    if (!drag_judge("I4", "RAM:WBGapD", "RAM:WBGapE", "d")) { fail("I7", "I4 failed: no d in RAM:WBGapE to drag"); goto out; }
    /* I7: d, where RAM:WBGapE's window changed, onto sub's icon in RAM:WBGapD's window */
    if (!(ew = find_window("WBGapE")) || !drag_settle(ew, before, gw, gh, ebox)) {
        fail("I7", "d's icon not seen in RAM:WBGapE's window after I4");
        goto out;
    }
    printf("INFO I7 d at %ld,%ld %ldx%ld in RAM:WBGapE's window\n", ebox[0], ebox[1], ebox[2], ebox[3]);
    wbgap_grab_point(ebox, from);
    from[0] += ew->LeftEdge + ew->BorderLeft;
    from[1] += ew->TopEdge + ew->BorderTop;
    if (!(dw = find_window("WBGapD"))) { fail("I7", "RAM:WBGapD's window went"); goto out; }
    wbgap_grab_point(sub_box, to);
    to[0] += dw->LeftEdge + dw->BorderLeft;
    to[1] += dw->TopEdge + dw->BorderTop;
    drop_drag(from, to, "I7");
    drag_judge("I7", "RAM:WBGapE", "RAM:WBGapD/sub", "d");
out:
    if (!keep) drag_clean();   /* KEEP: the windows left open, to look at */
    return fails ? 5 : 0;
}

/* ---- WBStartup ---- */
static void stamp(const char *what, const char *name)
{
    BPTR fh = Open((CONST_STRPTR)"RAM:wbgap-order", MODE_READWRITE);
    char line[64];
    if (!fh) return;
    Seek(fh, 0, OFFSET_END);
    snprintf(line, sizeof line, "%s %s\n", what, name);
    Write(fh, line, strlen(line));
    Close(fh);
}

static int from_workbench(struct WBStartup *wbs)
{
    struct WBArg *arg = wbs->sm_ArgList;
    const char *name = (const char *)arg->wa_Name;
    BPTR old = CurrentDir(arg->wa_Lock);
    struct DiskObject *d = GetDiskObject((CONST_STRPTR)name);
    long secs = 1;
    int rec = 0;
    char recfile[64] = "RAM:wbgap-launch";
    if (d) {
        STRPTR v = FindToolType((CONST_STRPTR *)d->do_ToolTypes, (CONST_STRPTR)"STAMPSECS");
        if (v) secs = atol((const char *)v);
        if ((v = FindToolType((CONST_STRPTR *)d->do_ToolTypes, (CONST_STRPTR)"LAUNCHREC"))) {   /* LAUNCHREC[=file] */
            rec = 1;
            if (*v) snprintf(recfile, sizeof recfile, "%s", (const char *)v);
        }
        FreeDiskObject(d);
    }
    CurrentDir(old);
    if (rec) {   /* LAUNCH: what this start got, as one line */
        struct Task *me = FindTask(NULL);
        char line[400];
        int i, n;
        BPTR fh = Open((CONST_STRPTR)recfile, MODE_NEWFILE);
        n = snprintf(line, sizeof line, "launch stack %lu pri %d args %ld",
                     (unsigned long)((char *)me->tc_SPUpper - (char *)me->tc_SPLower), (int)me->tc_Node.ln_Pri,
                     (long)wbs->sm_NumArgs);
        for (i = 0; i < wbs->sm_NumArgs && n < (int)sizeof line - 40; i++)
            n += snprintf(line + n, sizeof line - n, " %s", (const char *)wbs->sm_ArgList[i].wa_Name);
        snprintf(line + n, sizeof line - n, "\n");
        if (fh) { Write(fh, line, strlen(line)); Close(fh); }
        return 0;
    }
    stamp("start", name);
    Delay(secs * 50);
    stamp("end", name);
    return 0;
}

/* A10a: the LoadWB switches GFOpenBench says it took (ENV:GFOpenBench/LoadWB, words) are the ones on the
 * Startup-Sequence's line that started the desktop: "SYS:System/OpenBench ..." where that exists, else "LoadWB ..." */
static const char *const loadwb_switches[] = {"DEBUG", "-DEBUG", "DELAY", "CLEANUP", "NEWPATH", "SKIPWBSTARTUP", "SKIP",
                                              "SIMPLEGELS", "LEGACY"};
static int stricmp_n(const char *a, const char *b, int n)
{
    for (; n > 0; n--, a++, b++) {
        char x = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a, y = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (x != y) return 1;
        if (!x) return 0;
    }
    return 0;
}
static void upper(char *t) { for (; *t; t++) if (*t >= 'a' && *t <= 'z') *t -= 32; }
static int switches_of(char *text, char *out, int n)   /* the known switches in text, as " A B ", in the template's order */
{
    int k, count = 0;
    char *w;
    out[0] = 0;
    upper(text);
    for (k = 0; k < (int)(sizeof loadwb_switches / sizeof loadwb_switches[0]); k++) {
        const char *sw = loadwb_switches[k];
        char tmp[400];
        snprintf(tmp, sizeof tmp, "%s", text);
        for (w = strtok(tmp, " \t\r\n"); w; w = strtok(NULL, " \t\r\n"))
            if (!strcmp(w, sw)) {
                const char *name = !strcmp(sw, "-DEBUG") ? "DEBUG" : !strcmp(sw, "SKIP") ? "SKIPWBSTARTUP" : sw;
                if (!strstr(out, name) && (int)(strlen(out) + strlen(name) + 2) < n) { strcat(out, " "); strcat(out, name); count++; }
            }
    }
    return count;
}
static void check_loadwb(void)
{
    char env[200] = "", seq[400] = "", line[256], want[120], got[120];
    BPTR fh;
    if (!is_gfos()) { skip("A10a", "GFOS only: 3.2.3 keeps no record of LoadWB's switches"); return; }
    if (!(fh = Open((CONST_STRPTR)"ENV:GFOpenBench/LoadWB", MODE_OLDFILE))) { fail("A10a", "no ENV:GFOpenBench/LoadWB"); return; }
    { long n = Read(fh, env, sizeof env - 1); env[n > 0 ? n : 0] = 0; }
    Close(fh);
    {   /* the line that started the desktop: SYS:System/OpenBench's where it exists (Instance-32), else LoadWB's */
        int ob = exists("SYS:System/OpenBench");
        const char *cmd = ob ? "SYS:System/OpenBench" : "LoadWB";
        int cl = (int)strlen(cmd);
        if ((fh = Open((CONST_STRPTR)"S:Startup-Sequence", MODE_OLDFILE))) {
            while (FGets(fh, (STRPTR)line, sizeof line)) {
                char *p = line;
                for (;;) {   /* leading blanks, "Run" and redirections */
                    while (*p == ' ' || *p == '\t') p++;
                    if (!stricmp_n(p, "Run ", 4)) { p += 4; continue; }
                    if (*p == '>' || *p == '<') { while (*p && *p != ' ' && *p != '\t') p++; continue; }
                    break;
                }
                if (!ob && !stricmp_n(p, "C:LoadWB", 8)) p += 2;
                if (!stricmp_n(p, cmd, cl) && (p[cl] == ' ' || p[cl] == '\t' || p[cl] == '\n' || !p[cl])) {
                    snprintf(seq, sizeof seq, "%s", p + cl);
                    break;
                }
            }
            Close(fh);
        }
        printf("INFO A10a the desktop's line (the first %s line in S:Startup-Sequence): %s%s", cmd, cmd, seq[0] ? seq : "\n");
    }
    switches_of(seq, want, sizeof want);
    switches_of(env, got, sizeof got);
    printf("INFO A10a Startup-Sequence:%s; GFOpenBench took:%s\n", want[0] ? want : " none", got[0] ? got : " none");
    if (!strcmp(want, got)) pass("A10a"); else fail("A10a", "the switches taken aren't the Startup-Sequence's");
}

static int do_startup(const char *what)
{
    if (!stricmp(what, "SETUP")) {
        static STRPTR ta[] = {(STRPTR)"STARTPRI=-10", (STRPTR)"STAMPSECS=1", NULL};
        static STRPTR tb[] = {(STRPTR)"STARTPRI=10", (STRPTR)"DONOTWAIT", (STRPTR)"STAMPSECS=4", NULL};
        static STRPTR tc[] = {(STRPTR)"STAMPSECS=1", NULL};
        int ok = put_icon("SYS:WBStartup/WBGapA", WBTOOL, ta) && put_icon("SYS:WBStartup/WBGapB", WBTOOL, tb) &&
                 put_icon("SYS:WBStartup/WBGapC", WBTOOL, tc);
        /* S2d: D and E have no STARTPRI either; E was added before C, and C before D, so oldest first is E, C, D
         * where name order is C, D, E. The date added is the older of the program's and its icon's dates. */
        if (exists("SYS:WBStartup/WBGapD") && exists("SYS:WBStartup/WBGapE")) {
            static const char *const names[] = {"SYS:WBStartup/WBGapE", "SYS:WBStartup/WBGapC", "SYS:WBStartup/WBGapD"};
            struct DateStamp ds;
            char path[64];
            int i;
            ok = ok && put_icon("SYS:WBStartup/WBGapD", WBTOOL, tc) && put_icon("SYS:WBStartup/WBGapE", WBTOOL, tc);
            for (i = 0; i < 3 && ok; i++) {
                ds.ds_Days = 7000 + i;   /* 2 March 1997 on, a day apart */
                ds.ds_Minute = 600;
                ds.ds_Tick = 0;
                snprintf(path, sizeof path, "%s.info", names[i]);
                ok = SetFileDate((CONST_STRPTR)names[i], &ds) && SetFileDate((CONST_STRPTR)path, &ds);
            }
        } else printf("INFO no SYS:WBStartup/WBGapD and WBGapE: S2d not set up\n");
        if (exists("SYS:WBStartup/WBGapF")) {   /* I12a, I12b at WBStartup, which needs no registered Workbench */
            static char st[24];
            static STRPTR tf[] = {(STRPTR)"LAUNCHREC=RAM:wbgap-launchwb", (STRPTR)st, (STRPTR)"TOOLPRI=-3",
                                  (STRPTR)"DONOTWAIT", NULL};
            unsigned long want = stack_wanted();
            snprintf(st, sizeof st, "STACK=%lu", want);
            ok = ok && put_icon_stack("SYS:WBStartup/WBGapF", WBTOOL, tf, (LONG)want);
        } else printf("INFO no SYS:WBStartup/WBGapF: I12 at WBStartup not set up\n");
        if (exists("SYS:WBStartup/WBGapG")) {   /* I12c: STACK= alone, the icon's own field left at the default */
            static char sg[24];
            static STRPTR tg[] = {(STRPTR)"LAUNCHREC=RAM:wbgap-launchwb2", (STRPTR)sg, (STRPTR)"DONOTWAIT", NULL};
            snprintf(sg, sizeof sg, "STACK=%lu", stack_wanted());
            ok = ok && put_icon("SYS:WBStartup/WBGapG", WBTOOL, tg);
        } else printf("INFO no SYS:WBStartup/WBGapG: I12c not set up\n");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-order");
        printf("INFO WBStartup icons %s\n", ok ? "written" : "NOT written");
        return ok ? 0 : 20;
    }
    {   /* CHECK: B (10) first, C (0), A (-10) last; B doesn't hold C up; C holds A up */
        char buf[400];
        BPTR fh = Open((CONST_STRPTR)"RAM:wbgap-order", MODE_OLDFILE);
        long n;
        const char *sb, *sc, *sa, *eb, *ec;
        if (!fh) { fail("S1", "RAM:wbgap-order not written: nothing in WBStartup ran"); skip("S2", "-"); skip("S3", "-"); return 10; }
        n = Read(fh, buf, sizeof buf - 1);
        Close(fh);
        buf[n > 0 ? n : 0] = 0;
        printf("INFO order: ");
        for (n = 0; buf[n]; n++) putchar(buf[n] == '\n' ? ',' : buf[n]);
        putchar('\n');
        sa = strstr(buf, "start WBGapA"); sb = strstr(buf, "start WBGapB"); sc = strstr(buf, "start WBGapC");
        eb = strstr(buf, "end WBGapB"); ec = strstr(buf, "end WBGapC");
        if (sa && sb && sc) pass("S1"); else fail("S1", "not all three started");
        if (sa && sb && sc && sb < sc && sc < sa) pass("S2"); else fail("S2", "not in STARTPRI order B, C, A");
        if (sc && eb && ec && sa && sc < eb && ec < sa) pass("S3"); else fail("S3", "DONOTWAIT or the wait for a program not honoured");
        {   /* S2d: no STARTPRI, oldest date added first (GFOS); 3.2.3 goes by name */
            const char *sd = strstr(buf, "start WBGapD"), *se = strstr(buf, "start WBGapE");
            if (!sd || !se) skip("S2d", "WBGapD or WBGapE didn't start (not set up?)");
            else if (!is_gfos()) skip("S2d", "a GFOS decision; 3.2.3 orders by name");
            else if (se < sc && sc < sd) pass("S2d");
            else fail("S2d", "not by date added E, C, D");
        }
        check_launchrec("RAM:wbgap-launchwb", 0, icon_stack("SYS:WBStartup/WBGapF"));
        check_stack_alone("RAM:wbgap-launchwb2", icon_stack("SYS:WBStartup/WBGapG"));
        check_loadwb();
        return fails ? 5 : 0;
    }
}

/* ---- row X3: low memory ---- */
struct eaten { struct eaten *next; ULONG size; };

/* The front screen's low-memory requester (EasyRequest's window, titled "GFOpenBench" and holding "Retry"):
   where its left-hand button is, in screen pixels; 0 when there is none */
static int find_retry(int *x, int *y)
{
    struct Screen *s;
    struct Window *w;
    int found = 0;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (w = s->FirstWindow; w && !found; w = w->NextWindow) {
            struct Gadget *g, *left = NULL;
            if (!w->Title || strcmp((const char *)w->Title, "GFOpenBench")) continue;
            for (g = w->FirstGadget; g; g = g->NextGadget)
                if ((g->GadgetType & GTYP_GTYPEMASK) == GTYP_BOOLGADGET && !(g->GadgetType & GTYP_SYSGADGET)
                    && (g->Activation & GACT_RELVERIFY) && !(g->Flags & (GFLG_RELRIGHT | GFLG_RELBOTTOM))
                    && (!left || g->LeftEdge < left->LeftEdge))
                    left = g;
            if (!left) continue;
            *x = w->LeftEdge + left->LeftEdge + left->Width / 2;
            *y = w->TopEdge + left->TopEdge + left->Height / 2;
            found = 1;
        }
    UnlockIBase(lock);
    return found;
}

/* GFOpenBench's message (gfopenprefs Common/obmsg.h: keep the layout the same, as DragTest's) */
struct ob_msg {
    struct Message m;
    ULONG magic;
    LONG ok;
    char path[256];
};

static int do_lowmem(int secs, const char *path)
{
    struct eaten *list = NULL, *e, **pp;
    ULONG n, got = 0, kept = 0;
    int x = 0, y = 0, seen, sent = 0, i;
    char cmd[300];
    struct MsgPort *reply = CreateMsgPort(), *ob;
    struct ob_msg *m = AllocVec(sizeof *m, MEMF_PUBLIC | MEMF_CLEAR);   /* before the memory is taken */
    if (!reply || !m) { printf("FAIL X3 setup\n"); if (m) FreeVec(m); if (reply) DeleteMsgPort(reply); return 20; }
    m->m.mn_ReplyPort = reply;
    m->m.mn_Length = sizeof *m;
    m->magic = 0x4F424D31UL;
    strncpy(m->path, path, sizeof m->path - 1);
    printf("INFO X3 taking the memory for %d s, opening %s\n", secs, path);
    fflush(stdout);
    for (n = 1UL << 30; n >= 256;) {                /* the biggest pieces first, down to 256 bytes */
        if (AvailMem(MEMF_ANY | MEMF_LARGEST) < n || !(e = AllocMem(n, MEMF_ANY))) { n >>= 1; continue; }
        e->size = n;
        e->next = list;
        list = e;
        got += n;
    }
    /* 48 KiB back for the system, in pieces of 4 KiB at most: no program's code fits in one. Every other piece only,
       so that no two freed pieces lie side by side and join into a bigger one (the first lab run: they did, and the
       program loaded) */
    for (pp = &list, i = 0; *pp && kept < 49152;) {
        e = *pp;
        if (e->size > 4096 || (i++ & 1)) { pp = &e->next; continue; }
        *pp = e->next;
        kept += e->size;
        FreeMem(e, e->size);
    }
    printf("INFO X3 largest free piece now %lu bytes\n", (unsigned long)AvailMem(MEMF_ANY | MEMF_LARGEST));
    {   /* can a program load now at all: it mustn't, for the check to mean anything */
        BPTR seg = LoadSeg((CONST_STRPTR)path);
        printf("INFO X3 LoadSeg of %s here: %s\n", path, seg ? "loaded (memory not low enough)" : "failed, as it should");
        if (seg) UnLoadSeg(seg);
    }
    Forbid();
    if ((ob = FindPort((CONST_STRPTR)"GFOPENBENCH"))) { PutMsg(ob, &m->m); sent = 1; }
    Permit();
    /* hold it: whatever another task gives back meanwhile (more than 4 KiB in one piece) is taken too, every half
       second (the second lab run: something freed enough while it waited, and the program loaded) */
    for (i = 0; i < secs * 2; i++) {
        ULONG big;
        Delay(25);
        while ((big = AvailMem(MEMF_ANY | MEMF_LARGEST)) > 4096 && (e = AllocMem(big & ~7UL, MEMF_ANY))) {
            e->size = big & ~7UL;
            e->next = list;
            list = e;
            got += e->size;
        }
    }
    seen = find_retry(&x, &y);
    {   /* had the drawer opened already, with the memory taken (then there was nothing to ask) */
        struct Screen *s;
        struct Window *w;
        const char *name = (const char *)FilePart((CONST_STRPTR)path);
        int open = 0;
        ULONG lock = LockIBase(0);
        for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen)
            for (w = s->FirstWindow; w; w = w->NextWindow)
                if (w->Title && *name && strstr((const char *)w->Title, name)) open = 1;
        UnlockIBase(lock);
        printf("INFO X3 %s's window open while the memory was taken: %s\n", name, open ? "yes" : "no");
    }
    while ((e = list)) {
        list = e->next;
        FreeMem(e, e->size);
    }
    printf("INFO X3 took %lu KiB, largest free piece then %lu bytes\n", (unsigned long)(got >> 10),
           (unsigned long)AvailMem(MEMF_ANY | MEMF_LARGEST));
    if (seen) {
        printf("INFO X3 low-memory requester open, Retry at %d,%d\n", x, y);
        snprintf(cmd, sizeof cmd, "%s CLICK %d %d", dragtest, x, y);
        SystemTags((CONST_STRPTR)cmd, TAG_DONE);
    } else printf("FAIL X3 no low-memory requester\n");
    for (i = 0; sent && i < 100 && !GetMsg(reply); i++) Delay(10);   /* the desktop's answer, 20 s at most */
    if (!sent) printf("FAIL X3 no GFOPENBENCH port\n");
    else if (i < 100) printf("INFO X3 the desktop answered the open: ok %ld\n", (long)m->ok);
    else printf("FAIL X3 the desktop didn't answer the open\n");
    if (!sent || i < 100) { FreeVec(m); DeleteMsgPort(reply); }   /* else left: the desktop may still reply */
    return seen ? 0 : 5;
}

/* ---- launching (row 6's launcher) ---- */
/* the stack the launch tests ask for, in the icon's stack field (3.2.3 starts a tool with that) and as STACK=: 1 MB over the Workbench's default stack and over 1 MB, so neither the
 * default nor a launcher that gives every tool 1 MB (both seen in the labs) can pass for it */
static unsigned long stack_wanted(void)
{
    ULONG def = 0;
    WorkbenchControl(NULL, WBCTRLA_GetDefaultStackSize, (ULONG)&def, TAG_END);
    return (def > 1048576 ? def : 1048576) + 1048576;
}

/* STACK=n from an icon's tool types, 0 if none */
static unsigned long icon_stack(const char *path)
{
    struct DiskObject *d = GetDiskObject((CONST_STRPTR)path);
    unsigned long n = 0;
    if (d) {
        STRPTR v = FindToolType((CONST_STRPTR *)d->do_ToolTypes, (CONST_STRPTR)"STACK");
        if (v) n = strtoul((const char *)v, NULL, 10);
        FreeDiskObject(d);
    }
    return n;
}

/* the line a start with LAUNCHREC wrote: I12a stack (the icon's field and STACK=, 1 MB over the default stack and 1 MB), I12b TOOLPRI=-3, and with args, I11 three arguments.
 * 0 if there was none (each row SKIPped) */
/* I12c: a tool whose icon asks for its stack by STACK= alone (its stack field the default) gets it. On 3.2.3,
 * which takes the field, the result is for reference only. */
static void check_stack_alone(const char *file, unsigned long want)
{
    char buf[200], why[96];
    long stack = 0, n;
    BPTR fh = Open((CONST_STRPTR)file, MODE_OLDFILE);
    if (!want) { skip("I12c", "no STACK= read back from WBGapG's icon"); return; }
    if (!fh) { snprintf(why, sizeof why, "%s not written: WBGapG didn't start", file); skip("I12c", why); return; }
    n = Read(fh, buf, sizeof buf - 1);
    buf[n > 0 ? n : 0] = 0;
    Close(fh);
    sscanf(buf, "launch stack %ld", &stack);
    printf("INFO I12c STACK= alone: stack seen %ld, wanted %lu\n", stack, want);
    if ((unsigned long)stack >= want) pass("I12c");
    else if (!is_gfos()) skip("I12c", "reference: 3.2.3 takes the icon's stack field, not STACK= alone");
    else { snprintf(why, sizeof why, "STACK=%lu alone not given (%ld)", want, stack); fail("I12c", why); }
}

static int check_launchrec(const char *file, int args_too, unsigned long want_stack)
{
    char buf[400];
    long stack = 0, args = 0, n;
    int pri = 0;
    BPTR fh = Open((CONST_STRPTR)file, MODE_OLDFILE);
    if (!fh) {
        char why[96];
        snprintf(why, sizeof why, "%s not written: not launched", file);
        if (args_too) fail("I11", why);
        skip("I12a", why);
        skip("I12b", why);
        return 0;
    }
    n = Read(fh, buf, sizeof buf - 1);
    buf[n > 0 ? n : 0] = 0;
    Close(fh);
    printf("INFO %s: %s", file, buf);
    sscanf(buf, "launch stack %ld pri %d args %ld", &stack, &pri, &args);
    printf("INFO I12a stack seen %ld, wanted %lu\n", stack, want_stack);
    if (args_too) { if (wbgap_launch_args(buf, "LaunchT p1 p2")) pass("I11"); else fail("I11", "not 3 arguments LaunchT, p1, p2 in order"); }
    if (!want_stack) skip("I12a", "no stack asked for (the icon's STACK= unread)");
    else if ((unsigned long)stack >= want_stack) pass("I12a");
    else { char why[64]; snprintf(why, sizeof why, "STACK=%lu not given (%ld)", want_stack, stack); fail("I12a", why); }
    if (pri == -3) pass("I12b"); else fail("I12b", "TOOLPRI=-3 not given");
    return 1;
}

static int do_launch(void)
{
    static char st[24];
    static STRPTR tt[] = {(STRPTR)"LAUNCHREC", (STRPTR)st, (STRPTR)"TOOLPRI=-3", NULL};
    unsigned long want = stack_wanted();
    char cmd[320];
    BPTR dir, fh = 0;
    BOOL ok;
    int t;
    snprintf(st, sizeof st, "STACK=%lu", want);
    do_setup();
    {   /* this program, by its full name: a Shell's PROGDIR: is the Shell's */
        char self[256] = "WBGapTest";
        BPTR pd = GetProgramDir();
        if (pd && NameFromLock(pd, (STRPTR)self, sizeof self - 12)) AddPart((STRPTR)self, (CONST_STRPTR)"WBGapTest", sizeof self);
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGap/LaunchT QUIET", self);
    }
    if (run(cmd) != 0 || !put_icon_stack("RAM:WBGap/LaunchT", WBTOOL, tt, (LONG)want)) {
        fail("I11", "RAM:WBGap/LaunchT not made"); skip("I12a", "no LaunchT"); skip("I12b", "no LaunchT"); return 20;
    }
    write_file("RAM:WBGap/p1", "p1\n");
    put_icon("RAM:WBGap/p1", WBPROJECT, NULL);
    write_file("RAM:WBGap/p2", "p2\n");
    put_icon("RAM:WBGap/p2", WBPROJECT, NULL);
    DeleteFile((CONST_STRPTR)"RAM:wbgap-launch");
    printf("INFO largest free block before the launch: %lu bytes\n", (unsigned long)AvailMem(MEMF_LARGEST));
    dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
    ok = dir && OpenWorkbenchObject((STRPTR)"RAM:WBGap/LaunchT", WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p1",
                                    WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p2", TAG_DONE);
    for (t = 0; ok && t < 100 && !(fh = Open((CONST_STRPTR)"RAM:wbgap-launch", MODE_OLDFILE)); t++) Delay(5);
    if (dir) UnLock(dir);
    if (!ok) {   /* I12 is checked at WBStartup too (STARTUP CHECK), which needs no registered Workbench */
        /* not I11's FAIL: I11 is Shift-double-click on the desktop (LAUNCHDESK), which needs no registered Workbench */
        printf("INFO I11 OpenWorkbenchObjectA on a tool FALSE (IoErr %ld): A6's, see LAUNCHDESK\n", (long)IoErr());
        skip("I12a", "not launched here (A6); see STARTUP CHECK");
        skip("I12b", "not launched here (A6); see STARTUP CHECK");
        return 10;
    }
    if (fh) Close(fh);
    Delay(10);   /* the line written whole */
    check_launchrec("RAM:wbgap-launch", 1, want);
    return fails ? 5 : 0;
}

/* ---- I11 on the desktop: Shift-double-click, as a hand does it (WBGapDesk.rexx clicks) ----
 * SETUP: RAM:WBGapI holds LaunchT (a copy of this program, LAUNCHREC=RAM:wbgap-launchdesk) and projects p1 and p2,
 * left out on the desktop by RAM:.backdrop's lines (what was there before is kept in RAM:wbgap-backdrop-was; if
 * there was no .backdrop, RAM:wbgap-backdrop-made says SETUP made it), and
 * "a disk was inserted" so the desktop reads it again. WBGapDesk.rexx then clicks p1, Shift-clicks p2 and
 * Shift-double-clicks LaunchT. CHECK: I11 from what LaunchT got (LaunchT, p1, p2 in that order). DONE: RAM:.backdrop
 * as it was (put back, deleted only if SETUP made it, else left alone), and the desktop told again. */
/* I11 (LaunchT, p1, p2) and I10 (OpenT, a tool; Sub, a drawer), left out together */
static const char desk_lines[] = ":WBGapI/LaunchT\n:WBGapI/p1\n:WBGapI/p2\n:WBGapI/OpenT\n:WBGapI/Sub\n";

static long read_all(const char *path, char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_OLDFILE);
    long got;
    if (!fh) return -1;
    got = Read(fh, buf, n - 1);
    Close(fh);
    if (got < 0) got = 0;
    buf[got] = 0;
    return got;
}

static void desk_told(void)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "\"%s\" DISKIN", dragtest);
    run(cmd);
    Delay(100);   /* the desktop reads its disks again */
}

/* LAUNCHDESK CHECK, I11: what LaunchT got from the Shift-double-click */
static void desk_check_i11(void)
{
    char buf[400];
    int t;
    for (t = 0; t < 20 && read_all("RAM:wbgap-launchdesk", buf, sizeof buf) < 0; t++) Delay(10);
    if (read_all("RAM:wbgap-desk-bad", buf, sizeof buf) >= 0) {   /* a click failed: whatever started, no result */
        skip("I11", "a click failed (WBGapDesk.rexx)");
        return;
    }
    if (read_all("RAM:wbgap-launchdesk", buf, sizeof buf) < 0 && read_all("RAM:wbgap-desk-clicked", buf, sizeof buf) < 0) {
        skip("I11", "not clicked: WBGapDesk.rexx found no icons to click");
        return;
    }
    if (read_all("RAM:wbgap-launchdesk", buf, sizeof buf) < 0) {
        fail("I11", "Shift-double-clicking LaunchT with p1 and p2 selected started nothing");
        return;
    }
    printf("INFO RAM:wbgap-launchdesk: %s", buf);
    if (wbgap_launch_args(buf, "LaunchT p1 p2")) pass("I11");
    else fail("I11", "Shift-double-click: not 3 arguments LaunchT, p1, p2 in order");
}

/* LAUNCHDESK CHECK, I10: OpenT double-clicked ran with itself as its one argument, Sub double-clicked opened */
static void desk_check_i10(void)
{
    static char skipped[200], rec[400], why[120];
    char m[8];
    int t, r, have_skip, have_rec;
    for (t = 0; t < 20 && read_all("RAM:wbgap-launchopen", rec, sizeof rec) < 0; t++) Delay(10);
    have_skip = read_all("RAM:wbgap-open-skip", skipped, sizeof skipped) >= 0;
    if (have_skip) skipped[strcspn(skipped, "\r\n")] = 0;
    have_rec = read_all("RAM:wbgap-launchopen", rec, sizeof rec) >= 0;
    if (have_rec) printf("INFO RAM:wbgap-launchopen: %s", rec);
    r = wbgap_i10(have_skip ? skipped : NULL, read_all("RAM:wbgap-open-bad", m, sizeof m) >= 0,
                  read_all("RAM:wbgap-open-clicked", m, sizeof m) >= 0, have_rec ? rec : NULL,
                  read_all("RAM:wbgap-open-sub", m, sizeof m) >= 0, why, sizeof why);
    if (r > 0) pass("I10");
    else if (r < 0) skip("I10", why);
    else fail("I10", why);
}

static int do_desk(const char *what)
{
    static char was[2048], text[2048 + sizeof desk_lines];
    static STRPTR tt[] = {(STRPTR)"LAUNCHREC=RAM:wbgap-launchdesk", NULL};
    static STRPTR tt_open[] = {(STRPTR)"LAUNCHREC=RAM:wbgap-launchopen", NULL};
    long n;
    if (!stricmp(what, "SETUP")) {
        char cmd[320], self[256] = "WBGapTest";
        BPTR pd = GetProgramDir(), l;
        if (pd && NameFromLock(pd, (STRPTR)self, sizeof self - 12)) AddPart((STRPTR)self, (CONST_STRPTR)"WBGapTest", sizeof self);
        if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapI"))) UnLock(l);
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGapI/LaunchT QUIET", self);
        if (run(cmd) != 0 || !put_icon("RAM:WBGapI/LaunchT", WBTOOL, tt)) {
            fail("I11", "RAM:WBGapI/LaunchT not made");
            return 20;
        }
        write_file("RAM:WBGapI/p1", "p1\n");
        put_icon("RAM:WBGapI/p1", WBPROJECT, NULL);
        write_file("RAM:WBGapI/p2", "p2\n");
        put_icon("RAM:WBGapI/p2", WBPROJECT, NULL);
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-skip");
        /* I10: OpenT, the same tool recording to its own file, and Sub, a drawer, each double-clicked alone */
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGapI/OpenT QUIET", self);
        if (run(cmd) != 0 || !put_icon("RAM:WBGapI/OpenT", WBTOOL, tt_open))
            write_file("RAM:wbgap-open-skip", "RAM:WBGapI/OpenT not made");
        if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapI/Sub"))) UnLock(l);
        if (!put_icon("RAM:WBGapI/Sub", WBDRAWER, NULL)) write_file("RAM:wbgap-open-skip", "RAM:WBGapI/Sub's icon not made");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-launchopen");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-sub");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-clicked");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-bad");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-launchdesk");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-clicked");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-bad");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-covered");
        /* nothing over the desktop before WBGapDesk.rexx clicks it: BIG's window, if a run left it */
        if (!wbgap_desk_clickable(clear_drawer("RAM:WBGapBig", "WBGapBig", "I11"))) {
            write_file("RAM:wbgap-desk-covered", "RAM:WBGapBig's window stays over the desktop\n");
            skip("I11", "RAM:WBGapBig's window stays over the desktop: no clicks");
            skip("I10", "RAM:WBGapBig's window stays over the desktop: no clicks");
            return 0;
        }
        n = read_all("RAM:.backdrop", was, sizeof was);
        if (n < 0) was[0] = 0;
        /* markers for DONE: wbgap-backdrop-was, the copy to put back; wbgap-backdrop-made, SETUP made the file */
        switch (wbgap_backdrop_setup(n, (long)sizeof was, n >= 0 && strstr(was, desk_lines) != NULL)) {
        case WBGAP_BD_TOOLONG:
            printf("INFO I11 RAM:.backdrop is over %ld bytes: left as it is\n", (long)sizeof was - 2);
            skip("I11", "RAM:.backdrop too long to keep and put back");
            skip("I10", "RAM:.backdrop too long to keep and put back");
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
            return 0;
        case WBGAP_BD_MADE:
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
            write_file("RAM:wbgap-backdrop-made", "made by WBGapTest LAUNCHDESK SETUP\n");
            write_file("RAM:.backdrop", desk_lines);
            break;
        case WBGAP_BD_SAVED:
            write_file("RAM:wbgap-backdrop-was", was);
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
            snprintf(text, sizeof text, "%s%s%s", was, n && was[n - 1] != '\n' ? "\n" : "", desk_lines);
            write_file("RAM:.backdrop", text);
            break;
        default:   /* the lines there already: no marker changed (an earlier SETUP's stay for its DONE) */
            break;
        }
        desk_told();
        printf("INFO I11 RAM:WBGapI's LaunchT, p1 and p2 left out on the desktop; I10's OpenT and Sub too\n");
        return 0;
    }
    if (!stricmp(what, "CHECK")) {
        char buf[400];
        if (read_all("RAM:wbgap-desk-covered", buf, sizeof buf) >= 0) {   /* SETUP said SKIP already */
            skip("I11", "RAM:WBGapBig's window stays over the desktop: no clicks");
            skip("I10", "RAM:WBGapBig's window stays over the desktop: no clicks");
            return 0;
        }
        desk_check_i11();
        desk_check_i10();
        return fails ? 5 : 0;
    }
    if (!stricmp(what, "DONE")) {
        char m[8];
        n = read_all("RAM:wbgap-backdrop-was", was, sizeof was);
        switch (wbgap_backdrop_done(n >= 0, read_all("RAM:wbgap-backdrop-made", m, sizeof m) >= 0)) {
        case WBGAP_BD_RESTORE: write_file("RAM:.backdrop", was); break;
        case WBGAP_BD_DELETE: DeleteFile((CONST_STRPTR)"RAM:.backdrop"); break;
        default: break;   /* SETUP left .backdrop as it was */
        }
        DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
        if (WorkbenchBase) CloseWorkbenchObjectA((STRPTR)"RAM:WBGapI/Sub", NULL);   /* if WBGapDesk.rexx left it open */
        {   /* RAM:WBGapI and its icons gone */
            static const char *const gone[] = {"RAM:WBGapI/LaunchT.info", "RAM:WBGapI/LaunchT", "RAM:WBGapI/p1.info",
                                               "RAM:WBGapI/p1", "RAM:WBGapI/p2.info", "RAM:WBGapI/p2",
                                               "RAM:WBGapI/OpenT.info", "RAM:WBGapI/OpenT", "RAM:WBGapI/Sub.info",
                                               "RAM:WBGapI/Sub", "RAM:WBGapI", "RAM:wbgap-desk-clicked",
                                               "RAM:wbgap-desk-bad", "RAM:wbgap-desk-covered", "RAM:wbgap-open-skip",
                                               "RAM:wbgap-open-clicked", "RAM:wbgap-open-bad", "RAM:wbgap-open-sub"};
            int k;
            for (k = 0; k < (int)(sizeof gone / sizeof gone[0]); k++) DeleteFile((CONST_STRPTR)gone[k]);
        }
        /* told last, so the desktop reads RAM: again with the five icons' files gone as well as their .backdrop
         * lines (told before the delete, Ivanova's 0658 lab still showed them on the desktop a minute later) */
        desk_told();
        return 0;
    }
    printf("unknown: LAUNCHDESK %s\n", what);
    return 20;
}

/* ---- L1, the default layout: the drawer; WBGapLayout.rexx opens it and reads the icons' places ---- */
static int do_layout(void)
{
    static const struct { const char *name; LONG type; } in[] = {
        {"ztool", WBTOOL}, {"mproj", WBPROJECT}, {"ydrawer", WBDRAWER}, {"atool", WBTOOL}, {"cdrawer", WBDRAWER},
        {"bproj", WBPROJECT},
    };
    char path[64];
    BPTR l;
    int i, ok = 1;
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapL", NULL);
    l = CreateDir((CONST_STRPTR)"RAM:WBGapL");
    if (l) UnLock(l);
    ok = put_icon("RAM:WBGapL", WBDRAWER, NULL);
    for (i = 0; i < (int)(sizeof in / sizeof in[0]); i++) {
        snprintf(path, sizeof path, "RAM:WBGapL/%s", in[i].name);
        if (in[i].type == WBDRAWER) { if ((l = CreateDir((CONST_STRPTR)path))) UnLock(l); }
        else write_file(path, "WBGapTest L1\n");
        ok = put_icon(path, in[i].type, NULL) && ok;   /* NO_ICON_POSITION: the desktop lays them out */
    }
    printf("INFO RAM:WBGapL %s\n", ok ? "made" : "NOT made");
    return ok ? 0 : 20;
}

/* ---- the report ---- */
static void out_line(void *ctx, const char *line) { (void)ctx; printf("%s\n", line); }

static int do_report(const char *path)
{
    static struct wbgap_report r;   /* 19 KB: not on the stack */
    char line[300];
    FILE *f = fopen(path, "r");
    if (!f) { printf("FAIL setup %s can't be read\n", path); return 20; }
    wbgap_report_init(&r);
    while (fgets(line, sizeof line, f)) wbgap_report_line(&r, line);
    fclose(f);
    if (r.unknown) printf("INFO %d result line(s) naming no gap id\n", r.unknown);
    return wbgap_report_print(&r, out_line, NULL) ? 5 : 0;
}

/* ---- the rows in files of their own (tests/amiga/rows, wbgap_test.h) ----
 * Their modes come from gen-rows.sh's listing; the helpers they take are these. */
#define WBGAP_GEN_MODE(name, fn, newrows) int fn(int argc, char **argv);
#include "wbgap_gen_modes.inc"
#undef WBGAP_GEN_MODE
static const struct gen_mode { const char *name; int (*fn)(int, char **); int newrows; } gen_modes[] = {
#define WBGAP_GEN_MODE(name, fn, newrows) { name, fn, newrows },
#include "wbgap_gen_modes.inc"
#undef WBGAP_GEN_MODE
    { NULL, NULL, 0 }
};

static int gen_mode(int argc, char **argv, int *known)
{
    const struct gen_mode *g;
    *known = 0;
    for (g = gen_modes; g->name; g++)
        if (!stricmp(argv[1], g->name)) { *known = 1; return g->fn(argc, argv); }
    return 0;
}

/* every NEWROWS mode in name order, each as if run alone; the worst return code */
static int do_newrows(char *prog)
{
    const struct gen_mode *g;
    int rc = 0, r;
    for (g = gen_modes; g->name; g++) {
        char *av[3];
        if (!g->newrows) continue;
        av[0] = prog; av[1] = (char *)g->name; av[2] = NULL;
        printf("INFO NEWROWS %s\n", g->name);
        fflush(stdout);
        if ((r = g->fn(2, av)) > rc) rc = r;
    }
    return rc;
}

void wbt_pass(const char *row) { pass(row); }
void wbt_fail(const char *row, const char *why) { fail(row, why); }
void wbt_skip(const char *row, const char *why) { skip(row, why); }
void wbt_say(const char *row, int state, const char *why) { say_row(row, state, why); }
int wbt_fails(void) { return fails; }
int wbt_run(const char *cmd) { return run(cmd); }
long wbt_read_all(const char *path, char *buf, long n) { return read_all(path, buf, n); }
const char *wbt_dragtest(void) { return dragtest; }
int wbt_put_icon(const char *path, long type, char **tooltypes) { return put_icon(path, (LONG)type, (STRPTR *)tooltypes); }
struct Window *wbt_find_window(const char *part) { return find_window(part); }
struct Window *wbt_open_drawer(const char *path, const char *title, int secs, int *how) { return open_drawer(path, title, secs, how); }
int wbt_clear_drawer(const char *path, const char *title, const char *row) { return clear_drawer(path, title, row); }
void wbt_drawer(const char *dir, const char *first) { drag_drawer(dir, first); }
struct Window *wbt_place(const char *title, const long *avoid, long w, long h) { return drag_place(title, avoid, w, h); }
int wbt_make_icon(struct Window *w, const char *dir, const char *name, long type, int x, int y, long box[4])
{
    return drag_make(w, dir, name, (LONG)type, x, y, box);
}
int wbt_is_gfos(void) { return is_gfos(); }
int wbt_desktop_has_disk(const char *row, const char *vol, const char *when) { return diskio_has_row(row, vol, when); }
struct Window *wbt_drop_ready(int *stage) { return drop_ready(stage); }
void wbt_drop_from(struct Window *w, long xy[2]) { drop_from(w, xy); }
void wbt_drop_drag(const long from[2], const long to[2], const char *row) { drop_drag(from, to, row); }
void wbt_drop_clean(void) { drop_clean(); }

int main(int argc, char **argv)
{
    int known;
    int rc = 20;
    WorkbenchBase = OpenLibrary("workbench.library", 44);
    IconBase = OpenLibrary("icon.library", 44);
    CyberGfxBase = OpenLibrary("cybergraphics.library", 40);
    if (!WorkbenchBase || !IconBase) { printf("FAIL setup no workbench.library or icon.library 44\n"); goto out; }
    if (argc == 0) { rc = _WBenchMsg ? from_workbench(_WBenchMsg) : 20; goto out; }
    if (argc < 2) { printf("usage: WBGapTest MENUS [DUMP] | MENUDRIVE | SETUP | API | NOTIFY | SCREENS | SWITCH | BIG | APPDROP | DRAGFILE [KEEP] | STARTUP SETUP|CHECK | LOWMEM secs path | LAUNCH | LAUNCHDESK SETUP|CHECK|DONE | LAYOUT | INFOSAVE | REPORT <file>\n"); goto out; }
    find_dragtest();
    printf("INFO workbench.library %d.%d\n", WorkbenchBase->lib_Version, WorkbenchBase->lib_Revision);
    if (!stricmp(argv[1], "MENUS")) rc = do_menus(argc > 2 && !stricmp(argv[2], "DUMP"));
    else if (!stricmp(argv[1], "SETUP")) rc = do_setup();
    else if (!stricmp(argv[1], "API")) rc = do_api();
    else if (!stricmp(argv[1], "MENUDRIVE")) rc = do_menudrive();
    else if (!stricmp(argv[1], "NOTIFY")) rc = do_notify();
    else if (!stricmp(argv[1], "SCREENS")) rc = do_screens();
    else if (!stricmp(argv[1], "BIG")) rc = do_big();
    else if (!stricmp(argv[1], "BAR")) rc = do_bar();
    else if (!stricmp(argv[1], "DISKS")) rc = do_disks();
    else if (!stricmp(argv[1], "CLOSEWB")) rc = do_closewb();
    else if (!stricmp(argv[1], "CLOSEWBWAIT") && argc > 3) rc = do_closewb_wait(argv[2], atoi(argv[3]));
    else if (!stricmp(argv[1], "APPDROP")) rc = do_appdrop();
    else if (!stricmp(argv[1], "DRAGFILE")) rc = do_dragfile(argc > 2 && !stricmp(argv[2], "KEEP"));
    else if (!stricmp(argv[1], "STARTUP") && argc > 2) rc = do_startup(argv[2]);
    else if (!stricmp(argv[1], "LOWMEM") && argc > 2) rc = argc > 3 ? do_lowmem(atoi(argv[2]), argv[3]) : 20;
    else if (!stricmp(argv[1], "LAUNCHDESK") && argc > 2) rc = do_desk(argv[2]);
    else if (!stricmp(argv[1], "LAUNCH")) rc = do_launch();
    else if (!stricmp(argv[1], "LAYOUT")) rc = do_layout();
    else if (!stricmp(argv[1], "INFOSAVE")) rc = do_infosave();
    else if (!stricmp(argv[1], "REPORT") && argc > 2) rc = do_report(argv[2]);
    else if (!stricmp(argv[1], "NEWROWS")) rc = do_newrows(argv[0]);
    else if (!(rc = gen_mode(argc, argv, &known), known)) printf("unknown: %s\n", argv[1]);
out:
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (IconBase) CloseLibrary(IconBase);
    if (WorkbenchBase) CloseLibrary(WorkbenchBase);
    return rc;
}
