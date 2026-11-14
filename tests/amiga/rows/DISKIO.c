/* ---- D1: a disk's icon goes while inhibited and comes back ----
 * The drive (RAD: if none given) must be mounted with a volume on it, and that volume must have an icon on the
 * desktop. DragTest INHIBIT stops its file system (as Format does) and starts it again; the desktop's icons are read
 * before, while inhibited and after, by WBGapRoot.rexx beside this program (wbt_desktop_has_disk, shared with D2).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "DISKIO.h"

WBGAP_MODE(DISKIO, wbt_diskio, SCRIPT)
WBGAP_ROWMODE(D1, DISKIO)

static int diskio_inhibit(const char *drive, int on)
{
    char cmd[400];
    snprintf(cmd, sizeof cmd, "\"%s\" INHIBIT \"%s\" %s", wbt_dragtest(), drive, on ? "ON" : "OFF");
    return wbt_run(cmd);
}

int wbt_diskio(int argc, char **argv)
{
    const char *drive = argc > 2 ? argv[2] : "RAD:";
    char path[108], vol[40], why[64];
    BPTR l;
    int before, off, after, rc, v;
    if (!(l = Lock((CONST_STRPTR)drive, ACCESS_READ))) { wbt_skip("D1", "the drive isn't mounted"); return 0; }
    if (!NameFromLock(l, (STRPTR)path, sizeof path)) { UnLock(l); wbt_fail("D1", "the drive has no volume name"); return 5; }
    UnLock(l);
    wbgap_volume_name(path, vol, sizeof vol);
    printf("INFO D1 %s is \"%s\"\n", drive, vol);
    if ((before = wbt_desktop_has_disk("D1", vol, "before")) < 0) {
        wbt_skip("D1", "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
        return 0;
    }
    if (!before) { wbt_skip("D1", "the disk had no icon before"); return 0; }
    if ((rc = diskio_inhibit(drive, 1)) != 0) {
        diskio_inhibit(drive, 0);
        snprintf(why, sizeof why, "DragTest INHIBIT %s ON gave %d", drive, rc);
        wbt_skip("D1", why);
        return 0;
    }
    Delay(100);
    off = wbt_desktop_has_disk("D1", vol, "inhibited");
    diskio_inhibit(drive, 0);                       /* always given back, whatever was read */
    Delay(150);
    after = wbt_desktop_has_disk("D1", vol, "after");
    v = wbgap_diskio_judge(before, off, after, why, sizeof why);
    if (v == WBGAP_PASS) wbt_pass("D1");
    else if (v == WBGAP_SKIP) wbt_skip("D1", why);
    else wbt_fail("D1", why);
    return wbt_fails() ? 5 : 0;
}
