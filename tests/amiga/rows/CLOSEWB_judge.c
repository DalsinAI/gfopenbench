/* C6's judges (CLOSEWB.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "CLOSEWB.h"

int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_FAIL;
    if (closed && !reopened) r = "the screen closed and OpenWorkBench() didn't bring it back";
    else if (!closed && !same_screen) r = "CloseWorkBench() refused, but the Workbench screen changed";
    else if (!port) r = closed ? "the desktop didn't come back after OpenWorkBench()" : "the desktop went though CloseWorkBench() refused";
    else if (!drawer) r = closed ? "RAM:WBGapC's window wasn't opened again" : "RAM:WBGapC's window went though CloseWorkBench() refused";
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_closewb_over(const char *text)
{
    const char *p, *want = "INFO C6 done";
    size_t n = strlen(want);
    if (!text) return 0;
    for (p = text; (p = strstr(p, want)); p += n)
        if ((p == text || p[-1] == '\n') && (p[n] == '\n' || p[n] == '\r' || p[n] == 0)) return 1;
    return 0;
}
