/* I12a: the STACK= tool type a launch asks with (tests/amiga/rows/LAUNCH_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "LAUNCH.h"

void launch_plan(void);
WBGAP_PLAN(launch_plan)

void launch_plan(void)
{
    char st[24], tiny[8];
    CHECK(wbgap_launch_stack_tt(2097152UL, st, sizeof st) && !strcmp(st, "STACK=2097152"), "2 MB: \"%s\"", st);
    CHECK(wbgap_launch_stack_tt(4294967295UL, st, sizeof st) && !strcmp(st, "STACK=4294967295"),
          "the largest 32-bit stack still fits the mode's 24 bytes: \"%s\"", st);
    CHECK(!wbgap_launch_stack_tt(2097152UL, tiny, sizeof tiny) && !tiny[0], "no room: 0, nothing half written");
    CHECK(!wbgap_launch_stack_tt(1, NULL, 4) && !wbgap_launch_stack_tt(1, st, 0), "no buffer: 0");
}
