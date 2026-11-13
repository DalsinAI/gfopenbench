/* ob_launch: the launcher (ob_launch.h). Its start of a program is GFOpenBench's start_tool of 0.1.7 to 0.1.56,
 * moved out of the desktop's process: what each step was for is said where it is.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <devices/timer.h>
#include <workbench/startup.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "ob_launch.h"
#include "ob_tasks.h"

/* A program's run: its WBStartup and its arguments, back on wbs_port when it ends. */
typedef struct launch {
    struct WBStartup wbs;
    struct WBArg args[2];
    char names[2][108];
    struct launch *next;
    struct Segment *resident;               /* 0.1.32: the resident it runs (its code is the resident's), or NULL */
    unsigned long id;
    struct MsgPort *tell;                   /* OB_LR_ENDED to this when it ends or at deadline; NULL none */
    unsigned long deadline;                 /* in ticks (1/50 s) of now() */
} launch;

static struct MsgPort *volatile req_port;   /* the launcher's, for ob_lreq */
static struct Task *parent;
static BYTE ready_sig = -1;

/* the launcher's own, in its process only */
static struct MsgPort *wbs_port;
static launch *launches;
static int running;
static unsigned long next_id = 1;

static unsigned long now(void)
{
    struct DateStamp d;
    DateStamp(&d);
    return ((unsigned long)d.ds_Days * 1440UL + (unsigned long)d.ds_Minute) * 3000UL + (unsigned long)d.ds_Tick;
}

BPTR ob_parent_and_name(const char *path, char *name, int n)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ), p;
    const char *fp = (const char *)FilePart((STRPTR)path);
    if (!l) {
        /* 0.1.25: a project that is only its icon, as SYS:System/Shell is (Shell.info, default tool SYS:System/CLI,
         * no file Shell): its WBArg is its drawer and its name, as Workbench gives it */
        char dir[256];
        int k = (int)((const char *)PathPart((STRPTR)path) - path);
        if (!*fp || k >= (int)sizeof dir) return 0;
        memcpy(dir, path, (size_t)k);
        dir[k] = 0;
        if (!(p = Lock((CONST_STRPTR)dir, ACCESS_READ))) return 0;
        snprintf(name, (size_t)n, "%s", fp);
        return p;
    }
    if (!(p = ParentDir(l))) {
        /* 0.1.52: a disk's root ("DF0:"): its WBArg is the root's own lock and no name, as Workbench gives a disk */
        name[0] = 0;
        return l;
    }
    UnLock(l);
    snprintf(name, (size_t)n, "%s", fp);
    return p;
}

/* 0.1.32: a tool icon with no file runs the resident of its name, as the Shell runs a resident command; only when the
 * tool's file isn't there (LoadSeg's ERROR_OBJECT_NOT_FOUND). A user resident first, then a system one; a user one's
 * count goes up while it runs (resident_done takes it down). */
static BPTR resident_tool(const char *tool, struct Segment **res)
{
    const char *name = (const char *)FilePart((CONST_STRPTR)tool);
    struct Segment *s;
    BPTR seg = 0;
    *res = NULL;
    if (IoErr() != ERROR_OBJECT_NOT_FOUND || !*name) return 0;
    Forbid();
    if (!(s = FindSegment((CONST_STRPTR)name, NULL, FALSE))) s = FindSegment((CONST_STRPTR)name, NULL, TRUE);
    if (s && s->seg_Seg && s->seg_UC >= CMD_SYSTEM) {
        if (s->seg_UC >= 0) s->seg_UC++;
        seg = s->seg_Seg;
        *res = s;
    }
    Permit();
    return seg;
}

static void resident_done(struct Segment *s)
{
    Forbid();
    if (s->seg_UC > 0) s->seg_UC--;
    Permit();
}

/* A tool started with a WBStartup message: arg 0 the tool, arg 1 the project when there is one. Its id, or 0. */
static unsigned long start_tool(const ob_lreq *r)
{
    launch *L = AllocVec(sizeof *L, MEMF_PUBLIC | MEMF_CLEAR);
    const char *tool = r->tool, *project = r->project[0] ? r->project : NULL;
    BPTR seg;
    struct Process *p;
    int n = 1;
    if (!L) return 0;
    if (!(L->args[0].wa_Lock = ob_parent_and_name(tool, L->names[0], sizeof L->names[0]))) { FreeVec(L); return 0; }
    L->args[0].wa_Name = (STRPTR)L->names[0];
    if (project) {
        if (!(L->args[1].wa_Lock = ob_parent_and_name(project, L->names[1], sizeof L->names[1]))) {
            UnLock(L->args[0].wa_Lock);
            FreeVec(L);
            return 0;
        }
        L->args[1].wa_Name = (STRPTR)L->names[1];
        n = 2;
    }
    if (!(seg = LoadSeg((CONST_STRPTR)tool)) && !(seg = resident_tool(tool, &L->resident))) {
        UnLock(L->args[0].wa_Lock);
        if (n > 1) UnLock(L->args[1].wa_Lock);
        FreeVec(L);
        return 0;
    }
    /* As Workbench starts a tool: no input or output and no current directory (0.1.7: dos's default NIL: made every
     * libnix tool fail on quitting with 8700000E; 0.1.10: a current directory of its own was unlocked twice) */
    p = CreateNewProcTags(NP_Seglist, (ULONG)seg, NP_FreeSeglist, FALSE, NP_StackSize, (ULONG)r->stack,
                          NP_Name, (ULONG)L->names[0], NP_CurrentDir, 0,
                          NP_HomeDir, (ULONG)DupLock(L->args[0].wa_Lock), NP_Input, 0, NP_Output, 0,
                          NP_CloseInput, FALSE, NP_CloseOutput, FALSE, TAG_DONE);
    if (!p) {
        if (L->resident) resident_done(L->resident);
        else UnLoadSeg(seg);
        UnLock(L->args[0].wa_Lock);
        if (n > 1) UnLock(L->args[1].wa_Lock);
        FreeVec(L);
        return 0;
    }
    L->wbs.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
    L->wbs.sm_Message.mn_ReplyPort = wbs_port;
    L->wbs.sm_Message.mn_Length = sizeof(struct WBStartup);
    L->wbs.sm_Process = &p->pr_MsgPort;
    L->wbs.sm_Segment = seg;
    L->wbs.sm_NumArgs = n;
    L->wbs.sm_ToolWindow = NULL;
    L->wbs.sm_ArgList = L->args;
    L->id = next_id++;
    if (!next_id) next_id = 1;
    if (r->wait > 0) {
        L->tell = r->msg.mn_ReplyPort;
        L->deadline = now() + (unsigned long)r->wait * 50UL;
    }
    L->next = launches;
    launches = L;
    PutMsg(&p->pr_MsgPort, &L->wbs.sm_Message);
    running++;
    return L->id;
}

/* OB_LR_ENDED for L to its desktop, once */
static void tell_ended(launch *L)
{
    ob_lreq *e;
    if (!L->tell) return;
    if ((e = AllocVec(sizeof *e, MEMF_PUBLIC | MEMF_CLEAR))) {
        e->msg.mn_Length = sizeof *e;
        e->kind = OB_LR_ENDED;
        e->id = L->id;
        PutMsg(L->tell, &e->msg);
    }
    L->tell = NULL;
}

static void launch_done(launch *L)
{
    launch **pp;
    int i;
    for (pp = &launches; *pp; pp = &(*pp)->next)
        if (*pp == L) { *pp = L->next; break; }
    tell_ended(L);
    if (L->resident) resident_done(L->resident);   /* a resident's code stays: it is the resident list's */
    else UnLoadSeg(L->wbs.sm_Segment);
    for (i = 0; i < L->wbs.sm_NumArgs && i < 2; i++)
        if (L->args[i].wa_Lock) UnLock(L->args[i].wa_Lock);
    FreeVec(L);
    running--;
}

static void reap(void)
{
    struct Message *m;
    while ((m = GetMsg(wbs_port))) launch_done((launch *)m);
}

/* the waits that are over: told. 1 while one is still to come */
static int deadlines(void)
{
    unsigned long t = now();
    launch *L;
    int more = 0;
    for (L = launches; L; L = L->next) {
        if (!L->tell) continue;
        if ((long)(t - L->deadline) >= 0) tell_ended(L);
        else more = 1;
    }
    return more;
}

static void names(ob_lreq *r)
{
    const char *list[9];
    launch *L;
    int k = 0;
    for (L = launches; L && k < 9; L = L->next) list[k++] = L->names[0];
    ob_names_format(r->names, sizeof r->names, list, running);
    r->left = running;
}

/* 0.1.19: Ctrl-C to each, as a Shell's Break asks, then up to secs for them to end */
static void quit_all(int secs)
{
    launch *L;
    int i;
    reap();
    Forbid();
    for (L = launches; L; L = L->next)
        if (L->wbs.sm_Message.mn_Node.ln_Type != NT_REPLYMSG && L->wbs.sm_Process)
            Signal(L->wbs.sm_Process->mp_SigTask, SIGBREAKF_CTRL_C);
    Permit();
    for (i = 0; running && i < secs * 10; i++) {
        Delay(5);
        reap();
    }
}

static void launcher_entry(void)
{
    struct MsgPort *tport = NULL, *rp;
    struct timerequest *tr = NULL;
    int timer = 0, ticking = 0;
    ob_lreq *end = NULL;
    rp = CreateMsgPort();
    wbs_port = CreateMsgPort();
    if ((tport = CreateMsgPort()) && (tr = (struct timerequest *)CreateIORequest(tport, sizeof *tr)) &&
        !OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0))
        timer = 1;
    if (!rp || !wbs_port || !timer) {
        if (timer) CloseDevice((struct IORequest *)tr);
        if (tr) DeleteIORequest((struct IORequest *)tr);
        if (tport) DeleteMsgPort(tport);
        if (wbs_port) DeleteMsgPort(wbs_port);
        if (rp) DeleteMsgPort(rp);
        Forbid();
        Signal(parent, 1UL << ready_sig);   /* req_port stays NULL: no launcher */
        return;
    }
    req_port = rp;
    Signal(parent, 1UL << ready_sig);
    for (;;) {
        ULONG sigs = Wait((1UL << rp->mp_SigBit) | (1UL << wbs_port->mp_SigBit) | (1UL << tport->mp_SigBit));
        struct Message *m;
        if (ticking && (sigs & (1UL << tport->mp_SigBit)) && CheckIO((struct IORequest *)tr)) {
            WaitIO((struct IORequest *)tr);
            ticking = 0;
        }
        reap();
        while ((m = GetMsg(rp))) {
            ob_lreq *r = (ob_lreq *)m;
            switch (r->kind) {
            case OB_LR_START:
                r->tool[sizeof r->tool - 1] = r->project[sizeof r->project - 1] = 0;
                r->id = start_tool(r);
                r->ok = r->id != 0;
                ReplyMsg(m);
                break;
            case OB_LR_QUIT:
                quit_all(r->secs);
                r->left = running;
                ReplyMsg(m);
                break;
            case OB_LR_NAMES:
                reap();
                names(r);
                ReplyMsg(m);
                break;
            case OB_LR_END:
                end = r;
                break;
            default:
                ReplyMsg(m);
            }
        }
        if (deadlines() && !ticking) {          /* a tenth of a second at a time while a wait is on */
            tr->tr_node.io_Command = TR_ADDREQUEST;
            tr->tr_time.tv_secs = 0;
            tr->tr_time.tv_micro = 100000;
            SendIO((struct IORequest *)tr);
            ticking = 1;
        }
        if (end && !running) break;
    }
    if (ticking) {
        AbortIO((struct IORequest *)tr);
        WaitIO((struct IORequest *)tr);
    }
    CloseDevice((struct IORequest *)tr);
    DeleteIORequest((struct IORequest *)tr);
    DeleteMsgPort(tport);
    DeleteMsgPort(wbs_port);
    wbs_port = NULL;
    /* ends under Forbid: GFOpenBench, whose code this is, can't unload it before this process is gone */
    Forbid();
    req_port = NULL;
    DeleteMsgPort(rp);
    end->left = 0;
    ReplyMsg(&end->msg);
}

int ob_launch_start(void)
{
    struct Process *p;
    if (req_port) return 1;
    parent = FindTask(NULL);
    if ((ready_sig = AllocSignal(-1)) < 0) return 0;
    SetSignal(0, 1UL << ready_sig);
    p = CreateNewProcTags(NP_Entry, (ULONG)launcher_entry, NP_StackSize, 16384,
                          NP_Name, (ULONG)"GFOpenBench launcher", TAG_DONE);
    if (p) Wait(1UL << ready_sig);
    FreeSignal(ready_sig);
    ready_sig = -1;
    return req_port != NULL;
}

void ob_launch_send(ob_lreq *r)
{
    r->msg.mn_Node.ln_Type = NT_MESSAGE;
    r->msg.mn_Length = sizeof *r;
    PutMsg(req_port, &r->msg);
}
