/* ob_guard: see ob_guard.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/execbase.h>
#include <exec/lists.h>
#include <proto/exec.h>
#include <stdio.h>

#include "ob_guard.h"

int ob_task_held(struct Task *t, ob_held *h)
{
    struct Node *n;
    int held = 0;
    if (!t) return 0;
    Forbid();
    for (n = SysBase->TaskWait.lh_Head; n->ln_Succ; n = n->ln_Succ)
        if ((struct Task *)n == t) {
            unsigned long r = ob_guard_rec_at((unsigned long)t->tc_SPLower, (unsigned long)t->tc_SPUpper);
            int rec = r && ob_guard_read((const unsigned char *)r, h);
            if (!rec && h) h->vector = 0, h->pc = h->addr = 0;  /* held, the record not found: no why */
            held = ob_guard_held(1, t->tc_SigWait, rec);
            break;
        }
    Permit();
    return held;
}

void ob_task_describe(struct Task *t, char *out, int n)
{
    struct Node *n2;
    int waiting = 0;
    unsigned long lo, hi, sigwait, r, at = 0;
    int state;
    if (n <= 0) return;
    if (!t) {
        snprintf(out, (size_t)n, "no task");
        return;
    }
    Forbid();
    for (n2 = SysBase->TaskWait.lh_Head; n2->ln_Succ; n2 = n2->ln_Succ)
        if ((struct Task *)n2 == t) waiting = 1;
    lo = (unsigned long)t->tc_SPLower;
    hi = (unsigned long)t->tc_SPUpper;
    sigwait = t->tc_SigWait;
    state = t->tc_State;
    r = ob_guard_rec_at(lo, hi);
    if (r) at = *(volatile unsigned long *)r;
    Permit();
    snprintf(out, (size_t)n, "task %08lx: %s, tc_State %d, tc_SigWait %08lx, stack %08lx-%08lx, at %08lx: %08lx, held %d",
             (unsigned long)t, waiting ? "on TaskWait" : "not on TaskWait", state, sigwait, lo, hi, r, at,
             waiting && !sigwait);
}
