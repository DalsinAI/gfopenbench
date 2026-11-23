/* ob_popsel.h: the menu at the pointer in a drawer window (0.1.74; Dale, 23 November 2026: "right clicking on icons has
 * no menu"; 0.1.73 did the desktop's icons). What ob_dtask.c asks before it opens ob_popup over a drawer's icon: when
 * the right button is trapped, which icons a right press selects, and which of the drawer's Icons menu items go in
 * the menu. Plain C, no Amiga headers: tests/host/test_ob_popsel.c.
 *
 * As on the desktop: over an icon or a row the drawer's window traps the right button (WFLG_RMBTRAP, set and cleared
 * as the pointer moves); a right press selects the icon, or keeps the selection when the icon is already in it, and
 * opens the drawer's Icons menu's items that can be picked now beside the pointer. Anywhere else in the window the
 * right button shows the menus as before. Not while the left button is down (a drag or a lasso: the right button gives
 * it up, as before), and not while the window shows the search of every drive's hits (the drawer's icons are hidden
 * under them, so nothing there is the icon the pointer is on).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_POPSEL_H
#define OB_POPSEL_H

/* What wb_drawer_at says is at a place: the same numbers as gfopengadtools' WB_AT_* (wb_drawer.h; ob_dtask.c checks) */
enum { OB_POPSEL_NONE = 0, OB_POPSEL_SPACE, OB_POPSEL_DRAWER, OB_POPSEL_FILE };

/* 1 when the right button is to be trapped: at (OB_POPSEL_*) is an icon, the left button isn't down (left_down) and
 * the window isn't showing the search's hits (hits) */
int ob_popsel_trap(int at, int left_down, int hits);

/* The last part of a path, the entry's name as the drawer lists it: "Work:Tools/x" gives "x", "Work:x" gives "x" */
const char *ob_popsel_leaf(const char *path);

/* The answer for wb_drawer_each (0 unselect, 1 select, 2 leave) for the entry name, the right press being on the entry
 * named leaf; keep: that entry was selected already, so the selection it is in is kept whole */
int ob_popsel_select(const char *name, const char *leaf, int keep);

/* One of the drawer's Icons menu items, as its MenuItem says */
typedef struct ob_popsel_item {
    const char *label;          /* its text; NULL for a bar or a picture */
    int enabled;                /* ITEMENABLED */
    int sub;                    /* it has sub-items */
    unsigned long id;           /* GadTools' user data; 0 for none */
} ob_popsel_item;

/* The items that go in the menu, in the menu's order: their indexes into it[] in pick[] (up to max), and how many.
 * Left out: bars and pictures, greyed items, items with sub-items or no id, Select all (the drawer's, not the
 * icon's: as the desktop leaves out Clean Up), and Empty trash unless the icon is a trashcan (trash). */
int ob_popsel_items(const ob_popsel_item *it, int n, int trash, int *pick, int max);

#endif
