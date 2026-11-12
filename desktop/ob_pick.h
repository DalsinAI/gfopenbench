/* ob_pick.h: GFOpenBench's two small modal requesters (0.1.52): a line of text (Window > Find...'s words) and a
 * choice from a list (Workbench > Open Volume...'s volumes). Each opens a GadTools window on the screen, centred,
 * waits on its own port, and closes before it returns; the desktop's windows wait meanwhile, as they do behind an
 * EasyRequest. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_PICK_H
#define OB_PICK_H

#include <intuition/screens.h>

/* A line of text: title on the window, prompt beside the field, buf (n bytes) its start and the answer. 1 when the
 * user said OK (Return in the field, or the OK button) with something typed; 0 for Cancel, the close gadget, Esc,
 * nothing typed, or a window that couldn't open. */
int ob_pick_string(struct Screen *scr, const char *title, const char *prompt, const char *ok, char *buf, int n);

/* One of n names (at most OB_PICK_MAX): the index picked (a double-click, or a row then OK), or -1. */
#define OB_PICK_MAX 64
int ob_pick_list(struct Screen *scr, const char *title, const char *ok, const char *const *names, int n);

#endif
