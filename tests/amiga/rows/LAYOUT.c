/* ---- L1, the default layout: the drawer; WBGapLayout.rexx opens it and reads the icons' places ----
 * RAM:WBGapL made afresh with the icons of wbgap_layout_icons, none snapshotted (NO_ICON_POSITION: the desktop lays
 * them out). This mode prints no row: WBGapLayout.rexx, run after it in the script, prints L1.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "LAYOUT.h"

WBGAP_MODE(LAYOUT, wbt_layout, SCRIPT)

int wbt_layout(int argc, char **argv)
{
    char path[64];
    BPTR l;
    int i, ok = 1;
    (void)argc;
    (void)argv;
    CloseWorkbenchObjectA((STRPTR)"RAM:WBGapL", NULL);
    l = CreateDir((CONST_STRPTR)"RAM:WBGapL");
    if (l) UnLock(l);
    ok = wbt_put_icon("RAM:WBGapL", WBDRAWER, NULL);
    for (i = 0; i < WBGAP_LAYOUT_N; i++) {
        const struct wbgap_layout_icon *in = &wbgap_layout_icons[i];
        long type = in->kind == 'd' ? WBDRAWER : in->kind == 't' ? WBTOOL : WBPROJECT;
        snprintf(path, sizeof path, "RAM:WBGapL/%s", in->name);
        if (type == WBDRAWER) { if ((l = CreateDir((CONST_STRPTR)path))) UnLock(l); }
        else wbt_write_file(path, "WBGapTest L1\n");
        ok = wbt_put_icon(path, type, NULL) && ok;   /* NO_ICON_POSITION: the desktop lays them out */
    }
    printf("INFO RAM:WBGapL %s\n", ok ? "made" : "NOT made");
    return ok ? 0 : 20;
}
