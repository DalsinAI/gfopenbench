/* W1's judge (WINGAD.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "WINGAD.h"

static void wg_add(char *r, unsigned cap, const char *what)
{
    size_t n = strlen(r);
    if (n + 1 >= cap) return;
    snprintf(r + n, cap - n, "%s%s", n ? "; " : "", what);
}

static long wg_abs(long v) { return v < 0 ? -v : v; }

int wbgap_wingad_judge(const struct wbgap_wingad *m, char *why, unsigned cap)
{
    char r[240] = "";
    if (!m) {
        if (why && cap) snprintf(why, cap, "nothing measured");
        return WBGAP_FAIL;
    }
    if (!m->has_size)
        wg_add(r, sizeof r, "size: no size gadget");
    else if (m->size1[0] < m->size0[0] + 60 || m->size1[1] < m->size0[1] + 40)
        wg_add(r, sizeof r, "size: dragging the size gadget 80,60 didn't grow the window");
    if (!m->has_zoom)
        wg_add(r, sizeof r, "zoom: no zoom gadget");
    else if (m->zoom1[0] == m->zoom0[0] && m->zoom1[1] == m->zoom0[1])
        wg_add(r, sizeof r, "zoom: a click didn't change the window's size");
    else if (wg_abs(m->zoom2[0] - m->zoom0[0]) > 2 || wg_abs(m->zoom2[1] - m->zoom0[1]) > 2)
        wg_add(r, sizeof r, "zoom: a second click didn't give the size back");
    if (!m->has_depth)
        wg_add(r, sizeof r, "depth: no depth gadget");
    else if (m->front0 < 0 || m->front1 < 0 || m->front2 < 0)
        wg_add(r, sizeof r, "depth: the windows' order not read");
    else if (m->front0)
        wg_add(r, sizeof r, "depth: the cover window didn't cover the drawer's window to start with");
    else if (!m->front1)
        wg_add(r, sizeof r, "depth: a click didn't bring the window to the front");
    else if (m->front2)
        wg_add(r, sizeof r, "depth: a second click didn't send it behind");
    if (why && cap) snprintf(why, cap, "%s", r);
    return r[0] ? WBGAP_FAIL : WBGAP_PASS;
}

void wbgap_gadget_middle(long left, long top, long w, long h, int relright, int relbottom, int relwidth,
                         int relheight, long win_w, long win_h, long xy[2])
{
    long x = relright ? win_w - 1 + left : left, y = relbottom ? win_h - 1 + top : top;
    if (relwidth) w += win_w;
    if (relheight) h += win_h;
    xy[0] = x + w / 2;
    xy[1] = y + h / 2;
}
