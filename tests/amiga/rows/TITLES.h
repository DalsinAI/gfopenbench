/* W3 (TITLES): a drawer's window must be titled with the drawer's name (alone, or followed by a blank and more, as a
 * disk's usage), and the Workbench screen must have a title. win_title NULL: the window wasn't read (SKIP).
 * WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the reason in why. MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_title_judge(const char *win_title, const char *name, const char *screen_title, char *why, unsigned cap);
