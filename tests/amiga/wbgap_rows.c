/* wbgap_rows: WBGapTest's list of gap ids and its one-line-per-id report (wbgap_rows.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"

#define A WBGAP_AUTO
#define L WBGAP_LOOK
#define H WBGAP_HAND
#define MENUS "MENUS"
#define API "API"
#define REXX "WBGapRexx.rexx"

/* in the document's order; a part ("A8a") straight after its row. Rows added for the 13 Nov decisions say "(13 Nov)". */
static const struct wbgap_row rows[] = {
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
    {"M16", NULL, A, MENUS, "Window > View By"},
    {"M16t", "M16", L, NULL, "the drawer keeps its view as SORT= in its icon (13 Nov)"},
    {"M17", NULL, A, MENUS, "Icons > Open"},
    {"M18", NULL, A, MENUS, "Icons > Copy"},
    {"M18p", "M18", L, NULL, "Copy's progress bar on the source drawer or drive (13 Nov)"},
    {"M19", NULL, A, MENUS, "Icons > Rename"},
    {"M20", NULL, A, MENUS, "Icons > Information"},
    {"M21", NULL, A, MENUS, "Icons > Snapshot / Unsnapshot"},
    {"M22", NULL, A, MENUS, "Icons > Leave Out / Put Away"},
    {"M23", NULL, A, MENUS, "Icons > Delete"},
    {"M23r", "M23", A, NULL, "a deleted file recovers from the trash until the host reclaims it (13 Nov)"},
    {"M24", NULL, A, MENUS, "Icons > Format Disk; in GFOS a separate Format application (13 Nov)"},
    {"M25", NULL, A, MENUS, "Icons > Empty Trash"},
    {"M26", NULL, A, MENUS, "Tools > ResetWB; in GFOS Refresh OpenBench (13 Nov)"},
    {"M27", NULL, A, NULL, "Tools items added by programs (AddAppMenuItemA) shown"},
    {"M28", NULL, A, MENUS, "Amiga-keys as 3.2.3"},
    {"M28k", "M28", H, NULL, "KEYS= on the Workbench icon remaps them (13 Nov)"},
    {"M29", NULL, A, MENUS, "Window > Open volume (13 Nov)"},
    {"M30", NULL, A, MENUS, "Window > Clear selection"},
    {"M31", NULL, A, MENUS, "Window > Resize to fit"},
    {"M32", NULL, A, MENUS, "Window > Sort order > Reverse"},
    {"M32t", "M32", L, NULL, "reverse kept as SORT=<key>,REVERSE; a column header toggles it (13 Nov)"},
    {"M33", NULL, A, MENUS, "Window > Find"},
    {"M33r", "M33", L, NULL, "Find's results as icons in icon view, a list in list view (13 Nov)"},
    {"M34", NULL, A, MENUS, "Auto-arrange icons"},
    {"M34t", "M34", L, NULL, "Auto-arrange kept per drawer as ARRANGE= (13 Nov)"},
    {"M35", NULL, A, MENUS, "Icons > Eject, any disk (13 Nov)"},
    /* 2. Icons */
    {"I1", NULL, L, NULL, "click selects, click elsewhere deselects"},
    {"I2", NULL, L, NULL, "Shift-click adds to the selection"},
    {"I3", NULL, L, NULL, "lasso, icons only (13 Nov)"},
    {"I4", NULL, L, NULL, "drag between drawer windows"},
    {"I5", NULL, L, NULL, "drag to the backdrop"},
    {"I6", NULL, L, NULL, "drag to the Trashcan"},
    {"I7", NULL, L, NULL, "drag onto a drawer or disk icon"},
    {"I8", NULL, A, NULL, "drag onto an AppIcon"},
    {"I9", NULL, A, NULL, "drag onto an AppWindow"},
    {"I10", NULL, A, NULL, "double-click opens or runs"},
    {"I11", NULL, A, "LAUNCH", "several icons passed as arguments (Shift-double-click, as 3.2.3; needs A6)"},
    {"I12", NULL, A, NULL, "STACK, TOOLPRI, DONOTWAIT and WAIT honoured at launch (WAIT, DONOTWAIT: S3)"},
    {"I12a", "I12", A, "LAUNCH, STARTUP CHECK", "STACK= gives the tool that stack"},
    {"I12b", "I12", A, "LAUNCH, STARTUP CHECK", "TOOLPRI= gives the tool that priority"},
    {"I13", NULL, L, NULL, "icon formats: classic, NewIcons, GlowIcons, PNG"},
    {"I14", NULL, A, NULL, "files with no icon shown (Show All Files)"},
    {"I15", NULL, L, NULL, "a disk's status window with a pie chart, no gauge (13 Nov)"},
    {"I16", NULL, H, NULL, "keyboard selection everywhere: cursor keys, Return, Tab (13 Nov)"},
    /* 3. Information window */
    {"N1", NULL, L, NULL, "name, type, size, date shown"},
    {"N2", NULL, A, NULL, "protection bits SARWED saved"},
    {"N3", NULL, A, NULL, "comment saved"},
    {"N4", NULL, A, NULL, "tool types added, edited, deleted, saved"},
    {"N5", NULL, A, NULL, "default tool saved"},
    {"N6", NULL, A, NULL, "Stack field (13 Nov: as 3.2.3)"},
    {"N7", NULL, A, NULL, "H and P bits, in an Advanced section (13 Nov)"},
    {"N8", NULL, A, API, "WBInfo from a program opens it"},
    /* 4. workbench.library */
    {"A1", NULL, A, NULL, "AddAppWindowA and its messages"},
    {"A2", NULL, A, NULL, "AddAppWindowDropZoneA"},
    {"A3", NULL, A, API, "AddAppIconA"},
    {"A4", NULL, A, API, "AddAppMenuItemA"},
    {"A5", NULL, A, NULL, "WorkbenchControlA tags"},
    {"A6", NULL, A, API, "OpenWorkbenchObjectA"},
    {"A6b", "A6", A, API, "CloseWorkbenchObjectA"},
    {"A7", NULL, A, API, "UpdateWorkbench, the refresh proven (13 Nov)"},
    {"A8", NULL, A, NULL, "the object calls"},
    {"A8a", "A8", A, API, "MakeWorkbenchObjectVisibleA"},
    {"A8b", "A8", A, API, "ChangeWorkbenchSelectionA"},
    {"A8c", "A8", A, NULL, "WhichWorkbenchObjectA"},
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
    {"S3", NULL, A, "STARTUP CHECK", "DONOTWAIT and WAIT honoured"},
    /* 7. Disks */
    {"D1", NULL, A, NULL, "disk in or out: its icon comes or goes"},
    {"D2", NULL, A, NULL, "a volume mounted later gets an icon"},
    {"D3", NULL, A, MENUS, "Format: in GFOS a separate application, removable media (13 Nov)"},
    {"D4", NULL, A, MENUS, "DiskCopy: a separate application, removable media (13 Nov)"},
    {"D5", NULL, A, NULL, "RAM Disk icon"},
    {"D6", NULL, L, NULL, "ACFS, IDE SSD and USB volumes, USB eject"},
    /* 8. Screens */
    {"C1", NULL, A, "SCREENS", "depth gadget with a second screen"},
    {"C2", NULL, A, NULL, "Left-Amiga-M and N"},
    {"C2a", "C2", A, "SCREENS", "Left-Amiga-M"},
    {"C2b", "C2", A, "SCREENS", "Left-Amiga-N"},
    {"C3", NULL, L, NULL, "screen dragging; composited, an offset (13 Nov)"},
    {"C4", NULL, A, "SCREENS", "LockPubScreen(Workbench)"},
    {"C5", NULL, L, NULL, "ScreenMode change with windows open"},
    {"C6", NULL, A, NULL, "CloseWorkBench / OpenWorkBench with programs running"},
    /* 9. Windows */
    {"W1", NULL, L, NULL, "sizing, zoom, depth on drawer windows"},
    {"W2", NULL, A, NULL, "keyboard window cycling"},
    {"W3", NULL, A, NULL, "window and screen titles"},
    /* 10. Notifications */
    {"F1", NULL, A, "NOTIFY", "a new file shows without Update"},
    {"F2", NULL, A, "NOTIFY", "a changed .info updates its icon"},
    /* 11. Prefs */
    {"P1", NULL, L, NULL, "WBPattern"},
    {"P2", NULL, L, NULL, "icon text font"},
    {"P3", NULL, L, NULL, "Palette, through GFOpenLayout's pens (13 Nov)"},
    {"P4", NULL, L, NULL, "Pointer, as 3.2.3 (13 Nov)"},
    {"P4b", "P4", L, NULL, "on RTG the pointer is a sprite layer (13 Nov)"},
    {"P5", NULL, H, NULL, "IControl, as 3.2.3 (13 Nov)"},
    {"P6", NULL, L, NULL, "locale dates, sizes and sort order in lists"},
    {"P6r", "P6", L, NULL, "relative dates in lists, as an option (13 Nov)"},
    {"P7", NULL, A, NULL, "Workbench prefs: hidden devices, default stack"},
    /* 12. Robustness */
    {"X1", NULL, A, "BIG", "a drawer of 1000 files opens in 10 s"},
    {"X2", NULL, A, NULL, "long names and deep paths (13 Nov: GFOS and compat modes)"},
    {"X2a", "X2", A, NULL, "GFOS mode: a name and path past 3.2.3's limits"},
    {"X2b", "X2", A, NULL, "compat mode: the compat name ACFS offers"},
    {"X3", NULL, H, NULL, "low memory: a requester from the reserve pool (13 Nov)"},
    {"X4", NULL, H, NULL, "a crashed drawer task or AppWindow owner: the desktop stays up (13 Nov)"},
    /* 13. Layout (Dale, 13 Nov 06:43): on 3.2.3, reference only */
    {"L1", NULL, A, NULL, "a drawer with no snapshot: tools, projects, drawers, each by name; none clipped; the window fits"},
    {"L1a", "L1", A, "WBGapLayout.rexx", "grouped tools, then projects, then drawers, by name in each"},
    {"L1b", "L1", A, "WBGapLayout.rexx", "no icon clipped by the window"},
    {"L1c", "L1", A, "WBGapLayout.rexx", "the window fits its icons: none clipped, about one icon cell spare at most"},
};

#define NROWS ((int)(sizeof rows / sizeof rows[0]))

int wbgap_row_count(void) { return NROWS; }

const struct wbgap_row *wbgap_row(int i) { return i >= 0 && i < NROWS ? &rows[i] : NULL; }

int wbgap_find(const char *id)
{
    int i;
    if (!id || !*id) return -1;
    for (i = 0; i < NROWS; i++)
        if (!strcmp(rows[i].id, id)) return i;
    return -1;
}

int wbgap_has_parts(int i)
{
    int j;
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
    if (key[n - 1] == '*') {   /* "FAIL M* why": the checks didn't run, for every checked M row with no line yet */
        for (i = 0; i < NROWS; i++)
            if (rows[i].mode && !strncmp(rows[i].id, key, n - 1) && rows[i].id[n - 1] >= '0' && rows[i].id[n - 1] <= '9') {
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
