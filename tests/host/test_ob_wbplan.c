/* ob_wbplan's host test: GFOpenBench as workbench.library's Workbench (0.1.56, gap item 4, rows A4, A8a, A8b): a
 * path into its drawer and name (MakeWorkbenchObjectVisibleA, AppMessages' WBArgs), a ChangeWorkbenchSelectionA
 * hook's answer, and where programs' AppMenuItems go (Tools, sub items, their own menus).
 *   test_ob_wbplan
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_wbplan.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static int split_is(const char *path, const char *dir, const char *name)
{
    char d[64];
    const char *n = ob_wbp_split(path, d, sizeof d);
    return !strcmp(d, dir) && !strcmp(n, name);
}

/* the rows as text: "T:name" title, "I:name" item, "S:name" sub item, space between */
static const char *rows_text(const ob_wbp_row *r, int n, const char *const *names)
{
    static char t[512];
    int i;
    t[0] = 0;
    for (i = 0; i < n; i++) {
        size_t k = strlen(t);
        snprintf(t + k, sizeof t - k, "%s%c:%s", i ? " " : "", "TIS"[r[i].level], names[r[i].index]);
    }
    return t;
}

int main(void)
{
    char small[5];
    const char *n;
    int own, k;
    ob_wbp_row rows[32];

    /* ---- paths ---- */
    CHECK(split_is("Work:Pictures/x", "Work:Pictures", "x"), "a file in a drawer");
    CHECK(split_is("RAM:WBGap/a", "RAM:WBGap", "a"), "WBGapTest's A8a");
    CHECK(split_is("RAM:WBGap", "RAM:", "WBGap"), "a drawer at the root");
    CHECK(split_is("Work:", "Work:", ""), "a disk: its root and no name");
    CHECK(split_is("Work:a/b/c", "Work:a/b", "c"), "deeper");
    CHECK(split_is("x", "", "x"), "a name alone");
    CHECK(split_is("", "", ""), "nothing");
    n = ob_wbp_split("Work:Pictures/x", small, sizeof small);
    CHECK(!strcmp(small, "Work") && !strcmp(n, "x"), "the drawer cut to the room given: '%s'", small);

    /* ---- the hook's answers ---- */
    CHECK(ob_wbp_hook_answer(OB_WBP_UNSELECT) == 0, "Unselect");
    CHECK(ob_wbp_hook_answer(OB_WBP_SELECT) == 1, "Select");
    CHECK(ob_wbp_hook_answer(OB_WBP_IGNORE) == 2, "Ignore");
    CHECK(ob_wbp_hook_answer(OB_WBP_STOP) == -1, "Stop");
    CHECK(ob_wbp_hook_answer(99) == 2 && ob_wbp_hook_answer(-1) == 2, "anything else: left as it is");

    /* ---- where Tools goes ---- */
    {
        const char *mine[] = { "GFOpenBench", "Window", "Icons", "Disks", "Tools" };
        const char *no_tools[] = { "GFOpenBench", "Window", "Icons", "Disks" };
        const char *no_icons[] = { "GFOpenBench", "Window" };
        CHECK(ob_wbp_tools_at(mine, 5, 1, &own) == 4 && own, "GFOpenBench's own Tools: at its end");
        CHECK(ob_wbp_tools_at(mine, 5, 0, &own) == 4 && own, "its own Tools, with no items too");
        CHECK(ob_wbp_tools_at(no_tools, 4, 1, &own) == 3 && !own, "no Tools of its own: one made after Icons");
        CHECK(ob_wbp_tools_at(no_icons, 2, 1, &own) == 2 && !own, "no Icons either: at the end");
        CHECK(ob_wbp_tools_at(no_tools, 4, 0, &own) == -1 && !own, "no Tools and no items: none made");
    }

    /* ---- the programs' AppMenuItems ---- */
    {
        /* handles are the entries' own addresses */
        static ob_wbp_menu m[9];
        const char *names[9] = { "Clock", "Convert", "to IFF", "to PNG", "MyApp", "Do it", "hidden", "Late", "gone" };
        m[0] = (ob_wbp_menu){ &m[0], NULL, 0, 1 };      /* Tools > Clock */
        m[1] = (ob_wbp_menu){ &m[1], NULL, 1, 1 };      /* Tools > Convert, a parent item */
        m[2] = (ob_wbp_menu){ &m[2], &m[1], 0, 1 };     /*   > to IFF */
        m[3] = (ob_wbp_menu){ &m[3], &m[1], 0, 1 };     /*   > to PNG */
        m[4] = (ob_wbp_menu){ &m[4], NULL, 2, 1 };      /* the program's own menu MyApp */
        m[5] = (ob_wbp_menu){ &m[5], &m[4], 0, 1 };     /*   MyApp > Do it */
        m[6] = (ob_wbp_menu){ &m[6], NULL, 0, 0 };      /* not shown (a parent with no sub items yet) */
        m[7] = (ob_wbp_menu){ &m[7], NULL, 0, 1 };      /* added last: last in Tools */
        m[8] = (ob_wbp_menu){ &m[8], &m[4], 0, 0 };     /* MyApp's, not shown */
        k = ob_wbp_items(m, 9, NULL, rows, 32);
        CHECK(!strcmp(rows_text(rows, k, names), "I:Clock I:Convert S:to IFF S:to PNG I:Late"), "Tools: %s",
              rows_text(rows, k, names));
        k = ob_wbp_own(m, 9, rows, 32);
        CHECK(!strcmp(rows_text(rows, k, names), "T:MyApp I:Do it"), "own menus: %s", rows_text(rows, k, names));
        CHECK(ob_wbp_any_tools(m, 9), "Tools has items");
        k = ob_wbp_items(m, 9, NULL, rows, 3);
        CHECK(k == 3 && !strcmp(rows_text(rows, k, names), "I:Clock I:Convert S:to IFF"), "cut to max: %s",
              rows_text(rows, k, names));
        /* only the program's own menu: no Tools items */
        k = ob_wbp_items(m + 4, 2, NULL, rows, 32);
        CHECK(k == 0 && !ob_wbp_any_tools(m + 4, 2), "a title alone makes no Tools items");
        CHECK(ob_wbp_items(m, 0, NULL, rows, 32) == 0 && ob_wbp_own(m, 0, rows, 32) == 0, "none");
    }

    printf("ob_wbplan: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
