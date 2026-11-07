/* GFOpenBenchmark: 2D drawing through graphics.library, in a window on the Workbench screen (on
 * AmiCradle, GFOpenRTG's): RectFill, ClipBlit and Text, each ended with WaitBlit so that the work
 * counted is done. Other tasks run meanwhile (the screen's driver may need them).
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/gfx.h>
#include <graphics/rastport.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include "gfb.h"

#define SQ 128
static struct Window *win;
static const char text32[] = "GFOpenBenchmark text 0123456789A";

static double w_rect(void *u, unsigned long n)
{
    struct RastPort *rp = win->RPort;
    WORD x = win->BorderLeft, y = win->BorderTop;
    (void)u;
    while (n--) {
        SetAPen(rp, (n & 1) + 1);
        RectFill(rp, x, y, x + SQ - 1, y + SQ - 1);
    }
    WaitBlit();
    return 0;
}

static double w_blit(void *u, unsigned long n)
{
    struct RastPort *rp = win->RPort;
    WORD x = win->BorderLeft, y = win->BorderTop;
    (void)u;
    while (n--) ClipBlit(rp, x + (n & 1), y, rp, x + SQ + 8, y, SQ, SQ, 0xC0);
    WaitBlit();
    return 0;
}

static double w_text(void *u, unsigned long n)
{
    struct RastPort *rp = win->RPort;
    WORD x = win->BorderLeft, y = win->BorderTop + SQ + 8 + rp->TxBaseline;
    (void)u;
    SetAPen(rp, 1);
    SetDrMd(rp, JAM2);
    while (n--) {
        Move(rp, x, y + (WORD)((n & 7) * rp->TxHeight));
        Text(rp, (CONST_STRPTR)text32, 32);
    }
    WaitBlit();
    return 0;
}

static void one(const char *key, const char *label, const char *unit, gfb_work w)
{
    struct gfb_result r;
    memset(&r, 0, sizeof r);
    r.key = key;
    r.label = label;
    r.unit = unit;
    if (!win) gfb_fail(&r, "no window on the Workbench screen");
    else gfb_time(&r, w, NULL, 1.0, 0);
    gfb_report(&r);
}

void gfb_2d_tests(void)
{
    struct Screen *s;
    char buf[96];
    gfb_section("2D (graphics.library on the Workbench screen)");
    if ((s = LockPubScreen(NULL))) {
        sprintf(buf, "%dx%d, %lu bit planes", s->Width, s->Height, (unsigned long)GetBitMapAttr(s->RastPort.BitMap, BMA_DEPTH));
        gfb_info("wb_screen", "Workbench screen", buf);
        win = OpenWindowTags(NULL, WA_PubScreen, (ULONG)s, WA_Left, 20, WA_Top, 20, WA_InnerWidth, 2 * SQ + 16 + 256,
                             WA_InnerHeight, SQ + 16 + 8 * 24, WA_Title, (ULONG) "GFOpenBenchmark 2D", WA_DragBar, TRUE,
                             WA_DepthGadget, TRUE, WA_SmartRefresh, TRUE, WA_Activate, FALSE, TAG_DONE);
        UnlockPubScreen(NULL, s);
    }
    one("rectfill_ops", "RectFill 128x128", "fills/s", w_rect);
    one("blit_ops", "ClipBlit 128x128", "blits/s", w_blit);
    one("text_ops", "Text, 32 chars (screen font)", "strings/s", w_text);
    if (win) CloseWindow(win);
    win = NULL;
}
