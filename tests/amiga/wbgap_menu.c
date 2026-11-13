/* wbgap_menu: where the mouse goes to pick a menu item (wbgap_menu.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_menu.h"

int wbgap_menu_path(int bar_height, const struct wbgap_box *menu, const struct wbgap_box *item,
                    const struct wbgap_box *sub, int xy[2 * WBGAP_MENU_MAXPTS])
{
    int n = 0, tx, ty, ix, iy, il, it;
    if (!menu || !item || menu->width <= 0 || item->width <= 0 || item->height <= 0 || bar_height < 0) return 0;
    if (sub && (sub->width <= 0 || sub->height <= 0)) return 0;
    tx = menu->left + menu->width / 2;
    ty = bar_height / 2;
    il = menu->left + item->left;
    it = bar_height + 1 + item->top;
    ix = il + item->width / 2;
    iy = it + item->height / 2;
    xy[n++] = tx, xy[n++] = ty;
    if (tx < il || tx >= il + item->width) {        /* the title isn't over the items' column: into it first */
        int cx = tx < il ? il + 2 : il + item->width - 3;
        xy[n++] = cx, xy[n++] = iy;
    }
    xy[n++] = ix, xy[n++] = iy;
    if (sub) {
        int sl = il + sub->left, st = it + sub->top;
        int sx = sl + sub->width / 2, sy = st + sub->height / 2;
        int row = iy >= st && iy < st + sub->height;   /* the item's row runs into the sub-items' box */
        xy[n++] = row ? sx : sl + 2, xy[n++] = iy;
        xy[n++] = sx, xy[n++] = sy;
    }
    return n / 2;
}
