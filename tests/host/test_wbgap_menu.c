/* wbgap_menu's host test: the mouse's path to a menu item and a sub-item, as WBGapTest MENUDRIVE gives DragTest.
 *   test_wbgap_menu
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_menu.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static int inside(const struct wbgap_box *b, int x, int y)
{
    return x >= b->left && x < b->left + b->width && y >= b->top && y < b->top + b->height;
}

int main(void)
{
    int xy[2 * WBGAP_MENU_MAXPTS], n;
    /* a 3.2.3-like Window menu: title at 70, 80 wide; items 160 wide, 10 high, the third at top 20 */
    struct wbgap_box menu = {70, 0, 80, 11}, item = {0, 20, 160, 10}, sub = {150, 0, 90, 30};
    struct wbgap_box ibox = {70, 11 + 20, 160, 10};          /* the item on the screen, bar height 10 */
    struct wbgap_box sbox = {70 + 150, 11 + 20, 90, 30};
    struct wbgap_box tbox = {70, 0, 80, 11};
    int i;

    n = wbgap_menu_path(10, &menu, &item, NULL, xy);
    CHECK(n == 2, "an item under its title: title, item (%d points)", n);
    CHECK(inside(&tbox, xy[0], xy[1]), "starts on the title: %d,%d", xy[0], xy[1]);
    CHECK(xy[2] == 150 && xy[3] == 36 && inside(&ibox, xy[2], xy[3]), "ends in the item's middle: %d,%d", xy[2], xy[3]);

    n = wbgap_menu_path(10, &menu, &item, &sub, xy);
    CHECK(n == 4, "a sub-item: title, item, along its row, the sub-item (%d points)", n);
    CHECK(inside(&ibox, xy[2], xy[3]), "through the item");
    CHECK(xy[5] == xy[3], "along the item's row: %d", xy[5]);
    CHECK(inside(&sbox, xy[4], xy[5]), "into the sub-items' box on that row: %d,%d", xy[4], xy[5]);
    CHECK(inside(&sbox, xy[6], xy[7]) && xy[6] == 70 + 150 + 45 && xy[7] == 31 + 15, "ends on the sub-item: %d,%d", xy[6], xy[7]);
    for (i = 1; i < n; i++)   /* every leg is straight across or straight down, or ends in the next box */
        CHECK(xy[2 * i] == xy[2 * i - 2] || xy[2 * i + 1] == xy[2 * i - 1] || i == 1, "leg %d is straight", i);

    /* a title wider to the right than its items (a short menu at the screen's right) */
    {
        struct wbgap_box m2 = {500, 0, 120, 11}, it2 = {-60, 0, 100, 10};
        struct wbgap_box ib2 = {440, 11, 100, 10};
        n = wbgap_menu_path(10, &m2, &it2, NULL, xy);
        CHECK(n == 3, "a title past its items: title, into the column, item (%d)", n);
        CHECK(xy[0] == 560 && inside(&ib2, xy[2], xy[3]) && inside(&ib2, xy[4], xy[5]),
              "down beside, then into the column: %d,%d then %d,%d", xy[2], xy[3], xy[4], xy[5]);
    }

    /* a sub-items' box starting below the item's row */
    {
        struct wbgap_box s3 = {150, 15, 90, 30};
        struct wbgap_box sb3 = {220, 46, 90, 30};
        n = wbgap_menu_path(10, &menu, &item, &s3, xy);
        CHECK(n == 4 && xy[4] == 222 && xy[5] == 36, "along the row to the box's left edge: %d,%d", xy[4], xy[5]);
        CHECK(inside(&sb3, xy[6], xy[7]), "then to the sub-item");
    }

    {
        struct wbgap_box empty = {0, 0, 0, 0};
        CHECK(!wbgap_menu_path(10, &menu, &empty, NULL, xy), "an empty item");
        CHECK(!wbgap_menu_path(10, &empty, &item, NULL, xy), "an empty title");
        CHECK(!wbgap_menu_path(10, &menu, &item, &empty, xy), "an empty sub-item");
        CHECK(!wbgap_menu_path(10, NULL, &item, NULL, xy), "no menu");
    }
    printf("test_wbgap_menu: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
