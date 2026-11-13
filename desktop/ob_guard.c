/* ob_guard: see ob_guard.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/execbase.h>
#include <exec/lists.h>
#include <proto/exec.h>

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
            held = !t->tc_SigWait && r && ob_guard_read((const unsigned char *)r, h);
            break;
        }
    Permit();
    return held;
}
