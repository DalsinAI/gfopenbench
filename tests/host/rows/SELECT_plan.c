/* I1, I2: click and Shift-click select (tests/amiga/rows/SELECT_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "SELECT.h"

void select_plan(void);
WBGAP_PLAN(select_plan)

static int mode_is(const char *id, const char *mode)
{
    int i = wbgap_find(id);
    return i >= 0 && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, mode);
}

void select_plan(void)
{
    char why[300];
    CHECK(wbgap_select_judge("a\n", "a", "a click on a", why, sizeof why) == WBGAP_PASS && !why[0], "a alone: PASS");
    CHECK(wbgap_select_judge("a", "a", "x", why, sizeof why) == WBGAP_PASS, "no final newline: PASS");
    CHECK(wbgap_select_judge("a\r\n", "a", "x", why, sizeof why) == WBGAP_PASS, "CR LF: PASS");
    CHECK(wbgap_select_judge("", "", "x", why, sizeof why) == WBGAP_PASS, "nothing, and nothing wanted: PASS");
    CHECK(wbgap_select_judge("\n\r\n", NULL, "x", why, sizeof why) == WBGAP_PASS, "blank lines are nothing: PASS");
    CHECK(wbgap_select_judge("b\na\n", "a b", "x", why, sizeof why) == WBGAP_PASS, "both, in the other order: PASS");
    CHECK(wbgap_select_judge("a\n", "", "a click on empty space", why, sizeof why) == WBGAP_FAIL &&
              strstr(why, "after a click on empty space: a selected, not nothing"),
          "a still selected after a click elsewhere: FAIL, \"%s\"", why);
    CHECK(wbgap_select_judge("", "a", "a click on a", why, sizeof why) == WBGAP_FAIL && strstr(why, "nothing selected, not a"),
          "nothing selected: FAIL, \"%s\"", why);
    CHECK(wbgap_select_judge("a\nb\n", "b", "a click on b", why, sizeof why) == WBGAP_FAIL && strstr(why, "a b selected, not b"),
          "a kept by a plain click on b: FAIL, \"%s\"", why);
    CHECK(wbgap_select_judge("b\n", "a b", "a Shift-click on b", why, sizeof why) == WBGAP_FAIL && strstr(why, "b selected, not a b"),
          "Shift-click replaced the selection: FAIL, \"%s\"", why);
    CHECK(wbgap_select_judge("a\nc\n", "a b", "x", why, sizeof why) == WBGAP_FAIL, "same count, another name: FAIL");
    CHECK(wbgap_select_judge("A\n", "a", "x", why, sizeof why) == WBGAP_FAIL, "a name's case changed: FAIL");
    CHECK(wbgap_select_judge("ab\n", "a", "x", why, sizeof why) == WBGAP_FAIL, "a longer name: FAIL");
    CHECK(wbgap_select_judge(NULL, "a", "a click on a", why, sizeof why) == WBGAP_SKIP && strstr(why, "weren't read after a click on a"),
          "not read: SKIP, \"%s\"", why);
    CHECK(wbgap_select_judge("a\n", "a", NULL, NULL, 0) == WBGAP_PASS, "no step, no room for a reason: still judged");
    CHECK(mode_is("I1", "SELECT") && mode_is("I2", "SELECT"), "I1's and I2's lines come from SELECT");
}
