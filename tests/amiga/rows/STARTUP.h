/* S1-S3, S2d, I12a-c at WBStartup, A10a (STARTUP): WBGapTest STARTUP SETUP puts icons for copies of WBGapTest in
 * SYS:WBStartup (WBGapA-G); at the next boot the desktop starts them, each writing "start NAME" and "end NAME" lines
 * to RAM:wbgap-order (or, F and G, what its start got to RAM:wbgap-launchwb and -launchwb2); STARTUP CHECK reads them.
 *
 * wbgap_startup_order: the verdict for row ("S1", "S2", "S3" or "S2d") from order, RAM:wbgap-order's text; gfos: the
 * desktop is GFOpenBench (S2d is a GFOS decision; 3.2.3 orders by name). WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the
 * reason in why. S1: A, B and C all started. S2: in STARTPRI order B (10), C (0), A (-10). S3: B (DONOTWAIT) doesn't
 * hold C up, C holds A up. S2d: D and E have no STARTPRI, E added before C and C before D: started E, C, D.
 * wbgap_startup_switches: the LoadWB switches in text (DEBUG, -DEBUG as DEBUG, DELAY, CLEANUP, NEWPATH,
 * SKIPWBSTARTUP, SKIP as SKIPWBSTARTUP, SIMPLEGELS, LEGACY; case not minded) into out as " A B", in that order, each
 * once: how many. text is not changed.
 * wbgap_startup_desk_rest: line, an S:Startup-Sequence line, after its leading blanks, "Run " and redirections: the
 * rest after the command when that command started the desktop (ob: "SYS:System/OpenBench", else "LoadWB" or
 * "C:LoadWB", case not minded, then a blank or the end), else NULL.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_startup_order(const char *order, const char *row, int gfos, char *why, unsigned cap);
int wbgap_startup_switches(const char *text, char *out, int n);
const char *wbgap_startup_desk_rest(const char *line, int ob);
