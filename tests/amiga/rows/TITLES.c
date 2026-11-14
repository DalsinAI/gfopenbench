/* ---- W3: window and screen titles ----
 * RAM:WBGapW3, a drawer with an icon, opened on the desktop; its window's title must be the drawer's name, and the
 * window's screen must have a title. The titles are copied under LockIBase; the drawer is cleared afterwards.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include <exec/types.h>
#include <intuition/intuitionbase.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "TITLES.h"

WBGAP_MODE(TITLES, wbt_titles, SCRIPT)
WBGAP_ROWMODE(W3, TITLES)

int wbt_titles(int argc, char **argv)
{
    struct Window *w;
    char wt[128], st[128], sw[128], why[320];
    int how, v, got = 0;
    ULONG lock;
    BPTR l = CreateDir((CONST_STRPTR)"RAM:WBGapW3");
    (void)argc, (void)argv;
    if (l) UnLock(l);
    wbt_put_icon("RAM:WBGapW3", WBDRAWER, NULL);
    wt[0] = st[0] = sw[0] = 0;
    if ((w = wbt_open_drawer("RAM:WBGapW3", "WBGapW3", 10, &how))) {
        lock = LockIBase(0);
        if ((w = wbt_find_window("WBGapW3"))) {     /* looked for again: it may have gone since */
            snprintf(wt, sizeof wt, "%s", w->Title ? (const char *)w->Title : "");
            snprintf(st, sizeof st, "%s", w->WScreen && w->WScreen->Title ? (const char *)w->WScreen->Title : "");
            snprintf(sw, sizeof sw, "%s", w->ScreenTitle ? (const char *)w->ScreenTitle : "");
            got = 1;
        }
        UnlockIBase(lock);
    }
    if (got) printf("INFO W3 the window \"%s\"; the screen \"%s\"; the window's screen title \"%s\"\n", wt, st, sw);
    v = wbgap_title_judge(got ? wt : NULL, "WBGapW3", st, why, sizeof why);
    if (v == WBGAP_SKIP && !got) snprintf(why, sizeof why, "RAM:WBGapW3's window didn't open");
    if (v == WBGAP_PASS && why[0]) printf("INFO W3 %s\n", why);   /* the chosen difference (TITLES.h) */
    wbt_say("W3", v, why);
    wbt_clear_drawer("RAM:WBGapW3", "WBGapW3", "W3");
    DeleteFile((CONST_STRPTR)"RAM:WBGapW3.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapW3");
    return wbt_fails() ? 5 : 0;
}
