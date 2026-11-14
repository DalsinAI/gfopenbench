/* ---- F1, F2: the drawer shows changes without Update ----
 * RAM:WBGap's window opened (OpenWorkbenchObjectA, else DragTest OPEN) and made big enough to show its icons
 * (wbgap_notify_size); a new file with an icon must show in it within 5 s (F1), and "a"'s icon changed to a tool's
 * must show within 5 s (F2). A change is the window's inside changing (wbt_changes_within).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "NOTIFY.h"

WBGAP_MODE(NOTIFY, wbt_notify, SCRIPT)
WBGAP_ROWMODE(F1, NOTIFY)
WBGAP_ROWMODE(F2, NOTIFY)

int wbt_notify(int argc, char **argv)
{
    struct Window *w;
    unsigned long before;
    long size[2];
    int how;
    (void)argc, (void)argv;
    DeleteFile((CONST_STRPTR)"RAM:WBGap/new1");        /* a run before left them: the file must be new */
    DeleteFile((CONST_STRPTR)"RAM:WBGap/new1.info");
    wbt_setup();
    if (!(w = wbt_open_drawer("RAM:WBGap", "WBGap", 10, &how))) {
        wbt_fail("F1", "RAM:WBGap's window didn't open");
        wbt_skip("F2", "no window");
        return 10;
    }
    printf("INFO RAM:WBGap opened by %s\n", how ? "DragTest OPEN" : "OpenWorkbenchObjectA");
    if (wbgap_notify_size(w->Width, w->Height, size)) {   /* a new drawer may open too small to show its icons (L1) */
        ChangeWindowBox(w, w->LeftEdge, w->TopEdge, size[0], size[1]);
        Delay(50);
        printf("INFO RAM:WBGap's window sized to %dx%d for F1 and F2\n", w->Width, w->Height);
    }
    Delay(100);   /* its icons drawn */
    before = wbt_inside_sum(w);
    wbt_write_file("RAM:WBGap/new1", "new\n");
    wbt_put_icon("RAM:WBGap/new1", WBPROJECT, NULL);
    if (wbt_changes_within(w, before, 5)) wbt_pass("F1");
    else wbt_fail("F1", "a new file with an icon didn't show in 5 s");
    Delay(50);
    if (!(w = wbt_find_window("WBGap"))) { wbt_skip("F2", "window gone"); return 5; }
    before = wbt_inside_sum(w);
    wbt_put_icon("RAM:WBGap/a", WBTOOL, NULL);   /* "a"'s icon now a tool's image */
    if (wbt_changes_within(w, before, 5)) wbt_pass("F2");
    else wbt_fail("F2", "a changed .info didn't show in 5 s");
    return wbt_fails() ? 5 : 0;
}
