/* ob_pick.c: GFOpenBench's small modal requesters (ob_pick.h, 0.1.52).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/lists.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <devices/inputevent.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <clib/alib_protos.h>
#include <string.h>
#include <stdio.h>

#include "ob_pick.h"

enum { G_FIELD = 1, G_OK, G_CANCEL };

typedef struct {
    struct Screen *scr;
    APTR vi;
    struct Gadget *glist, *field;
    struct Window *win;
    WORD fh, m;                                 /* the font's height, the margin */
} pick;

static int pick_begin(pick *p, struct Screen *scr)
{
    memset(p, 0, sizeof *p);
    p->scr = scr;
    if (!scr || !(p->vi = GetVisualInfo(scr, TAG_DONE))) return 0;
    p->fh = scr->Font ? scr->Font->ta_YSize : 8;
    p->m = 8;
    return CreateContext(&p->glist) != NULL;
}

static void pick_end(pick *p)
{
    if (p->win) CloseWindow(p->win);
    if (p->glist) FreeGadgets(p->glist);
    if (p->vi) FreeVisualInfo(p->vi);
}

static int pick_open(pick *p, const char *title, WORD iw, WORD ih)
{
    WORD bl = p->scr->WBorLeft, bt = p->scr->WBorTop + p->fh + 1, br = p->scr->WBorRight, bb = p->scr->WBorBottom;
    WORD w = iw + bl + br, h = ih + bt + bb;
    p->win = OpenWindowTags(NULL, WA_Left, (p->scr->Width - w) / 2, WA_Top, (p->scr->Height - h) / 2,
                            WA_Width, w, WA_Height, h, WA_PubScreen, (ULONG)p->scr, WA_Title, (ULONG)title,
                            WA_Gadgets, (ULONG)p->glist, WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE,
                            WA_Activate, TRUE, WA_RMBTrap, TRUE, WA_NewLookMenus, TRUE,
                            WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_VANILLAKEY | IDCMP_REFRESHWINDOW |
                                      LISTVIEWIDCMP | STRINGIDCMP | BUTTONIDCMP, TAG_DONE);
    if (!p->win) return 0;
    GT_RefreshWindow(p->win, NULL);
    return 1;
}

static struct Gadget *button(pick *p, struct Gadget *g, WORD x, WORD y, WORD w, const char *label, UWORD id)
{
    struct NewGadget ng;
    memset(&ng, 0, sizeof ng);
    ng.ng_LeftEdge = x;
    ng.ng_TopEdge = y;
    ng.ng_Width = w;
    ng.ng_Height = p->fh + 6;
    ng.ng_GadgetText = (UBYTE *)label;
    ng.ng_TextAttr = p->scr->Font;
    ng.ng_GadgetID = id;
    ng.ng_Flags = PLACETEXT_IN;
    ng.ng_VisualInfo = p->vi;
    return CreateGadget(BUTTON_KIND, g, &ng, TAG_DONE);
}

/* the window's messages until it is answered: G_OK (or a double-click's row, in *row), G_CANCEL */
static int pick_wait(pick *p, WORD *row)
{
    ULONG last_s = 0, last_m = 0;
    WORD last_row = -1;
    for (;;) {
        struct IntuiMessage *im;
        WaitPort(p->win->UserPort);
        while ((im = GT_GetIMsg(p->win->UserPort))) {
            ULONG cl = im->Class, s = im->Seconds, mi = im->Micros;
            UWORD code = im->Code;
            struct Gadget *g = (struct Gadget *)im->IAddress;
            GT_ReplyIMsg(im);
            switch (cl) {
            case IDCMP_CLOSEWINDOW: return G_CANCEL;
            case IDCMP_REFRESHWINDOW:
                GT_BeginRefresh(p->win);
                GT_EndRefresh(p->win, TRUE);
                break;
            case IDCMP_VANILLAKEY:
                if (code == 27) return G_CANCEL;
                if ((code == 13 || code == 10) && (!row || *row >= 0)) return G_OK;
                break;
            case IDCMP_GADGETUP:
                if (g->GadgetID == G_OK) { if (!row || *row >= 0) return G_OK; break; }
                if (g->GadgetID == G_CANCEL) return G_CANCEL;
                if (g->GadgetID == G_FIELD && !row) {
                    /* Return in the field ends it with OK; Tab (or Shift-Tab, Help) only leaves it */
                    if (code != 0x09 && code != 0x5F) return G_OK;
                    break;
                }
                if (g->GadgetID == G_FIELD && row) {
                    if ((WORD)code == last_row && DoubleClick(last_s, last_m, s, mi)) { *row = (WORD)code; return G_OK; }
                    *row = last_row = (WORD)code;
                    last_s = s;
                    last_m = mi;
                }
                break;
            }
        }
    }
}

int ob_pick_string(struct Screen *scr, const char *title, const char *prompt, const char *ok, char *buf, int n)
{
    pick p;
    struct NewGadget ng;
    struct Gadget *g;
    WORD pw, iw = 320, ih, bw = 90, y;
    int ans = 0;
    if (!pick_begin(&p, scr)) { pick_end(&p); return 0; }
    {
        struct RastPort rp;
        InitRastPort(&rp);
        if (scr->RastPort.Font) SetFont(&rp, scr->RastPort.Font);
        pw = (WORD)TextLength(&rp, (STRPTR)prompt, (UWORD)strlen(prompt)) + p.m;
    }
    iw = pw + 260 + 2 * p.m;
    memset(&ng, 0, sizeof ng);
    ng.ng_LeftEdge = p.scr->WBorLeft + p.m + pw;
    ng.ng_TopEdge = p.scr->WBorTop + p.fh + 1 + p.m;
    ng.ng_Width = 260;
    ng.ng_Height = p.fh + 6;
    ng.ng_GadgetText = (UBYTE *)prompt;
    ng.ng_TextAttr = scr->Font;
    ng.ng_GadgetID = G_FIELD;
    ng.ng_Flags = PLACETEXT_LEFT;
    ng.ng_VisualInfo = p.vi;
    p.field = g = CreateGadget(STRING_KIND, p.glist, &ng, GTST_String, (ULONG)buf, GTST_MaxChars, (ULONG)(n - 1), TAG_DONE);
    y = ng.ng_TopEdge + ng.ng_Height + p.m;
    g = button(&p, g, p.scr->WBorLeft + p.m, y, bw, ok, G_OK);
    g = button(&p, g, p.scr->WBorLeft + iw - p.m - bw, y, bw, "Cancel", G_CANCEL);
    ih = (y - (p.scr->WBorTop + p.fh + 1)) + p.fh + 6 + p.m;
    if (g && pick_open(&p, title, iw, ih)) {
        ActivateGadget(p.field, p.win, NULL);
        if (pick_wait(&p, NULL) == G_OK) {
            struct StringInfo *si = (struct StringInfo *)p.field->SpecialInfo;
            snprintf(buf, (size_t)n, "%s", si && si->Buffer ? (const char *)si->Buffer : "");
            ans = buf[0] != 0;
        }
    }
    pick_end(&p);
    return ans;
}

int ob_pick_list(struct Screen *scr, const char *title, const char *ok, const char *const *names, int n)
{
    pick p;
    struct NewGadget ng;
    struct Gadget *g;
    struct List list;
    struct Node nodes[OB_PICK_MAX];
    WORD iw = 260, ih, bw = 90, y, rows, row = -1;
    int i, ans = -1;
    if (n > OB_PICK_MAX) n = OB_PICK_MAX;
    if (n <= 0 || !pick_begin(&p, scr)) { pick_end(&p); return -1; }
    NewList(&list);
    for (i = 0; i < n; i++) {
        memset(&nodes[i], 0, sizeof nodes[i]);
        nodes[i].ln_Name = (char *)names[i];
        AddTail(&list, &nodes[i]);
    }
    rows = n < 4 ? 4 : n > 12 ? 12 : (WORD)n;
    memset(&ng, 0, sizeof ng);
    ng.ng_LeftEdge = p.scr->WBorLeft + p.m;
    ng.ng_TopEdge = p.scr->WBorTop + p.fh + 1 + p.m;
    ng.ng_Width = iw - 2 * p.m;
    ng.ng_Height = rows * (p.fh + 1) + 4;
    ng.ng_TextAttr = scr->Font;
    ng.ng_GadgetID = G_FIELD;
    ng.ng_VisualInfo = p.vi;
    g = CreateGadget(LISTVIEW_KIND, p.glist, &ng, GTLV_Labels, (ULONG)&list, GTLV_ShowSelected, 0, TAG_DONE);
    y = ng.ng_TopEdge + ng.ng_Height + p.m;
    g = button(&p, g, p.scr->WBorLeft + p.m, y, bw, ok, G_OK);
    g = button(&p, g, p.scr->WBorLeft + iw - p.m - bw, y, bw, "Cancel", G_CANCEL);
    ih = (y - (p.scr->WBorTop + p.fh + 1)) + p.fh + 6 + p.m;
    if (g && pick_open(&p, title, iw, ih) && pick_wait(&p, &row) == G_OK && row >= 0 && row < n) ans = row;
    pick_end(&p);
    return ans;
}
