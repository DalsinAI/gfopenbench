/* D1's judge (DISKIO.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "DISKIO.h"

int wbgap_diskio_judge(int before, int inhibited, int after, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_PASS;
    if (!before) { r = "the disk had no icon before"; v = WBGAP_SKIP; }
    else if (before < 0 || inhibited < 0 || after < 0) { r = "the desktop's icons not read each time"; v = WBGAP_SKIP; }
    else if (inhibited && !after) { r = "its icon stayed while inhibited and then went"; v = WBGAP_FAIL; }
    else if (inhibited) { r = "its icon stayed while inhibited"; v = WBGAP_FAIL; }
    else if (!after) { r = "its icon went but did not come back"; v = WBGAP_FAIL; }
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
