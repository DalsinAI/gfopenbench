/* ---- D5: the RAM Disk's icon on the desktop ----
 * WBGapRoot.rexx wrote the desktop's icons to RAM:wbgap-root ("<type> <name>" a line); RAM:'s volume name comes from
 * NameFromLock ("Ram Disk:"). D5r: RAM Disk first in the volume column, from the disks' places it wrote to
 * RAM:wbgap-root-pos (wbgap_ram_first).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "DISKS.h"

WBGAP_MODE(DISKS, wbt_disks, SCRIPT)
WBGAP_ROWMODE(D5, DISKS)
WBGAP_ROWMODE(D5r, DISKS)

int wbt_disks(int argc, char **argv)
{
    static char list[8192];
    char path[108], vol[40];
    BPTR l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ);
    long n;
    (void)argc, (void)argv;
    if (!l || !NameFromLock(l, (STRPTR)path, sizeof path)) {
        if (l) UnLock(l);
        DeleteFile((CONST_STRPTR)"RAM:wbgap-root");
        wbt_fail("D5", "RAM: has no volume name");
        wbt_fail("D5r", "RAM: has no volume name");
        return 5;
    }
    UnLock(l);
    wbgap_volume_name(path, vol, sizeof vol);
    if ((n = wbt_read_all("RAM:wbgap-root", list, sizeof list)) < 0) {
        wbt_skip("D5", "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
        wbt_skip("D5r", "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
        return 0;
    }
    printf("INFO D5 RAM: is \"%s\"; %ld bytes of the desktop's icons\n", vol, n);
    if (wbgap_root_has_disk(list, vol)) wbt_pass("D5");
    else {
        char why[128];
        if (n == 0) snprintf(why, sizeof why, "the port gave no icons");
        else if (wbgap_has_nocase(list, vol)) snprintf(why, sizeof why, "\"%s\" is there but not as a DISK (the port's .type)", vol);
        else snprintf(why, sizeof why, "no disk icon \"%s\" on the desktop", vol);
        wbt_fail("D5", why);
    }
    DeleteFile((CONST_STRPTR)"RAM:wbgap-root");
    /* D5r: RAM Disk first in the volume column (Dale, 11 Nov), from the disks' places WBGapRoot.rexx wrote */
    if ((n = wbt_read_all("RAM:wbgap-root-pos", list, sizeof list)) < 0)
        wbt_skip("D5r", "the disks' places not read (WBGapRoot.rexx)");
    else {
        char top[40], why[128];
        int r = wbgap_ram_first(list, vol, top, sizeof top);
        printf("INFO D5r the column's top disk is \"%s\"\n", top);
        if (r > 0) wbt_pass("D5r");
        else if (r < 0) wbt_fail("D5r", "no disk icon's place read");
        else {
            snprintf(why, sizeof why, "the column starts with \"%s\", not \"%s\"", top, vol);
            wbt_fail("D5r", why);
        }
        DeleteFile((CONST_STRPTR)"RAM:wbgap-root-pos");
    }
    return wbt_fails() ? 5 : 0;
}
