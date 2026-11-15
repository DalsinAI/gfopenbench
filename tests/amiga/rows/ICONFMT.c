/* ---- I13: the icon formats a drawer shows: classic, NewIcons, GlowIcons (OS 3.5), PNG ----
 * RAM:WBGapIcF (a snapshotted at 20,10: an empty drawer's window draws otherwise) opened at the top-left, 400x200.
 * The four icons of ICONFMT_icons.h (iconfmt_gen.py) are written in one at a time, at x 10, 70, 130 and 190, y 70
 * (their own snapshots), each told to the desktop and found where the window's inside changed (as DRAGFILE finds
 * a made icon). The top three-fifths of each one's box (the picture, above its label) is read every 2 pixels: each
 * must be its own colour (ICONFMT.h) in a quarter of its points. A desktop that reads only the classic picture shows
 * pen 1 for NewIcons and GlowIcons, and that is said. The drawer is cleared afterwards.
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
#include "WBPATTERN.h"   /* wbgap_rgb_near */
#include "ICONFMT.h"
#include "ICONFMT_icons.h"

WBGAP_MODE(ICONFMT, wbt_iconfmt, NEWROWS)
WBGAP_ROWMODE(I13, ICONFMT)

#define IF_DIR "RAM:WBGapIcF"
#define IF_TITLE "WBGapIcF"

static unsigned long before[WBT_GRAB_N];

static int put_bytes(const char *path, const unsigned char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG w;
    if (!fh) return 0;
    w = Write(fh, (APTR)buf, n);
    Close(fh);
    return w == n;
}

/* icon k written in as name with its .info, told to the desktop, its box in w's inside found: 1 */
static int if_make(struct Window *w, int k, long box[4])
{
    static const unsigned char *const data[WBGAP_ICONFMT_N] = {iconfmt_classic, iconfmt_newicons, iconfmt_glowicons, iconfmt_png};
    static const long size[WBGAP_ICONFMT_N] = {sizeof iconfmt_classic, sizeof iconfmt_newicons, sizeof iconfmt_glowicons,
                                               sizeof iconfmt_png};
    char path[96];
    BPTR l;
    int gw, gh;
    wbt_grab(w, before, &gw, &gh);
    snprintf(path, sizeof path, IF_DIR "/%s", wbgap_iconfmt_name(k));
    if (!wbt_write_file(path, "x\n")) return 0;
    snprintf(path, sizeof path, IF_DIR "/%s.info", wbgap_iconfmt_name(k));
    if (!put_bytes(path, data[k], size[k])) return 0;
    if ((l = Lock((CONST_STRPTR)IF_DIR, ACCESS_READ))) {
        UpdateWorkbench((CONST_STRPTR)wbgap_iconfmt_name(k), l, UPDATEWB_ObjectAdded);
        UnLock(l);
    }
    return wbt_settle(w, before, gw, gh, box);
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

int wbt_iconfmt(int argc, char **argv)
{
    static const unsigned long want[WBGAP_ICONFMT_N] = {0, WBGAP_ICONFMT_NEWICONS_RGB, WBGAP_ICONFMT_GLOW_RGB, WBGAP_ICONFMT_PNG_RGB};
    struct Window *w;
    struct Screen *s;
    long box[WBGAP_ICONFMT_N][4];
    int n[WBGAP_ICONFMT_N] = {0}, near[WBGAP_ICONFMT_N] = {0}, pen1[WBGAP_ICONFMT_N] = {0};
    unsigned long p1;
    char why[300];
    int how, k, x, y, v;
    (void)argc, (void)argv;
    if_clean();
    wbt_drawer(IF_DIR, "a");
    if (!wbt_open_drawer(IF_DIR, IF_TITLE, 10, &how) || !(w = wbt_place(IF_TITLE, NULL, 400, 200))) {
        wbt_fail("I13", IF_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    for (k = 0; k < WBGAP_ICONFMT_N; k++)
        if (!(w = wbt_find_window(IF_TITLE)) || !if_make(w, k, box[k])) {
            char m[120];
            snprintf(m, sizeof m, "the %s icon never showed in " IF_DIR "'s window (5 s after UpdateWorkbench)", wbgap_iconfmt_name(k));
            wbt_fail("I13", m);
            goto out;
        }
    Delay(50);
    if (!(w = wbt_find_window(IF_TITLE)) || !(s = w->WScreen)) { wbt_fail("I13", IF_DIR "'s window went"); goto out; }
    {
        ULONG rgb[3];
        GetRGB32(s->ViewPort.ColorMap, 1, 1, rgb);
        p1 = (rgb[0] >> 24) << 16 | (rgb[1] >> 24) << 8 | rgb[2] >> 24;
    }
    for (k = 0; k < WBGAP_ICONFMT_N; k++) {
        long x0 = w->LeftEdge + w->BorderLeft + box[k][0], y0 = w->TopEdge + w->BorderTop + box[k][1];
        unsigned long c = k ? want[k] : p1;
        for (y = 0; y < box[k][3] * 3 / 5; y += 2)
            for (x = 0; x < box[k][2]; x += 2) {
                unsigned long got = wbt_screen_rgb(s, (int)(x0 + x), (int)(y0 + y));
                n[k]++;
                near[k] += wbgap_rgb_near(got, c, WBGAP_ICONFMT_TOL);
                pen1[k] += wbgap_rgb_near(got, p1, WBGAP_ICONFMT_TOL);
            }
        if (!k) pen1[k] = -1;
        printf("INFO I13 %s at %ld,%ld %ldx%ld: %d of %d points #%06lX, %d pen 1 (#%06lX)\n", wbgap_iconfmt_name(k), box[k][0],
               box[k][1], box[k][2], box[k][3], near[k], n[k], c, pen1[k], p1);
    }
    v = wbgap_iconfmt_judge(n, near, pen1, why, sizeof why);
    wbt_say("I13", v, why);
out:
    if_clean();
    return wbt_fails() ? 5 : 0;
}
