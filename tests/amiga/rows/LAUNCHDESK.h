/* I10, I11 (LAUNCHDESK): icons left out on the desktop, clicked by WBGapDesk.rexx as a hand does it.
 *
 * wbgap_desk_i11: I11's verdict. bad: WBGapDesk.rexx says a click failed (RAM:wbgap-desk-bad); rec: the line LaunchT
 * wrote when it started (RAM:wbgap-launchdesk), NULL if it didn't; clicked: WBGapDesk.rexx clicked
 * (RAM:wbgap-desk-clicked). A failed click or no click at all is a SKIP; clicked and nothing started is a FAIL;
 * otherwise LaunchT must have got itself, p1 and p2 in that order (wbgap_launch_args). WBGAP_PASS, WBGAP_FAIL or
 * WBGAP_SKIP, the reason in why. I10's verdict is wbgap_i10 and the .backdrop steps wbgap_backdrop_setup and
 * wbgap_backdrop_done (wbgap_rows.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_desk_i11(int bad, const char *rec, int clicked, char *why, unsigned cap);
