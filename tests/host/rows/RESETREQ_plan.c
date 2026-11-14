/* C5a: the Workbench reset requester (tests/amiga/rows/RESETREQ_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "RESETREQ.h"

void resetreq_plan(void);
WBGAP_PLAN(resetreq_plan)

void resetreq_plan(void)
{
    char why[200];
    struct wbgap_resetreq ok = { 0, 1, 1, 24, 16, 16, 1, 1, 1, 16 }, r;
    /* FORM <36> PREF, PRHD <6>, SCRM <28>: depth 24 at the SCRM data's +24 */
    unsigned char f[12 + 14 + 36];
    int old = 0;
    memset(f, 0, sizeof f);
    memcpy(f, "FORM", 4); f[7] = sizeof f - 8; memcpy(f + 8, "PREF", 4);
    memcpy(f + 12, "PRHD", 4); f[19] = 6;
    memcpy(f + 26, "SCRM", 4); f[33] = 28; f[34 + 25] = 24;
    CHECK(wbgap_resetreq_judge(&ok, why, sizeof why) == WBGAP_PASS && !why[0], "shown, gone, reset, Cancel kept it: PASS");
    r = ok; r.cancel_tried = 0; r.cancel_shown = r.cancel_gone = 0; r.depth_cancel = 0;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_PASS, "without the Cancel half: PASS");
    r = ok; r.no_prefs = 1;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_SKIP && strstr(why, "ScreenMode prefs"), "no prefs: SKIP, \"%s\"", why);
    r = ok; r.shown = 0;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "no requester"), "none came up: FAIL, \"%s\"", why);
    r = ok; r.gone = 0;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "stayed"), "it stayed: FAIL, \"%s\"", why);
    r = ok; r.depth_after = 24;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "depth 24, asked for 16"),
          "not reset: FAIL, \"%s\"", why);
    r = ok; r.cancel_gone = 0;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "Cancel didn't"), "Cancel left it up: FAIL");
    r = ok; r.depth_cancel = 24;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "changed anyway"), "reset after Cancel: FAIL");
    r = ok; r.cancel_shown = 0;
    CHECK(wbgap_resetreq_judge(&r, why, sizeof why) == WBGAP_FAIL && strstr(why, "Cancel half"), "no requester for Cancel: FAIL");
    CHECK(wbgap_resetreq_judge(NULL, why, sizeof why) == WBGAP_SKIP, "nothing read: SKIP");
    CHECK(wbgap_smod_depth(f, sizeof f, 16, &old) == 1 && old == 24 && f[34 + 24] == 0 && f[34 + 25] == 16,
          "the SCRM depth found past PRHD, 24 put to 16");
    CHECK(wbgap_smod_depth(f, sizeof f - 4, 8, &old) == 0, "a cut SCRM: not touched");
    memcpy(f + 26, "XXXX", 4);
    CHECK(wbgap_smod_depth(f, sizeof f, 8, &old) == 0, "no SCRM: 0");
    CHECK(wbgap_other_depth(24) == 16 && wbgap_other_depth(32) == 16 && wbgap_other_depth(16) == 24
          && wbgap_other_depth(8) == 7 && wbgap_other_depth(1) == 2, "another depth for the change");
    CHECK(wbgap_find("C5a") >= 0 && wbgap_row(wbgap_find("C5a"))->mode && !strcmp(wbgap_row(wbgap_find("C5a"))->mode, "RESETREQ"),
          "C5a's line comes from RESETREQ");
}
