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
    if (!m->has_zoom && m->zoom_off)
        ;                                   /* GFOS's design: no zoom gadget anywhere (WINGAD.h) */
    else if (!m->has_zoom)
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

static int wg_word(const char **p, const char *word)
{
    const char *s = *p;
    while (*word) {
        char c = *s >= 'A' && *s <= 'Z' ? (char)(*s - 'A' + 'a') : *s;
        if (c != *word) return 0;
        s++, word++;
    }
    *p = s;
    return 1;
}

/* 1 if look has a line of just these words (any case, split by spaces or tabs, spaces round them, a CR at the end) */
static int wg_line(const char *look, const char *const *words)
{
    const char *s = look;
    while (s && *s) {
        const char *q = s;
        int i, ok = 1;
        for (i = 0; ok && words[i]; i++) {
            while (*q == ' ' || *q == '\t') q++;
            ok = wg_word(&q, words[i]) && (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n' || !*q);
        }
        if (ok) {
            while (*q == ' ' || *q == '\t' || *q == '\r') q++;
            if (!*q || *q == '\n') return 1;
        }
        while (*s && *s != '\n') s++;
        if (*s) s++;
    }
    return 0;
}

/* 1 if the look's theme is Classic, as GFOpenLook itself reads it (gfopengadtools look/openlook.c read_look_text, the
 * reader that takes zoom gadgets away; not lib/ogt_look.c's ogt_look_load, which the apps use for colours): the
 * theme is the first word of line 1 ("Classic", "Classic light", "\"Classic\" dark"...), and it stays Classic when
 * line 1 is blank or its first word starts with ';'. Classic on a later line doesn't count. The name is a file's, so
 * any case. */
static int wg_classic_theme(const char *look)
{
    const char *q = look;
    int quoted = 0;
    if (!q) return 0;
    while (*q == ' ' || *q == '\t') q++;
    if (!*q || *q == '\r' || *q == '\n' || *q == ';') return 1;   /* line 1 names no theme: GFOpenLook keeps Classic */
    if (*q == '"') quoted = 1, q++;
    if (!wg_word(&q, "classic")) return 0;
    return (quoted && *q == '"') || (!quoted && (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n' || !*q));
}

/* gfopengadtools DESIGN.md: the Classic theme passes everything through, and Classic screens keep their zoom gadgets,
 * whatever "zoom off" says. Out of scope here: a "never" line naming Workbench, "profile" presets, and which of two
 * "screen workbench" lines wins: in GFOpenLook the last one does (read_look_text); here any Classic one counts */
int wbgap_wingad_zoom_off(const char *look)
{
    static const char *const zoom_off[] = {"zoom", "off", NULL};
    static const char *const classic[] = {"screen", "workbench", "classic", NULL};
    return wg_line(look, zoom_off) && !wg_line(look, classic) && !wg_classic_theme(look);
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
