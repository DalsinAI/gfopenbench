/* INFOSAVE: the Information window's Save (tests/amiga/rows/INFOSAVE.c), its judges.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "INFOSAVE.h"

void infosave_plan(void);
WBGAP_PLAN(infosave_plan)

void infosave_plan(void)
{
    static const char *const two[] = {"a=3", "b=2", NULL}, *const one[] = {"a=3", NULL};
    static const char *const old[] = {"a=1", "d=4", NULL};
    char why[120], got[16];
    CHECK(wbgap_infosave_prot(WBGAP_FIBF_DELETE, why, sizeof why) == WBGAP_PASS && !why[0], "N2: only Deletable off: PASS");
    CHECK(wbgap_infosave_prot(0, why, sizeof why) == WBGAP_FAIL && strstr(why, "0x0"), "N2: still deletable: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_prot(WBGAP_FIBF_DELETE | WBGAP_FIBF_WRITE, why, sizeof why) == WBGAP_FAIL && strstr(why, "0x5"),
          "N2: Write lost too: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_prot(WBGAP_FIBF_DELETE | 0x40, why, sizeof why) == WBGAP_PASS, "N2: the other bits (S, P, A) don't count");
    CHECK(wbgap_infosave_same("comment", "gapnote", "gapnote", why, sizeof why) == WBGAP_PASS && !why[0], "N3: the comment: PASS");
    CHECK(wbgap_infosave_same("comment", "", "gapnote", why, sizeof why) == WBGAP_FAIL && strstr(why, "comment \"\""),
          "N3: no comment: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_same("default tool", NULL, "dir", why, sizeof why) == WBGAP_FAIL && strstr(why, "(none)"),
          "N5: no default tool: FAIL, \"%s\"", why);
    wbgap_join_tooltypes(two, got, sizeof got);
    CHECK(!strcmp(got, "a=3,b=2"), "N4: two tool types joined: \"%s\"", got);
    CHECK(wbgap_infosave_same("tool types", got, "a=3,b=2", why, sizeof why) == WBGAP_PASS, "N4: edited, deleted, added: PASS");
    wbgap_join_tooltypes(one, got, sizeof got);
    CHECK(wbgap_infosave_same("tool types", got, "a=3,b=2", why, sizeof why) == WBGAP_FAIL, "N4: b=2 not added: FAIL");
    wbgap_join_tooltypes(old, got, sizeof got);
    CHECK(!strcmp(got, "a=1,d=4") && wbgap_infosave_same("tool types", got, "a=3,b=2", NULL, 0) == WBGAP_FAIL,
          "N4: nothing saved: FAIL, no room for a reason");
    wbgap_join_tooltypes(NULL, got, sizeof got);
    CHECK(!got[0], "N4: no tool types: empty");
    wbgap_join_tooltypes(two, got, 5);
    CHECK(!strcmp(got, "a=3,"), "N4: cut to the room there is: \"%s\"", got);
    /* N6: the stack field saved into the icon */
    CHECK(wbgap_infosave_stack(8192, 8192, why, sizeof why) == WBGAP_PASS && !why[0], "N6: stack 8192: PASS");
    CHECK(wbgap_infosave_stack(4096, 8192, why, sizeof why) == WBGAP_FAIL && strstr(why, "4096") && strstr(why, "8192"),
          "N6: the old stack kept: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_stack(0, 8192, NULL, 0) == WBGAP_FAIL, "N6: no stack: FAIL, no room for a reason");
    /* N7: Hold (0x80) and Pure (0x20) set, nothing else moved */
    CHECK(wbgap_infosave_hp(0, 0xa0, why, sizeof why) == WBGAP_PASS && !why[0], "N7: both set from none: PASS");
    CHECK(wbgap_infosave_hp(0x50, 0xf0, why, sizeof why) == WBGAP_PASS, "N7: Script and Archive kept: PASS");
    CHECK(wbgap_infosave_hp(0, 0x80, why, sizeof why) == WBGAP_FAIL && strstr(why, "Pure not set"), "N7: Hold only: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_hp(0, 0x20, why, sizeof why) == WBGAP_FAIL && strstr(why, "Hold not set"), "N7: Pure only: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_hp(0, 0, why, sizeof why) == WBGAP_FAIL && strstr(why, "neither"), "N7: nothing saved: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_hp(0, 0xa1, why, sizeof why) == WBGAP_FAIL && strstr(why, "other bits"),
          "N7: Deletable lost on the way: FAIL, \"%s\"", why);
    CHECK(wbgap_infosave_hp(0x10, 0xa0, why, sizeof why) == WBGAP_FAIL, "N7: Archive dropped: FAIL");
    CHECK(wbgap_infosave_hp(0xa0, 0xa0, NULL, 0) == WBGAP_PASS, "N7: already set and kept: PASS, no room needed");
}
