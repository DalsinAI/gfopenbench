/* ---- SCREENS: C4 (the Workbench screen locked), P4a/P4b (Intuition's pointer), C1 C2a C2b (a screen of our own) ----
 * P4, as 3.2.3: the pointer goes where it is moved (P4a); on RTG it is a sprite layer (P4b): with the pointer over
 * a square of the Workbench screen, the screen's own pixels there are what they are with it away.
 * Then a screen of our own in front: its depth gadget clicked (C1), Left-Amiga-M (C2a) and Left-Amiga-N (C2b)
 * sent through DragTest; Intuition's front screen read after each.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "SCREENS.h"

WBGAP_MODE(SCREENS, wbt_screens, SCRIPT)
WBGAP_ROWMODE(C1, SCREENS)
WBGAP_ROWMODE(C2a, SCREENS)
WBGAP_ROWMODE(C2b, SCREENS)
WBGAP_ROWMODE(C4, SCREENS)
WBGAP_ROWMODE(P4a, SCREENS)
WBGAP_ROWMODE(P4b, SCREENS)

extern struct IntuitionBase *IntuitionBase;

static void move_to(int x, int y)
{
    char cmd[320];
    snprintf(cmd, sizeof cmd, "%s MOVE %d %d", wbt_dragtest(), x, y);
    wbt_run(cmd);
    Delay(25);
}

static void pointer(struct Screen *wb)
{
    char why[96];
    int cx = wb->Width / 2, cy = wb->Height / 2, half = 12;
    unsigned long id = wbt_mode_id(wb), away1, over, away2;
    ScreenToFront(wb);
    move_to(wb->Width - 30, wb->Height - 30);
    away1 = wbt_screen_box(wb, cx, cy, half);
    move_to(cx - 4, cy - 4);
    if (wbgap_pointer_at(wb->MouseX, wb->MouseY, cx - 4, cy - 4)) wbt_pass("P4a");
    else { snprintf(why, sizeof why, "moved to %d,%d, the screen says %d,%d", cx - 4, cy - 4, wb->MouseX, wb->MouseY); wbt_fail("P4a", why); }
    over = wbt_screen_box(wb, cx, cy, half);
    move_to(wb->Width - 30, wb->Height - 30);
    away2 = wbt_screen_box(wb, cx, cy, half);
    printf("INFO P4 mode $%08lx, box %08lx / %08lx / %08lx\n", id, away1, over, away2);
    wbt_say("P4b", wbgap_pointer_layer(id, away1, over, away2, why, sizeof why), why);
}

/* a key with Left-Amiga held, sent through DragTest, then Intuition's front screen */
static struct Screen *amiga_key(int code)
{
    char cmd[320];
    snprintf(cmd, sizeof cmd, "%s KEY 0x%02x QUAL LAMIGA", wbt_dragtest(), code);
    wbt_run(cmd);
    Delay(50);
    return IntuitionBase->FirstScreen;
}

int wbt_screens(int argc, char **argv)
{
    struct Screen *wb = LockPubScreen((CONST_STRPTR)"Workbench"), *s;
    struct Gadget *g;
    char cmd[320];
    long xy[2] = {-1, -1};
    (void)argc;
    (void)argv;
    if (wb) wbt_pass("C4"); else { wbt_fail("C4", "LockPubScreen(Workbench) NULL"); return 20; }
    pointer(wb);
    s = OpenScreenTags(NULL, SA_LikeWorkbench, TRUE, SA_Title, (ULONG)"WBGapScreen", SA_ShowTitle, TRUE, TAG_DONE);
    if (!s) { wbt_fail("C1", "no test screen"); wbt_skip("C2", "no test screen"); UnlockPubScreen(NULL, wb); return 10; }
    ScreenToFront(s);
    Delay(50);
    for (g = s->FirstGadget; g; g = g->NextGadget)
        if ((g->GadgetType & GTYP_SYSTYPEMASK) == GTYP_SDEPTH)
            wbgap_depth_point((g->Flags & GFLG_RELRIGHT) != 0, g->LeftEdge, g->TopEdge, g->Width, g->Height, s->Width, xy);
    if (xy[0] < 0) wbt_fail("C1", "the screen has no depth gadget");
    else {
        snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), xy[0], xy[1]);
        wbt_run(cmd);
        Delay(50);
        if (IntuitionBase->FirstScreen != s) wbt_pass("C1"); else wbt_fail("C1", "the depth gadget left the screen in front");
    }
    ScreenToFront(s);
    Delay(50);
    if (amiga_key(0x37) != s) wbt_pass("C2a"); else wbt_fail("C2a", "Left-Amiga-M left the front screen in front");   /* M: front to back */
    ScreenToFront(s);
    Delay(50);
    if (amiga_key(0x36) == wb) wbt_pass("C2b"); else wbt_fail("C2b", "Left-Amiga-N didn't bring Workbench to the front"); /* N */
    CloseScreen(s);
    UnlockPubScreen(NULL, wb);
    return wbt_fails() ? 5 : 0;
}
