/* the host checks' CHECK, shared by test_wbgap_rows.c and the per-row plans (tests/host/rows/<MODE>_plan.c, each
 * with a WBGAP_PLAN(<function>) line that tests/amiga/gen-rows.sh lists). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_CHECK_H
#define WBGAP_CHECK_H
#include <stdio.h>

extern int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)
#define WBGAP_PLAN(fn)

#endif
