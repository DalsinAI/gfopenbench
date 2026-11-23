/* ob_popup.c: GFOpenBench's menu at the pointer (ob_popup.h, 0.1.73).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/rastport.h>
#include <graphics/text.h>
#include <devices/inputevent.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <string.h>

#include "ob_popup.h"

#define PAD_X 8                                 /* text from the sides */
#define PAD_Y 2                                 /* above and below each item's text */
#define EDGE 2                                  /* the frame */

typedef struct {
    struct Window *win;
    struct DrawInfo *dri;
    const char *const *labels;
    int n, cur;
    WORD ih, w, base;                           /* an item's height, the inside's width, the text's baseline */
} popup;

static UWORD pen(const popup *p, int which, UWORD dflt)
{
    return p->dri && which < p->dri->dri_NumPens ? p->dri->dri_Pens[which] : dflt;
}

static void draw_item(popup *p, int i)
{
    struct RastPort *rp = p->win->RPort;
    WORD y = EDGE + i * p->ih;
    int on = i == p->cur;
    SetAPen(rp, on ? pen(p, FILLPEN, 3) : pen(p, BACKGROUNDPEN, 0));
    RectFill(rp, EDGE, y, EDGE + p->w - 1, y + p->ih - 1);
    SetAPen(rp, on ? pen(p, FILLTEXTPEN, 2) : pen(p, TEXTPEN, 1));
    SetDrMd(rp, JAM1);
    Move(rp, EDGE + PAD_X, y + PAD_Y + p->base);
    Text(rp, (STRPTR)p->labels[i], (ULONG)strlen(p->labels[i]));
}

static void draw_all(popup *p)
{
    struct RastPort *rp = p->win->RPort;
    WORD r = p->win->Width - 1, b = p->win->Height - 1;
    int i;
    SetAPen(rp, pen(p, BACKGROUNDPEN, 0));
    RectFill(rp, 0, 0, r, b);
    SetAPen(rp, pen(p, SHINEPEN, 2));           /* the frame, raised, as a menu's */
    Move(rp, 0, b); Draw(rp, 0, 0); Draw(rp, r, 0);
    SetAPen(rp, pen(p, SHADOWPEN, 1));
    Move(rp, r, 1); Draw(rp, r, b); Draw(rp, 1, b);
    for (i = 0; i < p->n; i++) draw_item(p, i);
}

static void set_cur(popup *p, int i)
{
    int was = p->cur;
    if (i == was) return;
    p->cur = i;
    if (was >= 0) draw_item(p, was);
    if (i >= 0) draw_item(p, i);
}

/* the item under the window's point, or -1 */
static int item_at(const popup *p, WORD x, WORD y)
{
    if (x < EDGE || x >= EDGE + p->w || y < EDGE || y >= EDGE + p->n * p->ih) return -1;
    return (y - EDGE) / p->ih;
}

int ob_popup(struct Screen *scr, WORD x, WORD y, const char *const *labels, int n, int held)
{
    popup p;
    struct RastPort trp;
    struct TextFont *font;
    WORD ww, wh, tw;
    int i, taken = -1, done = 0, entered = 0;

    if (!scr || !labels || n <= 0) return -1;
    if (n > OB_POPUP_MAX) n = OB_POPUP_MAX;
    memset(&p, 0, sizeof p);
    p.labels = labels;
    p.n = n;
    p.cur = -1;
    font = scr->RastPort.Font;
    InitRastPort(&trp);
    if (font) SetFont(&trp, font);
    p.w = 0;
    for (i = 0; i < n; i++) {
        tw = (WORD)TextLength(&trp, (STRPTR)labels[i], (ULONG)strlen(labels[i]));
        if (tw > p.w) p.w = tw;
    }
    p.w += 2 * PAD_X;
    p.base = font ? font->tf_Baseline : 6;
    p.ih = (font ? font->tf_YSize : 8) + 2 * PAD_Y;
    ww = p.w + 2 * EDGE;
    wh = n * p.ih + 2 * EDGE;
    if (x + ww > scr->Width) x = scr->Width - ww;     /* moved in to fit, as Intuition's menus are */
    if (y + wh > scr->Height) y = scr->Height - wh;
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    p.win = OpenWindowTags(NULL, WA_Left, x, WA_Top, y, WA_Width, ww, WA_Height, wh, WA_CustomScreen, (ULONG)scr,
                           WA_Borderless, TRUE, WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_ReportMouse, TRUE,
                           WA_SmartRefresh, TRUE,
                           WA_IDCMP, IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_INACTIVEWINDOW | IDCMP_RAWKEY,
                           TAG_DONE);
    if (!p.win) return -1;
    p.dri = GetScreenDrawInfo(scr);
    SetFont(p.win->RPort, font);
    draw_all(&p);

    while (!done) {
        struct IntuiMessage *m;
        WaitPort(p.win->UserPort);
        while (!done && (m = (struct IntuiMessage *)GetMsg(p.win->UserPort))) {
            ULONG cl = m->Class;
            UWORD code = m->Code;
            WORD mx = m->MouseX, my = m->MouseY;
            int at;
            ReplyMsg(&m->ExecMessage);
            at = item_at(&p, mx, my);
            switch (cl) {
            case IDCMP_MOUSEMOVE:
                if (at >= 0) entered = 1;
                set_cur(&p, at);
                break;
            case IDCMP_MOUSEBUTTONS:
                if (code == MENUUP && held) {
                    held = 0;
                    if (at >= 0) { taken = at; done = 1; }
                    else if (entered) done = 1;     /* dragged onto the menu and off again: nothing */
                    /* else: a press let go where it was made: the menu stays, for a click */
                } else if (code == SELECTDOWN || code == MENUDOWN) {
                    if (at < 0) done = 1;           /* a click outside: closed */
                } else if (code == SELECTUP || code == MENUUP) {
                    if (at >= 0) { taken = at; done = 1; }
                }
                break;
            case IDCMP_RAWKEY:
                if (code & IECODE_UP_PREFIX) break;
                if (code == 0x45) done = 1;                                 /* Esc */
                else if (code == 0x4c) set_cur(&p, p.cur <= 0 ? n - 1 : p.cur - 1);
                else if (code == 0x4d) set_cur(&p, p.cur < 0 || p.cur >= n - 1 ? 0 : p.cur + 1);
                else if ((code == 0x44 || code == 0x43) && p.cur >= 0) { taken = p.cur; done = 1; }
                break;
            case IDCMP_INACTIVEWINDOW:
                done = 1;
                break;
            }
        }
    }
    {
        struct IntuiMessage *m;                 /* what came after: answered before the window goes */
        while ((m = (struct IntuiMessage *)GetMsg(p.win->UserPort))) ReplyMsg(&m->ExecMessage);
    }
    if (p.dri) FreeScreenDrawInfo(scr, p.dri);
    CloseWindow(p.win);
    return taken;
}
