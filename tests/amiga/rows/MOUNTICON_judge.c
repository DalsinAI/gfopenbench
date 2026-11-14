/* D2's judge (MOUNTICON.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "MOUNTICON.h"

int wbgap_mount_judge(int was_there, int before, int mount_rc, int vol_up, int after, char *why, unsigned cap)
{
    char r[96] = "";
    int v = WBGAP_SKIP;
    if (was_there) snprintf(r, sizeof r, "the drive was mounted before the test: no later mount to watch");
    else if (before < 0) snprintf(r, sizeof r, "the desktop's icons not read before the mount");
    else if (mount_rc) snprintf(r, sizeof r, "Mount gave %d (no mount list for the drive?)", mount_rc);
    else if (!vol_up) snprintf(r, sizeof r, "mounted, but no volume came up on it");
    else if (after < 0) snprintf(r, sizeof r, "the desktop's icons not read after the mount");
    else if (!after) { snprintf(r, sizeof r, "its volume came up but got no icon on the desktop"); v = WBGAP_FAIL; }
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
