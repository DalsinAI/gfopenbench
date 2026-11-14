/* ---- W1: a drawer window's size, zoom and depth gadgets ----
 * RAM:WBGapW (a snapshotted at 20,10) opened at the top-left, 400x300. A window of our own, "W1 cover", is opened
 * over the middle of it (clear of its gadgets) and left in front. Then, each with DragTest at the middle of the
 * window's gadget (GTYP_SIZING, GTYP_WZOOM, GTYP_WDEPTH in its gadget list; a sizing gadget of the window's own
 * counts, as Intuition sizes through it all the same):
 *   - the size gadget is dragged 80,60: the window must grow;
 *   - the zoom gadget is clicked: its size must change; clicked again: the size must come back;
 *   - the depth gadget is clicked: the window must come in front of the cover; again: go behind it.
 * The order of the two is read from the screen's layers. 3.2.3's Workbench windows do all of this through
 * Intuition; the row is about a desktop's drawer windows having the gadgets and them working.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <graphics/clip.h>
#include <graphics/layers.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/layers.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "WINGAD.h"

WBGAP_MODE(WINGAD, wbt_wingad, NEWROWS)
WBGAP_ROWMODE(W1, WINGAD)

extern struct Library *LayersBase;   /* WBGapTest.c's; opened here for the row and closed again, as SWITCH does */

#define WG_DIR "RAM:WBGapW"
#define WG_W 400
#define WG_H 300

static void wg_middle(struct Window *w, struct Gadget *g, long xy[2])
{
    wbgap_gadget_middle(g->LeftEdge, g->TopEdge, g->Width, g->Height, (g->Flags & GFLG_RELRIGHT) != 0,
                        (g->Flags & GFLG_RELBOTTOM) != 0, (g->Flags & GFLG_RELWIDTH) != 0,
                        (g->Flags & GFLG_RELHEIGHT) != 0, w->Width, w->Height, xy);
    xy[0] += w->LeftEdge;
    xy[1] += w->TopEdge;
}

/* the middle of w's gadget of type (GTYP_SIZING, _WZOOM, _WDEPTH) on the screen: 1 and xy, or 0. A sizing gadget
 * of the window's own (GTYP_SIZING without GTYP_SYSGADGET, as GFOpenBench's drawers have) counts: Intuition sizes
 * the window through it all the same */
static int wg_gadget(struct Window *w, UWORD type, long xy[2])
{
    struct Gadget *g;
    for (g = w->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSTYPEMASK) == type && ((g->GadgetType & GTYP_SYSGADGET) || type == GTYP_SIZING)) {
            wg_middle(w, g, xy);
            return 1;
        }
    return 0;
}

/* a in front of b on screen s: 1, 0, or -1 if either isn't among the screen's layers */
static int wg_front(struct Screen *s, struct Window *a, struct Window *b)
{
    struct Layer *l;
    int r = -1;
    LockLayerInfo(&s->LayerInfo);
    for (l = s->LayerInfo.top_layer; l && r < 0; l = l->back) {
        if (l == a->WLayer) r = 1;
        else if (l == b->WLayer) r = 0;
    }
    UnlockLayerInfo(&s->LayerInfo);
    return r;
}

static void wg_click(long xy[2])
{
    char cmd[200];
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), xy[0], xy[1]);
    wbt_run(cmd);
    Delay(50);
}

static void wg_clean(void)
{
    static const char *const files[] = {WG_DIR "/a.info", WG_DIR "/a", WG_DIR ".info", WG_DIR};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)WG_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window("WBGapW"); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
}

int wbt_wingad(int argc, char **argv)
{
    struct wbgap_wingad m;
    struct Window *w, *cover = NULL;
    struct Screen *s;
    long xy[2], to[2];
    char cmd[320], why[240];
    int how;
    (void)argc, (void)argv;
    memset(&m, 0, sizeof m);
    m.front0 = m.front1 = m.front2 = -1;
    if (!(LayersBase = OpenLibrary((CONST_STRPTR)"layers.library", 39))) { wbt_skip("W1", "no layers.library"); return 0; }
    wg_clean();
    wbt_drawer(WG_DIR, "a");
    if (!wbt_open_drawer(WG_DIR, "WBGapW", 10, &how) || !(w = wbt_place("WBGapW", NULL, WG_W, WG_H))) {
        wbt_fail("W1", WG_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    s = w->WScreen;
    printf("INFO W1 %s's window at %d,%d %dx%d, flags $%lx\n", WG_DIR, w->LeftEdge, w->TopEdge, w->Width, w->Height,
           (unsigned long)w->Flags);

    /* size: dragged 80,60 */
    m.size0[0] = w->Width;
    m.size0[1] = w->Height;
    if ((m.has_size = wg_gadget(w, GTYP_SIZING, xy))) {
        to[0] = xy[0] + 80;
        to[1] = xy[1] + 60;
        snprintf(cmd, sizeof cmd, "%s DRAG %ld %ld %ld %ld STEPS 16", wbt_dragtest(), xy[0], xy[1], to[0], to[1]);
        wbt_run(cmd);
        Delay(50);
        if (!(w = wbt_find_window("WBGapW"))) { wbt_fail("W1", "the drawer's window went after the size drag"); goto out; }
        m.size1[0] = w->Width;
        m.size1[1] = w->Height;
        printf("INFO W1 size: %ldx%ld, dragged from %ld,%ld: %ldx%ld\n", m.size0[0], m.size0[1], xy[0], xy[1],
               m.size1[0], m.size1[1]);
    }

    /* zoom: clicked twice */
    m.zoom0[0] = w->Width;
    m.zoom0[1] = w->Height;
    if ((m.has_zoom = wg_gadget(w, GTYP_WZOOM, xy))) {
        wg_click(xy);
        if (!(w = wbt_find_window("WBGapW"))) { wbt_fail("W1", "the drawer's window went after the zoom click"); goto out; }
        m.zoom1[0] = w->Width;
        m.zoom1[1] = w->Height;
        if (wg_gadget(w, GTYP_WZOOM, xy)) wg_click(xy);   /* where it is now: a zoom moves it */
        if (!(w = wbt_find_window("WBGapW"))) { wbt_fail("W1", "the drawer's window went after the zoom click"); goto out; }
        m.zoom2[0] = w->Width;
        m.zoom2[1] = w->Height;
        printf("INFO W1 zoom: %ldx%ld, %ldx%ld, %ldx%ld\n", m.zoom0[0], m.zoom0[1], m.zoom1[0], m.zoom1[1], m.zoom2[0],
               m.zoom2[1]);
    }

    /* depth: a window of our own over the middle of it, then the gadget twice */
    cover = OpenWindowTags(NULL, WA_PubScreen, (ULONG)s, WA_Left, w->LeftEdge + w->Width / 4,
                           WA_Top, w->TopEdge + w->Height / 4, WA_Width, w->Width / 2, WA_Height, w->Height / 2,
                           WA_Title, (ULONG)"W1 cover", WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_Activate, TRUE,
                           TAG_DONE);
    if (!cover) { wbt_fail("W1", "our cover window didn't open"); goto out; }
    Delay(50);
    if (!(w = wbt_find_window("WBGapW"))) { wbt_fail("W1", "the drawer's window went"); goto out; }
    m.front0 = wg_front(s, w, cover);
    if ((m.has_depth = wg_gadget(w, GTYP_WDEPTH, xy))) {
        wg_click(xy);
        m.front1 = wg_front(s, w, cover);
        wg_click(xy);
        m.front2 = wg_front(s, w, cover);
        printf("INFO W1 depth at %ld,%ld: in front of the cover %d, %d, %d\n", xy[0], xy[1], m.front0, m.front1,
               m.front2);
    }
    if (!m.has_size || !m.has_zoom || !m.has_depth) {   /* what the window has instead, for whoever closes the gap */
        struct Gadget *g;
        int n = 0;
        for (g = w->FirstGadget; g && n < 24; g = g->NextGadget, n++)
            printf("INFO W1 gadget %d: type $%04x flags $%04x at %d,%d %dx%d id %u\n", n, (unsigned)g->GadgetType,
                   (unsigned)g->Flags, g->LeftEdge, g->TopEdge, g->Width, g->Height, (unsigned)g->GadgetID);
    }
    wbt_say("W1", wbgap_wingad_judge(&m, why, sizeof why), why);
out:
    if (cover) CloseWindow(cover);
    wg_clean();
    CloseLibrary(LayersBase);
    LayersBase = NULL;
    return wbt_fails() ? 5 : 0;
}
