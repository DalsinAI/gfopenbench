/* ob_dtask.c: one task per drawer window (ob_dtask.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>
#include <stdio.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/timer.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/var.h>
#include <libraries/gadtools.h>
#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>

#include "ob_dtask.h"
#include "ob_guard.h"
#include "wb_find.h"

#define DT_NOTE_SIG SIGBREAKF_CTRL_F    /* a drawer's notes come on its Ctrl-F: no signal to allocate first */
#define DT_FIND     0x40000000UL        /* notes of this file's own */
#define DT_CLOSE    0x80000000UL
#define DT_MAX      64
#define DT_STACK    32768

enum { MK_CALL = 1, MK_EVENT, MK_QUERY };
enum { CB_OPEN = 1, CB_INFO, CB_DROP, CB_SNAP, CB_LEAVE_OUT, CB_NEW_DRAWER, CB_COPY, CB_EMPTY_TRASH, CB_RENAME,
       CB_DELETE, CB_PACK, CB_OPEN_WITH };

typedef struct dt_msg {
    struct Message m;
    int kind;
    ob_dt *t;
} dt_msg;

typedef struct dt_call {                /* a drawer's call: on its stack, while it waits for the answer */
    dt_msg h;
    int fn;
    const char *s;                      /* the path or the drawer */
    const char *const *v;               /* names, paths */
    int n;
    wb_snapitem *items;
    int flag;                           /* snap's unsnap */
    WORD sx, sy;
    UWORD qual;
    wb_drawer *from;
    ob_dpack *pack;
    int result;
} dt_call;

typedef struct dt_event {               /* a drawer's event: AllocVec'd and freed by the desktop, or the handle's own */
    dt_msg h;
    int ev;
    long err;
    unsigned long largest;
    int own;                            /* the handle's: not to be freed */
} dt_event;

enum { DQ_AT, DQ_SHOW, DQ_EACH };
typedef struct dt_query {               /* the desktop's question: AllocVec'd (left when a drawer crashes on it) */
    dt_msg h;
    int op;
    WORD sx, sy;                        /* DQ_AT: the place; DQ_SHOW: out holds the name */
    int at;                             /* the answer */
    char out[256];
    int (*fn)(void *ctx, const char *name, int type, int selected, WORD x, WORD y);   /* DQ_EACH */
    void *ctx;
    volatile int state;                 /* OB_Q_QUEUED, TAKEN or WITHDRAWN (ob_tasks.h), under Forbid */
} dt_query;

struct ob_dt {
    struct Message start;
    char path[256];
    struct IBox box;
    int has_box;
    APTR winptr;                        /* the desktop's DOS requesters' window */
    struct Task *volatile task;
    struct MsgPort *volatile qport;     /* the drawer's: the desktop's questions */
    struct MsgPort *creply;             /* the drawer's: its calls' answers */
    wb_drawer *volatile d;
    struct Window *volatile win;
    volatile ULONG notes;
    volatile int gone;                  /* its task has ended (or crashed): nothing more is sent to it */
    char find[200];
    ob_dpack pack;                      /* the drawer's: hidden and snapped */
    dt_event end;                       /* its last event (FAILED, CLOSED): sent when nothing more can be allocated */
    unsigned long user;
};

static ob_dt_env env;
static struct MsgPort *callport, *evport, *qreply, *tport;   /* calls and events apart: ob_dt_wait_open */
static BYTE dsig = -1;                  /* the one signal the desktop's four ports share */
static struct MsgPort *other;          /* ob_dt_port's: the desktop's 2-second timer */
static struct timerequest *tr;
static int timer_open;
static ob_dt *live[DT_MAX];
static int nlive;
static int closing_all;
static volatile unsigned long share_gen = 1;
static dt_query *qbuf;                  /* the question being asked, reused */

/* ---- the drawer's side ---- */
static ob_dt *self(void)
{
    return (ob_dt *)FindTask(NULL)->tc_UserData;
}

static void answer_queries(ob_dt *t)
{
    dt_query *q;
    while ((q = (dt_query *)GetMsg(t->qport))) {
        int take;
        Forbid();
        take = ob_q_take(&q->state);
        Permit();
        if (!take) {                    /* the desktop gave up on it (ask): nobody waits for it, and it's ours */
            FreeVec(q);
            continue;
        }
        if (q->op == DQ_SHOW) q->at = t->d ? wb_drawer_show_name(t->d, q->out) : 0;
        else if (q->op == DQ_EACH) q->at = t->d ? wb_drawer_each(t->d, q->fn, q->ctx) : -1;
        else {
            q->out[0] = 0;
            q->at = t->d ? wb_drawer_at(t->d, q->sx, q->sy, q->out, sizeof q->out) : WB_AT_NONE;
        }
        ReplyMsg(&q->h.m);
    }
}

/* the desktop runs the call; the drawer answers the desktop's questions while it waits */
static int call(dt_call *c)
{
    ob_dt *t = self();
    c->h.m.mn_Node.ln_Type = NT_MESSAGE;
    c->h.m.mn_Length = sizeof *c;
    c->h.m.mn_ReplyPort = t->creply;
    c->h.kind = MK_CALL;
    c->h.t = t;
    c->result = 0;
    PutMsg(callport, &c->h.m);
    for (;;) {
        answer_queries(t);
        if (GetMsg(t->creply)) return c->result;
        Wait((1UL << t->creply->mp_SigBit) | (1UL << t->qport->mp_SigBit));
    }
}

static void send_event(ob_dt *t, int ev)
{
    dt_event *e = AllocVec(sizeof *e, MEMF_PUBLIC | MEMF_CLEAR);
    if (!e) return;                     /* an activation or click the desktop doesn't hear of */
    e->h.m.mn_Node.ln_Type = NT_MESSAGE;
    e->h.m.mn_Length = sizeof *e;
    e->h.kind = MK_EVENT;
    e->h.t = t;
    e->ev = ev;
    PutMsg(evport, &e->h.m);
}

static void call_init(dt_call *c, int fn)
{
    memset(c, 0, sizeof *c);
    c->fn = fn;
}

static int st_open(const char *path)
{
    dt_call c;
    call_init(&c, CB_OPEN);
    c.s = path;
    return call(&c);
}

static int st_info(const char *path)
{
    dt_call c;
    call_init(&c, CB_INFO);
    c.s = path;
    return call(&c);
}

static int st_drop(const char *const *paths, int n, WORD sx, WORD sy, UWORD qual, wb_drawer *from)
{
    dt_call c;
    call_init(&c, CB_DROP);
    c.v = paths; c.n = n; c.sx = sx; c.sy = sy; c.qual = qual; c.from = from;
    return call(&c);
}

static void pack_fresh(const char *dir)
{
    ob_dt *t = self();
    dt_call c;
    if (ob_dpack_fresh(&t->pack, dir, share_gen)) return;
    ob_dpack_reset(&t->pack, dir, share_gen);
    call_init(&c, CB_PACK);
    c.s = dir;
    c.pack = &t->pack;
    call(&c);
}

static int st_hidden(const char *dir, const char *name)
{
    pack_fresh(dir);
    return ob_dpack_hidden(&self()->pack, name);
}

static int st_snapped(const char *dir, const char *name, int *gx, int *gy, LONG *x, LONG *y)
{
    long lx, ly;
    pack_fresh(dir);
    if (!ob_dpack_snapped(&self()->pack, name, gx, gy, &lx, &ly)) return 0;
    *x = (LONG)lx;
    *y = (LONG)ly;
    return 1;
}

static int st_snap(const char *dir, wb_snapitem *items, int n, int unsnap, wb_drawer *from)
{
    dt_call c;
    call_init(&c, CB_SNAP);
    c.s = dir; c.items = items; c.n = n; c.flag = unsnap; c.from = from;
    return call(&c);
}

static int st_names(int fn, const char *dir, const char *const *names, int n, wb_drawer *from)
{
    dt_call c;
    call_init(&c, fn);
    c.s = dir; c.v = names; c.n = n; c.from = from;
    return call(&c);
}

static int st_leave_out(const char *dir, const char *const *names, int n, wb_drawer *from)
{
    return st_names(CB_LEAVE_OUT, dir, names, n, from);
}

static int st_copy(const char *dir, const char *const *names, int n, wb_drawer *from)
{
    return st_names(CB_COPY, dir, names, n, from);
}

static int st_empty_trash(const char *dir, const char *const *names, int n, wb_drawer *from)
{
    return st_names(CB_EMPTY_TRASH, dir, names, n, from);
}

static int st_delete(const char *dir, const char *const *names, int n, wb_drawer *from)
{
    return st_names(CB_DELETE, dir, names, n, from);
}

static int st_new_drawer(const char *dir, wb_drawer *from)
{
    return st_names(CB_NEW_DRAWER, dir, NULL, 0, from);
}

static int st_rename(const char *path, wb_drawer *from)
{
    return st_names(CB_RENAME, path, NULL, 0, from);
}

#ifdef WB_LOOK_OPEN_WITH
static int st_open_with(const char *const *paths, int n, int clicked, wb_drawer *from)
{
    dt_call c;
    call_init(&c, CB_OPEN_WITH);
    c.v = paths; c.n = n; c.flag = clicked; c.from = from;
    return call(&c);
}
#endif

/* the desktop's look, with this task's port and the calls that go to the desktop */
static void look_for_task(wb_look *lk, struct MsgPort *port)
{
    const wb_look *L = env.look;
    *lk = *L;
    lk->port = port;
    lk->open_object = L->open_object ? st_open : NULL;
    lk->info_object = L->info_object ? st_info : NULL;
    lk->drop = L->drop ? st_drop : NULL;
    lk->hidden = L->hidden ? st_hidden : NULL;
    lk->snapped = L->snapped ? st_snapped : NULL;
    lk->snap = L->snap ? st_snap : NULL;
    lk->leave_out = L->leave_out ? st_leave_out : NULL;
    lk->new_drawer = L->new_drawer ? st_new_drawer : NULL;
    lk->copy = L->copy ? st_copy : NULL;
    lk->empty_trash = L->empty_trash ? st_empty_trash : NULL;
    lk->rename_icon = L->rename_icon ? st_rename : NULL;
    lk->delete_icons = L->delete_icons ? st_delete : NULL;
#ifdef WB_LOOK_OPEN_WITH
    lk->open_with = L->open_with ? st_open_with : NULL;
#endif
}

static ULONG take_notes(ob_dt *t)
{
    ULONG n;
    Forbid();
    n = t->notes;
    t->notes = 0;
    Permit();
    return n;
}

/* the drawer's last moment: no more questions, its ports gone, and the desktop told (under Forbid, so that the
 * desktop, whose code this is, can't let it go before this process has ended) */
static void dt_exit(ob_dt *t, struct MsgPort *idcmp, int ev, long err, unsigned long largest)
{
    struct Message *m;
    Forbid();
    t->gone = 1;
    if (t->qport) {
        answer_queries(t);              /* t->d is NULL by now: WB_AT_NONE */
        DeleteMsgPort(t->qport);
        t->qport = NULL;
    }
    if (t->creply) DeleteMsgPort(t->creply);
    if (idcmp) {
        while ((m = GetMsg(idcmp))) ReplyMsg(m);
        DeleteMsgPort(idcmp);
    }
    ob_dpack_free(&t->pack);
    t->end.h.m.mn_Node.ln_Type = NT_MESSAGE;
    t->end.h.m.mn_Length = sizeof t->end;
    t->end.h.kind = MK_EVENT;
    t->end.h.t = t;
    t->end.ev = ev;
    t->end.err = err;
    t->end.largest = largest;
    t->end.own = 1;
    PutMsg(evport, &t->end.h.m);
}

/* WBGapTest X4 for the drawers, in a lab only: with ENV:GFOpenBench/X4Test set, a drawer named WBGapCrash makes its
 * task crash once its window is open (an illegal instruction), for the task guard to hold and the supervisor to find.
 * Nothing happens without the variable. */
static void x4_test(const char *path)
{
    char v[4];
    if (strcmp((const char *)FilePart((CONST_STRPTR)path), "WBGapCrash") ||
        GetVar((CONST_STRPTR)"GFOpenBench/X4Test", (STRPTR)v, sizeof v, GVF_GLOBAL_ONLY) < 0)
        return;
    __asm volatile("illegal");
}

static void dt_entry(void)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    struct Message *sm;
    struct MsgPort *idcmp;
    struct IntuiMessage *im;
    wb_look lk;
    ob_dt *t;
    wb_drawer *d;
    int open = 1;

    WaitPort(&me->pr_MsgPort);
    sm = GetMsg(&me->pr_MsgPort);
    t = (ob_dt *)sm;                    /* the handle starts with its start message */
    me->pr_Task.tc_UserData = t;
    me->pr_WindowPtr = t->winptr;
    idcmp = CreateMsgPort();
    t->creply = CreateMsgPort();
    t->qport = CreateMsgPort();
    if (!idcmp || !t->creply || !t->qport) {
        dt_exit(t, idcmp, OB_DTE_FAILED, ERROR_NO_FREE_STORE, AvailMem(MEMF_ANY | MEMF_LARGEST));
        return;
    }
    look_for_task(&lk, idcmp);
    SetIoErr(0);
    if (!(d = wb_drawer_open(&lk, t->path))) {
        long err = IoErr();
        dt_exit(t, idcmp, OB_DTE_FAILED, err, AvailMem(MEMF_ANY | MEMF_LARGEST));
        return;
    }
    if (t->has_box) ChangeWindowBox(wb_drawer_window(d), t->box.Left, t->box.Top, t->box.Width, t->box.Height);
    Forbid();
    t->d = d;
    t->win = wb_drawer_window(d);
    Permit();
    send_event(t, OB_DTE_OPENED);
    x4_test(t->path);

    while (open) {
        ULONG sigs = Wait((1UL << idcmp->mp_SigBit) | (1UL << t->qport->mp_SigBit) | DT_NOTE_SIG |
                          wb_notify_sigmask() | wb_find_sigmask());
        ULONG notes;
        answer_queries(t);
        if ((sigs & DT_NOTE_SIG) && (notes = take_notes(t))) {
            if (notes & DT_CLOSE) break;
            if (notes & OB_DT_REREAD) wb_drawer_reread(d);
            if (notes & OB_DT_REDRAW) wb_drawer_redraw(d);
            if (notes & OB_DT_THUMBS) wb_drawer_thumbs(d);
            if (notes & OB_DT_FIND_UPDATE) wb_drawer_find_update(d);
            if (notes & DT_FIND) {
                char words[sizeof t->find];
                Forbid();
                memcpy(words, t->find, sizeof words);
                Permit();
                WindowToFront(t->win);
                ActivateWindow(t->win);
                if (!wb_drawer_find(d, words)) DisplayBeep(t->win->WScreen);
            }
        }
        if (sigs & wb_notify_sigmask()) wb_notify_collect();       /* its drawer changed: read again 200 ms after */
        if ((sigs & wb_find_sigmask()) && wb_find_collect()) {     /* its search's hits */
            wb_drawer_find_update(d);
            send_event(t, OB_DTE_FOUND);
        }
        while ((im = GT_GetIMsg(idcmp))) {
            ULONG cl = im->Class, secs = im->Seconds, micros = im->Micros;
            UWORD code = im->Code, qual = im->Qualifier;
            APTR ia = im->IAddress;
            WORD mx = im->MouseX, my = im->MouseY;
            GT_ReplyIMsg(im);
            if (cl == IDCMP_MOUSEBUTTONS && code == SELECTDOWN) send_event(t, OB_DTE_CLICK);
            if (!wb_drawer_event(d, cl, code, qual, ia, mx, my, secs, micros)) {
                open = 0;
                break;
            }
            if (cl == IDCMP_ACTIVEWINDOW) send_event(t, OB_DTE_ACTIVE);
        }
    }
    Forbid();
    t->d = NULL;
    t->win = NULL;
    Permit();
    wb_drawer_close(d);
    wb_notify_end();                    /* this task's notifies, and their port */
    dt_exit(t, idcmp, OB_DTE_CLOSED, 0, 0);
}

/* ---- the desktop's side ---- */
static int live_at(const ob_dt *t)
{
    int i;
    for (i = 0; i < nlive; i++)
        if (live[i] == t) return i;
    return -1;
}

static void unlink_dt(ob_dt *t)
{
    int i = live_at(t);
    if (i >= 0) live[i] = live[--nlive];
}

/* A port of the desktop's on signal dsig: the desktop's signals are nearly all taken (its ARexx ports came after) */
static struct MsgPort *shared_port(void)
{
    struct MsgPort *p = AllocVec(sizeof *p, MEMF_PUBLIC | MEMF_CLEAR);
    if (!p) return NULL;
    p->mp_Node.ln_Type = NT_MSGPORT;
    p->mp_Flags = PA_SIGNAL;
    p->mp_SigBit = (UBYTE)dsig;
    p->mp_SigTask = FindTask(NULL);
    NewList(&p->mp_MsgList);
    return p;
}

struct MsgPort *ob_dt_port(void)
{
    if (dsig < 0 || other) return NULL;    /* one */
    return other = shared_port();
}

/* What came on the shared signal while ask or ob_dt_wait_open took it (Wait cleared it): the signal again, for the
 * desktop's loop */
static void resignal(void)
{
    if (!IsListEmpty(&callport->mp_MsgList) || !IsListEmpty(&evport->mp_MsgList) ||
        (other && !IsListEmpty(&other->mp_MsgList)))
        SetSignal(1UL << dsig, 1UL << dsig);
}

int ob_dt_init(const ob_dt_env *e)
{
    env = *e;
    if ((dsig = AllocSignal(-1)) < 0) return 0;
    callport = shared_port();
    evport = shared_port();
    qreply = shared_port();
    tport = shared_port();
    tr = tport ? (struct timerequest *)CreateIORequest(tport, sizeof *tr) : NULL;
    timer_open = tr && !OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_VBLANK, (struct IORequest *)tr, 0);
    qbuf = AllocVec(sizeof *qbuf, MEMF_PUBLIC | MEMF_CLEAR);
    if (!callport || !evport || !qreply || !timer_open || !qbuf) {
        ob_dt_end();
        return 0;
    }
    return 1;
}

void ob_dt_end(void)
{
    if (timer_open) CloseDevice((struct IORequest *)tr);
    timer_open = 0;
    if (tr) DeleteIORequest((struct IORequest *)tr);
    tr = NULL;
    if (tport) FreeVec(tport);
    if (qreply) FreeVec(qreply);
    if (callport) FreeVec(callport);
    if (evport) FreeVec(evport);
    tport = qreply = callport = evport = NULL;
    other = NULL;                       /* the desktop's, freed by it (after its timer stopped) */
    if (dsig >= 0) FreeSignal(dsig);
    dsig = -1;
    if (qbuf) FreeVec(qbuf);
    qbuf = NULL;
}

ULONG ob_dt_sigmask(void)
{
    return callport && evport ? 1UL << dsig : 0;
}

void ob_dt_set_gen(unsigned long gen)
{
    share_gen = gen;
}

static int run_call(dt_call *c)
{
    const wb_look *L = env.look;
    if (closing_all) return 0;          /* the screen is closing: nothing more is done for a drawer */
    switch (c->fn) {
    case CB_OPEN: return L->open_object(c->s);
    case CB_INFO: return L->info_object(c->s);
    case CB_DROP: return L->drop(c->v, c->n, c->sx, c->sy, c->qual, c->from);
    case CB_SNAP: return L->snap(c->s, c->items, c->n, c->flag, c->from);
    case CB_LEAVE_OUT: return L->leave_out(c->s, c->v, c->n, c->from);
    case CB_NEW_DRAWER: return L->new_drawer(c->s, c->from);
    case CB_COPY: return L->copy(c->s, c->v, c->n, c->from);
    case CB_EMPTY_TRASH: return L->empty_trash(c->s, c->v, c->n, c->from);
    case CB_RENAME: return L->rename_icon(c->s, c->from);
    case CB_DELETE: return L->delete_icons(c->s, c->v, c->n, c->from);
    case CB_PACK:
        if (env.pack) env.pack(c->s, c->pack);
        return 1;
#ifdef WB_LOOK_OPEN_WITH
    case CB_OPEN_WITH: return L->open_with(c->v, c->n, c->flag, c->from);
#endif
    }
    return 0;
}

static void take_events(void)
{
    struct Message *m;
    while ((m = GetMsg(evport))) {
        dt_event *e = (dt_event *)m;
        ob_dt *t = e->h.t;
        int own = e->own, ev = e->ev;
        if (live_at(t) >= 0) {
            if (env.event) env.event(t, ev, e->err, e->largest);
            if (ev == OB_DTE_FAILED || ev == OB_DTE_CLOSED) {
                unlink_dt(t);
                FreeVec(t);             /* e was t's own */
                own = 1;
            }
        }
        if (!own) FreeVec(e);
    }
}

void ob_dt_service(void)
{
    struct Message *m;
    if (!callport) return;
    while ((m = GetMsg(callport))) {
        dt_call *c = (dt_call *)m;
        c->result = run_call(c);
        if (env.gen) share_gen = env.gen();     /* before the drawer goes on: what it reads next is hidden so */
        ReplyMsg(m);
    }
    take_events();
}

ob_dt *ob_dt_open(const char *path, const struct IBox *box)
{
    struct Process *p;
    ob_dt *t;
    if (!callport || nlive >= DT_MAX) return NULL;
    if (!(t = AllocVec(sizeof *t, MEMF_PUBLIC | MEMF_CLEAR))) return NULL;
    snprintf(t->path, sizeof t->path, "%s", path);
    if (box) {
        t->box = *box;
        t->has_box = 1;
    }
    t->winptr = ((struct Process *)FindTask(NULL))->pr_WindowPtr;
    ob_dpack_init(&t->pack);
    p = CreateNewProcTags(NP_Entry, (ULONG)dt_entry, NP_StackSize, DT_STACK, NP_Name, (ULONG)"GFOpenBench drawer",
                          TAG_DONE);
    if (!p) {
        FreeVec(t);
        return NULL;
    }
    t->task = &p->pr_Task;
    live[nlive++] = t;
    t->start.mn_Node.ln_Type = NT_MESSAGE;
    t->start.mn_Length = sizeof *t;
    PutMsg(&p->pr_MsgPort, &t->start);
    return t;
}

void ob_dt_note(ob_dt *t, ULONG notes)
{
    if (!t) return;
    Forbid();
    if (!t->gone && t->task) {
        t->notes |= notes;
        Signal(t->task, DT_NOTE_SIG);
    }
    Permit();
}

void ob_dt_close(ob_dt *t)
{
    ob_dt_note(t, DT_CLOSE);
}

void ob_dt_find(ob_dt *t, const char *words)
{
    if (!t) return;
    Forbid();
    snprintf(t->find, sizeof t->find, "%s", words);
    Permit();
    ob_dt_note(t, DT_FIND);
}

/* half a second of waiting on mask, or less when it comes; 1 when it came */
/* the desktop's ports share one signal: 1 when something came before the half second was over, 0 when it was */
static int wait_half(void)
{
    int over;
    tr->tr_node.io_Command = TR_ADDREQUEST;
    tr->tr_time.tv_secs = 0;
    tr->tr_time.tv_micro = 500000;
    SendIO((struct IORequest *)tr);
    Wait(1UL << dsig);
    over = CheckIO((struct IORequest *)tr) != NULL;
    if (!over) AbortIO((struct IORequest *)tr);
    WaitIO((struct IORequest *)tr);
    return !over;
}

static int held(const ob_dt *t)
{
    int h;
    Forbid();
    h = t->task && !t->gone && ob_task_held(t->task, NULL);
    Permit();
    return h;
}

/* qbuf, filled in by the caller but for its header, asked of t's task and waited for: 1 when answered */
static unsigned long ticks_between(const struct DateStamp *a, const struct DateStamp *b)
{
    long d = (b->ds_Days - a->ds_Days) * 24L * 60 * TICKS_PER_SECOND * 60 + (b->ds_Minute - a->ds_Minute) * 60L *
             TICKS_PER_SECOND + (b->ds_Tick - a->ds_Tick);
    return d > 0 ? (unsigned long)d : 0;
}

static int ask(ob_dt *t, dt_query *q)
{
    struct DateStamp since, now;
    q->h.m.mn_Node.ln_Type = NT_MESSAGE;
    q->h.m.mn_Length = sizeof *q;
    q->h.m.mn_ReplyPort = qreply;
    q->h.kind = MK_QUERY;
    q->h.t = t;
    q->state = OB_Q_QUEUED;
    Forbid();
    if (t->gone || !t->qport || !t->d) {
        Permit();
        return 0;
    }
    PutMsg(t->qport, &q->h.m);
    Permit();
    DateStamp(&since);
    while (!GetMsg(qreply)) {
        int gave_up;
        if (!wait_half() && held(t)) {  /* crashed with the question on its port: the question stays with it */
            qbuf = AllocVec(sizeof *qbuf, MEMF_PUBLIC | MEMF_CLEAR);
            resignal();
            return 0;
        }
        /* not taken within a second (the drawer busy on a slow disk): withdrawn, left for the task to free, and the
         * desktop goes on as if the drawer had nothing there. One it took is answered from memory: waited for. */
        DateStamp(&now);
        Forbid();
        gave_up = ob_q_withdraw(&q->state, ticks_between(&since, &now));
        Permit();
        if (gave_up) {
            qbuf = AllocVec(sizeof *qbuf, MEMF_PUBLIC | MEMF_CLEAR);
            resignal();
            return 0;
        }
    }
    resignal();
    return 1;
}

int ob_dt_at(ob_dt *t, WORD sx, WORD sy, char *out, int n)
{
    dt_query *q = qbuf;
    if (n > 0) out[0] = 0;
    if (!t || !q) return WB_AT_NONE;
    q->op = DQ_AT;
    q->sx = sx;
    q->sy = sy;
    if (!ask(t, q)) return WB_AT_NONE;
    if (n > 0) snprintf(out, (size_t)n, "%s", q->out);
    return q->at;
}

int ob_dt_show_name(ob_dt *t, const char *name)
{
    dt_query *q = qbuf;
    if (!t || !q || !name || strlen(name) >= sizeof q->out) return 0;
    q->op = DQ_SHOW;
    strcpy(q->out, name);
    return ask(t, q) ? q->at : 0;
}

int ob_dt_each(ob_dt *t, int (*fn)(void *ctx, const char *name, int type, int selected, WORD x, WORD y), void *ctx)
{
    dt_query *q = qbuf;
    if (!t || !q || !fn) return -1;
    q->op = DQ_EACH;
    q->fn = fn;
    q->ctx = ctx;
    return ask(t, q) ? q->at : -1;
}

void ob_dt_close_all(void)
{
    int i;
    closing_all = 1;
    for (i = 0; i < nlive; i++) ob_dt_close(live[i]);
    while (nlive) {
        ob_dt_service();
        if (!nlive) break;
        if (wait_half()) continue;
        for (i = 0; i < nlive; i++)
            if (held(live[i])) {
                ob_dt_abandon(live[i]);
                break;
            }
    }
    closing_all = 0;
}

/* While the desktop waits for a window: the drawer's questions about its own drawer (hidden, snapped) answered,
 * which it asks while it opens; every other call stays on the port, in its order, for the desktop's loop */
static void take_packs(void)
{
    struct List keep;
    struct Message *m;
    NewList(&keep);
    while ((m = GetMsg(callport))) {
        dt_call *c = (dt_call *)m;
        if (c->fn == CB_PACK) {
            c->result = run_call(c);
            if (env.gen) share_gen = env.gen();
            ReplyMsg(m);
        } else
            AddTail(&keep, &m->mn_Node);
    }
    Forbid();
    while ((m = (struct Message *)RemTail(&keep))) AddHead(&callport->mp_MsgList, &m->mn_Node);
    Permit();
}

int ob_dt_wait_open(ob_dt *t, int secs)
{
    int halves = secs * 2;
    while (live_at(t) >= 0 && !t->win) {
        take_packs();
        take_events();
        if (live_at(t) < 0 || t->win) break;
        if (!wait_half() && (--halves <= 0 || held(t))) break;
    }
    resignal();                                 /* what was left, for the desktop's loop */
    return live_at(t) >= 0 && t->win != NULL;
}

struct Window *ob_dt_window(const ob_dt *t)
{
    return t ? t->win : NULL;
}

const char *ob_dt_path(const ob_dt *t)
{
    return t ? t->path : "";
}

wb_drawer *ob_dt_drawer(const ob_dt *t)
{
    return t ? t->d : NULL;
}

struct Task *ob_dt_task(const ob_dt *t)
{
    return t && !t->gone ? t->task : NULL;
}

void ob_dt_abandon(ob_dt *t)
{
    struct Window *w;
    struct IntuiMessage *m, *next;
    if (!t) return;
    Forbid();
    t->gone = 1;
    w = t->win;
    t->win = NULL;
    t->d = NULL;
    if (w && w->UserPort) {             /* its messages answered, its port (the held task's) let go */
        for (m = (struct IntuiMessage *)w->UserPort->mp_MsgList.lh_Head;
             (next = (struct IntuiMessage *)m->ExecMessage.mn_Node.ln_Succ); m = next) {
            Remove(&m->ExecMessage.mn_Node);
            ReplyMsg(&m->ExecMessage);
        }
        w->UserPort = NULL;
        ModifyIDCMP(w, 0);
    }
    Permit();
    if (w) {
        ClearMenuStrip(w);
        CloseWindow(w);
    }
    unlink_dt(t);                       /* t itself stays: the held task's memory, as Suspend leaves it */
}

void ob_dt_set_user(ob_dt *t, unsigned long user)
{
    if (t) t->user = user;
}

unsigned long ob_dt_user(const ob_dt *t)
{
    return t ? t->user : 0;
}
