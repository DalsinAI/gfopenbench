/* X1's judge (BIG.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "BIG.h"

long wbgap_big_ticks(long days_a, long min_a, long tick_a, long days_b, long min_b, long tick_b)
{
    return (days_b - days_a) * 24L * 60 * 3000 + (min_b - min_a) * 3000L + (tick_b - tick_a) - WBGAP_BIG_SETTLE;
}

int wbgap_big_judge(long ticks, char *why, unsigned cap)
{
    int pass = ticks < WBGAP_BIG_LIMIT;
    if (why && cap) snprintf(why, cap, "%s", pass ? "" : "over 10 s");
    return pass ? WBGAP_PASS : WBGAP_FAIL;
}
