/* ob_name: the name requester of Icons > Rename... (CM4 item 12, row M19), as OS 3.2.3's Workbench asks: a window
 * "Rename" with "Enter a new name for 'x'.", the name to edit, OK and Cancel. See ob_name.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_NAME_H
#define OB_NAME_H

#include <intuition/screens.h>

/* Asks on screen scr for a new name for old (its name alone, "GFOpenMeters"), started with old and selected for
 * typing over. Waits for the answer, as 3.2.3's does. 1 with the name in out (size bytes, not empty, no ':' or '/'),
 * 0 for Cancel, Esc, the close gadget, or no window. */
int ob_name_ask(struct Screen *scr, const char *old, char *out, int size);

#endif
