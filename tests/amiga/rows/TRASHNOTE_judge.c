/* I6t's and I6p's judges (TRASHNOTE.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "TRASHNOTE.h"

static int lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

static int same_nocase(const char *a, const char *b, size_t n)
{
    size_t i;
    for (i = 0; i < n; i++)
        if (lower((unsigned char)a[i]) != lower((unsigned char)b[i])) return 0;
    return 1;
}

int wbgap_trashnote_line(const char *note, const char *name, const char *tail)
{
    size_t nl = name ? strlen(name) : 0, tl = tail ? strlen(tail) : 0;
    const char *p = note;
    if (!note || !nl || !tl) return 0;
    while (*p) {
        const char *end = strchr(p, '\n'), *path;
        size_t ll = end ? (size_t)(end - p) : strlen(p), pl;
        if (ll > nl && !strncmp(p, name, nl) && p[nl] == '\t') {   /* the engine's names keep their case */
            path = p + nl + 1;
            pl = ll - nl - 1;
            if (pl > tl && same_nocase(path + pl - tl, tail, tl) && (path[pl - tl - 1] == ':' || path[pl - tl - 1] == '/'))
                return 1;
        }
        if (!end) break;
        p = end + 1;
    }
    return 0;
}

int wbgap_trashnote_judge(int src, int src_info, int dst, int dst_info, const char *note, const char *name,
                          const char *tail, char *why, unsigned cap)
{
    char r[200] = "", icon[64], itail[96];
    int v;
    if (!name) name = "f";
    if (!tail) tail = name;
    snprintf(icon, sizeof icon, "%s.info", name);
    snprintf(itail, sizeof itail, "%s.info", tail);
    v = wbgap_move_judge(src, src_info, dst, dst_info, name, "its volume's Trashcan", r, sizeof r);
    if (v == WBGAP_PASS) {
        if (!note)
            snprintf(r, sizeof r, "in the Trashcan, but the Trashcan has no note (.GFOpenFiles) of where it came from");
        else if (!wbgap_trashnote_line(note, name, tail))
            snprintf(r, sizeof r, "in the Trashcan, but its note has no line for %.40s from ...%.60s", name, tail);
        else if (!wbgap_trashnote_line(note, icon, itail))
            snprintf(r, sizeof r, "in the Trashcan, but its note has no line for its icon %.40s", icon);
        v = r[0] ? WBGAP_FAIL : WBGAP_PASS;
    }
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_putback_judge(long rc, int back, int back_info, int left, int noted, const char *name, char *why,
                        unsigned cap)
{
    char r[200] = "";
    int v = WBGAP_FAIL;
    if (!name) name = "the file";
    if (rc == WBGAP_PUTBACK_NOLIB) {
        snprintf(r, sizeof r, "no gfopenfiles.library to put it back with");
        v = WBGAP_SKIP;
    } else if (rc == WBGAP_PUTBACK_NOTRIED) {
        snprintf(r, sizeof r, "not tried: %.40s never reached the Trashcan (I6t)", name);
        v = WBGAP_SKIP;
    } else if (rc == WBGAP_PUTBACK_NOCTX)
        snprintf(r, sizeof r, "gfopenfiles.library's OFNewContext gave no context, so OFRestore wasn't called");
    else if (rc)
        snprintf(r, sizeof r, "OFRestore of %.40s answered %ld", name, rc);
    else if (!back || !back_info)
        snprintf(r, sizeof r, "put back, but %.40s%s isn't where it came from", name, !back ? "" : "'s icon");
    else if (left)
        snprintf(r, sizeof r, "back where it came from, but still in the Trashcan too (copied, not moved)");
    else if (noted)
        snprintf(r, sizeof r, "back where it came from, but the Trashcan's note still has its line");
    else
        v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_trashnote_iconbox(const char *line, const long *changed, long box[4])
{
    long b[4];
    char extra;
    const long slack = 2;
    if (!line || !box || sscanf(line, "%ld %ld %ld %ld %c", &b[0], &b[1], &b[2], &b[3], &extra) != 4) return 0;
    if (b[0] < 0 || b[1] < 0 || b[2] <= 0 || b[3] <= 0) return 0;
    memcpy(box, b, sizeof b);
    if (changed && changed[2] > 0 && changed[3] > 0 &&
        (b[0] < changed[0] - slack || b[1] < changed[1] - slack || b[0] + b[2] > changed[0] + changed[2] + slack ||
         b[1] + b[3] > changed[1] + changed[3] + slack))
        return -1;
    return 1;
}
