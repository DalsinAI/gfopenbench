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
 *   WBGapTest RESETREQ          row C5a (rows/RESETREQ.c): a ScreenMode change with a window of ours in the way brings up
 *                               the reset requester, which goes once that window closes (the Workbench reset into
 *                               the new depth); Cancel on it keeps the depth. With this process's Shell window on the
 *                               Workbench (WBGapTest.script) the requester is answered with Cancel, SKIP when that works;
 *                               the whole row needs no Shell window (amiga_run)
 *   WBGapTest CLOSEWBWAIT f s   row C6's line: waits up to s seconds for that CLOSEWB run to be over and prints file f;
 *                               not over in time is FAIL C6, so a CloseWorkBench() that never returns can't hang the run
 *   WBGapTest BAR               rows B1-B3: GFOpenSpeaker's port there (B3); GFOpenTitle's Power icon clicked, its list looked at (where it hangs;
 *                               its height: Reboot, Restart, Power off, and an Eject row for each USB volume mounted)
 *                               and closed with Esc; nothing in it is picked
 *   WBGapTest DISKS             rows D5 and D5r: RAM:'s volume (NameFromLock) has a disk icon among the desktop's icons,
 *                               and is first in the volume column (RAM:wbgap-root-pos),
 *                               as WBGapRoot.rexx wrote them to RAM:wbgap-root
 *   WBGapTest PREFSWB           row P7 (rows/PREFSWB.c): Workbench prefs with RAM: hidden and the default stack 65536
 *                               (GFOS: the ENV file written; 3.2.3: Prefs/Workbench FROM USE): the RAM Disk's icon must go and
 *                               WBCTRLA_GetDefaultStackSize give 65536; the old file back, the icon must come back
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
 *   WBGapTest INFOSAVE          rows N2-N7: RAM:WBGapN/n's Information window (WBInfo) driven with DragTest: Deletable
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
#include "wbgap_rows.h"
#include "wbgap_probe.h"
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

/* the inside of a window, sampled every second pixel (a wider grid in a big window: wbgap_probe.h), as one number */
static ULONG inside_sum(struct Window *w)
{
    struct RastPort *rp = w->RPort;
    int deep = CyberGfxBase && GetBitMapAttr(rp->BitMap, BMA_DEPTH) > 8;
    ULONG sum = 0;
    int x, y, step = wbg_probe_step(w->Width - w->BorderLeft - w->BorderRight, w->Height - w->BorderTop - w->BorderBottom);
    for (y = w->BorderTop; y < w->Height - w->BorderBottom; y += step)
        for (x = w->BorderLeft; x < w->Width - w->BorderRight; x += step)
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

/* ---- workbench.library (the API mode is rows/API.c; M27 asks the menus walked here) ---- */
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

/* ---- screens ---- */
static int run(const char *cmd) { return SystemTags((CONST_STRPTR)cmd, TAG_DONE); }

/* a square of a screen's own pixels, as one number: what the screen holds, never the pointer if that is a sprite
 * (SCREENS' P4b, rows/SCREENS.c) */
unsigned long wbt_screen_box(struct Screen *s, int cx, int cy, int half)
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

unsigned long wbt_mode_id(struct Screen *s) { return GetVPModeID(&s->ViewPort); }

/* ---- windows and keys: MENUDRIVE's (rows/MENUDRIVE.c) and other row files', through wbt_windows_now, wbt_still_there
 * and wbt_key ---- */
#define MAXWINS 48

static int windows_now(struct Screen *s, struct Window **out, int n)
{
    struct Window *w;
    int k = 0;
    ULONG lock = LockIBase(0);
    for (w = s->FirstWindow; w && k < n; w = w->NextWindow) out[k++] = w;
    UnlockIBase(lock);
    return k;
}

static int still_there(struct Screen *s, struct Window *w)
{
    struct Window *now[MAXWINS];
    int m = windows_now(s, now, MAXWINS), k;
    for (k = 0; k < m; k++) if (now[k] == w) return 1;
    return 0;
}

static void key(int code)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "%s KEY 0x%x", dragtest, code);
    run(cmd);
    Delay(15);
}

static long read_all(const char *path, char *buf, long n);

static void say_row(const char *row, int state, const char *why)
{
    if (state == WBGAP_PASS) pass(row);
    else if (state == WBGAP_FAIL) fail(row, why);
    else skip(row, why);
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

/* ---- RAM:WBGapD and its project d: APPDROP's drawer (I8, I9: rows/APPDROP.c), APPWIN's and DRAGFILE's too ---- */

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

/* ---- drawer windows and icons made and found: DRAGFILE's (I4, I7: rows/DRAGFILE.c), and other row files' ---- */
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

/* an object made in the drawer dir (its window w open; a volume's root, "RAM:", too) at x,y and told to the desktop;
 * its box found as d's is. A drawer or a trashcan (WBGARBAGE) is made as a drawer */
static int drag_make(struct Window *w, const char *dir, const char *name, LONG type, int x, int y, long box[4])
{
    static unsigned long before[DROP_GW * DROP_GH];
    char path[96];
    struct DiskObject *o;
    BPTR l;
    int gw, gh;
    drop_grab(w, before, &gw, &gh);
    snprintf(path, sizeof path, "%s%s%s", dir, *dir && dir[strlen(dir) - 1] == ':' ? "" : "/", name);   /* "RAM:" too */
    if (type == WBDRAWER || type == WBGARBAGE) { if ((l = CreateDir((CONST_STRPTR)path))) UnLock(l); }
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

/* ---- WBStartup (from_workbench: what each start writes; the STARTUP mode is rows/STARTUP.c) ---- */
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

/* ---- launching (row 6's launcher; the LAUNCH mode is rows/LAUNCH.c, STARTUP CHECK uses these too) ---- */
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

/* the bytes of path into buf (at most n - 1, then a NUL): how many, -1 if it is not there */
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
int wbt_write_file(const char *path, const char *text) { return write_file(path, text); }
unsigned long wbt_inside_sum(struct Window *w) { return inside_sum(w); }
int wbt_changes_within(struct Window *w, unsigned long before, int secs) { return changes_within(w, before, secs); }
int wbt_setup(void) { return do_setup(); }
int wbt_put_icon_stack(const char *path, long type, char **tooltypes, long stack)
{
    return put_icon_stack(path, (LONG)type, (STRPTR *)tooltypes, (LONG)stack);
}
unsigned long wbt_stack_wanted(void) { return stack_wanted(); }
int wbt_check_launchrec(const char *file, int args_too, unsigned long want_stack) { return check_launchrec(file, args_too, want_stack); }
void wbt_norm(char *d, const char *s, int n) { norm(d, s, n); }
const char *wbt_item_text(struct MenuItem *it) { return item_text(it); }
int wbt_icons_have(const char *row, const char *win, const char *name) { return icons_have(row, win, name); }
int wbt_tools_item_within(const char *item, int want, int ticks) { return tools_item_within(item, want, ticks); }
unsigned long wbt_icon_stack(const char *path) { return icon_stack(path); }
void wbt_check_stack_alone(const char *file, unsigned long want) { check_stack_alone(file, want); }
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
void wbt_grab(struct Window *w, unsigned long *buf, int *gw, int *gh) { drop_grab(w, buf, gw, gh); }
int wbt_settle(struct Window *w, const unsigned long *before, int gw, int gh, long box[4]) { return drag_settle(w, before, gw, gh, box); }
typedef char wbt_grab_n_is_drop_gw_gh[WBT_GRAB_N == DROP_GW * DROP_GH ? 1 : -1];
int wbt_exists(const char *path) { return exists(path); }
int wbt_windows_now(struct Screen *s, struct Window **out, int n) { return windows_now(s, out, n); }
int wbt_still_there(struct Screen *s, struct Window *w) { return still_there(s, w); }
void wbt_key(int code) { key(code); }

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
    else if (!stricmp(argv[1], "REPORT") && argc > 2) rc = do_report(argv[2]);
    else if (!stricmp(argv[1], "NEWROWS")) rc = do_newrows(argv[0]);
    else if (!(rc = gen_mode(argc, argv, &known), known)) printf("unknown: %s\n", argv[1]);
out:
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (IconBase) CloseLibrary(IconBase);
    if (WorkbenchBase) CloseLibrary(WorkbenchBase);
    return rc;
}
