/* ---- S1-S3, S2d, I12a-c at WBStartup, A10a: WBGapTest STARTUP SETUP | CHECK ----
 * SETUP puts icons in SYS:WBStartup for WBGapA (STARTPRI=-10), WBGapB (STARTPRI=10, DONOTWAIT, 4 s) and WBGapC (no
 * STARTPRI), copies of this program, each STAMPSECS long; with WBGapD and WBGapE there too, icons for them and dates
 * (on each program and its icon) a day apart, E then C then D (S2d); with WBGapF, LAUNCHREC=RAM:wbgap-launchwb, STACK=
 * (its stack field the same) and TOOLPRI=-3 (I12a, I12b); with WBGapG, STACK= alone (I12c). RAM:wbgap-order is cleared.
 * At the next boot the desktop starts them (from_workbench in WBGapTest.c writes the lines). CHECK judges
 * RAM:wbgap-order (wbgap_startup_order), the two launch records, and A10a: ENV:GFOpenBench/LoadWB's switches against
 * the desktop's line in S:Startup-Sequence. The rows' table modes ("STARTUP CHECK", "LAUNCH, STARTUP CHECK") stay as
 * they are, so no WBGAP_ROWMODE here.
 * For lab instances only: SETUP writes to SYS:WBStartup.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "STARTUP.h"

WBGAP_MODE(STARTUP, wbt_startup, SCRIPT)

static void check_loadwb(void)
{
    char env[200] = "", seq[400] = "", line[256], want[120], got[120];
    BPTR fh;
    if (!wbt_is_gfos()) { wbt_skip("A10a", "GFOS only: 3.2.3 keeps no record of LoadWB's switches"); return; }
    if (!(fh = Open((CONST_STRPTR)"ENV:GFOpenBench/LoadWB", MODE_OLDFILE))) { wbt_fail("A10a", "no ENV:GFOpenBench/LoadWB"); return; }
    { long n = Read(fh, env, sizeof env - 1); env[n > 0 ? n : 0] = 0; }
    Close(fh);
    {   /* the line that started the desktop: SYS:System/OpenBench's where it exists (Instance-32), else LoadWB's */
        int ob = wbt_exists("SYS:System/OpenBench");
        const char *cmd = ob ? "SYS:System/OpenBench" : "LoadWB";
        if ((fh = Open((CONST_STRPTR)"S:Startup-Sequence", MODE_OLDFILE))) {
            while (FGets(fh, (STRPTR)line, sizeof line)) {
                const char *rest = wbgap_startup_desk_rest(line, ob);
                if (rest) {
                    snprintf(seq, sizeof seq, "%s", rest);
                    break;
                }
            }
            Close(fh);
        }
        printf("INFO A10a the desktop's line (the first %s line in S:Startup-Sequence): %s%s", cmd, cmd, seq[0] ? seq : "\n");
    }
    wbgap_startup_switches(seq, want, sizeof want);
    wbgap_startup_switches(env, got, sizeof got);
    printf("INFO A10a Startup-Sequence:%s; GFOpenBench took:%s\n", want[0] ? want : " none", got[0] ? got : " none");
    if (!strcmp(want, got)) wbt_pass("A10a"); else wbt_fail("A10a", "the switches taken aren't the Startup-Sequence's");
}

static int startup_setup(void)
{
    static char *ta[] = {"STARTPRI=-10", "STAMPSECS=1", NULL};
    static char *tb[] = {"STARTPRI=10", "DONOTWAIT", "STAMPSECS=4", NULL};
    static char *tc[] = {"STAMPSECS=1", NULL};
    int ok = wbt_put_icon("SYS:WBStartup/WBGapA", WBTOOL, ta) && wbt_put_icon("SYS:WBStartup/WBGapB", WBTOOL, tb) &&
             wbt_put_icon("SYS:WBStartup/WBGapC", WBTOOL, tc);
    /* S2d: D and E have no STARTPRI either; E was added before C, and C before D, so oldest first is E, C, D
     * where name order is C, D, E. The date added is the older of the program's and its icon's dates. */
    if (wbt_exists("SYS:WBStartup/WBGapD") && wbt_exists("SYS:WBStartup/WBGapE")) {
        static const char *const names[] = {"SYS:WBStartup/WBGapE", "SYS:WBStartup/WBGapC", "SYS:WBStartup/WBGapD"};
        struct DateStamp ds;
        char path[64];
        int i;
        ok = ok && wbt_put_icon("SYS:WBStartup/WBGapD", WBTOOL, tc) && wbt_put_icon("SYS:WBStartup/WBGapE", WBTOOL, tc);
        for (i = 0; i < 3 && ok; i++) {
            ds.ds_Days = 7000 + i;   /* 2 March 1997 on, a day apart */
            ds.ds_Minute = 600;
            ds.ds_Tick = 0;
            snprintf(path, sizeof path, "%s.info", names[i]);
            ok = SetFileDate((CONST_STRPTR)names[i], &ds) && SetFileDate((CONST_STRPTR)path, &ds);
        }
    } else printf("INFO no SYS:WBStartup/WBGapD and WBGapE: S2d not set up\n");
    if (wbt_exists("SYS:WBStartup/WBGapF")) {   /* I12a, I12b at WBStartup, which needs no registered Workbench */
        static char st[24];
        static char *tf[] = {"LAUNCHREC=RAM:wbgap-launchwb", st, "TOOLPRI=-3", "DONOTWAIT", NULL};
        unsigned long want = wbt_stack_wanted();
        snprintf(st, sizeof st, "STACK=%lu", want);
        ok = ok && wbt_put_icon_stack("SYS:WBStartup/WBGapF", WBTOOL, tf, (long)want);
    } else printf("INFO no SYS:WBStartup/WBGapF: I12 at WBStartup not set up\n");
    if (wbt_exists("SYS:WBStartup/WBGapG")) {   /* I12c: STACK= alone, the icon's own field left at the default */
        static char sg[24];
        static char *tg[] = {"LAUNCHREC=RAM:wbgap-launchwb2", sg, "DONOTWAIT", NULL};
        snprintf(sg, sizeof sg, "STACK=%lu", wbt_stack_wanted());
        ok = ok && wbt_put_icon("SYS:WBStartup/WBGapG", WBTOOL, tg);
    } else printf("INFO no SYS:WBStartup/WBGapG: I12c not set up\n");
    DeleteFile((CONST_STRPTR)"RAM:wbgap-order");
    printf("INFO WBStartup icons %s\n", ok ? "written" : "NOT written");
    return ok ? 0 : 20;
}

static int startup_check(void)
{   /* B (10) first, C (0), A (-10) last; B doesn't hold C up; C holds A up */
    static const char *const rows[] = {"S1", "S2", "S3", "S2d"};
    char buf[400], why[120];
    long n;
    int i, gfos;
    if ((n = wbt_read_all("RAM:wbgap-order", buf, sizeof buf)) < 0) {
        wbt_fail("S1", "RAM:wbgap-order not written: nothing in WBStartup ran"); wbt_skip("S2", "-"); wbt_skip("S3", "-");
        return 10;
    }
    printf("INFO order: ");
    for (n = 0; buf[n]; n++) putchar(buf[n] == '\n' ? ',' : buf[n]);
    putchar('\n');
    gfos = wbt_is_gfos();
    for (i = 0; i < 4; i++) wbt_say(rows[i], wbgap_startup_order(buf, rows[i], gfos, why, sizeof why), why);
    wbt_check_launchrec("RAM:wbgap-launchwb", 0, wbt_icon_stack("SYS:WBStartup/WBGapF"));
    wbt_check_stack_alone("RAM:wbgap-launchwb2", wbt_icon_stack("SYS:WBStartup/WBGapG"));
    check_loadwb();
    return wbt_fails() ? 5 : 0;
}

int wbt_startup(int argc, char **argv)
{
    if (argc < 3) { printf("usage: WBGapTest STARTUP SETUP|CHECK\n"); return 20; }
    if (!stricmp(argv[2], "SETUP")) return startup_setup();
    return startup_check();
}
