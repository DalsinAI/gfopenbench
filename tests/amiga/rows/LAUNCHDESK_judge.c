/* I11's judge (LAUNCHDESK.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "LAUNCHDESK.h"

int wbgap_desk_i11(int bad, const char *rec, int clicked, char *why, unsigned cap)
{
    const char *r = "";
    int v;
    if (bad) v = WBGAP_SKIP, r = "a click failed (WBGapDesk.rexx)";
    else if (!rec && !clicked) v = WBGAP_SKIP, r = "not clicked: WBGapDesk.rexx found no icons to click";
    else if (!rec) v = WBGAP_FAIL, r = "Shift-double-clicking LaunchT with p1 and p2 selected started nothing";
    else if (wbgap_launch_args(rec, "LaunchT p1 p2")) v = WBGAP_PASS;
    else v = WBGAP_FAIL, r = "Shift-double-click: not 3 arguments LaunchT, p1, p2 in order";
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
