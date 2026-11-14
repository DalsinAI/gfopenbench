/* I4, I7 (DRAGFILE): a file dragged between drawer windows, and onto a drawer's icon. MIT, Copyright (c) 2026 AmiCradle Team.
 * What is where after a drag is judged by wbgap_move_judge (wbgap_rows.h; TRASH and TRASHNOTE use it too).
 * wbgap_dragfile_width: each of the two drawer windows' width on a screen screen_w wide: half of it, at most
 * WBGAP_DRAGFILE_WW; 0 (both rows SKIP) when half is under WBGAP_DRAGFILE_MINW, or there is no screen (0). */
#define WBGAP_DRAGFILE_WW 440   /* room for three icons in a row and no scroller */
#define WBGAP_DRAGFILE_MINW 320 /* half the screen narrower than this: no room for both windows */
long wbgap_dragfile_width(long screen_w);
