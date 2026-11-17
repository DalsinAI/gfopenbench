/* ---- P3: Palette's colour 0 reaches a drawer's background (ENV:Sys/palette.prefs) ----
 * RAM:WBGapPal opened at the top-left, 400x300, with one icon at its top-left, so the inside's bottom-right is empty.
 * A 120 x 80 corner of it (every 8th pixel, where the drawer is the front window) is read before, after (up to 15 s)
 * and with the old file put back. The prefs are made from the file as it is with colour 0 set to WBGAP_PAL_RGB (with
 * no file: Palette's default colours with the screen's own pens; putting it back then writes the screen's own colours 0-7 as read before, and deletes
 * the file). The corner must be
 * one colour before (a drawer pattern or picture SKIPs), at least half the prefs' colour after, and lose it again.
 * 3.2.3: IPrefs resets the Workbench screen with the new colour 0, a drawer's BACKGROUNDPEN; the row opens the
 * drawer again on the new screen and reads it there. GFOS: the design has GFOpenLayout's
 * theme take Palette's pens (designs/WORKBENCH-REDESIGN-20261113.md P3); until it does, this row FAILs there.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "WBPATTERN.h"
#include "PALETTE.h"

WBGAP_MODE(PALETTE, wbt_palette, SCRIPT)
WBGAP_ROWMODE(P3, PALETTE)

#define PAL_FILE "ENV:Sys/palette.prefs"
#define PTRN_FILE "ENV:Sys/WBPattern.prefs"
#define PAL_DIR "RAM:WBGapPal"
#define PAL_TITLE "WBGapPal"
#define PAL_MAX 200
#define MAXW 48

static unsigned char was_buf[4096], new_buf[4096], tmp_buf[4096], pt_was[4096], pt_new[4096];
static short px[PAL_MAX], py[PAL_MAX];
static unsigned long seen[PAL_MAX];

static int put_file(const char *path, const unsigned char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG w;
    if (!fh) return 0;
    w = Write(fh, (APTR)buf, n);
    Close(fh);
    return w == n;
}

/* the inside's bottom-right corner, where d is the front window: the screen points into px, py */
static int points(struct Screen *s, struct Window *d)
{
    struct Window *w[MAXW];
    int nw = wbt_windows_now(s, w, MAXW), n = 0, x, y, k;
    int x1 = d->LeftEdge + d->Width - d->BorderRight - 8, y1 = d->TopEdge + d->Height - d->BorderBottom - 8;
    for (y = y1 - 80; y < y1 && n < PAL_MAX; y += 8)
        for (x = x1 - 120; x < x1 && n < PAL_MAX; x += 8) {
            struct Window *f = NULL;
            for (k = 0; k < nw && !f; k++)   /* front first */
                if (x >= w[k]->LeftEdge && x < w[k]->LeftEdge + w[k]->Width && y >= w[k]->TopEdge && y < w[k]->TopEdge + w[k]->Height)
                    f = w[k];
            if (f == d) px[n] = (short)x, py[n] = (short)y, n++;
        }
    return n;
}

static int count(struct Screen *s, int n)
{
    int i, c = 0;
    for (i = 0; i < n; i++) c += wbgap_rgb_near(wbt_screen_rgb(s, px[i], py[i]), WBGAP_PAL_RGB, WBGAP_PAL_TOL);
    return c;
}

/* the drawer still on s: an IPrefs reset of the Workbench screen closes s, and its pixels mustn't be read then */
static int still(struct Screen *s)
{
    struct Window *w = wbt_find_window(PAL_TITLE);
    return w && w->WScreen == s;
}

/* -1 if the screen went */
static int watch(struct Screen *s, int n, int want, int secs)
{
    int t, c, best = want ? 0 : n;
    for (t = 0; t <= secs * 2; t++) {
        if (!still(s)) return -1;
        c = count(s, n);
        if (want ? c > best : c < best) best = c;
        if (want ? c * 2 >= n : c * 5 < n) break;
        Delay(25);
    }
    return best;
}

/* the drawer opened again after a reset of the Workbench screen: the new screen and window in *s, *d, its points
 * read again into *np; 0 if it couldn't be */
static int reopen(struct Screen **s, struct Window **d, int *np)
{
    int t, how;
    for (t = 0; t < 30; t++) {   /* up to 15 s for Workbench to come back */
        Delay(25);
        if (wbt_open_drawer(PAL_DIR, PAL_TITLE, 2, &how) && wbt_place(PAL_TITLE, NULL, 400, 300)) {
            Delay(50);
            if (!(*d = wbt_find_window(PAL_TITLE)) || !(*s = (*d)->WScreen)) return 0;
            *np = points(*s, *d);
            printf("INFO P3 the Workbench screen was reset by the change (as IPrefs does for Palette): the drawer opened again, "
                   "%d points\n", *np);
            return *np >= 20;
        }
    }
    return 0;
}

/* watch, following the drawer onto a new screen if IPrefs resets Workbench (3.2.3 does for palette.prefs): -1 if it
 * couldn't be found again */
static int follow(struct Screen **s, struct Window **d, int *np, int want, int secs)
{
    int k, r = -1;
    for (k = 0; k < 3; k++) {
        if ((r = watch(*s, *np, want, secs)) >= 0) return r;
        if (!reopen(s, d, np)) return -1;
    }
    return r;
}

static void clean(void)
{
    static const char *const files[] = {PAL_DIR "/a.info", PAL_DIR "/a", PAL_DIR ".info", PAL_DIR};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)PAL_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window(PAL_TITLE); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
}

int wbt_palette(int argc, char **argv)
{
    struct Screen *s = NULL;
    struct Window *d;
    char why[220];
    unsigned long mode = 0, was0 = 0, scr_col[8];
    unsigned short pens[31];
    long was_n, n;
    int had, np, same, i, how, before, after = 0, back = 0, put_back = 0, v, npens = 0, pt_changed = 0;
    long pt_n;
    (void)argc, (void)argv;
    clean();
    was_n = wbt_read_all(PAL_FILE, (char *)was_buf, sizeof was_buf);
    had = was_n >= 0;
    if (had && was_n >= (long)sizeof was_buf - 1) { wbt_skip("P3", "ENV:Sys/palette.prefs is bigger than this row reads; left as it is"); return 0; }
    pt_n = wbt_read_all(PTRN_FILE, (char *)pt_was, sizeof pt_was);
    if (pt_n > 0 && pt_n < (long)sizeof pt_was - 1 && (n = wbgap_ptrn_drop(pt_was, pt_n, 1, pt_new, sizeof pt_new)) > 0) {
        /* drawers filled with a picture or pattern: colour 0 doesn't reach it, so the row runs with none, then puts it back */
        if (!put_file(PTRN_FILE, pt_new, n)) { put_file(PTRN_FILE, pt_was, pt_n); wbt_fail("P3", PTRN_FILE " could not be written"); goto out; }
        pt_changed = 1;
        printf("INFO P3 %s gives drawers a fill: taken out for the row, put back after\n", PTRN_FILE);
        Delay(150);
    }
    wbt_drawer(PAL_DIR, "a");
    if (!wbt_open_drawer(PAL_DIR, PAL_TITLE, 10, &how) || !(d = wbt_place(PAL_TITLE, NULL, 400, 300))) {
        wbt_fail("P3", PAL_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    if (!(d = wbt_find_window(PAL_TITLE)) || !(s = d->WScreen)) { wbt_fail("P3", PAL_DIR "'s window went"); goto out; }
    for (i = 0; i < 8; i++) {   /* the screen's colours 0-7 before the change, for the put back with no file */
        ULONG rgb[3];
        GetRGB32(s->ViewPort.ColorMap, (ULONG)i, 1, rgb);
        scr_col[i] = (rgb[0] >> 24) << 16 | (rgb[1] >> 24) << 8 | rgb[2] >> 24;
    }
    {   /* the screen's own pens, for a file made from nothing: IPrefs resets the screen for new pens */
        struct DrawInfo *dri = GetScreenDrawInfo(s);
        if (dri) {
            for (; npens < (int)dri->dri_NumPens && npens < 31; npens++) pens[npens] = dri->dri_Pens[npens];
            FreeScreenDrawInfo(s, dri);
        }
    }
    np = points(s, d);
    for (i = 0; i < np; i++) seen[i] = wbt_screen_rgb(s, px[i], py[i]);
    same = wbgap_palette_mode(seen, np, &mode);
    before = count(s, np);
    printf("INFO P3 %s %s (%ld bytes)%s; %d points of the drawer's corner, %d of them #%06lX, %d the prefs' colour\n", PAL_FILE,
           had ? "there" : "not there", had ? was_n : 0L,
           had && wbgap_palette_colour(was_buf, was_n, 0, &was0) ? ", colour 0 set" : "", np, same, mode, before);
    if (np < 20 || same * 10 < np * 9 || before * 5 >= np) {
        v = wbgap_palette_judge(np, same, before, 0, 0, 1, why, sizeof why);
        wbt_say("P3", v, why);
        goto out;
    }
    n = wbgap_palette_make(had ? was_buf : NULL, had ? was_n : 0, 0, WBGAP_PAL_RGB, new_buf, sizeof new_buf);
    if (n && !had && !wbgap_palette_pens(new_buf, n, pens, npens)) n = 0;
    {
        unsigned long got = 0;
        if (!n || !wbgap_palette_colour(new_buf, n, 0, &got) || got != WBGAP_PAL_RGB) {
            wbt_fail("P3", "the row's own prefs file came out wrong (a test fault, not the desktop's); nothing written");
            goto out;
        }
    }
    if (!put_file(PAL_FILE, new_buf, n)) {
        if (had) put_file(PAL_FILE, was_buf, was_n);
        else DeleteFile((CONST_STRPTR)PAL_FILE);
        wbt_fail("P3", "ENV:Sys/palette.prefs could not be written");
        goto out;
    }
    after = follow(&s, &d, &np, 1, 15);
    if (after < 0) {
        if (had) put_file(PAL_FILE, was_buf, was_n);
        else DeleteFile((CONST_STRPTR)PAL_FILE);
        wbt_fail("P3", "the Workbench screen was reset by the new palette.prefs and the drawer couldn't be opened again");
        goto out;
    }
    printf("INFO P3 after the change: %d of %d points the prefs' colour\n", after, np);
    if (had) put_back = put_file(PAL_FILE, was_buf, was_n);
    else {   /* no file before: the screen's own colours 0-7 as they were (IPrefs keeps the last colours it was given,
              * so Palette's defaults would leave a themed screen changed: Ackerman, #227), then no file */
        n = 0;
        for (i = 0; i < 8; i++) {
            long m = wbgap_palette_make(i ? new_buf : NULL, i ? n : 0, i, scr_col[i], tmp_buf, sizeof tmp_buf);
            if (!m) { n = 0; break; }
            memcpy(new_buf, tmp_buf, (size_t)m);
            n = m;
        }
        put_back = n && wbgap_palette_pens(new_buf, n, pens, npens) && put_file(PAL_FILE, new_buf, n);
    }
    back = follow(&s, &d, &np, 0, 15);
    if (!had) DeleteFile((CONST_STRPTR)PAL_FILE);
    if (back < 0) {
        wbt_fail("P3", "the Workbench screen was reset by the old palette.prefs going back and the drawer couldn't be opened again");
        goto out;
    }
    printf("INFO P3 after the old prefs%s: %d of %d points the prefs' colour\n", put_back ? "" : " (NOT put back)", back, np);
    v = wbgap_palette_judge(np, same, before, after, back, put_back, why, sizeof why);
    wbt_say("P3", v, why);
out:
    clean();
    if (pt_changed && !put_file(PTRN_FILE, pt_was, pt_n)) wbt_fail("P3", PTRN_FILE " could not be put back");
    return wbt_fails() ? 5 : 0;
}
