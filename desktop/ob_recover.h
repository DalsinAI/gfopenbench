/* ob_recover: Icons > Recover deleted... and Put back (Dale 13 Nov 2026). The window of a volume's deleted files,
 * which ACFS keeps until it needs the space, drawn through GFOpenLayout UI (gfopenlayout.library), in a process of
 * its own; the work through gfopenfiles.library 1.1 (ob_recover_core.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_RECOVER_H
#define OB_RECOVER_H

struct Screen;
/* Icons > Recover deleted...: the window of vol's ("Work:") deleted files, on GFOpenBench's screen scr (kept open
 * until ob_recover_close); one already open shows vol's instead and comes to the front. 1 when it opened. */
int ob_recover_open(const char *vol, struct Screen *scr);
/* Icons > Put back: the file picked in that window put back where it was. 0 (and the caller beeps) when no window
 * is open. */
int ob_recover_put_back(void);
/* Closes the window and waits for its process to end (GFOpenBench quitting). */
void ob_recover_close(void);

#endif
