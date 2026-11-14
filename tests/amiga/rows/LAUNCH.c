/* ---- I12a, I12b (and I11's argument check) on a tool started by OpenWorkbenchObject ----
 * RAM:WBGap/LaunchT, a copy of this program, with LAUNCHREC, STACK=n (its stack field n too) and TOOLPRI=-3, is
 * opened with p1 and p2 as arguments; the line it writes (RAM:wbgap-launch) is judged by wbt_check_launchrec. Its
 * rows' lines come from STARTUP CHECK too (the table's mode "LAUNCH, STARTUP CHECK"), so no WBGAP_ROWMODE here.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "LAUNCH.h"

WBGAP_MODE(LAUNCH, wbt_launch, SCRIPT)

int wbt_launch(int argc, char **argv)
{
    static char st[24];
    static char *tt[] = {"LAUNCHREC", st, "TOOLPRI=-3", NULL};
    unsigned long want = wbt_stack_wanted();
    char cmd[320];
    BPTR dir, fh = 0;
    BOOL ok;
    int t;
    (void)argc;
    (void)argv;
    wbgap_launch_stack_tt(want, st, sizeof st);
    wbt_setup();
    {   /* this program, by its full name: a Shell's PROGDIR: is the Shell's */
        char self[256] = "WBGapTest";
        BPTR pd = GetProgramDir();
        if (pd && NameFromLock(pd, (STRPTR)self, sizeof self - 12)) AddPart((STRPTR)self, (CONST_STRPTR)"WBGapTest", sizeof self);
        snprintf(cmd, sizeof cmd, "Copy \"%s\" RAM:WBGap/LaunchT QUIET", self);
    }
    if (wbt_run(cmd) != 0 || !wbt_put_icon_stack("RAM:WBGap/LaunchT", WBTOOL, tt, (long)want)) {
        wbt_fail("I11", "RAM:WBGap/LaunchT not made"); wbt_skip("I12a", "no LaunchT"); wbt_skip("I12b", "no LaunchT"); return 20;
    }
    wbt_write_file("RAM:WBGap/p1", "p1\n");
    wbt_put_icon("RAM:WBGap/p1", WBPROJECT, NULL);
    wbt_write_file("RAM:WBGap/p2", "p2\n");
    wbt_put_icon("RAM:WBGap/p2", WBPROJECT, NULL);
    DeleteFile((CONST_STRPTR)"RAM:wbgap-launch");
    printf("INFO largest free block before the launch: %lu bytes\n", (unsigned long)AvailMem(MEMF_LARGEST));
    dir = Lock((CONST_STRPTR)"RAM:WBGap", ACCESS_READ);
    ok = dir && OpenWorkbenchObject((STRPTR)"RAM:WBGap/LaunchT", WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p1",
                                    WBOPENA_ArgLock, dir, WBOPENA_ArgName, (ULONG)"p2", TAG_DONE);
    for (t = 0; ok && t < 100 && !(fh = Open((CONST_STRPTR)"RAM:wbgap-launch", MODE_OLDFILE)); t++) Delay(5);
    if (dir) UnLock(dir);
    if (!ok) {   /* I12 is checked at WBStartup too (STARTUP CHECK), which needs no registered Workbench */
        /* not I11's FAIL: I11 is Shift-double-click on the desktop (LAUNCHDESK), which needs no registered Workbench */
        printf("INFO I11 OpenWorkbenchObjectA on a tool FALSE (IoErr %ld): A6's, see LAUNCHDESK\n", (long)IoErr());
        wbt_skip("I12a", "not launched here (A6); see STARTUP CHECK");
        wbt_skip("I12b", "not launched here (A6); see STARTUP CHECK");
        return 10;
    }
    if (fh) Close(fh);
    Delay(10);   /* the line written whole */
    wbt_check_launchrec("RAM:wbgap-launch", 1, want);
    return wbt_fails() ? 5 : 0;
}
