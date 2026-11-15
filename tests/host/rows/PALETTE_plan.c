/* P3: Palette's colour 0 (tests/amiga/rows/PALETTE.c), its file maker and judge. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "PALETTE.h"

void palette_plan(void);
WBGAP_PLAN(palette_plan)

static unsigned be16(const unsigned char *p) { return (unsigned)(p[0] << 8 | p[1]); }

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

void palette_plan(void)
{
    static unsigned char saved[12 + 14 + 8 + WBGAP_PALT_LEN + 10], out[1024], out2[1024];
    unsigned char *d = saved + 34;
    unsigned long c = 0, cols[5] = {1, 2, 2, 3, 2};
    char why[220];
    long n, len = sizeof saved;
    int v;
    /* a saved file: PRHD, PALT (colours 0 = #112233 and 3 = #445566, then the end), and a chunk after it, "XTRA" */
    memcpy(saved, "FORM", 4), saved[4] = 0, saved[5] = 0, saved[6] = (unsigned char)((len - 8) >> 8), saved[7] = (unsigned char)(len - 8);
    memcpy(saved + 8, "PREFPRHD\0\0\0\6\0\0\0\0\0\0PALT\0\0\1\x90", 26);
    d[16] = 0, d[17] = 0, d[144] = 0, d[145] = 0, d[146] = d[147] = 0x11, d[148] = d[149] = 0x22, d[150] = d[151] = 0x33;
    d[152] = 0, d[153] = 3, d[154] = d[155] = 0x44, d[156] = d[157] = 0x55, d[158] = d[159] = 0x66;
    d[160] = 0xff, d[161] = 0xff;
    memcpy(saved + 34 + WBGAP_PALT_LEN, "XTRA\0\0\0\2ok", 10);
    CHECK(wbgap_palette_colour(saved, len, 0, &c) && c == 0x112233, "the saved file's colour 0: #%06lX", c);
    CHECK(wbgap_palette_colour(saved, len, 3, &c) && c == 0x445566, "its colour 3: #%06lX", c);
    CHECK(!wbgap_palette_colour(saved, len, 1, &c), "no colour 1 in it (the list ended)");
    n = wbgap_palette_make(saved, len, 0, WBGAP_PAL_RGB, out, sizeof out);
    CHECK(n == len, "changed in place, same length: %ld", n);
    CHECK(wbgap_palette_colour(out, n, 0, &c) && c == WBGAP_PAL_RGB, "colour 0 now #%06lX", c);
    CHECK(n > 182 && out[34 + 146] == 0xD0 && out[34 + 147] == 0xD0 && out[34 + 148] == 0x30 && out[34 + 150] == 0x90 && out[34 + 151] == 0x90,
          "each part 16-bit, the byte twice");
    CHECK(wbgap_palette_colour(out, n, 3, &c) && c == 0x445566, "colour 3 as it was");
    CHECK(n == len && !memcmp(out + 34 + WBGAP_PALT_LEN, "XTRA\0\0\0\2ok", 10) && !memcmp(out, saved, 34 + 144),
          "the chunk after, the header and the pens as they were");
    n = wbgap_palette_make(saved, len, 1, 0x010203, out2, sizeof out2);
    CHECK(n == len && wbgap_palette_colour(out2, n, 1, &c) && c == 0x010203 && wbgap_palette_colour(out2, n, 3, &c),
          "a colour not in the list: added, the list's end moved on");
    CHECK(n == len && out2[34 + 144 + 24] == 0xff && out2[34 + 144 + 25] == 0xff, "the end marker after it");
    n = wbgap_palette_make(NULL, 0, 0, WBGAP_PAL_RGB, out, sizeof out);
    CHECK(n == 12 + 14 + 8 + WBGAP_PALT_LEN && be32(out + 4) == (unsigned long)(n - 8) && !memcmp(out + 8, "PREFPRHD", 8)
          && !memcmp(out + 26, "PALT\0\0\1\x90", 8), "no file: FORM PREF, PRHD, a 400-byte PALT (%ld)", n);
    CHECK(wbgap_palette_colour(out, n, 0, &c) && c == WBGAP_PAL_RGB, "its colour 0 the one asked");
    CHECK(wbgap_palette_colour(out, n, 3, &c) && c == 0x6688BB && wbgap_palette_colour(out, n, 7, &c) && c == 0xEE9900,
          "the others Palette's defaults (3 #6688BB, 7 #EE9900)");
    CHECK(n > 60 && out[34 + 16] == 0 && out[34 + 17] == 0 && out[34 + 18] == 0 && out[34 + 19] == 1 && out[34 + 42] == 0xff,
          "the default pens, ending at ~0");
    {   /* the screen's own pens put in a file made from nothing: both lists, then ~0, the rest 0 */
        static const unsigned short p3[12] = {1, 2, 1, 2, 1, 3, 1, 0, 2, 1, 2, 1};
        int k, ok = wbgap_palette_pens(out, n, p3, 12);
        for (k = 0; ok && k < 12; k++)
            ok = be16(out + 34 + 16 + 2 * k) == p3[k] && be16(out + 34 + 80 + 2 * k) == p3[k];
        CHECK(ok && be16(out + 34 + 16 + 24) == 0xffff && be16(out + 34 + 80 + 24) == 0xffff && be16(out + 34 + 16 + 26) == 0,
              "the screen's pens in both lists, ended at ~0");
        CHECK(wbgap_palette_colour(out, n, 0, &c) && c == WBGAP_PAL_RGB, "the colours left as they were");
        CHECK(!wbgap_palette_pens(out, n, p3, 0) && !wbgap_palette_pens(out, n, p3, 32) && !wbgap_palette_pens(out, 20, p3, 12),
              "no pens, too many, or no PALT: 0");
    }
    CHECK(wbgap_palette_make(saved, len, 0, 1, out, 100) == 0 && wbgap_palette_make(NULL, 0, 0, 1, out, 100) == 0, "no room: 0");
    CHECK(wbgap_palette_make(saved, len, -1, 1, out, sizeof out) == 0, "no such colour: 0");
    {   /* the put back with no file: eight colours set one by one on a fresh file, as PALETTE.c builds it */
        static const unsigned long scr[8] = {0x101010, 0x202020, 0x303030, 0x404040, 0x505050, 0x606060, 0x707070, 0x808080};
        static unsigned char a[1024], b[1024];
        long m = 0;
        int k, all = 1;
        for (k = 0; k < 8; k++) {
            long r = wbgap_palette_make(k ? a : NULL, k ? m : 0, k, scr[k], b, sizeof b);
            if (!r) { all = 0; break; }
            memcpy(a, b, (size_t)r), m = r;
        }
        for (k = 0; all && k < 8; k++) all = wbgap_palette_colour(a, m, k, &c) && c == scr[k];
        CHECK(all && m == 12 + 14 + 8 + WBGAP_PALT_LEN, "the screen's own eight colours, each read back (%ld)", m);
    }
    CHECK(wbgap_palette_mode(cols, 5, &c) == 3 && c == 2, "the most common colour: 2, three times");
    v = wbgap_palette_judge(150, 150, 0, 140, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_PASS, "followed, then gave back: PASS (%s)", why);
    v = wbgap_palette_judge(150, 150, 0, 0, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "didn't follow"), "not followed: FAIL (%s)", why);
    v = wbgap_palette_judge(150, 150, 0, 140, 40, 1, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "kept"), "kept with the old prefs: FAIL (%s)", why);
    v = wbgap_palette_judge(150, 150, 0, 140, 0, 0, why, sizeof why);
    CHECK(v == WBGAP_FAIL && strstr(why, "put back"), "old file not back: FAIL (%s)", why);
    v = wbgap_palette_judge(150, 120, 0, 0, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_SKIP && strstr(why, "pattern"), "not one colour before (a pattern): SKIP (%s)", why);
    v = wbgap_palette_judge(150, 150, 40, 150, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_SKIP, "the prefs' colour there already: SKIP (%s)", why);
    v = wbgap_palette_judge(10, 10, 0, 10, 0, 1, why, sizeof why);
    CHECK(v == WBGAP_SKIP, "too few points: SKIP (%s)", why);
}
