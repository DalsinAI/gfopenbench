/* ob_popup.h: GFOpenBench's menu at the pointer (0.1.73): a right press on an icon on the desktop opens a menu of what
 * can be done to it, the Icons menu's items, beside the pointer (Dale, 23 November 2026: right-clicking an icon gave
 * no menu). A right press on the desktop's own space still shows the screen's menus, as on Workbench. Since 0.1.74
 * the drawer windows' icons have it too, from their own tasks (ob_dtask.c, ob_popsel.h).
 *
 * The menu is a borderless window of its own: let the right button go over an item to take it (the press held), or,
 * after a press let go without moving onto the menu, click an item with either button; Esc, a click outside or
 * another window made active closes it with nothing taken. The cursor keys and Return work too. It waits on its own
 * port and is closed before it returns, as ob_pick's requesters.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_POPUP_H
#define OB_POPUP_H

#include <intuition/screens.h>

#define OB_POPUP_MAX 24

/* labels[0..n-1] (n at most OB_POPUP_MAX) at x, y on scr (screen coordinates: the menu's top left, moved in to fit
 * the screen). held: the right button is down now. The index taken, or -1. */
int ob_popup(struct Screen *scr, WORD x, WORD y, const char *const *labels, int n, int held);

#endif
