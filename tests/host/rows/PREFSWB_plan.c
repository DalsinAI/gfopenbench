/* P7: Workbench prefs (tests/amiga/rows/PREFSWB.c), its file maker and judge. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "PREFSWB.h"

void prefswb_plan(void);
WBGAP_PLAN(prefswb_plan)

void prefswb_plan(void)
{
    /* a file as Prefs/Workbench writes it: PRHD, a 50-byte WBNC with stack 0x2000, WBHD "DF0", WBTF "x" (odd: a pad byte) */
    static const unsigned char saved[] = {
        'F','O','R','M', 0,0,0,98, 'P','R','E','F',
        'P','R','H','D', 0,0,0,6, 0,0,0,0,0,0,
        'W','B','N','C', 0,0,0,50, 0,0,0x20,0, 0,0,0,5, 0,0,0,16, 0xff,0xff,0xff,0xff, 0,1,0,1, 0,1,0,0, 0,25,0,1,
            0,1, 0,0,0,0, 0,1, 0,1, 0,0, 0,50, 0,0,0,0, 0,0,0,0,
        'W','B','H','D', 0,0,0,4, 'D','F','0',0,
        'W','B','T','F', 0,0,0,1, 'x',0 };
    unsigned char out[512];
    char why[160];
    long n;
    CHECK(wbgap_wbprefs_stack(saved, sizeof saved) == 0x2000, "the saved file's stack: %lu", wbgap_wbprefs_stack(saved, sizeof saved));
    CHECK(wbgap_wbprefs_hides(saved, sizeof saved, "DF0") && wbgap_wbprefs_hides(saved, sizeof saved, "df0:"),
          "DF0 hidden, case and ':' aside");
    CHECK(!wbgap_wbprefs_hides(saved, sizeof saved, "RAM"), "RAM not hidden in it");
    n = wbgap_wbprefs_make(saved, sizeof saved, 65536, "RAM", out, sizeof out);
    CHECK(n == (long)sizeof saved + 12, "RAM's WBHD added (12 bytes): %ld", n);
    CHECK(n > 8 && out[4] == 0 && out[5] == 0 && out[6] == 0 && out[7] == (unsigned char)(n - 8), "FORM's length the rest: %d", out[7]);
    CHECK(wbgap_wbprefs_stack(out, n) == 65536, "the stack set: %lu", wbgap_wbprefs_stack(out, n));
    CHECK(wbgap_wbprefs_hides(out, n, "RAM") && wbgap_wbprefs_hides(out, n, "DF0"), "RAM hidden, DF0 kept");
    CHECK(n > 0 && !memcmp(out + 12, saved + 12, 14) && !memcmp(out + 26 + 12, saved + 26 + 12, 46),
          "PRHD and the rest of WBNC as they were");
    CHECK(!memcmp(out + sizeof saved - 10, saved + sizeof saved - 10, 10), "WBTF kept, its pad byte too");
    CHECK(wbgap_wbprefs_make(saved, sizeof saved, 65536, "dF0:", out, sizeof out) == (long)sizeof saved,
          "a device already hidden isn't added again");
    n = wbgap_wbprefs_make(NULL, 0, 65536, "RAM", out, sizeof out);
    CHECK(n == 12 + 14 + 58 + 12 && wbgap_wbprefs_stack(out, n) == 65536 && wbgap_wbprefs_hides(out, n, "RAM"),
          "no file before: PRHD, WBNC and WBHD made: %ld", n);
    n = wbgap_wbprefs_make((const unsigned char *)"junk", 4, 4096, NULL, out, sizeof out);
    CHECK(n == 12 + 14 + 58 && wbgap_wbprefs_stack(out, n) == 4096 && !wbgap_wbprefs_hides(out, n, "RAM"),
          "not a FORM PREF: made afresh, nothing hidden: %ld", n);
    CHECK(wbgap_wbprefs_make(saved, sizeof saved, 65536, "RAM", out, 40) == 0, "no room: 0");
    {   /* a chunk that says it runs past the file: stopped there, not read past */
        unsigned char bad[24];
        memcpy(bad, saved, 20);
        bad[20] = 0, bad[21] = 0, bad[22] = 0x10, bad[23] = 0;
        CHECK(wbgap_wbprefs_stack(bad, sizeof bad) == 0 && !wbgap_wbprefs_hides(bad, sizeof bad, "RAM"), "a chunk too long: nothing read");
    }
    CHECK(wbgap_wbprefs_whole(106, 4096) && wbgap_wbprefs_whole(4094, 4096), "read whole: 106 and 4094 of 4096");
    CHECK(!wbgap_wbprefs_whole(4095, 4096), "4095 of 4096, all wbt_read_all reads: maybe cut short, not written back");
    CHECK(!wbgap_wbprefs_whole(-1, 4096), "nothing read: not whole");
    {   /* a WBNC bigger than 3.2.3's 50 bytes: its stack set in place, no second WBNC */
        unsigned char big[12 + 8 + 100], o2[200];
        long m;
        memset(big, 0, sizeof big);
        memcpy(big, "FORM", 4), big[7] = (unsigned char)(sizeof big - 8), memcpy(big + 8, "PREF", 4);
        memcpy(big + 12, "WBNC", 4), big[19] = 100, big[22] = 0x10, big[119] = 0x5a;
        m = wbgap_wbprefs_make(big, sizeof big, 65536, NULL, o2, sizeof o2);
        CHECK(m == (long)sizeof big && wbgap_wbprefs_stack(o2, m) == 65536 && o2[119] == 0x5a,
              "a 100-byte WBNC: kept whole, stack set, no second WBNC: %ld", m);
    }
    CHECK(wbgap_wbprefs_judge(1, 65536, 65536, 1, why, sizeof why) == WBGAP_PASS && !why[0], "hidden, stack, back: PASS");
    CHECK(wbgap_wbprefs_judge(-1, 0, 65536, -1, why, sizeof why) == WBGAP_SKIP && strstr(why, "not read"), "icons not read: SKIP");
    CHECK(wbgap_wbprefs_judge(0, 65536, 65536, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "still on the desktop"),
          "icon stayed: FAIL, \"%s\"", why);
    CHECK(wbgap_wbprefs_judge(1, 4096, 65536, 1, why, sizeof why) == WBGAP_FAIL && strstr(why, "4096") && strstr(why, "65536"),
          "stack not taken: FAIL, \"%s\"", why);
    CHECK(wbgap_wbprefs_judge(1, 65536, 65536, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't come back"),
          "not back: FAIL, \"%s\"", why);
    CHECK(wbgap_wbprefs_judge(1, 65536, 65536, -1, NULL, 0) == WBGAP_PASS, "back not read: still PASS, no room needed");
}
