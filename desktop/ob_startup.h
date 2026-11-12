/* ob_startup.h: the order and the waiting of SYS:WBStartup (GFOpenBench 0.1.50). Plain C with no Amiga headers,
 * tested on the host (tests/host/test_ob_startup.c).
 *
 * As OS 3.2.3's Workbench starts SYS:WBStartup: by each icon's STARTPRI (-128 to 127, 0 when it has none),
 * highest first, then by name; and one at a time: the next starts when this one has ended, unless its icon says
 * DONOTWAIT, and after at most WAIT=n seconds when it says that. Up to 0.1.49 GFOpenBench started them all at
 * once in the directory's order (WBGapTest rows S2 and S3).
 *
 * A program with neither DONOTWAIT nor WAIT that never ends would hold the desktop's start up for good (as it
 * does on 3.2.3); GFOpenBench gives up waiting for it after OB_START_WAIT_MAX seconds and goes on.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_STARTUP_H
#define OB_STARTUP_H

#define OB_START_WAIT_MAX 60          /* seconds, with no WAIT=n: the most a program holds up the next */

typedef struct ob_start {
    char name[108];                   /* the program's name in SYS:WBStartup (its icon's, without ".info") */
    int pri;                          /* STARTPRI, -128 to 127 */
    int wait;                         /* seconds to wait for it to end: 0 for DONOTWAIT */
} ob_start;

static inline int ob_start_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

/* The value of tool type key ("KEY=value" or "KEY" alone, the key's case aside), or NULL; "" for a bare key. */
static inline const char *ob_start_tt(const char *const *tt, const char *key)
{
    for (; tt && *tt; tt++) {
        const char *a = *tt, *b = key;
        while (*b && ob_start_lower((unsigned char)*a) == ob_start_lower((unsigned char)*b)) a++, b++;
        if (*b) continue;
        if (!*a) return a;
        if (*a == '=') return a + 1;
    }
    return 0;
}

/* A whole number at s (an optional sign, then digits); def when there is none. */
static inline long ob_start_num(const char *s, long def)
{
    long n = 0;
    int neg = 0, any = 0;
    if (!s) return def;
    while (*s == ' ') s++;
    if (*s == '-' || *s == '+') neg = *s++ == '-';
    for (; *s >= '0' && *s <= '9'; s++, any = 1) if (n < 100000) n = n * 10 + (*s - '0');
    return any ? (neg ? -n : n) : def;
}

/* The entry's priority and wait, from its icon's tool types (NULL: an icon without any). */
static inline void ob_start_read(ob_start *e, const char *const *tt)
{
    long p = ob_start_num(ob_start_tt(tt, "STARTPRI"), 0), w;
    e->pri = p < -128 ? -128 : p > 127 ? 127 : (int)p;
    if (ob_start_tt(tt, "DONOTWAIT")) e->wait = 0;
    else {
        w = ob_start_num(ob_start_tt(tt, "WAIT"), OB_START_WAIT_MAX);
        e->wait = w < 0 ? 0 : w > 3600 ? 3600 : (int)w;
    }
}

/* qsort's order: the higher STARTPRI first, then by name (case aside). */
static inline int ob_start_cmp(const void *x, const void *y)
{
    const ob_start *a = (const ob_start *)x, *b = (const ob_start *)y;
    const char *p = a->name, *q = b->name;
    if (a->pri != b->pri) return b->pri - a->pri;
    while (*p && ob_start_lower((unsigned char)*p) == ob_start_lower((unsigned char)*q)) p++, q++;
    return ob_start_lower((unsigned char)*p) - ob_start_lower((unsigned char)*q);
}

#endif
