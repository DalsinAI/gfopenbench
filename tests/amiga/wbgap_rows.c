/* wbgap_rows: WBGapTest's list of gap ids and its one-line-per-id report (wbgap_rows.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "wbgap_rows.h"

#define A WBGAP_AUTO
#define L WBGAP_LOOK
#define H WBGAP_HAND
#define MENUS "MENUS"
#define API "API"
#define REXX "WBGapRexx.rexx"
#define INDEX "WBGapIndex"

/* in the document's order; a part ("A8a") straight after its row. Rows added for the 13 Nov decisions say "(13 Nov)". */
static struct wbgap_row rows[] = {   /* modes NULL here may come from tests/amiga/rows (gen_modes) */
    /* 1. Menus */
    {"M1", NULL, A, MENUS, "Workbench > Backdrop; removed in GFOS (13 Nov)"},
    {"M2", NULL, A, MENUS, "Workbench > Execute Command"},
    {"M3", NULL, A, MENUS, "Workbench > Redraw All"},
    {"M4", NULL, A, MENUS, "Workbench > Update All; in GFOS Refresh OpenBench covers it (13 Nov)"},
    {"M5", NULL, A, MENUS, "Workbench > Last Message; removed in GFOS (13 Nov)"},
    {"M6", NULL, A, MENUS, "Workbench > About"},
    {"M7", NULL, A, MENUS, "Workbench > Quit"},
    {"M8", NULL, A, MENUS, "Window > New Drawer"},
    {"M9", NULL, A, MENUS, "Window > Open Parent"},
    {"M10", NULL, A, MENUS, "Window > Close"},
    {"M11", NULL, A, MENUS, "Window > Update"},
    {"M12", NULL, A, MENUS, "Window > Select Contents"},
    {"M13", NULL, A, MENUS, "Window > Clean Up"},
    {"M14", NULL, A, MENUS, "Window > Snapshot > Window/All"},
    {"M15", NULL, A, MENUS, "Window > Show > All Files"},
    {"M15t", "M15", A, INDEX, "SHOW= kept in the drawer's icon and the ACFS index (13 Nov)"},
    {"M16", NULL, A, MENUS, "Window > View By"},
    {"M16t", "M16", A, INDEX, "the drawer keeps its view as SORT= in its icon, copied to the ACFS index on change (13 Nov)"},
    {"M17", NULL, A, MENUS, "Icons > Open"},
    {"M18", NULL, A, MENUS, "Icons > Copy"},
    {"M18p", "M18", L, NULL, "Copy's progress bar on the source drawer or drive (13 Nov)"},
    {"M19", NULL, A, MENUS, "Icons > Rename"},
    {"M20", NULL, A, MENUS, "Icons > Information"},
    {"M21", NULL, A, MENUS, "Icons > Snapshot / Unsnapshot"},
    {"M22", NULL, A, MENUS, "Icons > Leave Out / Put Away"},
    {"M23", NULL, A, MENUS, "Icons > Delete"},
    {"M23r", "M23", A, INDEX, "a deleted file recovers from the trash until the host reclaims it (13 Nov)"},
    {"M23s", "M23", A, INDEX, "how long the host keeps trashed bytes, a boot-screen setting (13 Nov)"},
    {"M24", NULL, A, MENUS, "Icons > Format Disk; in GFOS a separate Format application (13 Nov)"},
    {"M25", NULL, A, MENUS, "Icons > Empty Trash"},
    {"M26", NULL, A, MENUS, "Tools > ResetWB; in GFOS Refresh OpenBench (13 Nov)"},
    {"M27", NULL, A, API, "Tools items added by programs (AddAppMenuItemA) shown"},
    {"M28", NULL, A, MENUS, "Amiga-keys as 3.2.3"},
    {"M28k", "M28", H, NULL, "KEYS= on the Workbench icon remaps them (13 Nov)"},
    {"M29", NULL, A, MENUS, "Window > Open volume (13 Nov)"},
    {"M30", NULL, A, MENUS, "Window > Clear selection"},
    {"M31", NULL, A, MENUS, "Window > Resize to fit"},
    {"M32", NULL, A, MENUS, "Window > Sort order > Reverse"},
    {"M32t", "M32", A, INDEX, "reverse kept as SORT=<key>,REVERSE (13 Nov)"},
    {"M32h", "M32", L, NULL, "a GFOpenFiles column header toggles REVERSE (13 Nov)"},
    {"M33", NULL, A, MENUS, "Window > Find"},
    {"M33r", "M33", L, NULL, "Find's results as icons in icon view, a list in list view (13 Nov)"},
    {"M34", NULL, A, MENUS, "Auto-arrange icons"},
    {"M34t", "M34", A, INDEX, "Auto-arrange kept per drawer as ARRANGE=, (ARRANGE=) when off (13 Nov)"},
    {"M35", NULL, A, MENUS, "Icons > Eject, any disk (13 Nov)"},
    /* 2. Icons */
    {"I1", NULL, L, NULL, "click selects, click elsewhere deselects"},
    {"I2", NULL, L, NULL, "Shift-click adds to the selection"},
    {"I3", NULL, A, "LAUNCHDESK", "lasso, icons only (13 Nov); WBGapDesk.rexx lassos p1 and p2 on the desktop"},
    {"I4", NULL, A, "DRAGFILE", "drag between drawer windows (a file moved, within a volume)"},
    {"I5", NULL, L, NULL, "drag to the backdrop"},
    {"I6", NULL, L, NULL, "drag to the Trashcan"},
    {"I7", NULL, A, "DRAGFILE", "drag onto a drawer or disk icon (DRAGFILE: onto a drawer's icon)"},
    {"I8", NULL, A, "APPDROP", "drag onto an AppIcon"},
    {"I9", NULL, A, "APPDROP", "drag onto an AppWindow"},
    {"I10", NULL, A, "LAUNCHDESK", "double-click opens or runs; WBGapDesk.rexx double-clicks the tool OpenT and the drawer Sub on the desktop"},
    {"I11", NULL, A, "LAUNCHDESK", "several icons passed as arguments (Shift-double-click on the desktop, as 3.2.3)"},
    {"I12", NULL, A, NULL, "STACK, TOOLPRI, DONOTWAIT and WAIT honoured at launch (WAIT, DONOTWAIT: S3)"},
    {"I12a", "I12", A, "LAUNCH, STARTUP CHECK", "the icon's stack (its field and STACK=) gives the tool that stack"},
    {"I12b", "I12", A, "LAUNCH, STARTUP CHECK", "TOOLPRI= gives the tool that priority"},
    {"I12c", "I12", A, "STARTUP CHECK", "STACK= alone, the icon's stack field the default, gives that stack; GFOS (3.2.3 reads the field)"},
    {"I13", NULL, L, NULL, "icon formats: classic, NewIcons, GlowIcons, PNG"},
    {"I14", NULL, A, MENUS, "files with no icon shown (Show All Files)"},
    {"I15", NULL, L, NULL, "a disk's status window with a pie chart, no gauge (13 Nov)"},
    {"I16", NULL, H, NULL, "keyboard selection everywhere: cursor keys, Return, Tab (13 Nov)"},
    /* 3. Information window */
    {"N1", NULL, L, NULL, "name, type, size, date shown"},
    {"N2", NULL, A, "INFOSAVE", "protection bits SARWED saved"},
    {"N3", NULL, A, "INFOSAVE", "comment saved"},
    {"N4", NULL, A, "INFOSAVE", "tool types added, edited, deleted, saved"},
    {"N5", NULL, A, "INFOSAVE", "default tool saved"},
    {"N6", NULL, A, NULL, "Stack field (13 Nov: as 3.2.3)"},
    {"N7", NULL, A, NULL, "H and P bits, in an Advanced section (13 Nov)"},
    {"N8", NULL, A, API, "WBInfo from a program opens it"},
    /* 4. workbench.library */
    {"A1", NULL, A, NULL, "AddAppWindowA and its messages"},
    {"A2", NULL, A, NULL, "AddAppWindowDropZoneA"},
    {"A3", NULL, A, API, "AddAppIconA"},
    {"A4", NULL, A, API, "AddAppMenuItemA"},
    {"A5", NULL, A, "WBCONTROL", "WorkbenchControlA tags"},
    {"A6", NULL, A, API, "OpenWorkbenchObjectA"},
    {"A6b", "A6", A, API, "CloseWorkbenchObjectA"},
    {"A7", NULL, A, API, "UpdateWorkbench, the refresh proven (13 Nov)"},
    {"A8", NULL, A, NULL, "the object calls"},
    {"A8a", "A8", A, API, "MakeWorkbenchObjectVisibleA"},
    {"A8b", "A8", A, API, "ChangeWorkbenchSelectionA"},
    {"A8c", "A8", A, API, "WhichWorkbenchObjectA"},
    {"A9", NULL, A, API, "WBInfo"},
    {"A10", NULL, A, NULL, "LoadWB switches as 3.2.3 (13 Nov)"},
    {"A10a", "A10", A, "STARTUP CHECK", "the switches GFOpenBench took (ENV:GFOpenBench/LoadWB) are the Startup-Sequence's"},
    {"A10b", "A10", H, NULL, "LoadWB SKIPWBSTARTUP: nothing in SYS:WBStartup runs (a boot of its own)"},
    /* 5. ARexx */
    {"R1", NULL, A, REXX, "WORKBENCH port"},
    {"R1b", "R1", A, REXX, "OPENBENCH port, on GFOpenBench"},
    {"R2", NULL, A, REXX, "WORKBENCH port's commands"},
    {"R2b", "R2", A, REXX, "OPENBENCH port's commands"},
    /* 6. WBStartup */
    {"S1", NULL, A, "STARTUP CHECK", "everything in SYS:WBStartup started"},
    {"S2", NULL, A, "STARTUP CHECK", "STARTPRI order"},
    {"S2d", "S2", A, "STARTUP CHECK", "no STARTPRI: oldest date added first, then name; GFOS (13 Nov)"},
    {"S2i", "S2", A, INDEX, "the ACFS index keeps the date added: moved out and back is added again (13 Nov)"},
    {"S3", NULL, A, "STARTUP CHECK", "DONOTWAIT and WAIT honoured"},
    /* 7. Disks */
    {"D1", NULL, A, "DISKIO", "disk in or out: its icon comes or goes"},
    {"D2", NULL, A, "MOUNTICON", "a volume mounted later gets an icon"},
    {"D3", NULL, A, MENUS, "Format: in GFOS a separate application, removable media (13 Nov)"},
    {"D4", NULL, A, MENUS, "DiskCopy: a separate application, removable media (13 Nov)"},
    {"D5", NULL, A, "DISKS", "RAM Disk icon"},
    {"D5r", NULL, A, "DISKS", "RAM Disk first in the desktop's volume column (14 Nov)"},
    {"D6", NULL, L, NULL, "ACFS, IDE SSD and USB volumes, USB eject"},
    /* 8. Screens */
    {"C1", NULL, A, "SCREENS", "depth gadget with a second screen"},
    {"C2", NULL, A, NULL, "Left-Amiga-M and N"},
    {"C2a", "C2", A, "SCREENS", "Left-Amiga-M"},
    {"C2b", "C2", A, "SCREENS", "Left-Amiga-N"},
    {"C3", NULL, L, NULL, "screen dragging; composited, an offset (13 Nov)"},
    {"C4", NULL, A, "SCREENS", "LockPubScreen(Workbench)"},
    {"C5", NULL, L, NULL, "ScreenMode change with windows open"},
    {"C6", NULL, A, "CLOSEWB", "CloseWorkBench / OpenWorkBench with programs running"},
    /* 9. Windows */
    {"W1", NULL, L, NULL, "sizing, zoom, depth on drawer windows"},
    {"W2", NULL, A, "SWITCH", "keyboard window cycling"},
    {"W3", NULL, A, NULL, "window and screen titles"},
    /* 10. Notifications */
    {"F1", NULL, A, "NOTIFY", "a new file shows without Update"},
    {"F2", NULL, A, "NOTIFY", "a changed .info updates its icon"},
    /* 11. Prefs */
    {"P1", NULL, L, NULL, "WBPattern"},
    {"P2", NULL, L, NULL, "icon text font"},
    {"P3", NULL, L, NULL, "Palette, through GFOpenLayout's pens (13 Nov)"},
    {"P4", NULL, A, NULL, "Pointer, as 3.2.3 (13 Nov)"},
    {"P4a", "P4", A, "SCREENS", "the pointer goes where it is moved, as 3.2.3 (13 Nov)"},
    {"P4b", "P4", A, "SCREENS", "on RTG the pointer is a sprite layer: the screen's pixels under it are untouched (13 Nov)"},
    {"P5", NULL, H, NULL, "IControl, as 3.2.3 (13 Nov)"},
    {"P6", NULL, L, NULL, "locale dates, sizes and sort order in lists"},
    {"P6r", "P6", L, NULL, "relative dates in lists, as an option (13 Nov)"},
    {"P7", NULL, A, NULL, "Workbench prefs: hidden devices, default stack"},
    /* 12. Robustness */
    {"X1", NULL, A, "BIG", "a drawer of 1000 files opens in 10 s"},
    {"X2", NULL, A, NULL, "long names and deep paths (13 Nov: GFOS and compat modes)"},
    {"X2a", "X2", A, INDEX, "GFOS mode: a 200-character name on an ACFS drive"},
    {"X2b", "X2", A, INDEX, "compat mode: the compat name ACFS offers, 107 characters at most"},
    {"X3", NULL, H, NULL, "low memory: a requester from the reserve pool (13 Nov)"},
    {"X4", NULL, H, NULL, "a crashed drawer task or AppWindow owner: the desktop stays up (13 Nov)"},
    /* 13. Layout (Dale, 13 Nov 06:43): on 3.2.3, reference only */
    {"L1", NULL, A, NULL, "a drawer with no snapshot: tools, projects, drawers, each by name; none clipped; the window fits"},
    {"L1a", "L1", A, "WBGapLayout.rexx", "grouped tools, then projects, then drawers, by name in each"},
    {"L1b", "L1", A, "WBGapLayout.rexx", "no icon clipped by the window"},
    {"L1c", "L1", A, "WBGapLayout.rexx", "the window fits its icons: none clipped, about one icon cell spare at most"},
    /* 14. The system bar (Dale, 14 Nov 03:41: the OS first) */
    {"B1", NULL, A, "BAR", "the Power icon's list: Reboot, Restart, Power off; Esc closes it, nothing picked (14 Nov)"},
    {"B2", NULL, A, "BAR", "an Eject row in Power's list for each USB volume mounted (14 Nov)"},
    {"B3", NULL, A, "BAR", "the speaker commodity runs after boot: GFOpenSpeaker's port is there (14 Nov)"},
};

#define NROWS ((int)(sizeof rows / sizeof rows[0]))
typedef char wbgap_rows_fit[NROWS <= WBGAP_MAXROWS ? 1 : -1];   /* the report has room for every row */

/* the modes of rows that have files of their own (tests/amiga/rows: WBGAP_ROWMODE lines, gen-rows.sh) */
static const char *const gen_rowmodes[][2] = {
#define WBGAP_GEN_ROWMODE(id, mode) { id, mode },
#include "wbgap_gen_rowmodes.inc"
#undef WBGAP_GEN_ROWMODE
    { NULL, NULL }
};

static void gen_modes(void)
{
    static int done;
    int k, i;
    if (done) return;
    done = 1;
    for (k = 0; gen_rowmodes[k][0]; k++)
        for (i = 0; i < NROWS; i++)
            if (!strcmp(rows[i].id, gen_rowmodes[k][0]) && !rows[i].mode) rows[i].mode = gen_rowmodes[k][1];
}

const char *wbgap_gen_rowmode(int k, const char **mode)
{
    if (k < 0 || k >= (int)(sizeof gen_rowmodes / sizeof gen_rowmodes[0]) - 1) return NULL;
    if (mode) *mode = gen_rowmodes[k][1];
    return gen_rowmodes[k][0];
}

int wbgap_row_count(void) { gen_modes(); return NROWS; }

const struct wbgap_row *wbgap_row(int i) { gen_modes(); return i >= 0 && i < NROWS ? &rows[i] : NULL; }

int wbgap_find(const char *id)
{
    int i;
    gen_modes();
    if (!id || !*id) return -1;
    for (i = 0; i < NROWS; i++)
        if (!strcmp(rows[i].id, id)) return i;
    return -1;
}

int wbgap_has_parts(int i)
{
    int j;
    gen_modes();
    if (i < 0 || i >= NROWS) return 0;
    for (j = 0; j < NROWS; j++)
        if (rows[j].parent && !strcmp(rows[j].parent, rows[i].id)) return 1;
    return 0;
}

void wbgap_report_init(struct wbgap_report *r) { memset(r, 0, sizeof *r); }

static void set(struct wbgap_report *r, int i, int state, const char *why, size_t len)
{
    if (state <= r->state[i]) return;
    r->state[i] = (unsigned char)state;
    if (len > WBGAP_WHY - 1) len = WBGAP_WHY - 1;
    memcpy(r->why[i], why, len);
    r->why[i][len] = 0;
}

int wbgap_report_line(struct wbgap_report *r, const char *line)
{
    const char *p = line, *id, *why;
    char key[16];
    size_t n, wl;
    int state, i, hit = 0;
    gen_modes();
    while (*p == ' ' || *p == '\t') p++;
    if (!strncmp(p, "PASS ", 5)) state = WBGAP_PASS;
    else if (!strncmp(p, "FAIL ", 5)) state = WBGAP_FAIL;
    else if (!strncmp(p, "SKIP ", 5)) state = WBGAP_SKIP;
    else return 0;
    id = p + 5;
    for (n = 0; id[n] && id[n] != ' ' && id[n] != '\r' && id[n] != '\n'; n++) {}
    if (!n || n >= sizeof key) return n ? -1 : 0;
    memcpy(key, id, n);
    key[n] = 0;
    why = id + n;
    while (*why == ' ') why++;
    for (wl = strlen(why); wl && (why[wl - 1] == '\n' || why[wl - 1] == '\r' || why[wl - 1] == ' '); wl--) {}
    if (key[n - 1] == '*') {   /* "FAIL M* why": the mode's checks didn't run, for each of its M rows with no line yet;
                                 * the mode is the first M row's that has one (M1's MENUS, A3's API) */
        const char *mode = NULL;
        for (i = 0; i < NROWS && !mode; i++)
            if (rows[i].mode && !strncmp(rows[i].id, key, n - 1) && rows[i].id[n - 1] >= '0' && rows[i].id[n - 1] <= '9')
                mode = rows[i].mode;
        for (i = 0; mode && i < NROWS; i++)
            if (rows[i].mode && !strcmp(rows[i].mode, mode) && !strncmp(rows[i].id, key, n - 1) &&
                rows[i].id[n - 1] >= '0' && rows[i].id[n - 1] <= '9') {
                hit = 1;
                if (r->state[i] == WBGAP_NONE) set(r, i, state, why, wl);
            }
        if (!hit) r->unknown++;
        return hit ? 1 : -1;
    }
    if ((i = wbgap_find(key)) < 0) { r->unknown++; return -1; }
    set(r, i, state, why, wl);
    return 1;
}

static const char *const words[] = {"", "SKIP", "PASS", "FAIL"};

int wbgap_report_print(const struct wbgap_report *r, wbgap_out out, void *ctx)
{
    char line[256];
    int i, j, npass = 0, nfail = 0, nskip = 0, notrun = 0;
    gen_modes();
    for (i = 0; i < NROWS; i++) {
        const struct wbgap_row *row = &rows[i];
        int state = r->state[i];
        if (state != WBGAP_NONE) {
            snprintf(line, sizeof line, "%s %s%s%s", words[state], row->id, r->why[i][0] ? " " : "", r->why[i]);
        } else if (!row->mode && wbgap_has_parts(i)) {   /* no check of its own: its parts decide it */
            char failed[96] = "", open[96] = "";
            int all = 1;
            for (j = i + 1; j < NROWS; j++) {
                const char *pid = rows[j].parent;
                char *to;
                if (!pid || strcmp(pid, row->id)) continue;
                if (r->state[j] == WBGAP_PASS) continue;
                all = 0;
                to = r->state[j] == WBGAP_FAIL ? failed : open;
                if (strlen(to) + strlen(rows[j].id) + 2 < sizeof failed) { strcat(to, " "); strcat(to, rows[j].id); }
            }
            if (failed[0]) { state = WBGAP_FAIL; snprintf(line, sizeof line, "FAIL %s failed:%s", row->id, failed); }
            else if (all) { state = WBGAP_PASS; snprintf(line, sizeof line, "PASS %s", row->id); }
            else if (row->how != WBGAP_AUTO)   /* a look or hand row: that, and its parts not settled */
                state = WBGAP_SKIP, snprintf(line, sizeof line, "SKIP %s %s: %s (and%s)", row->id,
                                             row->how == WBGAP_LOOK ? "look" : "hand", row->what, open);
            else { state = WBGAP_SKIP; snprintf(line, sizeof line, "SKIP %s not settled:%s", row->id, open); }
        } else {
            state = WBGAP_SKIP;
            if (row->how == WBGAP_LOOK) snprintf(line, sizeof line, "SKIP %s look: %s", row->id, row->what);
            else if (row->how == WBGAP_HAND) snprintf(line, sizeof line, "SKIP %s hand: %s", row->id, row->what);
            else if (row->mode) { snprintf(line, sizeof line, "SKIP %s not run: %s", row->id, row->mode); notrun++; }
            else snprintf(line, sizeof line, "SKIP %s not written: %s", row->id, row->what);
        }
        if (state == WBGAP_PASS) npass++;
        else if (state == WBGAP_FAIL) nfail++;
        else nskip++;
        out(ctx, line);
    }
    snprintf(line, sizeof line, "INFO rows %d: %d pass, %d fail, %d skip (%d not run)", NROWS, npass, nfail, nskip, notrun);
    out(ctx, line);
    return nfail;
}

int wbgap_launch_args(const char *rec, const char *want)
{
    long stack, args, words = 0;
    int pri, used = 0;
    const char *p, *w = want;
    if (sscanf(rec, "launch stack %ld pri %d args %ld%n", &stack, &pri, &args, &used) != 3 || !used) return 0;
    p = rec + used;
    for (;;) {
        size_t n, m;
        while (*p == ' ') p++;
        while (*w == ' ') w++;
        if (!*w) break;
        for (n = 0; p[n] && p[n] != ' ' && p[n] != '\n' && p[n] != '\r'; n++) {}
        for (m = 0; w[m] && w[m] != ' '; m++) {}
        if (n != m || strncmp(p, w, n)) return 0;
        p += n;
        w += m;
        words++;
    }
    while (*p == ' ' || *p == '\n' || *p == '\r') p++;
    return !*p && args == words;
}

int wbgap_i10(const char *skip, int bad, int clicked, const char *run_rec, int sub_opened, char *why, unsigned cap)
{
    int ran = run_rec && wbgap_launch_args(run_rec, "OpenT");
    const char *w;
    if (skip && *skip) w = skip;
    else if (bad) w = "a double-click failed (WBGapDesk.rexx)";
    else if (!clicked) w = "not clicked: WBGapDesk.rexx didn't get to OpenT and Sub";
    else if (!ran && !sub_opened) w = run_rec ? "OpenT started with other arguments, and Sub's window didn't open"
                                              : "double-clicking OpenT started nothing, and Sub's window didn't open";
    else if (!ran) w = run_rec ? "OpenT started, but not with itself as its one argument"
                               : "double-clicking OpenT started nothing";
    else if (!sub_opened) w = "double-clicking the drawer Sub opened no window";
    else w = "";
    if (why && cap) snprintf(why, cap, "%s", w);
    if ((skip && *skip) || bad || !clicked) return -1;
    return ran && sub_opened;
}

int wbgap_backdrop_setup(long n, long cap, int has_lines)
{
    if (n >= cap - 1) return WBGAP_BD_TOOLONG;   /* more than the buffer holds: written back cut, it would lose lines */
    if (n < 0) return WBGAP_BD_MADE;
    return has_lines ? WBGAP_BD_HAD : WBGAP_BD_SAVED;
}

int wbgap_backdrop_done(int have_was, int have_made)
{
    if (have_was) return WBGAP_BD_RESTORE;
    return have_made ? WBGAP_BD_DELETE : WBGAP_BD_LEAVE;
}

void wbgap_power_point(long left, long top, long width, long height, long xy[2])
{
    long cell = width < WBGAP_TRAY_CELL ? width : WBGAP_TRAY_CELL;
    xy[0] = left + width - (cell + 1) / 2;
    xy[1] = top + height / 2;
}

int wbgap_bar_menu_owner(long row_right, long menu_left, long menu_width)
{
    long right = menu_left + menu_width;
    if (right == row_right) return WBGAP_BAR_POWER;
    if (right == row_right - WBGAP_TRAY_CELL) return WBGAP_BAR_PREFS;
    if (menu_left == 0 && right < row_right) return WBGAP_BAR_UNSURE;   /* x clamped to 0: Power or Prefs */
    return WBGAP_BAR_OTHER;
}

int wbgap_point_in_box(long x, long y, long l, long t, long w, long h)
{
    return x >= l && x < l + w && y >= t && y < t + h;
}

long wbgap_power_height(int sticks, long row_height)
{
    if (sticks < 0) sticks = 0;
    return 3 + sticks * row_height + (sticks ? 6 : 0) + 3 * row_height + 3;
}

void wbgap_power_judge(long height, int sticks, long row_height, int *b1, int *b2)
{
    if (row_height > 0 && height == wbgap_power_height(sticks, row_height)) {
        *b1 = WBGAP_PASS;
        *b2 = sticks > 0 ? WBGAP_PASS : WBGAP_SKIP;
    } else if (row_height > 0 && sticks > 0 && height == wbgap_power_height(0, row_height)) {
        *b1 = WBGAP_PASS;   /* the power rows are there; the sticks' Eject rows aren't */
        *b2 = WBGAP_FAIL;
    } else {
        *b1 = WBGAP_FAIL;
        *b2 = sticks > 0 ? WBGAP_FAIL : WBGAP_SKIP;
    }
}

void wbgap_park_box(long sw, long sh, long minw, long minh, long box[4])
{
    long w = minw > 0 ? minw : 1, h = minh > 0 ? minh : 1;
    if (sw < 1) sw = 1;
    if (sh < 1) sh = 1;
    if (w > sw) w = sw;
    if (h > sh) h = sh;
    box[0] = sw - w;
    box[1] = sh - h;
    box[2] = w;
    box[3] = h;
}

int wbgap_clear_result(int gone, int at_park_box)
{
    if (gone) return WBGAP_CL_GONE;
    return at_park_box ? WBGAP_CL_PARKED : WBGAP_CL_STUCK;
}

int wbgap_desk_clickable(int clear_result)
{
    return clear_result == WBGAP_CL_GONE || clear_result == WBGAP_CL_PARKED;
}

void wbgap_volume_name(const char *path, char *out, unsigned cap)
{
    unsigned k = 0;
    if (!cap) return;
    while (path && path[k] && path[k] != ':' && k + 1 < cap) { out[k] = path[k]; k++; }
    out[k] = 0;
}

static int same_nocase(const char *a, const char *b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++) {
        int x = (unsigned char)a[i], y = (unsigned char)b[i];
        if (x >= 'a' && x <= 'z') x -= 32;
        if (y >= 'a' && y <= 'z') y -= 32;
        if (x != y) return 0;
    }
    return 1;
}

int wbgap_root_has_disk(const char *list, const char *vol)
{
    size_t vn = vol ? strlen(vol) : 0;
    const char *p = list;
    if (!list || !vn) return 0;
    while (*p) {
        const char *e = p, *name;
        size_t n;
        while (*e && *e != '\n') e++;
        n = (size_t)(e - p);
        while (n && (p[n - 1] == '\r' || p[n - 1] == ' ')) n--;
        if (n > 6 && p[n - 1] == ':') n--;          /* the WORKBENCH port names a disk "RAM Disk:" */
        if (n > 5 && same_nocase(p, "DISK ", 5)) {
            name = p + 5;
            if ((size_t)(p + n - name) == vn && same_nocase(name, vol, vn)) return 1;
        }
        p = *e ? e + 1 : e;
    }
    return 0;
}

/* One line of D5r's pos, [p, e), read on its own: a copy of it (up to 127 characters) is what sscanf sees, so a
 * blank in the format can't run on into the next line. The name is what follows the width, less trailing CR,
 * blanks and tabs and one trailing colon (a port that names disks "RAM Disk:", as #145 for D5). 1 with x, y, w,
 * name and its length; 0 for a line it can't read or a line with no name after the width (Mr Flibble, 7456fde7). */
static int pos_line(const char *p, const char *e, long *x, long *y, long *w, char *ln, const char **nm, size_t *nl)
{
    size_t n = (size_t)(e - p);
    int used = 0;
    if (n > 127) n = 127;
    memcpy(ln, p, n);
    ln[n] = 0;
    if (sscanf(ln, "%ld %ld %ld %n", x, y, w, &used) != 3 || !used || (size_t)used > n) return 0;
    n -= (size_t)used;
    while (n && (ln[used + n - 1] == '\r' || ln[used + n - 1] == ' ' || ln[used + n - 1] == '\t')) n--;
    if (n && ln[used + n - 1] == ':') n--;
    if (!n) return 0;
    *nm = ln + used;
    *nl = n;
    return 1;
}

/* D5r: pos holds the desktop's disk icons, "<left> <top> <width> <name>" a line (WBGapRoot.rexx's
 * RAM:wbgap-root-pos). The column is the disks whose left edge is less than one icon's width (16 at least) right of
 * the leftmost; its topmost (the first read wins a tie) must be vol, case not minded. top gets that icon's name.
 * A line with no name is passed over. 1: RAM Disk first; 0: another first; -1: no disk read (or cap 0). */
int wbgap_ram_first(const char *pos, const char *vol, char *top, unsigned cap)
{
    long minx = 0, cw = 16, besty = 0;
    int pass, have = 0;
    const char *p, *nm, *e;
    char ln[128];
    size_t vn = vol ? strlen(vol) : 0, n;
    if (!cap) return -1;
    top[0] = 0;
    if (!pos) return -1;
    for (p = pos; *p; p = *e ? e + 1 : e) {                 /* the leftmost disk and its width */
        long x, y, w;
        for (e = p; *e && *e != '\n'; e++) ;
        if (pos_line(p, e, &x, &y, &w, ln, &nm, &n)) {
            if (!have || x < minx) { minx = x; cw = w < 16 ? 16 : w; }
            have = 1;
        }
    }
    if (!have) return -1;
    have = 0;
    for (p = pos; *p; p = *e ? e + 1 : e) {                 /* the column's topmost */
        long x, y, w;
        for (e = p; *e && *e != '\n'; e++) ;
        if (pos_line(p, e, &x, &y, &w, ln, &nm, &n) && x - minx < cw && (!have || y < besty)) {
            if (n >= cap) n = cap - 1;
            memcpy(top, nm, n);
            top[n] = 0;
            besty = y;
            have = 1;
        }
    }
    pass = vn && strlen(top) == vn && same_nocase(top, vol, vn);
    return pass;
}

int wbgap_root_has_name(const char *list, const char *name)
{
    size_t nn = name ? strlen(name) : 0;
    const char *p = list;
    if (!list || !nn) return 0;
    while (*p) {
        const char *e = p, *sp;
        size_t n;
        while (*e && *e != '\n') e++;
        n = (size_t)(e - p);
        while (n && (p[n - 1] == '\r' || p[n - 1] == ' ')) n--;
        for (sp = p; sp < p + n && *sp != ' '; sp++) ;
        if (sp < p + n) {
            sp++;
            if ((size_t)(p + n - sp) == nn && same_nocase(sp, name, nn)) return 1;
        }
        p = *e ? e + 1 : e;
    }
    return 0;
}

int wbgap_showall_judge(int only_icons, int all_files, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_PASS;
    if (only_icons < 0 || all_files < 0) { r = "the drawer's icons not read each time"; v = WBGAP_SKIP; }
    else if (only_icons) { r = "noicon was shown with Only Icons"; v = WBGAP_FAIL; }
    else if (!all_files) { r = "noicon not shown with All Files"; v = WBGAP_FAIL; }
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_diskio_judge(int before, int inhibited, int after, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_PASS;
    if (!before) { r = "the disk had no icon before"; v = WBGAP_SKIP; }
    else if (before < 0 || inhibited < 0 || after < 0) { r = "the desktop's icons not read each time"; v = WBGAP_SKIP; }
    else if (inhibited && !after) { r = "its icon stayed while inhibited and then went"; v = WBGAP_FAIL; }
    else if (inhibited) { r = "its icon stayed while inhibited"; v = WBGAP_FAIL; }
    else if (!after) { r = "its icon went but did not come back"; v = WBGAP_FAIL; }
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_ctl_may_set(int get_ok) { return get_ok != 0; }

long wbgap_ctl_reading(int get_ok, long read_back) { return get_ok ? read_back : WBGAP_CTL_NOT_READ; }

int wbgap_pairs_judge(int n, const char *const names[], const long want[], const long got[], char *why, unsigned cap)
{
    int k, bad = 0, first = -1;
    for (k = 0; k < n; k++)
        if (want[k] != got[k]) { bad++; if (first < 0) first = k; }
    if (why && cap) {
        if (first < 0) why[0] = 0;
        else snprintf(why, cap, "%s: %ld, not %ld%s", names[first], got[first], want[first],
                      bad > 1 ? " (and more wrong)" : "");
    }
    return n <= 0 ? WBGAP_SKIP : bad ? WBGAP_FAIL : WBGAP_PASS;
}

int wbgap_mount_judge(int was_there, int before, int mount_rc, int vol_up, int after, char *why, unsigned cap)
{
    char r[96] = "";
    int v = WBGAP_SKIP;
    if (was_there) snprintf(r, sizeof r, "the drive was mounted before the test: no later mount to watch");
    else if (before < 0) snprintf(r, sizeof r, "the desktop's icons not read before the mount");
    else if (mount_rc) snprintf(r, sizeof r, "Mount gave %d (no mount list for the drive?)", mount_rc);
    else if (!vol_up) snprintf(r, sizeof r, "mounted, but no volume came up on it");
    else if (after < 0) snprintf(r, sizeof r, "the desktop's icons not read after the mount");
    else if (!after) { snprintf(r, sizeof r, "its volume came up but got no icon on the desktop"); v = WBGAP_FAIL; }
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_has_nocase(const char *text, const char *word)
{
    size_t wn = word ? strlen(word) : 0;
    const char *p;
    if (!text || !wn) return 0;
    for (p = text; *p; p++)
        if (strlen(p) >= wn && same_nocase(p, word, wn)) return 1;
    return 0;
}

int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_FAIL;
    if (closed && !reopened) r = "the screen closed and OpenWorkBench() didn't bring it back";
    else if (!closed && !same_screen) r = "CloseWorkBench() refused, but the Workbench screen changed";
    else if (!port) r = closed ? "the desktop didn't come back after OpenWorkBench()" : "the desktop went though CloseWorkBench() refused";
    else if (!drawer) r = closed ? "RAM:WBGapC's window wasn't opened again" : "RAM:WBGapC's window went though CloseWorkBench() refused";
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_closewb_over(const char *text)
{
    const char *p, *want = "INFO C6 done";
    size_t n = strlen(want);
    if (!text) return 0;
    for (p = text; (p = strstr(p, want)); p += n)
        if ((p == text || p[-1] == '\n') && (p[n] == '\n' || p[n] == '\r' || p[n] == 0)) return 1;
    return 0;
}

void wbgap_which_point(long bl, long bt, long cx, long cy, long xy[2])
{
    xy[0] = bl + cx + 4;
    xy[1] = bt + cy + 4;
}

int wbgap_switcher_on(const char *env, char *key, unsigned cap)
{
    const char *p = env;
    int on = 1;
    if (cap) snprintf(key, cap, "lcommand tab");
    while (p && *p) {
        const char *e = p, *v;
        size_t n;
        while (*e && *e != '\n') e++;
        n = (size_t)(e - p);
        while (n && (p[n - 1] == '\r' || p[n - 1] == ' ' || p[n - 1] == '\t')) n--;
        while (n && (*p == ' ' || *p == '\t')) { p++; n--; }
        if (n > 13 && !strncmp(p, "switcher.key", 12) && (p[12] == ' ' || p[12] == '\t')) {
            for (v = p + 13; v < p + n && (*v == ' ' || *v == '\t'); v++) {}
            if (cap) snprintf(key, cap, "%.*s", (int)(p + n - v), v);
        } else if (n > 9 && !strncmp(p, "switcher", 8) && (p[8] == ' ' || p[8] == '\t')) {
            for (v = p + 9; v < p + n && (*v == ' ' || *v == '\t'); v++) {}
            if ((size_t)(p + n - v) == 3 && !strncmp(v, "off", 3)) on = 0;
            else if ((size_t)(p + n - v) == 2 && !strncmp(v, "on", 2)) on = 1;
        }
        p = *e ? e + 1 : e;
    }
    return on;
}

int wbgap_switch_offers(unsigned long flags, int width, int height)
{
    if ((flags & 0x100UL) || width < 8 || height < 8) return 0;            /* WFLG_BACKDROP, tiny */
    if ((flags & 0x2000000UL) && !(flags & 0x8UL)) return 0;               /* WFLG_WBENCHWINDOW without CLOSEGADGET */
    return (flags & 0xFUL) != 0;                                            /* SIZE, DRAGBAR, DEPTH or CLOSE gadget */
}

int wbgap_switch_judge(int back, const unsigned long *before, int nb, const unsigned long *after, int na,
                       unsigned long active, char *why, unsigned cap)
{
    unsigned long want_top, want_active;
    if (!before || !after || nb < 2) {
        snprintf(why, cap, "fewer than two windows to cycle");
        return WBGAP_SKIP;
    }
    if (na != nb) {
        snprintf(why, cap, "%d windows before, %d after: a window came or went", nb, na);
        return WBGAP_FAIL;
    }
    if (!back) {
        want_top = before[nb - 1];
        want_active = before[nb - 1];
        if (after[0] != want_top) {
            snprintf(why, cap, "Amiga+Tab: the window at the back didn't come to the front");
            return WBGAP_FAIL;
        }
    } else {
        want_active = before[1];
        if (after[na - 1] != before[0]) {
            snprintf(why, cap, "Amiga+Shift+Tab: the front window didn't go to the back");
            return WBGAP_FAIL;
        }
    }
    if (active != want_active) {
        snprintf(why, cap, "%s: the window now %s isn't the active one", back ? "Amiga+Shift+Tab" : "Amiga+Tab",
                 back ? "in front" : "brought to the front");
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}

int wbgap_which_judge(unsigned long on_icon, const char *name, const char *want, unsigned long on_empty, char *why,
                      unsigned cap)
{
    if (on_icon != WBGAP_WBO_ICON) {
        snprintf(why, cap, "on the icon: type %lu, not WBO_ICON", on_icon);
        return WBGAP_FAIL;
    }
    if (!name || !want || strcmp(name, want)) {
        snprintf(why, cap, "on the icon: named \"%s\", not \"%s\"", name ? name : "", want ? want : "");
        return WBGAP_FAIL;
    }
    if (on_empty != WBGAP_WBO_DRAWER) {
        snprintf(why, cap, "on the window's empty corner: type %lu, not WBO_DRAWER", on_empty);
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}

static int boxes_meet(const long a[4], const long b[4])
{
    return a[0] < b[0] + b[2] && b[0] < a[0] + a[2] && a[1] < b[1] + b[3] && b[1] < a[1] + a[3];
}

int wbgap_drop_place(long sw, long sh, long top, long w, long h, const long *avoid, long box[4])
{
    int k;
    if (w < 1 || h < 1 || top < 0 || w > sw || h > sh - top) return 0;
    for (k = 0; k < 4; k++) {
        long b[4];
        b[0] = (k == 0 || k == 1) ? sw - w : 0;
        b[1] = (k == 1 || k == 2) ? sh - h : top;
        b[2] = w;
        b[3] = h;
        if (avoid && boxes_meet(b, avoid)) continue;
        memcpy(box, b, sizeof b);
        return 1;
    }
    return 0;
}

void wbgap_drop_point(const long box[4], long xy[2])
{
    xy[0] = box[0] + box[2] / 2;
    xy[1] = box[1] + box[3] / 2;
}

int wbgap_move_judge(int src, int src_info, int dst, int dst_info, const char *name, const char *where, char *why,
                     unsigned cap)
{
    if (!name) name = "";
    if (!where) where = "";
    if (!dst) {
        snprintf(why, cap, "%s isn't in %s after the drag%s", name, where, src ? " (still where it was)" : " (and gone from where it was)");
        return WBGAP_FAIL;
    }
    if (src) {
        snprintf(why, cap, "%s is in %s and still where it was: copied, not moved", name, where);
        return WBGAP_FAIL;
    }
    if (!dst_info) {
        snprintf(why, cap, "%s moved to %s without its icon (%s.info)%s", name, where, name, src_info ? ", which stayed behind" : "");
        return WBGAP_FAIL;
    }
    if (src_info) {
        snprintf(why, cap, "%s.info is in %s and still where it was", name, where);
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}

int wbgap_drop_judge(int got, long type, long want_type, unsigned long id, unsigned long want_id, long nargs,
                     const char *name, const char *dir, const char *want_name, const char *want_dir, char *why,
                     unsigned cap)
{
    size_t dn = dir ? strlen(dir) : 0, wn = want_dir ? strlen(want_dir) : 0;
    if (!got) {
        snprintf(why, cap, "no AppMessage came");
        return WBGAP_FAIL;
    }
    if (type != want_type || id != want_id) {
        snprintf(why, cap, "an AppMessage of type %ld id %lu, not type %ld id %lu", type, id, want_type, want_id);
        return WBGAP_FAIL;
    }
    if (nargs < 1) {
        snprintf(why, cap, "the AppMessage has no arguments (am_NumArgs %ld)", nargs);
        return WBGAP_FAIL;
    }
    if (!name || !want_name || strcmp(name, want_name)) {
        snprintf(why, cap, "its first argument is \"%s\", not \"%s\"", name ? name : "", want_name ? want_name : "");
        return WBGAP_FAIL;
    }
    if (!wn || dn <= wn || strcmp(dir + dn - wn, want_dir) || (dir[dn - wn - 1] != ':' && dir[dn - wn - 1] != '/')) {
        snprintf(why, cap, "its first argument's drawer is \"%s\", not ...%s", dir ? dir : "", want_dir ? want_dir : "");
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}

int wbgap_parse_box(const char *text, long box[4])
{
    long b[4];
    char *e;
    const char *p = text;
    int k;
    if (!p) return 0;
    for (k = 0; k < 4; k++) {
        b[k] = strtol(p, &e, 10);
        if (e == p) return 0;
        p = e;
    }
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p || b[2] < 1 || b[3] < 1) return 0;
    memcpy(box, b, sizeof b);
    return 1;
}

int wbgap_diff_box(const unsigned long *before, const unsigned long *after, int gw, int gh, int step, long box[4])
{
    int x, y, x0 = gw, y0 = gh, x1 = -1, y1 = -1;
    if (!before || !after || gw < 1 || gh < 1 || step < 1) return 0;
    for (y = 0; y < gh; y++)
        for (x = 0; x < gw; x++)
            if (before[y * gw + x] != after[y * gw + x]) {
                if (x < x0) x0 = x;
                if (x > x1) x1 = x;
                if (y < y0) y0 = y;
                if (y > y1) y1 = y;
            }
    if (x1 < 0) return 0;
    box[0] = (long)x0 * step;
    box[1] = (long)y0 * step;
    box[2] = (long)(x1 - x0 + 1) * step;
    box[3] = (long)(y1 - y0 + 1) * step;
    return 1;
}

void wbgap_grab_point(const long box[4], long xy[2])
{
    xy[0] = box[0] + box[2] / 2;
    xy[1] = box[1] + box[3] * 2 / 5;
}

int wbgap_rawkey(int c, int *shift)
{
    static const char *const rows[] = { "1234567890-=", "qwertyuiop", "asdfghjkl", "zxcvbnm" };
    static const int base[] = { 0x01, 0x10, 0x20, 0x31 };
    int r, i;
    *shift = c >= 'A' && c <= 'Z';
    if (*shift) c += 32;
    if (c == ' ') return 0x40;
    if (c == '.') return 0x39;
    if (c == '/') return 0x3A;
    for (r = 0; r < 4; r++)
        for (i = 0; rows[r][i]; i++)
            if (rows[r][i] == c) return base[r] + i;
    *shift = 0;
    return -1;
}

int wbgap_rx_line(char *buf, unsigned cap, const char *path, const char *args)
{
    int n;
    if (!cap) return 0;
    buf[0] = 0;
    if (!path || !*path || strpbrk(path, " \t\"'")) return 0;
    n = snprintf(buf, cap, "SYS:Rexxc/rx %s%s%s", path, args && *args ? " " : "", args ? args : "");
    if (n < 0 || (unsigned)n >= cap) { buf[0] = 0; return 0; }
    return 1;
}
