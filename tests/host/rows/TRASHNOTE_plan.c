/* I6t, I6p: GFOS's own trash, its note and putting back (tests/amiga/rows/TRASHNOTE_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "TRASHNOTE.h"

void trashnote_plan(void);
WBGAP_PLAN(trashnote_plan)

void trashnote_plan(void)
{
    char why[200];
    const char *both = "old\tWork:x/old\nf\tRAM:WBGapR/f\nf.info\tRAM:WBGapR/f.info\n";
    const char *named = "f\tRam Disk:WBGapR/f\nf.info\tRam Disk:WBGapR/f.info";   /* the volume's name, no last NL */
    int t = wbgap_find("I6t"), p = wbgap_find("I6p");

    /* the note's lines */
    CHECK(wbgap_trashnote_line(both, "f", "WBGapR/f"), "a line for f from RAM:WBGapR/f");
    CHECK(wbgap_trashnote_line(named, "f.info", "WBGapR/f.info"), "the volume's name and no last newline: found");
    CHECK(wbgap_trashnote_line("f\tram:wbgapr/F\n", "f", "WBGapR/f"), "the path's case not minded");
    CHECK(!wbgap_trashnote_line("F\tRAM:WBGapR/f\n", "f", "WBGapR/f"), "the name's case minded (the engine keeps it)");
    CHECK(!wbgap_trashnote_line("f\tRAM:XWBGapR/f\n", "f", "WBGapR/f"), "a path that only ends alike: not ours");
    CHECK(!wbgap_trashnote_line("f (2)\tRAM:WBGapR/f\n", "f", "WBGapR/f"), "another name for it (the trash had an f): not f's");
    CHECK(!wbgap_trashnote_line("f\tRAM:Other/f\n", "f", "WBGapR/f"), "from elsewhere: no");
    CHECK(!wbgap_trashnote_line("f.info\tRAM:WBGapR/f.info\n", "f", "WBGapR/f"), "the icon's line is not the file's");
    CHECK(!wbgap_trashnote_line(NULL, "f", "WBGapR/f") && !wbgap_trashnote_line(both, "", "WBGapR/f") &&
          !wbgap_trashnote_line(both, "f", ""), "no note, no name or no path: no");

    /* I6t */
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, both, "f", "WBGapR/f", why, sizeof why) == WBGAP_PASS && !why[0],
          "moved, both lines: PASS");
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, NULL, "f", "WBGapR/f", why, sizeof why) == WBGAP_FAIL && strstr(why, "no note"),
          "moved, no note: FAIL, \"%s\"", why);
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, "f.info\tRAM:WBGapR/f.info\n", "f", "WBGapR/f", why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "no line for f "), "the file's line missing: FAIL, \"%s\"", why);
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, "f\tRAM:WBGapR/f\n", "f", "WBGapR/f", why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "its icon"), "the icon's line missing: FAIL, \"%s\"", why);
    CHECK(wbgap_trashnote_judge(1, 1, 0, 0, both, "f", "WBGapR/f", why, sizeof why) == WBGAP_FAIL, "nothing moved: FAIL");
    CHECK(wbgap_trashnote_judge(0, 0, 0, 0, both, "f", "WBGapR/f", why, sizeof why) == WBGAP_FAIL,
          "gone altogether (deleted for good): FAIL");
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, both, NULL, NULL, NULL, 0) == WBGAP_PASS,
          "no name or path given: f, from any drawer's f");
    CHECK(wbgap_trashnote_judge(0, 0, 1, 1, both, "f", "WBGapR/f", NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");

    /* I6p */
    CHECK(wbgap_putback_judge(0, 1, 1, 0, 0, "f", why, sizeof why) == WBGAP_PASS && !why[0], "put back: PASS");
    CHECK(wbgap_putback_judge(WBGAP_PUTBACK_NOLIB, 0, 0, 1, 1, "f", why, sizeof why) == WBGAP_SKIP && strstr(why, "library"),
          "no library: SKIP, \"%s\"", why);
    CHECK(wbgap_putback_judge(WBGAP_PUTBACK_NOTRIED, 0, 0, 0, 0, "f", why, sizeof why) == WBGAP_SKIP && strstr(why, "I6t"),
          "I6t failed: SKIP, \"%s\"", why);
    CHECK(wbgap_putback_judge(WBGAP_PUTBACK_NOCTX, 0, 0, 1, 1, "f", why, sizeof why) == WBGAP_FAIL &&
          strstr(why, "OFNewContext") && !strstr(why, "answered"), "no context: FAIL of its own, \"%s\"", why);
    CHECK(wbgap_putback_judge(1, 1, 1, 0, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "answered 1"),
          "OFRestore's 1 (TRUE) is not OFERR_OK 0: FAIL, \"%s\"", why);
    CHECK(wbgap_putback_judge(205, 0, 0, 1, 1, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "205"),
          "OFRestore refused: FAIL, \"%s\"", why);
    CHECK(wbgap_putback_judge(0, 1, 0, 0, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "icon"),
          "the icon not back: FAIL, \"%s\"", why);
    CHECK(wbgap_putback_judge(0, 0, 1, 0, 0, "f", why, sizeof why) == WBGAP_FAIL, "the file not back: FAIL");
    CHECK(wbgap_putback_judge(0, 1, 1, 1, 0, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "copied"),
          "still in the trash: FAIL, \"%s\"", why);
    CHECK(wbgap_putback_judge(0, 1, 1, 0, 1, "f", why, sizeof why) == WBGAP_FAIL && strstr(why, "note"),
          "its line left in the note: FAIL, \"%s\"", why);

    CHECK(t >= 0 && wbgap_row(t)->mode && !strcmp(wbgap_row(t)->mode, "TRASHNOTE") && wbgap_row(t)->parent &&
          !strcmp(wbgap_row(t)->parent, "I6"), "I6t's line comes from TRASHNOTE, under I6");
    CHECK(p >= 0 && wbgap_row(p)->mode && !strcmp(wbgap_row(p)->mode, "TRASHNOTE"), "I6p's line comes from TRASHNOTE");
}
