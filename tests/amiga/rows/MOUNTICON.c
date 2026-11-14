/* ---- D2: a volume mounted later gets an icon ----
 * The drive (RAD: if none given) must not be mounted yet: looked for in the DOS list, so no "insert volume"
 * requester. The desktop's icons are read once (can they be read at all?), "Mount <drive>" is run, the volume's
 * name is taken from a lock on the drive, and the icons are read again once a second for up to 10 s. The drive
 * stays mounted afterwards, for DISKIO.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/dos.h>
#include <proto/exec.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "MOUNTICON.h"

WBGAP_MODE(MOUNTICON, wbt_mounticon, SCRIPT)
WBGAP_ROWMODE(D2, MOUNTICON)

static int drive_mounted(const char *drive)
{
    char name[32];
    struct DosList *dl;
    size_t n = strlen(drive);
    int found;
    if (n && drive[n - 1] == ':') n--;
    if (!n || n >= sizeof name) return 0;
    memcpy(name, drive, n);
    name[n] = 0;
    dl = LockDosList(LDF_DEVICES | LDF_READ);
    found = FindDosEntry(dl, (CONST_STRPTR)name, LDF_DEVICES) != NULL;
    UnLockDosList(LDF_DEVICES | LDF_READ);
    return found;
}

int wbt_mounticon(int argc, char **argv)
{
    const char *drive = argc > 2 ? argv[2] : "RAD:";
    char cmd[80], path[108], vol[40], why[96];
    int was, before, rc = 0, up = 0, after = -1, k, v;
    BPTR l;
    vol[0] = 0;
    was = drive_mounted(drive);
    before = was ? 0 : wbt_desktop_has_disk("D2", "", "before");
    if (!was && before >= 0) {
        snprintf(cmd, sizeof cmd, "C:Mount %s", drive);
        rc = wbt_run(cmd);
        if (!rc) {
            struct Process *me = (struct Process *)FindTask(NULL);
            APTR wp = me->pr_WindowPtr;
            me->pr_WindowPtr = (APTR)-1;            /* no volume on it: the Lock fails quietly, never a requester */
            if ((l = Lock((CONST_STRPTR)drive, ACCESS_READ))) {
                if (NameFromLock(l, (STRPTR)path, sizeof path)) { wbgap_volume_name(path, vol, sizeof vol); up = vol[0] != 0; }
                UnLock(l);
            }
            me->pr_WindowPtr = wp;
        }
        if (up) {
            printf("INFO D2 %s mounted, its volume \"%s\"\n", drive, vol);
            for (k = 0; k < 10; k++) {
                Delay(50);
                if ((after = wbt_desktop_has_disk("D2", vol, "after")) > 0) break;
            }
        }
    }
    v = wbgap_mount_judge(was, before, rc, up, after, why, sizeof why);
    wbt_say("D2", v, why);
    return wbt_fails() ? 5 : 0;
}
