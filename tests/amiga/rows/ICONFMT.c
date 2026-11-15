/* ---- I13: the icon formats a drawer shows: classic, NewIcons, GlowIcons (OS 3.5), PNG ----
 * RAM:WBGapIcF (a snapshotted at 20,10: an empty drawer's window draws otherwise) opened at the top-left, 400x240,
 * and its inside read every 2 pixels for pen 1. The four icons of ICONFMT_icons.h (iconfmt_gen.py) are written in
 * and told to the desktop (UpdateWorkbench); 2 s on, the inside is read again. Each format's colour (ICONFMT.h; the
 * classic one's, pen 3's) must be in WBGAP_ICONFMT_MIN of its points, wherever the desktop put the icon (GFOpenBench
 * lays a drawer out itself); pen 1's growth says whether a classic picture was shown in place of NewIcons or
 * GlowIcons. The drawer is cleared afterwards.
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
#include "ICONFMT.h"
#include "ICONFMT_icons.h"

WBGAP_MODE(ICONFMT, wbt_iconfmt, NEWROWS)
WBGAP_ROWMODE(I13, ICONFMT)

#define IF_DIR "RAM:WBGapIcF"
#define IF_TITLE "WBGapIcF"

static int put_bytes(const char *path, const unsigned char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG w;
    if (!fh) return 0;
    w = Write(fh, (APTR)buf, n);
    Close(fh);
    return w == n;
}

/* icon k written in with its .info and told to the desktop: 1 */
static int if_make(int k)
{
    static const unsigned char *const data[WBGAP_ICONFMT_N] = {iconfmt_classic, iconfmt_newicons, iconfmt_glowicons, iconfmt_png};
    static const long size[WBGAP_ICONFMT_N] = {sizeof iconfmt_classic, sizeof iconfmt_newicons, sizeof iconfmt_glowicons,
                                               sizeof iconfmt_png};
    char path[96];
    BPTR l;
    snprintf(path, sizeof path, IF_DIR "/%s", wbgap_iconfmt_name(k));
    if (!wbt_write_file(path, "x\n")) return 0;
    snprintf(path, sizeof path, IF_DIR "/%s.info", wbgap_iconfmt_name(k));
    if (!put_bytes(path, data[k], size[k])) return 0;
    if ((l = Lock((CONST_STRPTR)IF_DIR, ACCESS_READ))) {
        UpdateWorkbench((CONST_STRPTR)wbgap_iconfmt_name(k), l, UPDATEWB_ObjectAdded);
        UnLock(l);
    }
    return 1;
}

/* w's inside read every 2 pixels: how many points, and how many near each of want[0..nw-1] into cnt */
static int if_count(struct Window *w, const unsigned long *want, int nw, int *cnt)
{
    struct Screen *s = w->WScreen;
    int x, y, k, n = 0;
    long x0 = w->LeftEdge + w->BorderLeft, y0 = w->TopEdge + w->BorderTop;
    long iw = w->Width - w->BorderLeft - w->BorderRight, ih = w->Height - w->BorderTop - w->BorderBottom;
    for (k = 0; k < nw; k++) cnt[k] = 0;
    for (y = 0; y < ih; y += 2)
        for (x = 0; x < iw; x += 2) {
            unsigned long got = wbt_screen_rgb(s, (int)(x0 + x), (int)(y0 + y));
            n++;
            for (k = 0; k < nw; k++) cnt[k] += wbgap_iconfmt_near(got, want[k], WBGAP_ICONFMT_TOL);
        }
    return n;
}

static void if_clean(void)
{
    char path[96];
    int k, t, pass;
    CloseWorkbenchObjectA((STRPTR)IF_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window(IF_TITLE); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write the drawer's .info after the close */
        if (pass) Delay(25);
        for (k = 0; k < WBGAP_ICONFMT_N; k++) {
            snprintf(path, sizeof path, IF_DIR "/%s.info", wbgap_iconfmt_name(k));
            DeleteFile((CONST_STRPTR)path);
            snprintf(path, sizeof path, IF_DIR "/%s", wbgap_iconfmt_name(k));
            DeleteFile((CONST_STRPTR)path);
        }
        DeleteFile((CONST_STRPTR)IF_DIR "/a.info");
        DeleteFile((CONST_STRPTR)IF_DIR "/a");
        DeleteFile((CONST_STRPTR)IF_DIR ".info");
        DeleteFile((CONST_STRPTR)IF_DIR);
    }
}

static unsigned long pen_rgb(struct Screen *s, int pen)
{
    ULONG rgb[3];
    GetRGB32(s->ViewPort.ColorMap, (ULONG)pen, 1, rgb);
    return (rgb[0] >> 24) << 16 | (rgb[1] >> 24) << 8 | rgb[2] >> 24;
}

int wbt_iconfmt(int argc, char **argv)
{
    unsigned long want[WBGAP_ICONFMT_N + 1] = {0, WBGAP_ICONFMT_NEWICONS_RGB, WBGAP_ICONFMT_GLOW_RGB, WBGAP_ICONFMT_PNG_RGB, 0};
    struct Window *w;
    int cnt[WBGAP_ICONFMT_N + 1], pen1_before, n, how, k, v;
    char why[300];
    (void)argc, (void)argv;
    if_clean();
    wbt_drawer(IF_DIR, "a");
    if (!wbt_open_drawer(IF_DIR, IF_TITLE, 10, &how) || !(w = wbt_place(IF_TITLE, NULL, 400, 240))) {
        wbt_fail("I13", IF_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    if (!(w = wbt_find_window(IF_TITLE))) { wbt_fail("I13", IF_DIR "'s window went"); goto out; }
    want[0] = pen_rgb(w->WScreen, 3);
    want[WBGAP_ICONFMT_N] = pen_rgb(w->WScreen, 1);
    if_count(w, want + WBGAP_ICONFMT_N, 1, &pen1_before);
    for (k = 0; k < WBGAP_ICONFMT_N; k++)
        if (!if_make(k)) {
            char m[120];
            snprintf(m, sizeof m, "the %s icon couldn't be written in " IF_DIR, wbgap_iconfmt_name(k));
            wbt_fail("I13", m);
            goto out;
        }
    Delay(100);
    if (!(w = wbt_find_window(IF_TITLE))) { wbt_fail("I13", IF_DIR "'s window went"); goto out; }
    n = if_count(w, want, WBGAP_ICONFMT_N + 1, cnt);
    for (k = 0; k < WBGAP_ICONFMT_N; k++)
        printf("INFO I13 %s: %d of %d points #%06lX\n", wbgap_iconfmt_name(k), cnt[k], n, want[k]);
    printf("INFO I13 pen 1 (#%06lX): %d points before the icons, %d after\n", want[WBGAP_ICONFMT_N], pen1_before, cnt[WBGAP_ICONFMT_N]);
    v = wbgap_iconfmt_judge(n, cnt, cnt[WBGAP_ICONFMT_N] - pen1_before, why, sizeof why);
    wbt_say("I13", v, why);
out:
    if_clean();
    return wbt_fails() ? 5 : 0;
}
