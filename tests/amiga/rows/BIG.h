/* X1 (BIG): RAM:WBGapBig's 1000 icons shown in under 10 s.
 * wbgap_big_ticks: the ticks (50 a second) from DateStamp a (days, minute, tick) to b, b being the last change of
 * the inside seen, so the still second the mode waits for is not in it (WBGAP_BIG_SETTLE 0). wbgap_big_judge: WBGAP_PASS under 10 s (500 ticks), else WBGAP_FAIL
 * "over 10 s"; the reason in why (empty on a PASS).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_BIG_SETTLE 0
#define WBGAP_BIG_LIMIT (10 * 50)
long wbgap_big_ticks(long days_a, long min_a, long tick_a, long days_b, long min_b, long tick_b);
int wbgap_big_judge(long ticks, char *why, unsigned cap);
