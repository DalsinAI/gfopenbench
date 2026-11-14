/* A1, A2 (APPWIN): an icon dropped on an AppWindow of ours. MIT, Copyright (c) 2026 AmiCradle Team.
 * wbgap_appwin_msg_judge: the AppMessage that came (got 0: none in 3 s) must be want_type with the id and userdata
 * given, and say where it was let go: am_MouseX, am_MouseY inside the window (win_w x win_h) and within
 * WBGAP_APPWIN_SLACK pixels of want_xy, the drop point relative to the window's top left.
 * wbgap_appwin_none_judge: a drop that must send nothing (after RemoveAppWindow, or outside every drop zone): PASS
 * if none came, else FAIL naming the message's type and id; when says which drop it was.
 * wbgap_appwin_both: one row from two parts, the worse (FAIL, then SKIP, then PASS) and its reason.
 * wbgap_appwin_row: the row: FAIL if a drag wasn't made (dragged 0: RAM:WBGapD's window went), since a drop that must
 * send nothing would pass without one; else wbgap_appwin_both.
 * Each returns WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the reason in why (empty on a PASS). */
#define WBGAP_APPWIN_SLACK 8
int wbgap_appwin_msg_judge(int got, long type, long want_type, unsigned long id, unsigned long want_id,
                           unsigned long userdata, unsigned long want_userdata, long mx, long my, const long want_xy[2],
                           long win_w, long win_h, char *why, unsigned cap);
int wbgap_appwin_none_judge(int got, long type, unsigned long id, const char *when, char *why, unsigned cap);
int wbgap_appwin_both(int a, const char *why_a, int b, const char *why_b, char *why, unsigned cap);
int wbgap_appwin_row(int dragged, int a, const char *why_a, int b, const char *why_b, char *why, unsigned cap);
