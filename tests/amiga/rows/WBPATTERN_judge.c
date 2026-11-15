/* WBPATTERN's file makers and judge (WBPATTERN.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "WBPATTERN.h"

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

/* not inlined, as PREFSWB_judge.c's (os32-gcc16 -O2 -m68040 misplaced an inlined one's bytes, 14 Nov) */
static __attribute__((noinline)) void put32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v >> 24), p[1] = (unsigned char)(v >> 16), p[2] = (unsigned char)(v >> 8), p[3] = (unsigned char)v;
}

static __attribute__((noinline)) void put16(unsigned char *p, unsigned v)
{
    p[0] = (unsigned char)(v >> 8), p[1] = (unsigned char)v;
}

static int is_form(const unsigned char *buf, long len, const char *type)
{
    return buf && len >= 12 && !memcmp(buf, "FORM", 4) && !memcmp(buf + 8, type, 4);
}

/* the chunks of buf, one at a time: *at (from 12) the next one's start; 0 at the end or a chunk that runs past it */
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

/* a PTRN's data is a struct WBPatternPrefs (24 bytes: 4 reserved longs, wbp_Which, wbp_Flags, wbp_Revision,
 * wbp_Depth, wbp_DataLength) and then wbp_DataLength bytes: the picture's name, or the pattern */
#define PTRN_HEAD 24

static int is_root(const unsigned char *id, const unsigned char *d, unsigned long sz)
{
    return !memcmp(id, "PTRN", 4) && sz >= PTRN_HEAD && d[16] == 0 && d[17] == 0;
}

long wbgap_ptrn_make(const unsigned char *orig, long len, const char *name, unsigned char *out, long cap)
{
    long at = 12, n = 12;
    unsigned long sz, nl = name ? strlen(name) + 1 : 0, pl;
    const unsigned char *id, *d;
    if (!name || nl < 2 || nl > 255 || cap < 12) return 0;
    memcpy(out, "FORM", 4), memcpy(out + 8, "PREF", 4);
    if (is_form(orig, len, "PREF")) {
        while (next_chunk(orig, len, &at, &id, &d, &sz)) {
            unsigned long whole = 8 + sz + (sz & 1);
            if (is_root(id, d, sz)) continue;
            if (n + (long)whole > cap) return 0;
            memcpy(out + n, id, 8 + sz);
            if (sz & 1) out[n + 8 + sz] = 0;
            n += (long)whole;
        }
    } else {   /* PRHD as the prefs editors write it: version 0, type 0, flags 0 */
        if (n + 14 > cap) return 0;
        memcpy(out + n, "PRHD", 4), put32(out + n + 4, 6), memset(out + n + 8, 0, 6);
        n += 14;
    }
    pl = PTRN_HEAD + nl;
    if (n + 8 + (long)pl + (long)(pl & 1) > cap) return 0;
    memcpy(out + n, "PTRN", 4), put32(out + n + 4, pl);
    memset(out + n + 8, 0, PTRN_HEAD);
    out[n + 8 + 20] = 1;                    /* wbp_Revision; wbp_Which 0 (WBP_ROOT), wbp_Flags 0 (a picture) */
    put16(out + n + 8 + 22, (unsigned)nl);  /* wbp_DataLength, the name and its 0 */
    memcpy(out + n + 8 + PTRN_HEAD, name, nl);
    n += 8 + (long)pl;
    if (pl & 1) out[n++] = 0;
    put32(out + 4, (unsigned long)(n - 8));
    return n;
}

int wbgap_ptrn_root(const unsigned char *buf, long len, char *out, int cap)
{
    long at = 12;
    unsigned long sz;
    const unsigned char *id, *d;
    if (!is_form(buf, len, "PREF") || cap < 2) return 0;
    while (next_chunk(buf, len, &at, &id, &d, &sz)) {
        unsigned flags, dl;
        int k;
        if (!is_root(id, d, sz)) continue;
        flags = (unsigned)(d[18] << 8 | d[19]), dl = (unsigned)(d[22] << 8 | d[23]);
        if ((flags & 1) || dl <= 1 || PTRN_HEAD + dl > sz) continue;
        k = (int)dl < cap - 1 ? (int)dl : cap - 1;
        memcpy(out, d + PTRN_HEAD, (size_t)k);
        out[k] = 0;
        if (out[0]) return 1;
    }
    return 0;
}

long wbgap_ilbm_solid(int w, int h, unsigned r, unsigned g, unsigned b, unsigned char *out, long cap)
{
    long row = ((w + 15) / 16) * 2, body = row * h, n;
    if (w < 1 || w > 256 || h < 1 || h > 256) return 0;
    n = 12 + 8 + 20 + 8 + 6 + 8 + body;
    if (n > cap) return 0;
    memcpy(out, "FORM", 4), put32(out + 4, (unsigned long)(n - 8)), memcpy(out + 8, "ILBM", 4);
    memcpy(out + 12, "BMHD", 4), put32(out + 16, 20);
    memset(out + 20, 0, 20);
    put16(out + 20, (unsigned)w), put16(out + 22, (unsigned)h);   /* x, y 0 */
    out[28] = 1;                                                  /* nPlanes; masking, compression 0 */
    out[34] = 10, out[35] = 11;                                   /* xAspect, yAspect; transparentColor 0 */
    put16(out + 36, (unsigned)w), put16(out + 38, (unsigned)h);   /* pageWidth, pageHeight */
    memcpy(out + 40, "CMAP", 4), put32(out + 44, 6);
    out[48] = out[51] = (unsigned char)r, out[49] = out[52] = (unsigned char)g, out[50] = out[53] = (unsigned char)b;
    memcpy(out + 54, "BODY", 4), put32(out + 58, (unsigned long)body);
    memset(out + 62, 0, (size_t)body);
    return n;
}

static unsigned part_off(unsigned long a, unsigned long b, int shift)
{
    unsigned x = (unsigned)(a >> shift & 0xff), y = (unsigned)(b >> shift & 0xff);
    return x > y ? x - y : y - x;
}

int wbgap_rgb_near(unsigned long rgb, unsigned long want, unsigned tol)
{
    return part_off(rgb, want, 16) <= tol && part_off(rgb, want, 8) <= tol && part_off(rgb, want, 0) <= tol;
}

int wbgap_ptrn_judge(int n, int before, int after, int back, int put_back, char *why, unsigned cap)
{
    if (n < 20) {
        snprintf(why, cap, "only %d points of the desktop are uncovered (windows over it): nothing to judge", n);
        return WBGAP_SKIP;
    }
    if (before * 5 >= n) {
        snprintf(why, cap, "%d of %d desktop points were the picture's colour before the change: it can't be told apart", before, n);
        return WBGAP_SKIP;
    }
    if (after * 2 < n) {
        snprintf(why, cap, "the root picture named in ENV:Sys/WBPattern.prefs didn't show: %d of %d points its colour", after, n);
        return WBGAP_FAIL;
    }
    if (!put_back) {
        snprintf(why, cap, "the picture showed (%d of %d points), but the old ENV:Sys/WBPattern.prefs could not be put back", after, n);
        return WBGAP_FAIL;
    }
    if (back * 5 >= n) {
        snprintf(why, cap, "the picture showed (%d of %d points), but stayed with the old prefs back (%d)", after, n, back);
        return WBGAP_FAIL;
    }
    snprintf(why, cap, "the picture covered %d of %d points, and went with the old prefs back", after, n);
    return WBGAP_PASS;
}
