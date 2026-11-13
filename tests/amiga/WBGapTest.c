/* WBGapTest: what a desktop does that OS 3.2.3's Workbench does, row by row of amicradle-teams
 * designs/OPENBENCH-GAPS-20261112.md, on GFOpenBench and on OS 3.2.3's own Workbench alike (the Team,
 * 12 November 2026). Each check prints one line: "PASS <row>", "FAIL <row> <why>" or "SKIP <row> <why>".
 * WBGapTest.script runs them all in a lab; DragTest (beside it) gives the mouse and the keys.
 *
 *   WBGapTest MENUS [DUMP]      the Workbench windows' menus (WFLG_WBENCHWINDOW, on "Workbench"), the items
 *                               of rows M1-M26 by name; DUMP lists every menu, item and sub-item with its key
 *   WBGapTest SETUP             makes RAM:WBGap (a drawer, its icon, a tool "a" with an icon)
 *   WBGapTest API               rows A3, A4, A6-A8 on RAM:WBGap (AddAppIconA, AddAppMenuItemA,
 *                               OpenWorkbenchObjectA, UpdateWorkbench, MakeWorkbenchObjectVisibleA,
 *                               ChangeWorkbenchSelectionA, CloseWorkbenchObjectA)
 *   WBGapTest NOTIFY            rows F1, F2: RAM:WBGap's window opened (OpenWorkbenchObjectA, else DragTest OPEN),
 *                               then a file with an icon made and an icon changed from here: the window's
 *                               inside must change by itself within 5 seconds, with no Update
 *   WBGapTest SCREENS           rows C1, C2, C4: a screen of our own in front; its depth gadget clicked;
 *                               Left-Amiga-M and Left-Amiga-N
 *   WBGapTest BIG               row X1: RAM:WBGapBig with 1000 files with icons, opened and timed
 *   WBGapTest STARTUP SETUP     row S1-S3: icons for SYS:WBStartup/WBGapA, B and C (copies of this program,
 *                               made by the script); after a reboot each writes to RAM:wbgap-order
 *   WBGapTest STARTUP CHECK     reads RAM:wbgap-order (S1-S3, S2d), RAM:wbgap-launchwb (I12a, I12b: WBGapF, started
 *                               from WBStartup with STACK= (64 KB over the default stack) and TOOLPRI=-3) and ENV:GFOpenBench/LoadWB (A10a)
 *   WBGapTest LAUNCH            rows I11, I12a, I12b: RAM:WBGap/LaunchT (a copy of this program) started by
 *                               OpenWorkbenchObjectA with two projects as arguments, its icon's STACK= (64 KB over the default stack) and
 *                               TOOLPRI=-3; it writes what it got to RAM:wbgap-launch
 *   WBGapTest LAYOUT            row L1's drawer: RAM:WBGapL, no icon snapshotted, holding tools ztool and atool,
 *                               projects mproj and bproj, drawers ydrawer and cdrawer; WBGapLayout.rexx checks it
 *   WBGapTest REPORT <file>     the lines of the modes above, in <file>, as exactly one PASS, FAIL or SKIP line per
 *                               gap id (wbgap_rows.c, the gaps document plus the 13 Nov decisions), and a count
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
#include <workbench/workbench.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/wb.h>

#include "wbgap_rows.h"

extern struct IntuitionBase *IntuitionBase;
extern struct GfxBase *GfxBase;
extern struct WBStartup *_WBenchMsg;   /* libnix: the Workbench's message, when started from it */
struct Library *WorkbenchBase, *IconBase, *CyberGfxBase;

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

/* GFOS (13 Nov): Format and DiskCopy are applications of their own, not 3.2.3's (version 40 to 47) */
static void app_row(const char *row, const char *path, int gfos)
{
    char v[80], why[160];
    const char *num;
    if (!exists(path)) { snprintf(why, sizeof why, "no %s", path); fail(row, why); return; }
    ver_of(path, v, sizeof v);
    printf("INFO %s %s: %s\n", row, path, v[0] ? v : "no $VER");
    num = strchr(v, ' ');
    if (gfos && num && num[1] == '4' && num[2] >= '0' && num[2] <= '7' && num[3] == '.') {
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
    {"M29", "workbench|gfopenbench:open volume"},
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

static int put_icon(const char *path, LONG type, STRPTR *tooltypes)
{
    struct DiskObject *d = GetDefDiskObject(type);
    STRPTR *old;
    int ok;
    if (!d) return 0;
    old = d->do_ToolTypes;
    if (tooltypes) d->do_ToolTypes = tooltypes;
    d->do_CurrentX = d->do_CurrentY = NO_ICON_POSITION;
    ok = PutDiskObject((CONST_STRPTR)path, d);
    d->do_ToolTypes = old;
    FreeDiskObject(d);
    return ok;
}

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
    if (!port || !dob) { fail("A*", "setup"); return 20; }
    ai = AddAppIconA(1, 0, (STRPTR)"WBGapIcon", port, 0, dob, NULL);
    if (ai) { pass("A3"); RemoveAppIcon(ai); } else printf("FAIL A3 AddAppIconA NULL (IoErr %ld)\n", (long)IoErr()), fails++;
    mi = AddAppMenuItemA(2, 0, (STRPTR)"WBGapItem", port, NULL);
    if (mi) { pass("A4"); RemoveAppMenuItem(mi); } else printf("FAIL A4 AddAppMenuItemA NULL (IoErr %ld)\n", (long)IoErr()), fails++;
    ok = OpenWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) pass("A6"); else printf("FAIL A6 OpenWorkbenchObjectA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    Delay(50);
    {   /* UpdateWorkbench gives no answer; IoErr says (103 where no Workbench takes it) */
        BPTR dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
        SetIoErr(0);
        UpdateWorkbench((CONST_STRPTR)"new2", dir, UPDATEWB_ObjectAdded);
        if (dir) UnLock(dir);
    }
    printf("INFO UpdateWorkbench IoErr %ld\n", (long)IoErr());
    if (IoErr() == 0) pass("A7"); else fail("A7", "UpdateWorkbench set IoErr");
    ok = MakeWorkbenchObjectVisibleA((STRPTR)"RAM:WBGap/a", NULL);
    if (ok) pass("A8a"); else printf("FAIL A8a MakeWorkbenchObjectVisibleA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    memset(&hook, 0, sizeof hook);
    hook.h_Entry = (HOOKFUNC)sel_hook_fn;
    hook.h_Data = &seen;
    ok = ChangeWorkbenchSelectionA((STRPTR)"RAM:WBGap", &hook, NULL);
    if (ok && seen > 0) pass("A8b"); else printf("FAIL A8b ChangeWorkbenchSelectionA %s, %ld icons seen (IoErr %ld)\n", ok ? "TRUE" : "FALSE", (long)seen, (long)IoErr()), fails++;
    ok = CloseWorkbenchObjectA((STRPTR)"RAM:WBGap", NULL);
    if (ok) pass("A6b"); else printf("FAIL A6b CloseWorkbenchObjectA FALSE (IoErr %ld)\n", (long)IoErr()), fails++;
    FreeDiskObject(dob);
    DeleteMsgPort(port);
    return fails ? 5 : 0;
}

/* ---- screens ---- */
static int run(const char *cmd) { return SystemTags((CONST_STRPTR)cmd, TAG_DONE); }

static int do_screens(void)
{
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench"), *s;
    struct Gadget *g;
    char cmd[320];
    int x = -1, y = -1;
    if (wb) pass("C4"); else { fail("C4", "LockPubScreen(Workbench) NULL"); return 20; }
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
    if (!w) { fail("X1", "RAM:WBGapBig's window didn't open in 60 s"); return 10; }
    {   /* drawn: the inside stops changing for a second */
        ULONG last = inside_sum(w), now;
        int still = 0, t;
        for (t = 0; t < 600 && still < 5; t++) { Delay(10); now = inside_sum(w); still = now == last ? still + 1 : 0; last = now; }
    }
    DateStamp(&b);
    ticks = (b.ds_Days - a.ds_Days) * 24L * 60 * 3000 + (b.ds_Minute - a.ds_Minute) * 3000L + (b.ds_Tick - a.ds_Tick) - 50;
    printf("INFO X1 1000 icons shown in %ld.%02ld s\n", ticks / 50, (ticks % 50) * 2);
    if (ticks < 10 * 50) pass("X1"); else fail("X1", "over 10 s");
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
            snprintf(st, sizeof st, "STACK=%lu", stack_wanted());
            ok = ok && put_icon("SYS:WBStartup/WBGapF", WBTOOL, tf);
        } else printf("INFO no SYS:WBStartup/WBGapF: I12 at WBStartup not set up\n");
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
        check_loadwb();
        return fails ? 5 : 0;
    }
}

/* ---- launching (row 6's launcher) ---- */
/* the STACK= the launch tests ask for: 64 KB over the Workbench's default stack (at least 40000), so the
 * default can't pass for it */
static unsigned long stack_wanted(void)
{
    ULONG def = 0;
    WorkbenchControl(NULL, WBCTRLA_GetDefaultStackSize, (ULONG)&def, TAG_END);
    return (def > 40000 ? def : 40000) + 65536;
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

/* the line a start with LAUNCHREC wrote: I12a STACK= (64 KB over the default stack), I12b TOOLPRI=-3, and with args, I11 three arguments.
 * 0 if there was none (each row SKIPped) */
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
    if (args_too) { if (args == 3 && strstr(buf, " LaunchT p1 p2")) pass("I11"); else fail("I11", "not 3 arguments LaunchT, p1, p2 in order"); }
    if ((unsigned long)stack >= want_stack) pass("I12a");
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
    if (run(cmd) != 0 || !put_icon("RAM:WBGap/LaunchT", WBTOOL, tt)) {
        fail("I11", "RAM:WBGap/LaunchT not made"); skip("I12a", "no LaunchT"); skip("I12b", "no LaunchT"); return 20;
    }
    write_file("RAM:WBGap/p1", "p1\n");
    put_icon("RAM:WBGap/p1", WBPROJECT, NULL);
    write_file("RAM:WBGap/p2", "p2\n");
    put_icon("RAM:WBGap/p2", WBPROJECT, NULL);
    DeleteFile((CONST_STRPTR)"RAM:wbgap-launch");
    dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
    ok = dir && OpenWorkbenchObject((STRPTR)"RAM:WBGap/LaunchT", WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p1",
                                    WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p2", TAG_DONE);
    for (t = 0; ok && t < 100 && !(fh = Open((CONST_STRPTR)"RAM:wbgap-launch", MODE_OLDFILE)); t++) Delay(5);
    if (dir) UnLock(dir);
    if (!ok) {   /* I12 is checked at WBStartup too (STARTUP CHECK), which needs no registered Workbench */
        printf("FAIL I11 OpenWorkbenchObjectA on a tool FALSE (IoErr %ld): needs A6\n", (long)IoErr()), fails++;
        skip("I12a", "not launched here (A6); see STARTUP CHECK");
        skip("I12b", "not launched here (A6); see STARTUP CHECK");
        return 10;
    }
    if (fh) Close(fh);
    Delay(10);   /* the line written whole */
    check_launchrec("RAM:wbgap-launch", 1, want);
    return fails ? 5 : 0;
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

int main(int argc, char **argv)
{
    int rc = 20;
    WorkbenchBase = OpenLibrary("workbench.library", 44);
    IconBase = OpenLibrary("icon.library", 44);
    CyberGfxBase = OpenLibrary("cybergraphics.library", 40);
    if (!WorkbenchBase || !IconBase) { printf("FAIL setup no workbench.library or icon.library 44\n"); goto out; }
    if (argc == 0) { rc = _WBenchMsg ? from_workbench(_WBenchMsg) : 20; goto out; }
    if (argc < 2) { printf("usage: WBGapTest MENUS [DUMP] | SETUP | API | NOTIFY | SCREENS | BIG | STARTUP SETUP|CHECK | LAUNCH | LAYOUT | REPORT <file>\n"); goto out; }
    find_dragtest();
    printf("INFO workbench.library %d.%d\n", WorkbenchBase->lib_Version, WorkbenchBase->lib_Revision);
    if (!stricmp(argv[1], "MENUS")) rc = do_menus(argc > 2 && !stricmp(argv[2], "DUMP"));
    else if (!stricmp(argv[1], "SETUP")) rc = do_setup();
    else if (!stricmp(argv[1], "API")) rc = do_api();
    else if (!stricmp(argv[1], "NOTIFY")) rc = do_notify();
    else if (!stricmp(argv[1], "SCREENS")) rc = do_screens();
    else if (!stricmp(argv[1], "BIG")) rc = do_big();
    else if (!stricmp(argv[1], "STARTUP") && argc > 2) rc = do_startup(argv[2]);
    else if (!stricmp(argv[1], "LAUNCH")) rc = do_launch();
    else if (!stricmp(argv[1], "LAYOUT")) rc = do_layout();
    else if (!stricmp(argv[1], "REPORT") && argc > 2) rc = do_report(argv[2]);
    else printf("unknown: %s\n", argv[1]);
out:
    if (CyberGfxBase) CloseLibrary(CyberGfxBase);
    if (IconBase) CloseLibrary(IconBase);
    if (WorkbenchBase) CloseLibrary(WorkbenchBase);
    return rc;
}
