/* A3, A4, A6, A6b, A7, A8a-c, A9, M27, N8 (API): workbench.library's calls asked by a program.
 *
 * wbgap_api_update_judge (A7): ioerr, IoErr after UpdateWorkbench (UPDATEWB_ObjectAdded) for a new file and icon in
 * RAM:WBGap; have_window: RAM:WBGap's window was open; changed: its inside changed 100 ms later. IoErr left at 0, the
 * window open and changed: WBGAP_PASS, else WBGAP_FAIL with the first thing wrong in why.
 * The A8c judges (wbgap_which_*, wbgap_pick_desk) stay in wbgap_rows.h with their host checks.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_api_update_judge(long ioerr, int have_window, int changed, char *why, unsigned cap);
