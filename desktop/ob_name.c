/* ob_name: the name requester of Icons > Rename... (ob_name.h). OS 3.2.3's Workbench, read for what it shows (no code
 * taken): a window "Rename", the line "Enter a new name for 'x'.", a "New Name" field holding the old name, OK and
 * Cancel; Return in the field is OK. GadTools gadgets in the screen's font, laid out inside the window's real borders
 * (GFOpenFrames draws its own), as ob_info.c's are. It waits for its answer in the caller's task: the desktop waits
 * too, as 3.2.3's does while it asks.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/gadgetclass.h>
#include <libraries/gadtools.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <string.h>
#include <stdio.h>

#include "ob_name.h"

enum { G_NAME = 1, G_OK, G_CANCEL };

static WORD text_len(struct Screen *s, const char *t)
{
    struct RastPort rp;
    struct TextFont *f = OpenFont(s->Font);
    WORD w;
    InitRastPort(&rp);
    if (f) SetFont(&rp, f);
    w = TextLength(&rp, (STRPTR)t, strlen(t));
    if (f) CloseFont(f);
    return w;
}

/* One gadget; tags as a list of its own (see ob_info.c's gad: GCC 16 inlines a varargs copy) */
static struct Gadget *gad(struct Screen *scr, APTR vi, struct Gadget *prev, ULONG kind, int id, WORD x, WORD y, WORD w,
                          WORD h, const char *label, ULONG flags, struct TagItem *tags)
{
    struct NewGadget ng;
    memset(&ng, 0, sizeof ng);
    ng.ng_LeftEdge = x;
    ng.ng_TopEdge = y;
    ng.ng_Width = w;
    ng.ng_Height = h;
    ng.ng_GadgetText = (STRPTR)label;
    ng.ng_TextAttr = scr->Font;
    ng.ng_GadgetID = id;
    ng.ng_Flags = flags;
    ng.ng_VisualInfo = vi;
    return CreateGadgetA(kind, prev, &ng, tags);
}

static int name_ok(const char *s)
{
    return *s && !strchr(s, ':') && !strchr(s, '/');
}

int ob_name_ask(struct Screen *scr, const char *old, char *out, int size)
{
    char line[160];
    APTR vi;
    struct Window *w;
    struct Gadget *glist = NULL, *p, *str = NULL;
    struct TagItem st[3], none[1];
    WORD fh, rh, m = 8, lw, tw, iw, ih, bw = 90, x0, y;
    int done = 0, ok = 0;
    if (!scr || size < 2) return 0;
    snprintf(line, sizeof line, "Enter a new name for '%s'.", old);
    fh = scr->Font->ta_YSize;
    rh = fh + 6;
    lw = text_len(scr, "New Name") + 8;
    tw = text_len(scr, line);
    iw = tw + 2 * m;
    if (iw < lw + 280 + 2 * m) iw = lw + 280 + 2 * m;
    ih = m + fh + m + rh + m + rh + 2 + m;
    if (!(vi = GetVisualInfoA(scr, NULL))) return 0;
    w = OpenWindowTags(NULL, WA_PubScreen, (ULONG)scr, WA_InnerWidth, iw, WA_InnerHeight, ih, WA_Title, (ULONG)"Rename",
                       WA_Left, (scr->Width - iw) / 2, WA_Top, (scr->Height - ih) / 3,
                       WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_CloseGadget, TRUE, WA_Activate, TRUE,
                       WA_SimpleRefresh, TRUE, WA_AutoAdjust, TRUE,
                       WA_IDCMP, BUTTONIDCMP | STRINGIDCMP | IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_VANILLAKEY,
                       TAG_DONE);
    if (!w) {
        FreeVisualInfo(vi);
        return 0;
    }
    x0 = w->BorderLeft;
    y = w->BorderTop + m + fh + m;
    st[0].ti_Tag = GTST_String;
    st[0].ti_Data = (ULONG)old;
    st[1].ti_Tag = GTST_MaxChars;
    st[1].ti_Data = (ULONG)(size - 1 < 107 ? size - 1 : 107);
    st[2].ti_Tag = TAG_DONE;
    none[0].ti_Tag = TAG_DONE;
    p = CreateContext(&glist);
    if (p) p = str = gad(scr, vi, p, STRING_KIND, G_NAME, x0 + m + lw, y, iw - 2 * m - lw, rh, "New Name", PLACETEXT_LEFT, st);
    y += rh + m;
    if (p) p = gad(scr, vi, p, BUTTON_KIND, G_OK, x0 + m, y, bw, rh + 2, "OK", PLACETEXT_IN, none);
    if (p) p = gad(scr, vi, p, BUTTON_KIND, G_CANCEL, x0 + iw - m - bw, y, bw, rh + 2, "Cancel", PLACETEXT_IN, none);
    if (!p) {
        CloseWindow(w);
        FreeGadgets(glist);
        FreeVisualInfo(vi);
        return 0;
    }
    AddGList(w, glist, (UWORD)~0, -1, NULL);
    RefreshGList(glist, w, NULL, -1);
    GT_RefreshWindow(w, NULL);
    SetAPen(w->RPort, 1);
    Move(w->RPort, x0 + m, w->BorderTop + m + w->RPort->TxBaseline);
    Text(w->RPort, (STRPTR)line, strlen(line));
    ActivateGadget(str, w, NULL);
    while (!done) {
        struct IntuiMessage *im;
        WaitPort(w->UserPort);
        while (!done && (im = GT_GetIMsg(w->UserPort))) {
            ULONG cl = im->Class;
            UWORD code = im->Code;
            struct Gadget *g = (struct Gadget *)im->IAddress;
            GT_ReplyIMsg(im);
            switch (cl) {
            case IDCMP_REFRESHWINDOW:
                GT_BeginRefresh(w);
                Move(w->RPort, x0 + m, w->BorderTop + m + w->RPort->TxBaseline);
                Text(w->RPort, (STRPTR)line, strlen(line));
                GT_EndRefresh(w, TRUE);
                break;
            case IDCMP_CLOSEWINDOW: done = 1; break;
            case IDCMP_VANILLAKEY:
                if (code == 27 || code == 'c' || code == 'C') done = 1;
                else if (code == 'o' || code == 'O' || code == 13) done = ok = 1;
                else ActivateGadget(str, w, NULL);
                break;
            case IDCMP_GADGETUP:
                if (g->GadgetID == G_CANCEL) done = 1;
                else if (g->GadgetID == G_OK || (g->GadgetID == G_NAME && code != 9)) done = ok = 1;
                break;
            }
        }
        if (ok) {
            const char *s = (const char *)((struct StringInfo *)str->SpecialInfo)->Buffer;
            if (name_ok(s)) {
                strncpy(out, s, (size_t)size - 1);
                out[size - 1] = 0;
            } else {                            /* empty, or a ':' or '/': asked again */
                DisplayBeep(scr);
                done = ok = 0;
                ActivateGadget(str, w, NULL);
            }
        }
    }
    CloseWindow(w);
    FreeGadgets(glist);
    FreeVisualInfo(vi);
    return ok;
}
