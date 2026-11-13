/* gfopenlaunch.library: Workbench's launcher, as a module of its own (amicradle-teams
 * designs/WORKBENCH-BUILD-PLAN-20261113.md, row 6; it ships as one file in gfosmodules/, loaded when first opened).
 * Calls: launcher/ol_lib.h. Rules (STACK, TOOLPRI, the order of SYS:WBStartup, Shift-double-click, LoadWB's
 * switches): launcher/ol_core.c, tested on the host.
 *
 * A program is started as OS 3.2.3's Workbench starts one: a process with no input, output or current directory,
 * its home directory the program's drawer, and a WBStartup message with arg 0 the program and one WBArg (a lock on
 * the drawer and a name) for each argument. A disk's root is its own lock and no name. With no file of the
 * program's name, a resident of that name runs (GFOpenBench 0.1.32's rule: a user resident first, then a system
 * one, as the Shell does; its count goes up while it runs).
 *
 * The launcher never waits for a program. The WBStartups come back to a process of the library's own, which
 * unloads each program's code and unlocks its args. That process is started at the first launch and ends when no
 * program is running and the library is expunged.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stddef.h>
#include <exec/types.h>
#include <exec/resident.h>
#include <exec/libraries.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <exec/lists.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <workbench/startup.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/utility.h>

#include "ol_core.h"
#include "ol_lib.h"

#define REG(r, decl) register decl __asm(#r)

#define VERSION 1
#define REVISION 0

/* First in the file: run as a program, the library returns at once. */
__asm("\t.text\n\t.globl _ol_start\n_ol_start:\n\tmoveq #-1,d0\n\trts\n");

static const char lib_name[] = OL_LIBNAME;
static const char lib_id[] = "gfopenlaunch.library 1.0 (13.11.2026) GFOpenBench's launcher\r\n";
static const char lib_ver[] __attribute__((used)) =
    "$VER: gfopenlaunch.library 1.0 (13.11.2026) MIT, Copyright (c) 2026 AmiCradle Team";
static const char reaper_name[] = "GFOpenBench launcher";

struct ExecBase *SysBase;
struct DosLibrary *DOSBase;
struct Library *UtilityBase;

struct OLBase {
    struct Library lib;
    UWORD pad;
    BPTR seglist;
    struct SignalSemaphore lock;    /* runs, running, next_id, port */
    struct MinList runs;            /* every program started and not yet ended */
    LONG running;
    ULONG next_id;
    struct MsgPort *volatile port;  /* the reaper's: the WBStartups come back here */
    struct Process *volatile reaper;
};

static struct OLBase *Base;

/* One program started. Its WBStartup is the message sent, so the reaper finds the run from it. */
typedef struct ol_run {
    struct MinNode node;            /* in runs */
    struct WBStartup wbs;
    ULONG id;
    struct Segment *resident;       /* the resident it runs, or NULL: its code is then ours to unload */
    LONG nargs;
    struct WBArg *args;             /* nargs of them, after this */
    char *names;                    /* nargs * OL_NAME_LEN, after the args */
} ol_run;

static void o_copy(char *d, const char *s, ULONG n)
{
    if (!n) return;
    while (*s && n > 1) *d++ = *s++, n--;
    *d = 0;
}

/* ---- the lists ---- */

static void list_init(struct MinList *l)
{
    l->mlh_Head = (struct MinNode *)&l->mlh_Tail;
    l->mlh_Tail = NULL;
    l->mlh_TailPred = (struct MinNode *)&l->mlh_Head;
}

static ol_run *run_of_msg(struct Message *m) { return (ol_run *)((UBYTE *)m - offsetof(ol_run, wbs)); }

/* A program has ended: its code unloaded (a resident's count back down), its args unlocked. */
static void run_done(struct OLBase *b, ol_run *r)
{
    LONG i;
    ObtainSemaphore(&b->lock);
    Remove((struct Node *)&r->node);
    b->running--;
    ReleaseSemaphore(&b->lock);
    if (r->resident) {
        Forbid();
        if (r->resident->seg_UC > 0) r->resident->seg_UC--;
        Permit();
    } else if (r->wbs.sm_Segment)
        UnLoadSeg(r->wbs.sm_Segment);
    for (i = 0; i < r->nargs; i++)
        if (r->args[i].wa_Lock) UnLock(r->args[i].wa_Lock);
    FreeVec(r);
}

/* ---- the reaper ---- */

static LONG reaper_entry(void)
{
    struct OLBase *b = Base;
    struct MsgPort *p = CreateMsgPort();
    if (!p) {
        Forbid();
        b->reaper = NULL;
        return 0;
    }
    b->port = p;                                   /* the starter waits for this */
    for (;;) {
        ULONG s = Wait((1UL << p->mp_SigBit) | SIGBREAKF_CTRL_C);
        struct Message *m;
        while ((m = GetMsg(p))) run_done(b, run_of_msg(m));
        if (s & SIGBREAKF_CTRL_C) {
            ObtainSemaphore(&b->lock);
            if (!b->running) {
                Forbid();                          /* gone before anyone can unload our code */
                b->port = NULL;
                b->reaper = NULL;
                ReleaseSemaphore(&b->lock);
                DeleteMsgPort(p);
                return 0;
            }
            ReleaseSemaphore(&b->lock);
        }
    }
}

/* The reaper's port, the reaper started when it isn't there (call with the lock held). NULL when it can't be. */
static struct MsgPort *reaper_port(struct OLBase *b)
{
    int i;
    if (b->port) return b->port;
    if (!b->reaper) {
        struct TagItem t[] = {
            { NP_Entry, (ULONG)reaper_entry }, { NP_Name, (ULONG)reaper_name }, { NP_StackSize, 4096 },
            { NP_Priority, 1 }, { TAG_DONE, 0 }
        };
        if (!(b->reaper = CreateNewProc(t))) return NULL;
    }
    for (i = 0; i < 100 && !b->port && b->reaper; i++) Delay(1);   /* two seconds at the most */
    return b->port;
}

/* ---- starting a program ---- */

/* A WBArg for path: the lock on its drawer and its name (a disk's root: its own lock, no name). A project that is
 * only its icon (SYS:System/Shell.info) is its drawer and its name too. 0 when there is no such place. */
static BPTR parent_and_name(const char *path, char *name, ULONG n)
{
    BPTR l = Lock((CONST_STRPTR)path, ACCESS_READ), p;
    const char *fp = (const char *)FilePart((CONST_STRPTR)path);
    if (!l) {
        char dir[256];
        ULONG k = (ULONG)((const char *)PathPart((CONST_STRPTR)path) - path), i;
        if (!*fp || k >= sizeof dir) return 0;
        for (i = 0; i < k; i++) dir[i] = path[i];
        dir[k] = 0;
        if (!(p = Lock((CONST_STRPTR)dir, ACCESS_READ))) return 0;
        o_copy(name, fp, n);
        return p;
    }
    if (!(p = ParentDir(l))) {
        name[0] = 0;
        return l;
    }
    UnLock(l);
    o_copy(name, fp, n);
    return p;
}

/* The resident of the program's name, when there is no file of that name: its seglist, or 0 (*res the resident) */
static BPTR resident_tool(const char *tool, struct Segment **res)
{
    const char *name = (const char *)FilePart((CONST_STRPTR)tool);
    struct Segment *s;
    BPTR seg = 0;
    *res = NULL;
    if (IoErr() != ERROR_OBJECT_NOT_FOUND || !*name) return 0;
    Forbid();
    if (!(s = FindSegment((CONST_STRPTR)name, NULL, FALSE))) s = FindSegment((CONST_STRPTR)name, NULL, TRUE);
    if (s && s->seg_Seg && s->seg_UC >= CMD_SYSTEM) {      /* not a Shell built-in, nor one disabled */
        if (s->seg_UC >= 0) s->seg_UC++;
        seg = s->seg_Seg;
        *res = s;
    }
    Permit();
    return seg;
}

static void run_free_args(ol_run *r)
{
    LONG i;
    for (i = 0; i < r->nargs; i++)
        if (r->args[i].wa_Lock) UnLock(r->args[i].wa_Lock);
    FreeVec(r);
}

static ULONG L_LaunchA(REG(a0, struct TagItem *tags), REG(a6, struct OLBase *b))
{
    struct TagItem *t, *state = tags;
    const char *tool = (const char *)GetTagData(OLA_Tool, 0, tags);
    const char *argv[OL_ARGS_MAX];
    LONG n = 1, i;
    ol_opts o;
    ol_run *r;
    BPTR seg;
    struct Process *p;
    struct MsgPort *port;
    ULONG id;
    if (!tool || !*tool) return 0;
    while ((t = NextTagItem(&state)))
        if (t->ti_Tag == OLA_Arg && t->ti_Data && n < OL_ARGS_MAX) argv[n++] = (const char *)t->ti_Data;
    argv[0] = tool;
    ol_opts_read(&o, (const char *const *)GetTagData(OLA_ToolTypes, 0, tags), (long)GetTagData(OLA_StackSize, 0, tags),
                 (long)GetTagData(OLA_MinStack, 8192, tags));
    if (!(r = AllocVec(sizeof *r + (ULONG)n * (sizeof(struct WBArg) + OL_NAME_LEN), MEMF_PUBLIC | MEMF_CLEAR))) return 0;
    r->args = (struct WBArg *)(r + 1);
    r->names = (char *)(r->args + n);
    for (i = 0; i < n; i++) {
        char *nm = r->names + i * OL_NAME_LEN;
        if (!(r->args[i].wa_Lock = parent_and_name(argv[i], nm, OL_NAME_LEN))) {
            r->nargs = i;
            run_free_args(r);
            return 0;
        }
        r->args[i].wa_Name = (BYTE *)nm;
    }
    r->nargs = n;
    seg = LoadSeg((CONST_STRPTR)tool);
    if (!seg && !GetTagData(OLA_NoResident, FALSE, tags)) seg = resident_tool(tool, &r->resident);
    if (!seg) {
        run_free_args(r);
        return 0;
    }
    ObtainSemaphore(&b->lock);
    if (!(port = reaper_port(b))) {
        ReleaseSemaphore(&b->lock);
        if (r->resident) {
            Forbid();
            if (r->resident->seg_UC > 0) r->resident->seg_UC--;
            Permit();
        } else
            UnLoadSeg(seg);
        run_free_args(r);
        return 0;
    }
    {
        /* As Workbench starts a tool: no input or output and no current directory (its startup code makes arg
         * 0's lock its current directory); GFOpenBench 0.1.7 and 0.1.10 found what goes wrong otherwise */
        struct TagItem pt[] = {
            { NP_Seglist, (ULONG)seg }, { NP_FreeSeglist, FALSE }, { NP_StackSize, (ULONG)o.stack },
            { NP_Priority, (ULONG)(LONG)o.pri }, { NP_Name, (ULONG)r->names }, { NP_CurrentDir, 0 },
            { NP_HomeDir, (ULONG)DupLock(r->args[0].wa_Lock) }, { NP_Input, 0 }, { NP_Output, 0 },
            { NP_CloseInput, FALSE }, { NP_CloseOutput, FALSE }, { TAG_DONE, 0 }
        };
        p = CreateNewProc(pt);
        if (!p && pt[6].ti_Data) UnLock((BPTR)pt[6].ti_Data);
    }
    if (!p) {
        ReleaseSemaphore(&b->lock);
        if (r->resident) {
            Forbid();
            if (r->resident->seg_UC > 0) r->resident->seg_UC--;
            Permit();
        } else
            UnLoadSeg(seg);
        run_free_args(r);
        return 0;
    }
    if (!++b->next_id) b->next_id = 1;
    id = r->id = b->next_id;
    r->wbs.sm_Message.mn_Node.ln_Type = NT_MESSAGE;
    r->wbs.sm_Message.mn_ReplyPort = port;
    r->wbs.sm_Message.mn_Length = sizeof(struct WBStartup);
    r->wbs.sm_Process = &p->pr_MsgPort;
    r->wbs.sm_Segment = seg;
    r->wbs.sm_NumArgs = n;
    r->wbs.sm_ToolWindow = NULL;
    r->wbs.sm_ArgList = r->args;
    AddTail((struct List *)&b->runs, (struct Node *)&r->node);   /* listed before it can come back */
    b->running++;
    ReleaseSemaphore(&b->lock);
    PutMsg(&p->pr_MsgPort, &r->wbs.sm_Message);
    return id;
}

static BOOL L_Ended(REG(d0, ULONG id), REG(a6, struct OLBase *b))
{
    struct MinNode *nd;
    BOOL ended = TRUE;
    ObtainSemaphoreShared(&b->lock);
    for (nd = b->runs.mlh_Head; nd->mln_Succ; nd = nd->mln_Succ)
        if (((ol_run *)nd)->id == id) { ended = FALSE; break; }
    ReleaseSemaphore(&b->lock);
    return ended;
}

/* s written into buf at k, no further than len - 1; the new k */
static LONG put(char *buf, LONG k, LONG len, const char *s)
{
    while (*s && k < len - 1) buf[k++] = *s++;
    return k;
}

static LONG L_Running(REG(a0, STRPTR buf), REG(d0, LONG len), REG(a6, struct OLBase *b))
{
    struct MinNode *nd;
    LONG k = 0, shown = 0, count;
    ObtainSemaphoreShared(&b->lock);
    count = b->running;
    if (buf && len > 0) {
        for (nd = b->runs.mlh_Head; nd->mln_Succ; nd = nd->mln_Succ) {
            if (++shown > 8) {
                char num[12];
                LONG v = count - 8, j = 11;
                num[j] = 0;
                do num[--j] = (char)('0' + v % 10), v /= 10; while (v && j);
                k = put((char *)buf, k, len, "  and ");
                k = put((char *)buf, k, len, num + j);
                k = put((char *)buf, k, len, " more\n");
                break;
            }
            k = put((char *)buf, k, len, "  ");
            k = put((char *)buf, k, len, ((ol_run *)nd)->names);
            k = put((char *)buf, k, len, "\n");
        }
        buf[k] = 0;
    }
    ReleaseSemaphore(&b->lock);
    return count;
}

static LONG L_QuitAll(REG(d0, LONG secs), REG(a6, struct OLBase *b))
{
    struct MinNode *nd;
    LONG i, left;
    ObtainSemaphoreShared(&b->lock);
    Forbid();
    for (nd = b->runs.mlh_Head; nd->mln_Succ; nd = nd->mln_Succ) {
        ol_run *r = (ol_run *)nd;
        if (r->wbs.sm_Message.mn_Node.ln_Type != NT_REPLYMSG && r->wbs.sm_Process)
            Signal(r->wbs.sm_Process->mp_SigTask, SIGBREAKF_CTRL_C);
    }
    Permit();
    ReleaseSemaphore(&b->lock);
    for (i = 0; i < secs * 10; i++) {
        ObtainSemaphoreShared(&b->lock);
        left = b->running;
        ReleaseSemaphore(&b->lock);
        if (!left) break;
        Delay(5);
    }
    ObtainSemaphoreShared(&b->lock);
    left = b->running;
    ReleaseSemaphore(&b->lock);
    return left;
}

/* ---- the library ---- */

static struct Library *lib_init(REG(d0, struct OLBase *b), REG(a0, BPTR seg), REG(a6, struct ExecBase *sys))
{
    SysBase = sys;
    b->seglist = seg;
    b->lib.lib_Node.ln_Type = NT_LIBRARY;
    b->lib.lib_Node.ln_Name = (char *)lib_name;
    b->lib.lib_Flags = LIBF_SUMUSED | LIBF_CHANGED;
    b->lib.lib_Version = VERSION;
    b->lib.lib_Revision = REVISION;
    b->lib.lib_IdString = (APTR)lib_id;
    DOSBase = (struct DosLibrary *)OpenLibrary((CONST_STRPTR)"dos.library", 39);
    UtilityBase = OpenLibrary((CONST_STRPTR)"utility.library", 39);
    if (!DOSBase || !UtilityBase) {
        if (DOSBase) CloseLibrary((struct Library *)DOSBase);
        if (UtilityBase) CloseLibrary(UtilityBase);
        FreeMem((UBYTE *)b - b->lib.lib_NegSize, b->lib.lib_NegSize + b->lib.lib_PosSize);
        return NULL;
    }
    InitSemaphore(&b->lock);
    list_init(&b->runs);
    Base = b;
    return &b->lib;
}

static struct Library *lib_open(REG(a6, struct OLBase *b))
{
    b->lib.lib_OpenCnt++;
    b->lib.lib_Flags &= ~LIBF_DELEXP;
    return &b->lib;
}

/* Expunged only with nothing running and the reaper gone: a reaper still there is asked to end, and the next
 * expunge (the next low-memory call) finds it gone. */
static BPTR lib_expunge(REG(a6, struct OLBase *b))
{
    BPTR seg;
    if (b->lib.lib_OpenCnt || b->running || b->reaper) {
        if (b->reaper && !b->running) Signal(&b->reaper->pr_Task, SIGBREAKF_CTRL_C);
        b->lib.lib_Flags |= LIBF_DELEXP;
        return 0;
    }
    seg = b->seglist;
    Remove(&b->lib.lib_Node);
    CloseLibrary(UtilityBase);
    CloseLibrary((struct Library *)DOSBase);
    FreeMem((UBYTE *)b - b->lib.lib_NegSize, b->lib.lib_NegSize + b->lib.lib_PosSize);
    return seg;
}

static BPTR lib_close(REG(a6, struct OLBase *b))
{
    if (b->lib.lib_OpenCnt) b->lib.lib_OpenCnt--;
    if (!b->lib.lib_OpenCnt && (b->lib.lib_Flags & LIBF_DELEXP)) return lib_expunge(b);
    return 0;
}

static ULONG lib_null(void) { return 0; }

static const APTR functable[] = {
    (APTR)lib_open, (APTR)lib_close, (APTR)lib_expunge, (APTR)lib_null,
    (APTR)L_LaunchA,                /* -30 */
    (APTR)L_Ended,                  /* -36 */
    (APTR)L_Running,                /* -42 */
    (APTR)L_QuitAll,                /* -48 */
    (APTR)-1
};
typedef char check_count[sizeof functable / sizeof functable[0] == 4 + 4 + 1 ? 1 : -1];

static const APTR inittable[4] = { (APTR)sizeof(struct OLBase), (APTR)functable, NULL, (APTR)lib_init };

const struct Resident romtag __attribute__((used)) = {
    RTC_MATCHWORD, (struct Resident *)&romtag, (APTR)(&romtag + 1), RTF_AUTOINIT, VERSION, NT_LIBRARY, 0,
    (char *)lib_name, (char *)lib_id, (APTR)inittable
};
