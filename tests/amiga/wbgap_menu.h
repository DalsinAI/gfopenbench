/* wbgap_menu: where the mouse goes to pick a menu item, from Intuition's menu boxes, for WBGapTest MENUDRIVE (the M
 * rows driven, not only found). Plain C99, no Amiga headers: tests/host/test_wbgap_menu.c runs it on the host.
 *
 * Intuition's places (screen pixels): a menu's title box is Menu LeftEdge, 0, Width, BarHeight + 1; an item's box is
 * the menu's LeftEdge plus the item's LeftEdge, BarHeight + 1 plus the item's TopEdge; a sub-item's is the item's box
 * plus the sub-item's LeftEdge and TopEdge.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_MENU_H
#define WBGAP_MENU_H

struct wbgap_box { int left, top, width, height; };

#define WBGAP_MENU_MAXPTS 5

/* The path for DragTest DRAG RIGHT: the right button down on the title, then each point in turn, let go on the
 * last. Down from the title into the items' column, across to the item; for a sub-item, along the item's row into
 * the sub-items' box, then up or down to the sub-item, so the pointer never leaves the item for another on the
 * way. sub NULL for an item. xy gets x, y pairs; returns how many points, 0 if a box is empty. */
int wbgap_menu_path(int bar_height, const struct wbgap_box *menu, const struct wbgap_box *item,
                    const struct wbgap_box *sub, int xy[2 * WBGAP_MENU_MAXPTS]);

#endif
