/* D5r's judge (DISKS.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "DISKS.h"

/* One line of D5r's pos, [p, e), read on its own: a copy of it (up to 127 characters) is what sscanf sees, so a
 * blank in the format can't run on into the next line. The name is what follows the width, less trailing CR,
 * blanks and tabs and one trailing colon (a port that names disks "RAM Disk:", as #145 for D5). 1 with x, y, w,
 * name and its length; 0 for a line it can't read or a line with no name after the width (Mr Flibble, 7456fde7). */
static int pos_line(const char *p, const char *e, long *x, long *y, long *w, char *ln, const char **nm, size_t *nl)
{
    size_t n = (size_t)(e - p);
    int used = 0;
    if (n > 127) n = 127;
    memcpy(ln, p, n);
    ln[n] = 0;
    if (sscanf(ln, "%ld %ld %ld %n", x, y, w, &used) != 3 || !used || (size_t)used > n) return 0;
    n -= (size_t)used;
    while (n && (ln[used + n - 1] == '\r' || ln[used + n - 1] == ' ' || ln[used + n - 1] == '\t')) n--;
    if (n && ln[used + n - 1] == ':') n--;
    if (!n) return 0;
    *nm = ln + used;
    *nl = n;
    return 1;
}

int wbgap_ram_first(const char *pos, const char *vol, char *top, unsigned cap)
{
    long minx = 0, cw = 16, besty = 0;
    int pass, have = 0;
    const char *p, *nm, *e;
    char ln[128];
    size_t vn = vol ? strlen(vol) : 0, n;
    if (!cap) return -1;
    top[0] = 0;
    if (!pos) return -1;
    for (p = pos; *p; p = *e ? e + 1 : e) {                 /* the leftmost disk and its width */
        long x, y, w;
        for (e = p; *e && *e != '\n'; e++) ;
        if (pos_line(p, e, &x, &y, &w, ln, &nm, &n)) {
            if (!have || x < minx) { minx = x; cw = w < 16 ? 16 : w; }
            have = 1;
        }
    }
    if (!have) return -1;
    have = 0;
    for (p = pos; *p; p = *e ? e + 1 : e) {                 /* the column's topmost */
        long x, y, w;
        for (e = p; *e && *e != '\n'; e++) ;
        if (pos_line(p, e, &x, &y, &w, ln, &nm, &n) && x - minx < cw && (!have || y < besty)) {
            if (n >= cap) n = cap - 1;
            memcpy(top, nm, n);
            top[n] = 0;
            besty = y;
            have = 1;
        }
    }
    pass = vn && strlen(top) == vn && wbgap_has_nocase(top, vol);   /* the same length: the whole name */
    return pass;
}
