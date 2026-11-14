/* I6's judge (TRASH.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "TRASH.h"

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
