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
#define INDEX "WBGapIndex"

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
    {"M27", NULL, A, NULL, "Tools items added by programs (AddAppMenuItemA) shown"},
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
    {"I4", NULL, L, NULL, "drag between drawer windows"},
    {"I5", NULL, L, NULL, "drag to the backdrop"},
    {"I6", NULL, L, NULL, "drag to the Trashcan"},
    {"I7", NULL, L, NULL, "drag onto a drawer or disk icon"},
    {"I8", NULL, A, NULL, "drag onto an AppIcon"},
    {"I9", NULL, A, NULL, "drag onto an AppWindow"},
    {"I10", NULL, A, NULL, "double-click opens or runs"},
    {"I11", NULL, A, "LAUNCHDESK", "several icons passed as arguments (Shift-double-click on the desktop, as 3.2.3)"},
    {"I12", NULL, A, NULL, "STACK, TOOLPRI, DONOTWAIT and WAIT honoured at launch (WAIT, DONOTWAIT: S3)"},
    {"I12a", "I12", A, "LAUNCH, STARTUP CHECK", "the icon's stack (its field and STACK=) gives the tool that stack"},
    {"I12b", "I12", A, "LAUNCH, STARTUP CHECK", "TOOLPRI= gives the tool that priority"},
    {"I12c", "I12", A, "STARTUP CHECK", "STACK= alone, the icon's stack field the default, gives that stack; GFOS (3.2.3 reads the field)"},
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
};

#define NROWS ((int)(sizeof rows / sizeof rows[0]))
typedef char wbgap_rows_fit[NROWS <= WBGAP_MAXROWS ? 1 : -1];   /* the report has room for every row */

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

void wbgap_which_point(long bl, long bt, long cx, long cy, long xy[2])
{
    xy[0] = bl + cx + 4;
    xy[1] = bt + cy + 4;
}

int wbgap_which_judge(unsigned long on_icon, const char *name, const char *want, unsigned long on_empty, char *why,
                      unsigned cap)
{
    if (on_icon != 2) {
        snprintf(why, cap, "on the icon: type %lu, not WBO_ICON", on_icon);
        return WBGAP_FAIL;
    }
    if (!name || !want || strcmp(name, want)) {
        snprintf(why, cap, "on the icon: named \"%s\", not \"%s\"", name ? name : "", want ? want : "");
        return WBGAP_FAIL;
    }
    if (on_empty != 1) {
        snprintf(why, cap, "on the window's empty corner: type %lu, not WBO_DRAWER", on_empty);
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}
