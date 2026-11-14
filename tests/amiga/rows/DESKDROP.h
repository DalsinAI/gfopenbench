/* I5 (DESKDROP): a file dragged from its drawer's window onto the desktop's own space is left out there, as on
 * 3.2.3: it stays in its drawer on the disk (src, src_info: the file and its .info still there, each 1 if so), its
 * icon is on the desktop (on_desk) and no longer in the drawer's window (in_win); on_desk and in_win -1 where the
 * desktop's icons weren't read. WBGAP_PASS, WBGAP_FAIL (why says which) or WBGAP_SKIP (on_desk < 0).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_deskdrop_judge(int src, int src_info, int on_desk, int in_win, const char *name, char *why, unsigned cap);
/* a window's icons as the row's ARexx wrote them, "<left> <top> <width> <height> <name>" a line: whether name is
 * among them (1, 0) */
int wbgap_deskdrop_has(const char *list, const char *name);
/* whether the point x,y (the window's own coordinates) is at least margin from every icon in list: 1, else 0 */
int wbgap_deskdrop_clear(const char *list, long x, long y, long margin);
