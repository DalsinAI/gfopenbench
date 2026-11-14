/* W3 (TITLES): a drawer's window must be titled with the drawer's name (alone, or followed by a blank and more, as a
 * disk's usage), and the Workbench screen must have a title. win_title NULL: the window wasn't read (SKIP).
 * GFOpenBench's own title, "<path><name> - <n> item[s]" (RAM:WBGapW3 - 0 items), is a chosen difference from 3.2.3
 * (Dale, 14 November 2026: WBGAP_TITLE_DECISION): a PASS whose why starts "chosen difference", which the row prints
 * as an INFO line. WBGAP_PASS (why empty, or the chosen difference), WBGAP_FAIL or WBGAP_SKIP, the reason in why.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_TITLE_DECISION "teams/DECISION-DRAWER-TITLE-AND-FONTS-RELEASE-20261114-1505.md"
int wbgap_title_judge(const char *win_title, const char *name, const char *screen_title, char *why, unsigned cap);
