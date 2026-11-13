/* ob_guard.h: is one of the desktop's tasks held by the task guard (crashed)? The supervisor's eyes (build plan
 * row 3, WBGapTest X4; ob_tasks.h's ob_super decides what then). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_GUARD_H
#define OB_GUARD_H

#include <exec/tasks.h>
#include "ob_tasks.h"

/* 1 when t is waiting on no signal with the guard's record in its own stack: the guard holds it. Under Forbid.
 * h (may be NULL) gets the record. 0 for NULL, a task not waiting, or one the guard doesn't hold. */
int ob_task_held(struct Task *t, ob_held *h);

#endif
