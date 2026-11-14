/* I5's judge and its list readers (DESKDROP.h), plain C for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "wbgap_rows.h"
#include "DESKDROP.h"

int wbgap_deskdrop_judge(int src, int src_info, int on_desk, int in_win, const char *name, char *why, unsigned cap)
{
    char r[200] = "";
    int v = WBGAP_FAIL;
    if (!name) name = "the file";
    if (!src && !src_info)
        snprintf(r, sizeof r, "%.40s and its icon went from its drawer (moved or deleted, not left out)", name);
    else if (!src)
        snprintf(r, sizeof r, "%.40s went from its drawer, its icon left behind (left out means it stays there)", name);
    else if (!src_info)
        snprintf(r, sizeof r, "%.40s's icon (.info) went from its drawer (left out means it stays there)", name);
    else if (on_desk < 0) {
        snprintf(r, sizeof r, "the desktop's icons not read (no WORKBENCH port, or no rx)");
        v = WBGAP_SKIP;
    } else if (!on_desk)
        snprintf(r, sizeof r, "%.40s isn't on the desktop (not left out)", name);
    else if (in_win > 0)
        snprintf(r, sizeof r, "%.40s is on the desktop but still in its drawer's window", name);
    else
        v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

static int bd_same(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
    return !*a && !*b;
}

/* the next line of list, "left top width height name": its four numbers (n 4, else 0) and its name (the rest of
 * the line, spaces and all; "" if none); where the line after it starts */
static const char *bd_line(const char *p, char *name, unsigned cap, long box[4], int *n)
{
    const char *e = p, *q = p;
    unsigned k = 0;
    char *end;
    int i;
    while (*e && *e != '\n') e++;
    *n = 0;
    for (i = 0; i < 4; i++) {
        while (q < e && *q == ' ') q++;
        if (q >= e || !(isdigit((unsigned char)*q) || *q == '-')) break;
        box[i] = strtol(q, &end, 10);
        if (end == q || end > e) break;
        q = end;
        (*n)++;
    }
    if (*n == 4) {
        while (q < e && *q == ' ') q++;
        while (q < e && k + 1 < cap) name[k++] = *q++;
        while (k && (name[k - 1] == ' ' || name[k - 1] == '\r')) k--;
    } else
        *n = 0;
    name[k] = 0;
    return *e ? e + 1 : e;
}

int wbgap_deskdrop_has(const char *list, const char *name)
{
    char nm[64];
    long box[4];
    int n;
    const char *p = list;
    if (!list || !name || !*name) return 0;
    while (*p) {
        p = bd_line(p, nm, sizeof nm, box, &n);
        if (n == 4 && bd_same(nm, name)) return 1;
    }
    return 0;
}

int wbgap_deskdrop_clear(const char *list, long x, long y, long margin)
{
    char nm[64];
    long box[4];
    int n;
    const char *p = list;
    if (!list) return 1;
    while (*p) {
        p = bd_line(p, nm, sizeof nm, box, &n);
        if (n == 4 && x >= box[0] - margin && x < box[0] + box[2] + margin && y >= box[1] - margin &&
            y < box[1] + box[3] + margin)
            return 0;
    }
    return 1;
}
