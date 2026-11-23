/* I16: keyboard selection (tests/amiga/rows/KEYSEL.c), its steps checked against a model of the design's rule.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "KEYSEL.h"
#include "SELECT.h"

void keysel_plan(void);
WBGAP_PLAN(keysel_plan)

/* three icons in a row: a click selects one alone and leads from it; Right and Left step the lead (no wrap), the new
 * one selected alone or (Shift) added; Tab and Shift-Tab step it round (wrapping), selected alone */
static void model(int *lead, int sel[3], const struct wbgap_keysel_step *s)
{
    int i;
    if (s->click) { *lead = s->click - 'a'; for (i = 0; i < 3; i++) sel[i] = i == *lead; return; }
    if (s->key == WBGAP_RAW_RIGHT || s->key == WBGAP_RAW_LEFT) {
        int to = *lead + (s->key == WBGAP_RAW_RIGHT ? 1 : -1);
        if (to < 0 || to > 2) return;
        *lead = to;
        if (!s->shift) for (i = 0; i < 3; i++) sel[i] = 0;
        sel[to] = 1;
        return;
    }
    if (s->key == WBGAP_RAW_TAB) {
        *lead = (*lead + (s->shift ? 2 : 1)) % 3;
        for (i = 0; i < 3; i++) sel[i] = i == *lead;
    }
}

void keysel_plan(void)
{
    int n = wbgap_keysel_count(), i, lead = -1, sel[3] = {0, 0, 0}, right = 0, left = 0, tab = 0, back = 0, added = 0;
    char why[300], list[16];
    CHECK(n >= 6, "%d steps", n);
    CHECK(n > 0 && wbgap_keysel_step(0)->click, "the first step is a click: the keys need an icon to go from");
    CHECK(!wbgap_keysel_step(-1) && !wbgap_keysel_step(n), "no step outside the list");
    for (i = 0; i < n; i++) {
        const struct wbgap_keysel_step *s = wbgap_keysel_step(i);
        int k, at = 0;
        CHECK(s && s->what && s->what[0] && s->want, "step %d says what it is and what it wants", i);
        if (!s) continue;
        if (s->click) CHECK(s->click >= 'a' && s->click <= 'c' && !s->shift, "step %d clicks a, b or c, no Shift", i);
        else CHECK(s->key == WBGAP_RAW_RIGHT || s->key == WBGAP_RAW_LEFT || s->key == WBGAP_RAW_TAB,
                   "step %d: a cursor key or Tab (0x%x)", i, s->key);
        right += !s->click && s->key == WBGAP_RAW_RIGHT, left += !s->click && s->key == WBGAP_RAW_LEFT;
        tab += !s->click && s->key == WBGAP_RAW_TAB && !s->shift, back += !s->click && s->key == WBGAP_RAW_TAB && s->shift;
        added += !s->click && s->key != WBGAP_RAW_TAB && s->shift;
        model(&lead, sel, s);
        for (k = 0; k < 3; k++) if (sel[k]) { if (at) list[at++] = ' '; list[at++] = (char)('a' + k); }
        list[at] = 0;
        CHECK(!strcmp(list, s->want), "step %d (%s): the rule leaves \"%s\", the step wants \"%s\"", i, s->what, list, s->want);
        /* the judge passes the model's selection, a name a line as the ARexx reader writes it */
        {
            char lines[16];
            int j, m = 0;
            for (j = 0; list[j]; j++) lines[m++] = list[j] == ' ' ? '\n' : list[j];
            lines[m++] = '\n', lines[m] = 0;
            CHECK(wbgap_select_judge(at ? lines : "", s->want, s->what, why, sizeof why) == WBGAP_PASS,
                  "step %d: the judge takes the right selection (%s)", i, why);
        }
    }
    CHECK(right && left && tab && back && added, "the steps cover Right, Left, Tab, Shift-Tab and a Shift-added one");
    CHECK(wbgap_select_judge("a\n", "b", "Cursor Right", why, sizeof why) == WBGAP_FAIL && strstr(why, "Cursor Right"),
          "a key that didn't move the selection: FAIL, naming the step (%s)", why);
    CHECK(wbgap_select_judge(NULL, "b", "Tab", why, sizeof why) == WBGAP_SKIP, "the selection not read: SKIP (%s)", why);
}
