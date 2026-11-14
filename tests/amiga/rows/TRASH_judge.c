/* I6's judge (TRASH.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "TRASH.h"
#include "WINGAD.h"

int wbgap_trash_judge(int src, int src_info, int dst, int dst_info, int can_info, const char *name, char *why,
                      unsigned cap)
{
    char r[200] = "";
    int v;
    if (!name) name = "the file";
    if (can_info < 0) {
        snprintf(r, sizeof r, "the trashcan wasn't looked at");
        v = WBGAP_SKIP;
    } else if (!can_info) {
        snprintf(r, sizeof r, "the trashcan's own icon went (was the trashcan moved, not %.40s?)", name);
        v = WBGAP_FAIL;
    } else
        v = wbgap_move_judge(src, src_info, dst, dst_info, name, "the trashcan", r, sizeof r);
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_trash_title_ours(const char *title)
{
    const char *p = title;
    size_t n = strlen("WBGapT");
    if (!title) return 0;
    while ((p = strstr(p, "WBGapT"))) {
        char c = p[n];
        if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_')) return 1;
        p += n;
    }
    return 0;
}

int wbgap_trash_open_why(int found, const char *title, const long want[4], const long got[4], const long lim[4],
                         char *why, unsigned cap)
{
    char r[240];
    if (!found) {
        snprintf(r, sizeof r, "never showed (no window with WBGapT in its title in 10 s)");
    } else if (!wbgap_trash_title_ours(title)) {
        snprintf(r, sizeof r, "not found: the window found by \"WBGapT\" is \"%.60s\", not RAM:WBGapT's", title);
    } else
        return wbgap_wingad_place_why(want, got, lim, 1, why, cap);
    if (why && cap) snprintf(why, cap, "%s", r);
    return 1;
}
