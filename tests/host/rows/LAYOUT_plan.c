/* L1: the default layout's drawer contents (tests/amiga/rows/LAYOUT_judge.c), as WBGapLayout.rexx expects them.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "LAYOUT.h"

void layout_plan(void);
WBGAP_PLAN(layout_plan)

void layout_plan(void)
{
    static const char *const want[WBGAP_LAYOUT_N] = {"ztool", "mproj", "ydrawer", "atool", "cdrawer", "bproj"};
    int i, j, t = 0, p = 0, d = 0, same = 0, sorted = 1, kinds = 1;
    for (i = 0; i < WBGAP_LAYOUT_N; i++) {
        const struct wbgap_layout_icon *x = &wbgap_layout_icons[i];
        if (strcmp(x->name, want[i])) same = -1;
        if (x->kind == 't') t++;
        else if (x->kind == 'p') p++;
        else if (x->kind == 'd') d++;
        else kinds = 0;
        for (j = 0; j < i; j++) if (!strcmp(x->name, wbgap_layout_icons[j].name)) same = -2;
        if (i && strcmp(wbgap_layout_icons[i - 1].name, x->name) > 0) sorted = 0;
    }
    CHECK(same == 0, "the six names WBGapLayout.rexx looks for, in the order made (%d)", same);
    CHECK(kinds && t == 2 && p == 2 && d == 2, "two tools, two projects, two drawers (%d %d %d)", t, p, d);
    CHECK(!sorted, "made out of name order, so a layout by name shows");
    CHECK(wbgap_layout_icons[0].kind == 't' && wbgap_layout_icons[2].kind == 'd',
          "ztool a tool and ydrawer a drawer, as their names say");
}
