/* CLOSEWB, CLOSEWBWAIT (C6): judged. Plain C, for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
/* C6: CloseWorkBench() with programs running. Refused (closed 0: the Shell's window is on the screen) the same screen,
 * the desktop's port and RAM:WBGapC's drawer window must all stay; if it did close, OpenWorkBench() must bring the
 * screen back (reopened) and the desktop must open the drawer again. WBGAP_PASS or WBGAP_FAIL, the reason in why */
int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap);
/* What the CLOSEWB process (started with Run) has written so far: 1 when it
 * is over ("INFO C6 done", its last line), 0 while it isn't (or text is
 * NULL). CLOSEWBWAIT gives the row a FAIL of its own when it isn't over in
 * time, so a CloseWorkBench() that never returns can't hang the run. */
int wbgap_closewb_over(const char *text);
/* The last "INFO C6 step: <what>" line CLOSEWB has written so far (it flushes
 * each one), copied into out without the prefix: 1 when there is one, 0 when
 * there is none (or text is NULL; out is then ""). CLOSEWBWAIT names it in its
 * FAIL, so a run shows whether CloseWorkBench() itself or a later step hung. */
int wbgap_closewb_last_step(const char *text, char *out, unsigned cap);
