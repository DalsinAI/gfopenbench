/* I6 (TRASH): a file dragged onto a trashcan's icon on its own volume goes into that trashcan, its icon with it, as
 * on 3.2.3, and the trashcan stays where it was. What is where after the drag, each 1 if there: the file and its .info
 * where it came from (src, src_info) and in the trashcan (dst, dst_info), and the trashcan's own icon (can_info).
 * WBGAP_PASS, WBGAP_FAIL (why says which) or WBGAP_SKIP (can_info < 0: not looked at).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_trash_judge(int src, int src_info, int dst, int dst_info, int can_info, const char *name, char *why,
                      unsigned cap);
/* 1 if a window's title is RAM:WBGapT's: "WBGapT" not run on into a longer name (WBGapTest's own window is not) */
int wbgap_trash_title_ours(const char *title);
/* Why RAM:WBGapT's window isn't ready for I6 after it was opened and placed (want, got, lim as
 * wbgap_wingad_place_why, WINGAD.h). title: the title of the window found by "WBGapT", NULL if none was found.
 * 1 and why if it never showed, if the window found is another one, or if it isn't where it was asked to be;
 * 0 with why "placed late ..." if it is. */
int wbgap_trash_open_why(int found, const char *title, const long want[4], const long got[4], const long lim[4],
                         char *why, unsigned cap);
