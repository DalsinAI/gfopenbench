/* ---- I11 on the desktop: Shift-double-click, as a hand does it (WBGapDesk.rexx clicks) ----
 * SETUP: RAM:WBGapI holds LaunchT (a copy of this program, LAUNCHREC=RAM:wbgap-launchdesk) and projects p1 and p2,
 * left out on the desktop by RAM:.backdrop's lines (what was there before is kept in RAM:wbgap-backdrop-was; if
 * there was no .backdrop, RAM:wbgap-backdrop-made says SETUP made it), and
 * "a disk was inserted" so the desktop reads it again. WBGapDesk.rexx then clicks p1, Shift-clicks p2 and
 * Shift-double-clicks LaunchT. CHECK: I11 from what LaunchT got (LaunchT, p1, p2 in that order). DONE: RAM:.backdrop
 * as it was (put back, deleted only if SETUP made it, else left alone), and the desktop told again.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "LAUNCHDESK.h"

extern struct Library *WorkbenchBase;   /* WBGapTest.c's */

WBGAP_MODE(LAUNCHDESK, wbt_launchdesk, SCRIPT)
WBGAP_ROWMODE(I10, LAUNCHDESK)
WBGAP_ROWMODE(I11, LAUNCHDESK)

/* I11 (LaunchT, p1, p2) and I10 (OpenT, a tool; Sub, a drawer), left out together */
static const char desk_lines[] = ":WBGapI/LaunchT\n:WBGapI/p1\n:WBGapI/p2\n:WBGapI/OpenT\n:WBGapI/Sub\n";

static void desk_told(void)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "\"%s\" DISKIN", wbt_dragtest());
    wbt_run(cmd);
    Delay(100);   /* the desktop reads its disks again */
}

/* LAUNCHDESK CHECK, I11: what LaunchT got from the Shift-double-click (wbgap_desk_i11) */
static void desk_check_i11(void)
{
    char buf[400], m[8], why[120];
    int t, bad, have, clicked;
    for (t = 0; t < 20 && wbt_read_all("RAM:wbgap-launchdesk", buf, sizeof buf) < 0; t++) Delay(10);
    bad = wbt_read_all("RAM:wbgap-desk-bad", m, sizeof m) >= 0;
    have = wbt_read_all("RAM:wbgap-launchdesk", buf, sizeof buf) >= 0;
    clicked = wbt_read_all("RAM:wbgap-desk-clicked", m, sizeof m) >= 0;
    if (!bad && have) printf("INFO RAM:wbgap-launchdesk: %s", buf);
    wbt_say("I11", wbgap_desk_i11(bad, have ? buf : NULL, clicked, why, sizeof why), why);
}

/* LAUNCHDESK CHECK, I10: OpenT double-clicked ran with itself as its one argument, Sub double-clicked opened */
static void desk_check_i10(void)
{
    static char skipped[200], rec[400], why[120];
    char m[8];
    int t, r, have_skip, have_rec;
    for (t = 0; t < 20 && wbt_read_all("RAM:wbgap-launchopen", rec, sizeof rec) < 0; t++) Delay(10);
    have_skip = wbt_read_all("RAM:wbgap-open-skip", skipped, sizeof skipped) >= 0;
    if (have_skip) skipped[strcspn(skipped, "\r\n")] = 0;
    have_rec = wbt_read_all("RAM:wbgap-launchopen", rec, sizeof rec) >= 0;
    if (have_rec) printf("INFO RAM:wbgap-launchopen: %s", rec);
    r = wbgap_i10(have_skip ? skipped : NULL, wbt_read_all("RAM:wbgap-open-bad", m, sizeof m) >= 0,
                  wbt_read_all("RAM:wbgap-open-clicked", m, sizeof m) >= 0, have_rec ? rec : NULL,
                  wbt_read_all("RAM:wbgap-open-sub", m, sizeof m) >= 0, why, sizeof why);
    if (r > 0) wbt_pass("I10");
    else if (r < 0) wbt_skip("I10", why);
    else wbt_fail("I10", why);
}

int wbt_launchdesk(int argc, char **argv)
{
    static char was[2048], text[2048 + sizeof desk_lines];
    static char *tt[] = {"LAUNCHREC=RAM:wbgap-launchdesk", NULL};
    static char *tt_open[] = {"LAUNCHREC=RAM:wbgap-launchopen", NULL};
    const char *what;
    long n;
    if (argc < 3) { printf("usage: WBGapTest LAUNCHDESK SETUP|CHECK|DONE\n"); return 20; }
    what = argv[2];
    if (!stricmp(what, "SETUP")) {
        char cmd[320], self[256] = "WBGapTest";
        BPTR pd = GetProgramDir(), l;
        if (pd && NameFromLock(pd, (STRPTR)self, sizeof self - 12)) AddPart((STRPTR)self, (CONST_STRPTR)"WBGapTest", sizeof self);
        if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapI"))) UnLock(l);
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGapI/LaunchT QUIET", self);
        if (wbt_run(cmd) != 0 || !wbt_put_icon("RAM:WBGapI/LaunchT", WBTOOL, tt)) {
            wbt_fail("I11", "RAM:WBGapI/LaunchT not made");
            return 20;
        }
        wbt_write_file("RAM:WBGapI/p1", "p1\n");
        wbt_put_icon("RAM:WBGapI/p1", WBPROJECT, NULL);
        wbt_write_file("RAM:WBGapI/p2", "p2\n");
        wbt_put_icon("RAM:WBGapI/p2", WBPROJECT, NULL);
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-skip");
        /* I10: OpenT, the same tool recording to its own file, and Sub, a drawer, each double-clicked alone */
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGapI/OpenT QUIET", self);
        if (wbt_run(cmd) != 0 || !wbt_put_icon("RAM:WBGapI/OpenT", WBTOOL, tt_open))
            wbt_write_file("RAM:wbgap-open-skip", "RAM:WBGapI/OpenT not made");
        if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapI/Sub"))) UnLock(l);
        if (!wbt_put_icon("RAM:WBGapI/Sub", WBDRAWER, NULL)) wbt_write_file("RAM:wbgap-open-skip", "RAM:WBGapI/Sub's icon not made");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-launchopen");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-sub");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-clicked");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-open-bad");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-launchdesk");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-clicked");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-bad");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-desk-covered");
        /* nothing over the desktop before WBGapDesk.rexx clicks it: BIG's window, if a run left it */
        if (!wbgap_desk_clickable(wbt_clear_drawer("RAM:WBGapBig", "WBGapBig", "I11"))) {
            wbt_write_file("RAM:wbgap-desk-covered", "RAM:WBGapBig's window stays over the desktop\n");
            wbt_skip("I11", "RAM:WBGapBig's window stays over the desktop: no clicks");
            wbt_skip("I10", "RAM:WBGapBig's window stays over the desktop: no clicks");
            return 0;
        }
        n = wbt_read_all("RAM:.backdrop", was, sizeof was);
        if (n < 0) was[0] = 0;
        /* markers for DONE: wbgap-backdrop-was, the copy to put back; wbgap-backdrop-made, SETUP made the file */
        switch (wbgap_backdrop_setup(n, (long)sizeof was, n >= 0 && strstr(was, desk_lines) != NULL)) {
        case WBGAP_BD_TOOLONG:
            printf("INFO I11 RAM:.backdrop is over %ld bytes: left as it is\n", (long)sizeof was - 2);
            wbt_skip("I11", "RAM:.backdrop too long to keep and put back");
            wbt_skip("I10", "RAM:.backdrop too long to keep and put back");
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
            return 0;
        case WBGAP_BD_MADE:
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
            wbt_write_file("RAM:wbgap-backdrop-made", "made by WBGapTest LAUNCHDESK SETUP\n");
            wbt_write_file("RAM:.backdrop", desk_lines);
            break;
        case WBGAP_BD_SAVED:
            wbt_write_file("RAM:wbgap-backdrop-was", was);
            DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
            snprintf(text, sizeof text, "%s%s%s", was, n && was[n - 1] != '\n' ? "\n" : "", desk_lines);
            wbt_write_file("RAM:.backdrop", text);
            break;
        default:   /* the lines there already: no marker changed (an earlier SETUP's stay for its DONE) */
            break;
        }
        desk_told();
        printf("INFO I11 RAM:WBGapI's LaunchT, p1 and p2 left out on the desktop; I10's OpenT and Sub too\n");
        return 0;
    }
    if (!stricmp(what, "CHECK")) {
        char buf[400];
        if (wbt_read_all("RAM:wbgap-desk-covered", buf, sizeof buf) >= 0) {   /* SETUP said SKIP already */
            wbt_skip("I11", "RAM:WBGapBig's window stays over the desktop: no clicks");
            wbt_skip("I10", "RAM:WBGapBig's window stays over the desktop: no clicks");
            return 0;
        }
        desk_check_i11();
        desk_check_i10();
        return wbt_fails() ? 5 : 0;
    }
    if (!stricmp(what, "DONE")) {
        char m[8];
        n = wbt_read_all("RAM:wbgap-backdrop-was", was, sizeof was);
        switch (wbgap_backdrop_done(n >= 0, wbt_read_all("RAM:wbgap-backdrop-made", m, sizeof m) >= 0)) {
        case WBGAP_BD_RESTORE: wbt_write_file("RAM:.backdrop", was); break;
        case WBGAP_BD_DELETE: DeleteFile((CONST_STRPTR)"RAM:.backdrop"); break;
        default: break;   /* SETUP left .backdrop as it was */
        }
        DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-was");
        DeleteFile((CONST_STRPTR)"RAM:wbgap-backdrop-made");
        if (WorkbenchBase) CloseWorkbenchObjectA((STRPTR)"RAM:WBGapI/Sub", NULL);   /* if WBGapDesk.rexx left it open */
        {   /* RAM:WBGapI and its icons gone */
            static const char *const gone[] = {"RAM:WBGapI/LaunchT.info", "RAM:WBGapI/LaunchT", "RAM:WBGapI/p1.info",
                                               "RAM:WBGapI/p1", "RAM:WBGapI/p2.info", "RAM:WBGapI/p2",
                                               "RAM:WBGapI/OpenT.info", "RAM:WBGapI/OpenT", "RAM:WBGapI/Sub.info",
                                               "RAM:WBGapI/Sub", "RAM:WBGapI", "RAM:wbgap-desk-clicked",
                                               "RAM:wbgap-desk-bad", "RAM:wbgap-desk-covered", "RAM:wbgap-open-skip",
                                               "RAM:wbgap-open-clicked", "RAM:wbgap-open-bad", "RAM:wbgap-open-sub"};
            int k;
            for (k = 0; k < (int)(sizeof gone / sizeof gone[0]); k++) DeleteFile((CONST_STRPTR)gone[k]);
        }
        /* told last, so the desktop reads RAM: again with the five icons' files gone as well as their .backdrop
         * lines (told before the delete, Ivanova's 0658 lab still showed them on the desktop a minute later) */
        desk_told();
        return 0;
    }
    printf("unknown: LAUNCHDESK %s\n", what);
    return 20;
}

