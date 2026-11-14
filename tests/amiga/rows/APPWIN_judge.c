/* A1 and A2's judges (APPWIN.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "APPWIN.h"

static long dist(long a, long b) { return a > b ? a - b : b - a; }

int wbgap_appwin_msg_judge(int got, long type, long want_type, unsigned long id, unsigned long want_id,
                           unsigned long userdata, unsigned long want_userdata, long mx, long my, const long want_xy[2],
                           long win_w, long win_h, char *why, unsigned cap)
{
    char r[160] = "";
    int v = WBGAP_FAIL;
    if (!got) snprintf(r, sizeof r, "no AppMessage in 3 s");
    else if (type != want_type) snprintf(r, sizeof r, "am_Type %ld, not %ld", type, want_type);
    else if (id != want_id) snprintf(r, sizeof r, "am_ID %lu, not %lu", id, want_id);
    else if (userdata != want_userdata) snprintf(r, sizeof r, "am_UserData $%lx, not $%lx", userdata, want_userdata);
    else if (mx < 0 || my < 0 || mx >= win_w || my >= win_h)
        snprintf(r, sizeof r, "am_MouseX/Y %ld,%ld outside the window (%ldx%ld)", mx, my, win_w, win_h);
    else if (want_xy && (dist(mx, want_xy[0]) > WBGAP_APPWIN_SLACK || dist(my, want_xy[1]) > WBGAP_APPWIN_SLACK))
        snprintf(r, sizeof r, "am_MouseX/Y %ld,%ld, not where it was let go (%ld,%ld)", mx, my, want_xy[0], want_xy[1]);
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_appwin_none_judge(int got, long type, unsigned long id, const char *when, char *why, unsigned cap)
{
    if (why && cap) {
        if (got) snprintf(why, cap, "%s: an AppMessage came all the same (type %ld, id %lu)", when ? when : "a drop", type, id);
        else why[0] = 0;
    }
    return got ? WBGAP_FAIL : WBGAP_PASS;
}

int wbgap_appwin_both(int a, const char *why_a, int b, const char *why_b, char *why, unsigned cap)
{
    int v;
    const char *w;
    if (a == WBGAP_FAIL || (a == WBGAP_SKIP && b != WBGAP_FAIL)) { v = a; w = why_a; }
    else if (b != WBGAP_PASS) { v = b; w = why_b; }
    else { v = WBGAP_PASS; w = ""; }
    if (why && cap) snprintf(why, cap, "%s", w ? w : "");
    return v;
}
