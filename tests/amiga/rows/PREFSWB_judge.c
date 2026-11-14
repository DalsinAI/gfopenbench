/* PREFSWB's file maker and judge (PREFSWB.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "PREFSWB.h"

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

static void put32(unsigned char *p, unsigned long v)
{
    p[0] = (unsigned char)(v >> 24), p[1] = (unsigned char)(v >> 16), p[2] = (unsigned char)(v >> 8), p[3] = (unsigned char)v;
}

/* buf a FORM PREF whose chunks fit: 1 */
static int is_pref(const unsigned char *buf, long len)
{
    return buf && len >= 12 && !memcmp(buf, "FORM", 4) && !memcmp(buf + 8, "PREF", 4);
}

/* the chunks of buf, one at a time: *at (from 12) the next one's start, its id and data; 0 at the end or a chunk that
 * runs past the file */
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

static int same_dev(const unsigned char *d, unsigned long n, const char *dev)
{
    size_t k = strlen(dev);
    while (n && !d[n - 1]) n--;
    if (n && d[n - 1] == ':') n--;
    if (k && dev[k - 1] == ':') k--;
    if (n != k) return 0;
    while (k--) if ((d[k] | 0x20) != ((unsigned char)dev[k] | 0x20)) return 0;
    return 1;
}

int wbgap_wbprefs_hides(const unsigned char *buf, long len, const char *dev)
{
    const unsigned char *id, *d;
    unsigned long sz;
    long at = 12;
    if (!is_pref(buf, len) || !dev) return 0;
    while (next_chunk(buf, len, &at, &id, &d, &sz))
        if (!memcmp(id, "WBHD", 4) && same_dev(d, sz, dev)) return 1;
    return 0;
}

unsigned long wbgap_wbprefs_stack(const unsigned char *buf, long len)
{
    const unsigned char *id, *d;
    unsigned long sz;
    long at = 12;
    if (!is_pref(buf, len)) return 0;
    while (next_chunk(buf, len, &at, &id, &d, &sz))
        if (!memcmp(id, "WBNC", 4) && sz >= 4) return be32(d);
    return 0;
}

/* a chunk into out at *n: 0 if there's no room */
static int add_chunk(unsigned char *out, long cap, long *n, const char *id, const unsigned char *data, unsigned long sz)
{
    long need = 8 + (long)sz + (long)(sz & 1);
    if (*n + need > cap) return 0;
    memcpy(out + *n, id, 4);
    put32(out + *n + 4, sz);
    if (sz) memcpy(out + *n + 8, data, sz);
    if (sz & 1) out[*n + 8 + sz] = 0;
    *n += need;
    return 1;
}

long wbgap_wbprefs_make(const unsigned char *orig, long len, unsigned long stack, const char *hide, unsigned char *out,
                        long cap)
{
    /* mkprefs.py's WBNC, as Prefs/Workbench writes it (50 bytes): stack, restart 5, 16, -1, -1, 1, 1, 1, 25, 1, 1, then 0 */
    static const unsigned char wbnc0[50] = {0, 0, 0x20, 0, 0, 0, 0, 5, 0, 0, 0, 16, 0xff, 0xff, 0xff, 0xff, 0, 1, 0, 1,
                                            0, 1, 0, 0, 0, 25, 0, 1, 0, 1};
    unsigned char wbnc[64];
    const unsigned char *id, *d;
    unsigned long sz;
    long at = 12, n = 12;
    int had_wbnc = 0;
    if (!out || cap < 12) return 0;
    memcpy(out, "FORM", 4);
    memcpy(out + 8, "PREF", 4);
    if (is_pref(orig, len)) {
        while (next_chunk(orig, len, &at, &id, &d, &sz)) {
            char cid[5];
            memcpy(cid, id, 4);
            cid[4] = 0;
            if (!memcmp(id, "WBNC", 4) && sz >= 4 && sz <= sizeof wbnc) {
                memcpy(wbnc, d, sz);
                put32(wbnc, stack);
                if (!add_chunk(out, cap, &n, "WBNC", wbnc, sz)) return 0;
                had_wbnc = 1;
            } else if (!add_chunk(out, cap, &n, cid, d, sz)) return 0;
        }
    } else {
        static const unsigned char prhd[6] = {0};
        if (!add_chunk(out, cap, &n, "PRHD", prhd, sizeof prhd)) return 0;
    }
    if (!had_wbnc) {
        memcpy(wbnc, wbnc0, sizeof wbnc0);
        put32(wbnc, stack);
        if (!add_chunk(out, cap, &n, "WBNC", wbnc, sizeof wbnc0)) return 0;
    }
    if (hide && *hide && !wbgap_wbprefs_hides(out, n, hide)) {
        unsigned char h[40];
        size_t k = strlen(hide);
        if (k + 1 > sizeof h) return 0;
        memcpy(h, hide, k + 1);
        if (!add_chunk(out, cap, &n, "WBHD", h, (unsigned long)(k + 1))) return 0;
    }
    put32(out + 4, (unsigned long)(n - 8));
    return n;
}

int wbgap_wbprefs_judge(int gone, unsigned long got, unsigned long want, int back, char *why, unsigned cap)
{
    int v = WBGAP_PASS;
    char t[160];
    t[0] = 0;
    if (gone < 0) v = WBGAP_SKIP, snprintf(t, sizeof t, "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)");
    else if (!gone) v = WBGAP_FAIL, snprintf(t, sizeof t, "the hidden device's disk icon is still on the desktop");
    else if (got != want) v = WBGAP_FAIL, snprintf(t, sizeof t, "workbench.library's default stack %lu after the change, not the prefs' %lu", got, want);
    else if (back == 0) v = WBGAP_FAIL, snprintf(t, sizeof t, "hidden right, but the disk icon didn't come back with the old prefs");
    if (why && cap) snprintf(why, cap, "%s", t);
    return v;
}
