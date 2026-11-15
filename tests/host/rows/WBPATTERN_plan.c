/* P1: WBPattern's root picture (tests/amiga/rows/WBPATTERN.c), its file makers and judge.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "WBPATTERN.h"

void wbpattern_plan(void);
WBGAP_PLAN(wbpattern_plan)

/* GFOpenBench's own reader (desktop/openbench.c root_picture), copied as it is, so the file the row writes is read
 * by the desktop's rule and not only the row's */
static int desktop_root(const unsigned char *buf, long len, char *out, int n)
{
    long i;
    for (i = 12; i + 8 + 24 <= len; ) {
        unsigned long id = ((unsigned long)buf[i] << 24) | ((unsigned long)buf[i + 1] << 16) | ((unsigned long)buf[i + 2] << 8) | buf[i + 3];
        unsigned long sz = ((unsigned long)buf[i + 4] << 24) | ((unsigned long)buf[i + 5] << 16) | ((unsigned long)buf[i + 6] << 8) | buf[i + 7];
        const unsigned char *d = buf + i + 8;
        if (id == 0x5054524EUL && sz >= 24) {
            unsigned which = (unsigned)((d[16] << 8) | d[17]), flags = (unsigned)((d[18] << 8) | d[19]);
            unsigned dlen = (unsigned)((d[22] << 8) | d[23]);
            if (which == 0 && !(flags & 1) && dlen > 1 && 24 + dlen <= sz && i + 8 + 24 + dlen <= len) {
                int k = (int)dlen < n - 1 ? (int)dlen : n - 1;
                memcpy(out, d + 24, (size_t)k);
                out[k] = 0;
                return out[0] != 0;
            }
        }
        i += 8 + (long)sz + (long)(sz & 1);
    }
    return 0;
}

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

void wbpattern_plan(void)
{
    /* as Prefs/WBPattern saves one: PRHD, the root's PTRN naming "Work:a.iff" (11 with its 0: odd, a pad byte),
     * the drawers' (which 1) naming "Work:b.iff" */
    static const unsigned char saved[] = {
        'F','O','R','M', 0,0,0,96, 'P','R','E','F',
        'P','R','H','D', 0,0,0,6, 0,0,0,0,0,0,
        'P','T','R','N', 0,0,0,35, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0, 0,0, 1,0, 0,11,
            'W','o','r','k',':','a','.','i','f','f',0, 0,
        'P','T','R','N', 0,0,0,35, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,1, 0,0, 1,0, 0,11,
            'W','o','r','k',':','b','.','i','f','f',0, 0 };
    unsigned char out[512], pic[512];
    char name[64], why[160];
    long n, pn;
    int v;
    CHECK(wbgap_ptrn_root(saved, sizeof saved, name, sizeof name) && !strcmp(name, "Work:a.iff"), "the saved file's root: %s", name);
    n = wbgap_ptrn_make(saved, sizeof saved, "RAM:WBGapP1.ilbm", out, sizeof out);
    /* PRHD 14, the drawers' PTRN 44 (kept), the new root's 8 + 24 + 17 + a pad */
    CHECK(n == 12 + 14 + 44 + 8 + 42, "the made file's length: %ld", n);
    CHECK(n > 8 && be32(out + 4) == (unsigned long)(n - 8), "FORM's length the rest: %lu", n > 8 ? be32(out + 4) : 0);
    CHECK(n > 26 && !memcmp(out + 12, saved + 12, 14), "PRHD as it was");
    CHECK(n > 70 && !memcmp(out + 26, saved + 26 + 44, 44), "the drawers' PTRN as it was, the old root's gone");
    CHECK(wbgap_ptrn_root(out, n, name, sizeof name) && !strcmp(name, "RAM:WBGapP1.ilbm"), "the row reads the new root: %s", name);
    name[0] = 0;
    CHECK(desktop_root(out, n, name, sizeof name) && !strcmp(name, "RAM:WBGapP1.ilbm"), "GFOpenBench's reader reads it too: %s", name);
    {   /* byte for byte: the new PTRN's head (as the 68k put32/put16 must write it) */
        static const unsigned char head[] = {'P','T','R','N', 0,0,0,41, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0, 0,0, 1,0, 0,17};
        CHECK(n == 120 && !memcmp(out + 70, head, sizeof head) && out[119] == 0, "the root's PTRN byte for byte, its pad");
    }
    n = wbgap_ptrn_make(NULL, 0, "RAM:x", out, sizeof out);
    CHECK(n == 12 + 14 + 8 + 30 && wbgap_ptrn_root(out, n, name, sizeof name) && !strcmp(name, "RAM:x"),
          "no file before: PRHD and the root made afresh (%ld)", n);
    CHECK(n > 26 && !memcmp(out + 12, "PRHD\0\0\0\6\0\0\0\0\0\0", 14), "the fresh PRHD");
    CHECK(wbgap_ptrn_make(saved, sizeof saved, "RAM:WBGapP1.ilbm", out, 100) == 0, "no room: 0, not a cut file");
    CHECK(wbgap_ptrn_make(saved, sizeof saved, "", out, sizeof out) == 0 && wbgap_ptrn_make(saved, sizeof saved, NULL, out, sizeof out) == 0,
          "no name: 0");
    {   /* a root that is a pattern (WBPF_PATTERN) names no picture; a chunk past the end stops the reading */
        unsigned char pat[sizeof saved];
        memcpy(pat, saved, sizeof saved);
        pat[26 + 8 + 19] = 1;
        CHECK(!wbgap_ptrn_root(pat, sizeof pat, name, sizeof name), "a pattern root: no picture");
        pat[26 + 8 + 19] = 0;
        pat[26 + 7] = 200;
        CHECK(!wbgap_ptrn_root(pat, sizeof pat, name, sizeof name), "a PTRN running past the file: none");
    }
    pn = wbgap_ilbm_solid(32, 24, 0xD0, 0x30, 0x90, pic, sizeof pic);
    CHECK(pn == 62 + 4 * 24, "the ILBM's length: %ld", pn);
    CHECK(pn > 62 && !memcmp(pic, "FORM", 4) && be32(pic + 4) == (unsigned long)(pn - 8) && !memcmp(pic + 8, "ILBMBMHD", 8)
          && be32(pic + 16) == 20, "FORM ILBM, BMHD of 20");
    CHECK(pn > 62 && pic[20] == 0 && pic[21] == 32 && pic[22] == 0 && pic[23] == 24 && pic[28] == 1 && pic[29] == 0 && pic[30] == 0,
          "32 x 24, one plane, no mask, not packed");
    CHECK(pn > 62 && !memcmp(pic + 40, "CMAP\0\0\0\6\xD0\x30\x90\xD0\x30\x90", 14), "CMAP: both colours the one asked");
    CHECK(pn > 62 && !memcmp(pic + 54, "BODY", 4) && be32(pic + 58) == 96 && pic[62] == 0 && pic[pn - 1] == 0, "BODY: all colour 0");
    CHECK(wbgap_ilbm_solid(17, 1, 1, 2, 3, pic, sizeof pic) == 62 + 4, "17 wide: rows of two words");
    CHECK(wbgap_ilbm_solid(0, 4, 1, 2, 3, pic, sizeof pic) == 0 && wbgap_ilbm_solid(32, 24, 1, 2, 3, pic, 100) == 0, "no size, no room: 0");
    CHECK(wbgap_rgb_near(0xD03090, WBGAP_PTRN_RGB, 0) && wbgap_rgb_near(0xF85068, WBGAP_PTRN_RGB, 40)
          && !wbgap_rgb_near(0xD03090 + 41, WBGAP_PTRN_RGB, 40) && !wbgap_rgb_near(0x303090, WBGAP_PTRN_RGB, 40), "near: each part within tol");
    v = wbgap_ptrn_judge(300, 0, 250, 3, 1, why, sizeof why);
    CHECK(v == WBGAP_PASS, "shown, then gone: PASS (%s)", why);
    v = wbgap_ptrn_judge(300, 0, 149, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "didn't show"), "under half: FAIL (%s)", why);
    v = wbgap_ptrn_judge(300, 0, 150, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_PASS, "half exactly: PASS (%s)", why);
    v = wbgap_ptrn_judge(300, 0, 250, 60, 1, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "stayed"), "stays with the old prefs: FAIL (%s)", why);
    v = wbgap_ptrn_judge(300, 0, 250, 0, 0, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "put back"), "the old file not back: FAIL (%s)", why);
    v = wbgap_ptrn_judge(19, 0, 19, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_SKIP, "too few points: SKIP (%s)", why);
    v = wbgap_ptrn_judge(300, 60, 300, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_SKIP, "the desktop that colour already: SKIP (%s)", why);
}
