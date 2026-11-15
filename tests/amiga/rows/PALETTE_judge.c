/* PALETTE's file maker and judge (PALETTE.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "PALETTE.h"
#include "WBPATTERN.h"   /* wbgap_rgb_near */

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

static unsigned be16(const unsigned char *p) { return (unsigned)(p[0] << 8 | p[1]); }

/* not inlined, as PREFSWB_judge.c's (os32-gcc16 -O2 -m68040 misplaced an inlined one's bytes, 14 Nov) */
static __attribute__((noinline)) void put32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v >> 24), p[1] = (unsigned char)(v >> 16), p[2] = (unsigned char)(v >> 8), p[3] = (unsigned char)v;
}

static __attribute__((noinline)) void put16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8), p[1] = (unsigned char)v;
}

static int is_pref(const unsigned char *buf, long len)
{
    return buf && len >= 12 && !memcmp(buf, "FORM", 4) && !memcmp(buf + 8, "PREF", 4);
}

static int next_chunk(const unsigned char *buf, long len, long *at, const unsigned char **id, const unsigned char **data,
                      unsigned long *size)
{
    unsigned long sz;
    if (*at + 8 > len) return 0;
    sz = be32(buf + *at + 4);
    if (sz > (unsigned long)(len - *at - 8)) return 0;
    *id = buf + *at, *data = buf + *at + 8, *size = sz;
    *at += 8 + (long)sz + (long)(sz & 1);
    return 1;
}

/* the PALT's data in buf, or NULL */
static const unsigned char *find_palt(const unsigned char *buf, long len)
{
    long at = 12;
    unsigned long sz;
    const unsigned char *id, *d;
    if (!is_pref(buf, len)) return NULL;
    while (next_chunk(buf, len, &at, &id, &d, &sz))
        if (!memcmp(id, "PALT", 4) && sz >= WBGAP_PALT_LEN) return d;
    return NULL;
}

/* OS 3.2 Palette's defaults with no file (gfopenprefs Look/src/pf_prefs.c pf_palette_defaults) */
static void palt_defaults(unsigned char *d)
{
    static const unsigned pens[14] = {0, 1, 1, 2, 1, 3, 1, 0, 2, 1, 2, 1, 2, 0xffff};
    static const unsigned rgb[8][3] = {
        {0xaaaa, 0xaaaa, 0xaaaa}, {0x0000, 0x0000, 0x0000}, {0xffff, 0xffff, 0xffff}, {0x6666, 0x8888, 0xbbbb},
        {0xeeee, 0x4444, 0x4444}, {0x5555, 0xdddd, 0x5555}, {0x0000, 0x4444, 0xdddd}, {0xeeee, 0x9999, 0x0000}};
    int k;
    memset(d, 0, WBGAP_PALT_LEN);
    for (k = 0; k < 14; k++) put16(d + 16 + 2 * k, pens[k]), put16(d + 80 + 2 * k, pens[k]);
    for (k = 0; k < 8; k++) {
        unsigned char *c = d + 144 + 8 * k;
        put16(c, (unsigned)k), put16(c + 2, rgb[k][0]), put16(c + 4, rgb[k][1]), put16(c + 6, rgb[k][2]);
    }
    put16(d + 144 + 8 * 8, 0xffff);
}

/* colour index set in a PALT's data: 1, or 0 if the list is full */
static int palt_set(unsigned char *d, int index, unsigned long rgb)
{
    int k;
    for (k = 0; k < 32; k++) {
        unsigned char *c = d + 144 + 8 * k;
        unsigned at = be16(c);
        if (at == 0xffff || (int)at == index) {
            if (at == 0xffff) {
                if (k == 31) return 0;
                put16(c + 8, 0xffff);   /* the list's end moves on one */
            }
            put16(c, (unsigned)index);
            put16(c + 2, (unsigned)((rgb >> 16 & 0xff) * 0x101));
            put16(c + 4, (unsigned)((rgb >> 8 & 0xff) * 0x101));
            put16(c + 6, (unsigned)((rgb & 0xff) * 0x101));
            return 1;
        }
    }
    return 0;
}

long wbgap_palette_make(const unsigned char *orig, long len, int index, unsigned long rgb, unsigned char *out, long cap)
{
    if (index < 0 || index > 255) return 0;
    if (find_palt(orig, len)) {   /* the file as it is, PALT changed in place */
        unsigned char *d;
        if (len > cap) return 0;
        memcpy(out, orig, (size_t)len);
        d = out + (find_palt(orig, len) - orig);
        return palt_set(d, index, rgb) ? len : 0;
    }
    if (cap < 12 + 14 + 8 + WBGAP_PALT_LEN) return 0;
    memcpy(out, "FORM", 4), put32(out + 4, 4 + 14 + 8 + WBGAP_PALT_LEN), memcpy(out + 8, "PREF", 4);
    memcpy(out + 12, "PRHD", 4), put32(out + 16, 6), memset(out + 20, 0, 6);
    memcpy(out + 26, "PALT", 4), put32(out + 30, WBGAP_PALT_LEN);
    palt_defaults(out + 34);
    return palt_set(out + 34, index, rgb) ? 12 + 14 + 8 + WBGAP_PALT_LEN : 0;
}

int wbgap_palette_pens(unsigned char *buf, long len, const unsigned short *pens, int n)
{
    unsigned char *d = (unsigned char *)find_palt(buf, len);
    int k;
    if (!d || n < 1 || n > 31) return 0;
    for (k = 0; k < 32; k++) {
        unsigned v = k < n ? pens[k] : (k == n ? 0xffffu : 0u);
        put16(d + 16 + 2 * k, v), put16(d + 80 + 2 * k, v);
    }
    return 1;
}

long wbgap_ptrn_drop(const unsigned char *orig, long len, int which, unsigned char *out, long cap)
{
    long at = 12, o = 12, from;
    unsigned long sz;
    const unsigned char *id, *d;
    int dropped = 0;
    if (!is_pref(orig, len) || len > cap) return 0;
    memcpy(out, orig, 12);
    for (from = at; next_chunk(orig, len, &at, &id, &d, &sz); from = at) {
        if (!memcmp(id, "PTRN", 4) && sz >= 18 && (int)be16(d + 16) == which) { dropped = 1; continue; }
        memcpy(out + o, orig + from, (size_t)(at - from));
        o += at - from;
    }
    if (!dropped) return 0;
    put32(out + 4, (unsigned long)(o - 8));
    return o;
}

int wbgap_palette_colour(const unsigned char *buf, long len, int index, unsigned long *rgb)
{
    const unsigned char *d = find_palt(buf, len);
    int k;
    if (!d) return 0;
    for (k = 0; k < 32; k++) {
        const unsigned char *c = d + 144 + 8 * k;
        if (be16(c) == 0xffff) return 0;
        if ((int)be16(c) == index) {
            *rgb = (unsigned long)c[2] << 16 | (unsigned long)c[4] << 8 | c[6];
            return 1;
        }
    }
    return 0;
}

int wbgap_palette_mode(const unsigned long *c, int n, unsigned long *rgb)
{
    int i, j, best = 0;
    for (i = 0; i < n; i++) {
        int k = 0;
        for (j = 0; j < n; j++) k += c[j] == c[i];
        if (k > best) best = k, *rgb = c[i];
    }
    return best;
}

int wbgap_palette_judge(int n, int same, int before, int after, int back, int put_back, char *why, unsigned cap)
{
    if (n < 20) {
        snprintf(why, cap, "only %d points of the drawer's empty inside could be read: nothing to judge", n);
        return WBGAP_SKIP;
    }
    if (same * 10 < n * 9) {
        snprintf(why, cap, "the drawer's inside isn't one colour before the change (%d of %d points the same): a pattern or picture "
                           "is behind it, which colour 0 doesn't reach", same, n);
        return WBGAP_SKIP;
    }
    if (before * 5 >= n) {
        snprintf(why, cap, "%d of %d points were the prefs' colour before the change: it can't be told apart", before, n);
        return WBGAP_SKIP;
    }
    if (after * 2 < n) {
        snprintf(why, cap, "the drawer's background didn't follow Palette's colour 0 (ENV:Sys/palette.prefs): %d of %d points", after, n);
        return WBGAP_FAIL;
    }
    if (!put_back) {
        snprintf(why, cap, "the drawer followed (%d of %d points), but the old ENV:Sys/palette.prefs could not be put back", after, n);
        return WBGAP_FAIL;
    }
    if (back * 5 >= n) {
        snprintf(why, cap, "the drawer followed (%d of %d points), but kept the colour with the old prefs back (%d)", after, n, back);
        return WBGAP_FAIL;
    }
    snprintf(why, cap, "the drawer's background took colour 0 (%d of %d points), and gave it back with the old prefs", after, n);
    return WBGAP_PASS;
}
