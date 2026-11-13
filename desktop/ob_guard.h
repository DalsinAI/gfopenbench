/* ob_guard.h: is one of the desktop's tasks held by the task guard (crashed)? The supervisor's eyes (build plan
 * row 3, WBGapTest X4; ob_tasks.h's ob_super decides what then). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_GUARD_H
#define OB_GUARD_H

#include <exec/tasks.h>
#include "ob_tasks.h"

/* 1 when t is waiting on no signal (Wait(0)): the guard holds it (ob_guard_held). Under Forbid. h (may be NULL)
 * gets the guard's record, vector 0 when it isn't where t's stack bounds say. 0 for NULL or a task not so held. */
int ob_task_held(struct Task *t, ob_held *h);

/* For a lab's log (WBGapF3): what ob_task_held looks at, as one line: on TaskWait or not, tc_SigWait, tc_State,
 * the stack bounds and the long where the record should be. */
void ob_task_describe(struct Task *t, char *out, int n);

#endif
