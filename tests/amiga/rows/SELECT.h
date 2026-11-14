/* I1, I2 (SELECT): the icons a drawer's window has selected, as the WORKBENCH ARexx port lists them (one name a line,
 * CR LF or LF; list NULL: not read), against want, the names that should be (separated by blanks; "" or NULL: none).
 * The order doesn't matter. step says what was just done, for the reason. WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the
 * reason in why. MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_select_judge(const char *list, const char *want, const char *step, char *why, unsigned cap);
