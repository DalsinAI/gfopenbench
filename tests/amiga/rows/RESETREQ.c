/* ---- C5a: the Workbench reset requester ----
 * A window of ours on the Workbench (a program's: no WFLG_WBENCHWINDOW), then ENV:Sys/screenmode.prefs written
 * with another depth, as the ScreenMode editor's Use does. Within 20 s (the refused try's two tells to our system
 * windows may take 3 s each) a new window titled "Workbench" (not the Workbench's own, not ours) must come up: the
 * requester. Our window closed, it must go within 12 s and the Workbench come back in the new depth. Then the
 * Cancel half: our window again, the old prefs written, the requester, Left-Amiga B (its last gadget, Cancel)
 * sent to it: it must go and the Workbench keep its depth. At the end our window closes and
 * CloseWorkBench/OpenWorkBench put the Workbench in the old prefs (held since the Cancel); refused, an INFO line
 * says so. The prefs file is put back as it was in every case.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <devices/input.h>
#include <devices/inputevent.h>
#include <dos/dosextens.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <dos/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "RESETREQ.h"

WBGAP_MODE(RESETREQ, wbt_resetreq, SCRIPT)
WBGAP_ROWMODE(C5a, RESETREQ)

#define RR_PREFS "ENV:Sys/screenmode.prefs"
#define RR_MAXW 48
#define RR_FILE 512

static int rr_depth(void)
{
    struct Screen *s = LockPubScreen((CONST_STRPTR)"Workbench");
    struct DrawInfo *d;
    int depth = -1;
    if (!s) return -1;
    if ((d = GetScreenDrawInfo(s))) { depth = d->dri_Depth; FreeScreenDrawInfo(s, d); }
    UnlockPubScreen(NULL, s);
    return depth;
}

/* the Workbench screen's windows now, up to RR_MAXW */
static int rr_snap(struct Window **w)
{
    struct Screen *s;
    struct Window *x;
    int n = 0;
    ULONG l = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && (s->Flags & SCREENTYPE) != WBENCHSCREEN; s = s->NextScreen)
        ;
    for (x = s ? s->FirstWindow : NULL; x && n < RR_MAXW; x = x->NextWindow) w[n++] = x;
    UnlockIBase(l);
    return n;
}

/* 1 for the requester's window: titled "Workbench" (es_Title; ours) or "System Request" (OS 3.2.3's IPrefs,
 * Legion's lab on #176) with a gadget whose text is "Cancel" */
static int rr_is_req(struct Window *x)
{
    struct Gadget *g;
    if (!x->Title || (strcmp((char *)x->Title, "Workbench") && strcmp((char *)x->Title, "System Request"))) return 0;
    for (g = x->FirstGadget; g; g = g->NextGadget)
        if (!(g->Flags & GFLG_LABELIMAGE) && g->GadgetText && g->GadgetText->IText
            && !strcmp((char *)g->GadgetText->IText, "Cancel"))
            return 1;
    return 0;
}

/* the Workbench screen's windows, one INFO line: title, flags, size (what is up at each step) */
static void rr_list(const char *when)
{
    static char buf[1200];
    struct Screen *s;
    struct Window *x;
    size_t k = 0;
    ULONG l = LockIBase(0);                       /* written to buf under the lock, printed after it */
    for (s = IntuitionBase->FirstScreen; s && (s->Flags & SCREENTYPE) != WBENCHSCREEN; s = s->NextScreen)
        ;
    buf[0] = 0;
    for (x = s ? s->FirstWindow : NULL; x && k < sizeof buf - 80; x = x->NextWindow)
        k += (size_t)snprintf(buf + k, sizeof buf - k, " [%.24s $%08lx %ldx%ld%s]", x->Title ? (char *)x->Title : "-",
                              (unsigned long)x->Flags, (long)x->Width, (long)x->Height, rr_is_req(x) ? " REQ" : "");
    UnlockIBase(l);
    printf("INFO C5a windows %s:%s\n", when, buf);
}

/* a window on the Workbench now that wasn't in before[], isn't the Workbench's own, isn't mine and has the
 * requester's title ("Workbench") and Cancel gadget: the refused try tells our system windows to close and open again, so they come
 * up new beside it (lab, 14 November: an untitled one was taken for it and got the Cancel); NULL if none */
static struct Window *rr_new(struct Window **before, int nb, struct Window *mine)
{
    struct Screen *s;
    struct Window *x, *found = NULL;
    int i;
    ULONG l = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && (s->Flags & SCREENTYPE) != WBENCHSCREEN; s = s->NextScreen)
        ;
    for (x = s ? s->FirstWindow : NULL; x && !found; x = x->NextWindow) {
        if (x == mine || (x->Flags & WFLG_WBENCHWINDOW) || !rr_is_req(x)) continue;
        for (i = 0; i < nb && before[i] != x; i++)
            ;
        if (i == nb) found = x;
    }
    UnlockIBase(l);
    return found;
}

/* 1 while w is a window on the Workbench screen */
static int rr_there(struct Window *w)
{
    struct Window *now[RR_MAXW];
    int n = rr_snap(now), i;
    for (i = 0; i < n; i++)
        if (now[i] == w) return 1;
    return 0;
}

/* the Workbench's windows still for 2 s (up to 12 s): after a reset our system windows (the tray, the dock,
 * GFOpenMeters) open again one by one, and one of them must not be taken for the requester */
static int rr_settle(struct Window **w)
{
    struct Window *b[RR_MAXW];
    int n = rr_snap(w), m, i, still = 0, t;
    for (t = 0; t < 60 && still < 10; t++) {
        Delay(10);
        m = rr_snap(b);
        for (i = 0; i < m && i < n && b[i] == w[i]; i++)
            ;
        if (m == n && i == n) still++;
        else { still = 0; n = m; for (i = 0; i < m; i++) w[i] = b[i]; }
    }
    return n;
}

static struct Window *rr_wait_new(struct Window **before, int nb, struct Window *mine, int secs)
{
    struct Window *r = NULL;
    int t;
    for (t = 0; t < secs * 5 && !(r = rr_new(before, nb, mine)); t++) Delay(10);
    return r;
}

/* a requester's window (the title "Workbench", not the Workbench's own) on the Workbench now, other than not; NULL if none */
static struct Window *rr_req_up(struct Window *not)
{
    struct Window *w[RR_MAXW];
    int n = rr_snap(w), i;
    for (i = 0; i < n; i++)
        if (w[i] != not && !(w[i]->Flags & WFLG_WBENCHWINDOW) && rr_is_req(w[i]))
            return w[i];
    return NULL;
}

static int rr_wait_gone(struct Window *w, int secs)
{
    int t;
    for (t = 0; t < secs * 5 && rr_there(w); t++) Delay(10);
    return !rr_there(w);
}

static int rr_write(const char *path, const unsigned char *buf, long n)
{
    BPTR f = Open((CONST_STRPTR)path, MODE_NEWFILE);
    long put;
    if (!f) return 0;
    put = Write(f, (APTR)buf, n);
    Close(f);
    return put == n;
}

static struct Window *rr_mine(void)
{
    return OpenWindowTags(NULL, WA_Title, (ULONG)"WBGapC5a", WA_Left, 40, WA_Top, 40, WA_Width, 220, WA_Height, 60,
                          WA_DragBar, TRUE, WA_DepthGadget, TRUE, WA_IDCMP, 0, WA_PubScreenName, (ULONG)"Workbench",
                          TAG_END);
}

/* The window of this process's console when it is on the Workbench screen: WBGapTest.script's Shell. Asked of the
 * console handler (ACTION_DISK_INFO gives its window in id_VolumeNode, as the ROM's console does). NULL with no
 * console window (the Cradle agent's amiga_run) or one on another screen. */
static struct Window *rr_runner(void)
{
    struct Process *me = (struct Process *)FindTask(NULL);
    struct InfoData *id;
    struct Window *w = NULL, *x;
    struct Screen *s;
    if (me->pr_Task.tc_Node.ln_Type != NT_PROCESS || !me->pr_ConsoleTask) return NULL;
    if (!(id = AllocVec(sizeof *id, MEMF_PUBLIC | MEMF_CLEAR))) return NULL;
    if (DoPkt((struct MsgPort *)me->pr_ConsoleTask, ACTION_DISK_INFO, MKBADDR(id), 0, 0, 0, 0))
        w = (struct Window *)id->id_VolumeNode;
    FreeVec(id);
    if (w) {                                      /* only a window that is on the Workbench screen now */
        ULONG l = LockIBase(0);
        for (s = IntuitionBase->FirstScreen; s && (s->Flags & SCREENTYPE) != WBENCHSCREEN; s = s->NextScreen)
            ;
        for (x = s ? s->FirstWindow : NULL; x && x != w; x = x->NextWindow)
            ;
        UnlockIBase(l);
        if (!x) w = NULL;
    }
    return w;
}

/* Left-Amiga B, down and up, through input.device: the requester's last gadget */
static void rr_cancel(struct Window *req)
{
    struct MsgPort *p = CreateMsgPort();
    struct IOStdReq *io = p ? (struct IOStdReq *)CreateIORequest(p, sizeof *io) : NULL;
    struct InputEvent ie;
    int up;
    ActivateWindow(req);
    Delay(10);
    if (io && !OpenDevice((CONST_STRPTR)"input.device", 0, (struct IORequest *)io, 0)) {
        for (up = 0; up < 2; up++) {
            memset(&ie, 0, sizeof ie);
            ie.ie_Class = IECLASS_RAWKEY;
            ie.ie_Code = 0x35 | (up ? IECODE_UP_PREFIX : 0);
            ie.ie_Qualifier = IEQUALIFIER_LCOMMAND;
            io->io_Command = IND_WRITEEVENT;
            io->io_Data = &ie;
            io->io_Length = sizeof ie;
            DoIO((struct IORequest *)io);
            Delay(2);
        }
        CloseDevice((struct IORequest *)io);
    }
    if (io) DeleteIORequest((struct IORequest *)io);
    if (p) DeleteMsgPort(p);
}

/* Any reset requester still up after a while (the prefs put back may owe a reset of their own): Cancel on it, so
 * none is left on the Workbench for the rows after this one; with no timeout it would wait there for ever */
static void rr_none_left(int secs)
{
    struct Window *req;
    int t;
    for (t = 0; t < secs * 5; t++) {
        if ((req = rr_req_up(NULL))) {
            printf("INFO C5a a requester left up: Cancel on it\n");
            rr_cancel(req);
            rr_wait_gone(req, 3);
            return;
        }
        Delay(10);
    }
}

int wbt_resetreq(int argc, char **argv)
{
    static unsigned char was[RR_FILE], now[RR_FILE];
    struct wbgap_resetreq r;
    struct Window *before[RR_MAXW], *mine, *req;
    char why[200], ctitle[40] = "";
    long n;
    int nb, old = 0, v;
    (void)argc, (void)argv;
    memset(&r, 0, sizeof r);
    n = wbt_read_all(RR_PREFS, (char *)was, sizeof was);
    memcpy(now, was, n > 0 ? (size_t)n : 0);
    r.depth_before = rr_depth();
    if (n <= 0 || r.depth_before < 1 || !wbgap_smod_depth(now, n, wbgap_other_depth(r.depth_before), &old)) {
        r.no_prefs = 1;
        v = wbgap_resetreq_judge(&r, why, sizeof why);
        wbt_say("C5a", v, why);
        return 0;
    }
    r.depth_want = wbgap_other_depth(r.depth_before);
    printf("INFO C5a the Workbench at depth %d (prefs %d), the change asks for %d\n", r.depth_before, old, r.depth_want);

    /* the reset owed with our window in its way: the requester; then our window closed: it goes, the reset made */
    rr_list("at the start");
    if (rr_runner()) {
        /* WBGapTest.script's Shell window stays in the reset's way, so the requester can't go by itself: it must
         * come up, and Cancel take it down with the depth kept; the reset half is for a run with no Shell window */
        r.runner = 1;
        printf("INFO C5a this Shell's window is on the Workbench: the requester is answered with Cancel\n");
        if ((mine = rr_mine())) {
            nb = rr_settle(before);
            rr_write(RR_PREFS, now, n);
            req = rr_wait_new(before, nb, mine, 20);
            r.shown = req != NULL;
            if (req) {                            /* Cancel with our window still up, as the Cancel half does */
                if (rr_there(req)) rr_cancel(req);
                r.cancel_gone = rr_wait_gone(req, 3);
            }
            CloseWindow(mine);                    /* after a Cancel no reset is owed: 5 s, the depth must stay */
            Delay(250);
            r.depth_cancel = rr_depth();
            printf("INFO C5a %s; depth %d\n", !req ? "no requester in 20 s" : r.cancel_gone ? "Cancel took the requester down"
                                                                                         : "the requester stayed after Cancel",
                   r.depth_cancel);
        }
        rr_write(RR_PREFS, was, n);               /* the prefs as they were */
        rr_none_left(20);
        if (rr_req_up(NULL)) printf("INFO C5a a requester is still up\n");
        v = wbgap_resetreq_judge(&r, why, sizeof why);
        wbt_say("C5a", v, why);
        return wbt_fails() ? 5 : 0;
    }
    if ((mine = rr_mine())) {
        nb = rr_settle(before);
        rr_write(RR_PREFS, now, n);
        req = rr_wait_new(before, nb, mine, 20);
        r.shown = req != NULL;
        printf("INFO C5a %s%s%s\n", req ? "a requester came up: " : "no requester in 20 s",
               req && req->Title ? (char *)req->Title : "", req && !req->Title ? "(no title)" : "");
        CloseWindow(mine);
        r.gone = req ? rr_wait_gone(req, 12) : 0;
        Delay(100);                                   /* the reset's own reopen */
        r.depth_after = rr_depth();
        printf("INFO C5a after our window closed: the requester %s, depth %d\n",
               !req ? "never up" : r.gone ? "gone" : "still there", r.depth_after);
    }

    /* the Cancel half: the old prefs back, our window in the way, Cancel: the depth kept */
    if (r.shown && r.gone && r.depth_after == r.depth_want && (mine = rr_mine())) {
        r.cancel_tried = 1;
        nb = rr_settle(before);
        rr_list("before the Cancel half's change");
        rr_write(RR_PREFS, was, n);
        req = rr_wait_new(before, nb, mine, 20);
        r.cancel_shown = req != NULL;
        if (req) {
            ctitle[0] = 0;
            if (req->Title) { strncpy(ctitle, (char *)req->Title, sizeof ctitle - 1); ctitle[sizeof ctitle - 1] = 0; }
            rr_cancel(req);
            r.cancel_gone = rr_wait_gone(req, 3);
        }
        {   /* what follows the Cancel, each half second for 3 s, then 5 s after our window closed: for the log */
            int t, d0 = rr_depth();
            for (t = 0; t < 6; t++) {
                struct Window *again = rr_req_up(req);
                if (again || rr_depth() != d0) {
                    printf("INFO C5a %d.%d s after Cancel: %s, depth %d\n", t / 2, (t % 2) * 5,
                           again ? "a requester up again" : "no requester", rr_depth());
                    break;
                }
                Delay(25);
            }
        }
        CloseWindow(mine);
        {
            int t, d0 = rr_depth();
            for (t = 0; t < 10; t++) {
                Delay(25);                            /* 5 s: no reset may come now */
                if (rr_depth() != d0) {
                    printf("INFO C5a %d.%d s after our window closed: depth %d\n", (t + 1) / 2, ((t + 1) % 2) * 5,
                           rr_depth());
                    break;
                }
            }
            while (++t < 10) Delay(25);
        }
        r.depth_cancel = rr_depth();
        printf("INFO C5a Cancel: requester %s (%s), %s; depth %d\n", req ? "up" : "not up",
               req ? ctitle : "-", r.cancel_gone ? "gone" : "not gone", r.depth_cancel);
    }

    rr_write(RR_PREFS, was, n);                       /* the prefs as they were, every way */
    if (!r.shown || !r.gone) rr_none_left(5);         /* a requester the row lost track of: not left up */
    if (rr_depth() != r.depth_before) {               /* the Workbench in them: held since the Cancel */
        int closed = CloseWorkBench() != 0;
        if (closed) OpenWorkBench();
        printf("INFO C5a %s\n", closed ? "CloseWorkBench/OpenWorkBench: back in the old prefs"
                                       : "CloseWorkBench refused: the Workbench stays in the changed depth until a reset");
    }
    v = wbgap_resetreq_judge(&r, why, sizeof why);
    wbt_say("C5a", v, why);
    return wbt_fails() ? 5 : 0;
}
