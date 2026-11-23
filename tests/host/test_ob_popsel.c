/* ob_popsel's host test: the menu at the pointer in a drawer window (GFOpenBench 0.1.75): when the right button is
 * trapped, which icons a right press selects, and which Icons menu items go in the menu.
 *   test_ob_popsel
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_popsel.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

/* the drawer's Icons menu as gfopengadtools' wb_drawer.c makes it (ids as its M_* numbers there) */
static const ob_popsel_item icons_menu[] = {
    { "Open", 1, 0, 15 },
    { "Information...", 1, 0, 16 },
    { "Snapshot", 1, 0, 25 },
    { "Unsnapshot", 1, 0, 26 },
    { "Leave Out", 1, 0, 27 },
    { "Copy", 1, 0, 30 },
    { "Rename...", 1, 0, 32 },
    { "Delete...", 1, 0, 33 },
    { "Empty trash", 1, 0, 31 },
    { NULL, 1, 0, 0 },                  /* the bar */
    { "Select all", 1, 0, 17 },
};
#define N_ICONS ((int)(sizeof icons_menu / sizeof icons_menu[0]))

static void test_trap(void)
{
    CHECK(ob_popsel_trap(OB_POPSEL_FILE, 0, 0));
    CHECK(ob_popsel_trap(OB_POPSEL_DRAWER, 0, 0));
    CHECK(!ob_popsel_trap(OB_POPSEL_SPACE, 0, 0));          /* the drawer's own space: the menus, as before */
    CHECK(!ob_popsel_trap(OB_POPSEL_NONE, 0, 0));           /* the frame, the icon line, the heading */
    CHECK(!ob_popsel_trap(OB_POPSEL_FILE, 1, 0));           /* a drag or a lasso: the right button gives it up */
    CHECK(!ob_popsel_trap(OB_POPSEL_DRAWER, 1, 0));
    CHECK(!ob_popsel_trap(OB_POPSEL_FILE, 0, 1));           /* the search's hits over the icons */
    CHECK(!ob_popsel_trap(OB_POPSEL_DRAWER, 0, 1));
    CHECK(!ob_popsel_trap(-1, 0, 0));
    CHECK(!ob_popsel_trap(99, 0, 0));
}

static void test_leaf(void)
{
    CHECK(!strcmp(ob_popsel_leaf("Work:Tools/GFOpenMeters"), "GFOpenMeters"));
    CHECK(!strcmp(ob_popsel_leaf("Work:Letters"), "Letters"));
    CHECK(!strcmp(ob_popsel_leaf("RAM:a/b/c d.txt"), "c d.txt"));
    CHECK(!strcmp(ob_popsel_leaf("name"), "name"));
    CHECK(!strcmp(ob_popsel_leaf(""), ""));
    CHECK(!strcmp(ob_popsel_leaf(NULL), ""));
}

/* the two passes ob_dtask.c makes over a drawer's entries (wb_drawer_each) */
static void press(const char *const *names, unsigned char *sel, int n, const char *path)
{
    const char *leaf = ob_popsel_leaf(path);
    int i, keep = 0, r;
    for (i = 0; i < n; i++)
        if (!strcmp(names[i], leaf)) keep = sel[i];
    for (i = 0; i < n; i++) {
        r = ob_popsel_select(names[i], leaf, keep);
        if (r == 0 || r == 1) sel[i] = (unsigned char)r;
    }
}

static void test_select(void)
{
    static const char *const names[] = { "Tools", "Letters", "ReadMe", "Letters.bak" };
    unsigned char sel[4];

    /* nothing selected: the icon pressed alone */
    memset(sel, 0, sizeof sel);
    press(names, sel, 4, "Work:Letters");
    CHECK(!sel[0] && sel[1] && !sel[2] && !sel[3]);

    /* others selected, not this one: this one alone, as a click would leave it */
    memset(sel, 0, sizeof sel);
    sel[0] = sel[2] = 1;
    press(names, sel, 4, "Work:Letters");
    CHECK(!sel[0] && sel[1] && !sel[2] && !sel[3]);

    /* pressed on one of several selected: they all stay, for the menu to act on all of them */
    memset(sel, 0, sizeof sel);
    sel[0] = sel[2] = 1;
    press(names, sel, 4, "Work:ReadMe");
    CHECK(sel[0] && !sel[1] && sel[2] && !sel[3]);

    /* the whole name, not its start */
    memset(sel, 0, sizeof sel);
    press(names, sel, 4, "Work:Letters.bak");
    CHECK(!sel[1] && sel[3]);

    CHECK(ob_popsel_select("x", "x", 1) == 2);
    CHECK(ob_popsel_select("x", "y", 1) == 2);
    CHECK(ob_popsel_select("x", "x", 0) == 1);
    CHECK(ob_popsel_select("x", "y", 0) == 0);
    CHECK(ob_popsel_select("X", "x", 0) == 0);              /* the same entry, not another one in another case */
    CHECK(ob_popsel_select(NULL, "x", 0) == 0);
}

static void test_items(void)
{
    int pick[24], n, i;
    ob_popsel_item m[N_ICONS];

    n = ob_popsel_items(icons_menu, N_ICONS, 0, pick, 24);
    CHECK(n == 8);                                          /* not Empty trash, the bar or Select all */
    CHECK(n == 8 && pick[0] == 0 && pick[7] == 7);
    for (i = 0; i < n; i++) {
        CHECK(icons_menu[pick[i]].label != NULL);
        CHECK(strcmp(icons_menu[pick[i]].label, "Select all") != 0);
        CHECK(strcmp(icons_menu[pick[i]].label, "Empty trash") != 0);
    }

    n = ob_popsel_items(icons_menu, N_ICONS, 1, pick, 24);  /* a trashcan: Empty trash too */
    CHECK(n == 9 && pick[8] == 8);

    memcpy(m, icons_menu, sizeof m);                        /* greyed (GFOpenDrawer's Snapshot, no leave_out) */
    m[2].enabled = m[3].enabled = m[4].enabled = 0;
    n = ob_popsel_items(m, N_ICONS, 0, pick, 24);
    CHECK(n == 5 && pick[0] == 0 && pick[1] == 1 && pick[2] == 5);

    memcpy(m, icons_menu, sizeof m);                        /* sub-items, no id */
    m[0].sub = 1;
    m[1].id = 0;
    n = ob_popsel_items(m, N_ICONS, 0, pick, 24);
    CHECK(n == 6 && pick[0] == 2);

    n = ob_popsel_items(icons_menu, N_ICONS, 1, pick, 3);   /* no more than max */
    CHECK(n == 3 && pick[2] == 2);
    CHECK(ob_popsel_items(icons_menu, 0, 1, pick, 24) == 0);
    CHECK(ob_popsel_items(icons_menu, N_ICONS, 1, pick, 0) == 0);
}

int main(void)
{
    test_trap();
    test_leaf();
    test_select();
    test_items();
    printf("test_ob_popsel: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
