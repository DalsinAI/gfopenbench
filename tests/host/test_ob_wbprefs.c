/* ob_wbprefs's host test: ENV:Sys/Workbench.prefs read as GFOpenBench follows it (WBGapTest P7).
 *   test_ob_wbprefs
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_wbprefs.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

static long put32(unsigned char *b, long at, unsigned long v)
{
    b[at] = (unsigned char)(v >> 24); b[at + 1] = (unsigned char)(v >> 16);
    b[at + 2] = (unsigned char)(v >> 8); b[at + 3] = (unsigned char)v;
    return at + 4;
}

/* a FORM PREF with a PRHD, a WBNC of stack, and the hidden devices (each a WBHD) */
static long make(unsigned char *b, unsigned long stack, const char *const *hid, int nh)
{
    long n = 12, at;
    int i;
    n = put32(b, n, 0x50524844UL); n = put32(b, n, 6); memset(b + n, 0, 6); n += 6;   /* PRHD */
    n = put32(b, n, 0x57424E43UL); n = put32(b, n, 12); n = put32(b, n, stack);     /* WBNC */
    memset(b + n, 0, 8); n += 8;
    for (i = 0; i < nh; i++) {
        unsigned long sz = (unsigned long)strlen(hid[i]) + 1;
        n = put32(b, n, 0x57424844UL); n = put32(b, n, sz);
        memcpy(b + n, hid[i], sz); n += (long)sz;
        if (sz & 1) b[n++] = 0;
    }
    at = put32(b, 0, 0x464F524DUL);
    put32(b, at, (unsigned long)(n - 8));
    put32(b, 8, 0x50524546UL);
    return n;
}

int main(void)
{
    static unsigned char b[512];
    static const char *const hid[] = {"RAM", "DF0:"};
    ob_wbprefs p;
    long n;

    /* the P7 lab case: 65536 asked, RAM hidden; workbench.library's default must become 65536 */
    n = make(b, 65536, hid, 1);
    CHECK(ob_wbprefs_parse(b, n, &p));
    CHECK(p.stack == 65536);
    CHECK(p.nhidden == 1 && !strcmp(p.hidden[0], "RAM"));
    CHECK(ob_wbprefs_lib_stack(&p) == 65536);
    CHECK(ob_wbprefs_min_stack(&p) == 65536);

    /* the old prefs put back: 4096; the library goes back to 4096, the tools' least stays 8192 */
    n = make(b, 4096, NULL, 0);
    CHECK(ob_wbprefs_parse(b, n, &p) && p.stack == 4096 && p.nhidden == 0);
    CHECK(ob_wbprefs_lib_stack(&p) == 4096);
    CHECK(ob_wbprefs_min_stack(&p) == 8192);

    /* a device name with its colon, an odd-sized chunk padded */
    n = make(b, 8192, hid, 2);
    CHECK(ob_wbprefs_parse(b, n, &p) && p.nhidden == 2 && !strcmp(p.hidden[1], "DF0"));

    /* out of range: the library left as it is */
    n = make(b, 100, NULL, 0);
    CHECK(ob_wbprefs_parse(b, n, &p) && ob_wbprefs_lib_stack(&p) == 0 && ob_wbprefs_min_stack(&p) == 8192);
    n = make(b, 2UL * 1024 * 1024, NULL, 0);
    CHECK(ob_wbprefs_parse(b, n, &p) && ob_wbprefs_lib_stack(&p) == 0 && ob_wbprefs_min_stack(&p) == 8192);

    /* not a FORM PREF, cut short, nothing */
    CHECK(!ob_wbprefs_parse(b, 8, &p) && p.stack == 0 && ob_wbprefs_lib_stack(&p) == 0);
    CHECK(!ob_wbprefs_parse(NULL, 0, &p));
    memcpy(b + 8, "ILBM", 4);
    CHECK(!ob_wbprefs_parse(b, n, &p));
    /* a chunk longer than the file: what came before it is kept */
    n = make(b, 65536, hid, 1);
    CHECK(ob_wbprefs_parse(b, n - 2, &p) && p.stack == 65536 && p.nhidden == 0);

    printf("test_ob_wbprefs: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
