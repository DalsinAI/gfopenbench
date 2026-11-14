/* ob_port: see ob_port.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/memory.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>

#include "ob_port.h"
#include "ob_sigplan.h"

static ob_share share = OB_SHARE_INIT;

static int alloc_sig(void *ctx)
{
    (void)ctx;
    return (int)AllocSignal(-1);
}

struct MsgPort *ob_port_new(void)
{
    struct MsgPort *mp;
    int sig;
    if (!(mp = AllocVec(sizeof *mp, MEMF_PUBLIC | MEMF_CLEAR))) return NULL;
    if ((sig = ob_share_take(&share, alloc_sig, NULL)) < 0) {
        FreeVec(mp);
        return NULL;
    }
    mp->mp_Node.ln_Type = NT_MSGPORT;
    mp->mp_Flags = PA_SIGNAL;
    mp->mp_SigBit = (UBYTE)sig;
    mp->mp_SigTask = FindTask(NULL);
    NewList(&mp->mp_MsgList);
    return mp;
}

void ob_port_free(struct MsgPort *mp)
{
    int sig;
    if (!mp) return;
    mp->mp_SigTask = (struct Task *)-1;     /* as DeleteMsgPort leaves it */
    mp->mp_MsgList.lh_Head = (struct Node *)-1;
    FreeVec(mp);
    if ((sig = ob_share_give(&share)) >= 0) FreeSignal(sig);
}
