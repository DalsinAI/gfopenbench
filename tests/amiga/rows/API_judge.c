/* A7's judge (API.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "API.h"

int wbgap_api_update_judge(long ioerr, int have_window, int changed, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_FAIL;
    if (ioerr != 0) r = "UpdateWorkbench set IoErr";
    else if (!have_window) r = "RAM:WBGap's window isn't open";
    else if (!changed) r = "the drawer didn't show the new icon 100 ms after UpdateWorkbench";
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
