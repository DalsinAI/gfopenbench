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
/* C6's own verdict in what CLOSEWB has written so far: the last whole line
 * "PASS C6", "FAIL C6 <why>" (its stdout), or "INFO C6 verdict: PASS" /
 * "INFO C6 verdict: FAIL <why>" (WBGAP_C6_STEPS, written before the clean-up).
 * WBGAP_PASS or WBGAP_FAIL with the reason in why, WBGAP_NONE when there is
 * none (or text is NULL). CLOSEWBWAIT reports it when the row decided but its
 * clean-up then didn't end in time, so a hung clean-up isn't counted as C6. */
int wbgap_closewb_verdict(const char *text, char *why, unsigned cap);
/* The same for any row CLOSEWB writes (C6, C6a): "PASS|FAIL|SKIP <row> <why>"
 * or "INFO <row> verdict: PASS|FAIL|SKIP <why>"; WBGAP_SKIP too, its why kept. The 15 Nov 00:32 lab run on
 * 3.2.3 showed why both files count: CLOSEWB ended (Status listed no
 * WBGapTest) yet none of its stdout reached the Run file, so its verdicts,
 * and its "INFO C6 done", are written to WBGAP_C6_STEPS too. */
int wbgap_closewb_row_verdict(const char *text, const char *row, char *why, unsigned cap);
/* CLOSEWB's step lines also go to this file, opened and closed for each one:
 * the Run >file output stays open (and may be held) until CLOSEWB ends. */
#define WBGAP_C6_STEPS "RAM:wbgap-c6-steps"
/* When CLOSEWB doesn't end, where GFOpenBench's task is (the 15 Nov 02:23 GFOS lab: after C5a its ARexx port
 * stopped answering): from its tc_SigWait, and the signal bits of its GFOPENBENCH and OPENBENCH ports (-1 when a
 * port isn't there). WBGAP_OBW_MAIN when it waits for both (its main loop's Wait), WBGAP_OBW_ELSEWHERE when it
 * waits for neither (inside a call: a requester, a reply), WBGAP_OBW_PART when for one only, WBGAP_OBW_RUNNING
 * when it isn't waiting (state not TS_WAIT), WBGAP_OBW_NONE when there is no GFOPENBENCH port. The words for each
 * in out (wbgap_closewb_obwait_text). */
enum { WBGAP_OBW_NONE, WBGAP_OBW_MAIN, WBGAP_OBW_PART, WBGAP_OBW_ELSEWHERE, WBGAP_OBW_RUNNING };
int wbgap_closewb_obwait(int found, int waiting, unsigned long sigwait, int pub_bit, int rexx_bit);
const char *wbgap_closewb_obwait_text(int where);
