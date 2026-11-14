/* LAUNCH's tool type (LAUNCH.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "LAUNCH.h"

int wbgap_launch_stack_tt(unsigned long want, char *out, unsigned cap)
{
    int n;
    if (!out || !cap) return 0;
    n = snprintf(out, cap, "STACK=%lu", want);
    if (n < 0 || (unsigned)n >= cap) { out[0] = 0; return 0; }
    return 1;
}
