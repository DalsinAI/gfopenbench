/* ob_recover: the Recover deleted window (Dale 13 Nov 2026), drawn through GFOpenLayout UI.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/semaphores.h>
#include <intuition/intuition.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "gfopenlayout_ui.h"
#include "gfopenlayout_ui_amiga.h"
#include "of_ops.h"
#include "of_path.h"
#include "of_fs_amiga.h"
#include "ob_recover_core.h"
#include "ob_recover.h"

/* gfopenlayout.library through its link calls, opened here when the window opens rather than before main(), so
 * GFOpenBench runs without it (the window then says so) */
struct Library *OpenLayoutBase;
#define OL_LIB_NAME "gfopenlayout.library"
#define OL_LIB_VERSION 10

#define SIG_RELOAD SIGBREAKF_CTRL_E             /* show rw.vol's files */
#define SIG_BACK SIGBREAKF_CTRL_F               /* Icons > Put back */
#define MAX_ROWS 256                            /* newest first: the rest stay kept */

static const char markup[] =
    "<list id=files style='flex:1'></list>"
    "<label id=status> </label>"
    "<row style='justify-content:end'><button id=back>Put back</button><button id=close>Close</button></row>";
static const char css[] = "button{width:90px} #status{width:auto}";

static struct {
    struct SignalSemaphore sem;
    int ready;
    struct Task *task;                          /* the window's process, while it runs */
    char vol[OF_PATH_MAX];
    struct Screen *scr;                         /* GFOpenBench's screen: open while the window is (ob_recover_close) */
} rw;

typedef struct win {
    olui *ui;
    of_ctx c;
    of_fs fs;
    char vol[OF_PATH_MAX];
    uint32_t id[MAX_ROWS];
    int n;
    int done;
} win;

static void sem_init(void)
{
    if (!rw.ready) {
        InitSemaphore(&rw.sem);
        rw.ready = 1;
    }
}

static int add_row(void *u, uint32_t id, const char *row)
{
    win *w = u;
    char v[12];
    if (w->n >= MAX_ROWS) return 1;
    snprintf(v, sizeof v, "%d", w->n);
    w->id[w->n++] = id;
    return !olui_list_add(w->ui, olui_find(w->ui, "files"), NULL, row, v);
}

static void fill(win *w)
{
    char msg[OF_PATH_MAX + 128];
    olui_el *list = olui_find(w->ui, "files");
    olui_list_clear(w->ui, list);
    w->n = 0;
    ob_rc_fill(&w->c, w->vol, add_row, w, msg, sizeof msg);
    olui_set_text(w->ui, olui_find(w->ui, "status"), msg);
    olui_set_disabled(w->ui, olui_find(w->ui, "back"), 1);
}

static void put_back(win *w)
{
    char msg[OF_PATH_MAX + 128];
    olui_el *it = olui_list_selected(olui_find(w->ui, "files"));
    const char *v = it ? olui_get_value(it) : NULL;
    int i = v ? atoi(v) : -1;
    if (i < 0 || i >= w->n) {
        DisplayBeep(NULL);
        return;
    }
    if (ob_rc_put_back(&w->c, w->vol, w->id[i], msg, sizeof msg) == OF_OK) {
        fill(w);                                /* it is no longer kept */
    }
    olui_set_text(w->ui, olui_find(w->ui, "status"), msg);
}

static void on_select(olui *ui, const olui_event *ev, void *u)
{
    (void)ev;
    olui_set_disabled(ui, olui_find(ui, "back"), !olui_list_selected(olui_find(ui, "files")));
    (void)u;
}
static void on_back(olui *ui, const olui_event *ev, void *u) { (void)ui; (void)ev; put_back(u); }
static void on_close(olui *ui, const olui_event *ev, void *u) { (void)ui; (void)ev; ((win *)u)->done = 1; }

static void set_title(struct Window *wn, win *w)
{
    static char title[OF_PATH_MAX + 24];
    snprintf(title, sizeof title, "Recover deleted: %s", w->vol);
    SetWindowTitles(wn, (CONST_STRPTR)title, (CONST_STRPTR)~0);
}

static void recover_entry(void)
{
    struct Screen *scr;
    struct Window *wn = NULL;
    olui_amiga *a = NULL;
    win *w;
    char err[128];
    if (!(w = AllocVec(sizeof *w, MEMF_ANY | MEMF_CLEAR))) goto out;
    ObtainSemaphore(&rw.sem);
    of_strlcpy(w->vol, rw.vol, sizeof w->vol);
    scr = rw.scr;
    ReleaseSemaphore(&rw.sem);
    if (!scr) goto out;
    of_fs_amiga_init(&w->fs);
    of_ctx_init(&w->c, &w->fs);
    ((struct Process *)FindTask(NULL))->pr_WindowPtr = (APTR)-1;
    if (!OpenLayoutBase && !(OpenLayoutBase = OpenLibrary((CONST_STRPTR)OL_LIB_NAME, OL_LIB_VERSION))) {
        struct EasyStruct es = { sizeof(struct EasyStruct), 0, (UBYTE *)"Workbench",
                                 (UBYTE *)"Recover deleted needs gfopenlayout.library %ld or later in LIBS:.",
                                 (UBYTE *)"OK" };
        /* Built, not EasyRequest: ob_recover_close's CTRL-C takes it down with the desktop, so it never outlives
         * desktop_down (an EasyRequest would block this process until someone clicked OK). */
        ULONG args[1];
        struct Window *req;
        args[0] = (ULONG)OL_LIB_VERSION;
        req = BuildEasyRequestArgs(NULL, &es, 0, args);
        if (req && req != (struct Window *)1) {
            for (;;) {
                ULONG got = Wait((1UL << req->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C);
                if (got & SIGBREAKF_CTRL_C) break;
                if (SysReqHandler(req, NULL, FALSE) != -2) break;
            }
        }
        if (req) FreeSysRequest(req);
        goto out;
    }
    wn = OpenWindowTags(NULL, WA_CustomScreen, (ULONG)scr, WA_Title, (ULONG)"Recover deleted",
                        WA_Width, 460, WA_Height, 240, WA_MinWidth, 280, WA_MinHeight, 140,
                        WA_MaxWidth, ~0UL, WA_MaxHeight, ~0UL, WA_DragBar, TRUE, WA_DepthGadget, TRUE,
                        WA_CloseGadget, TRUE, WA_SizeGadget, TRUE, WA_SizeBBottom, TRUE, WA_Activate, TRUE,
                        WA_SimpleRefresh, TRUE, WA_ReportMouse, TRUE, WA_IDCMP, olui_amiga_idcmp(), TAG_DONE);
    if (!wn || !(w->ui = olui_new()) || !(a = olui_amiga_new(w->ui, wn))) goto out;
    olui_amiga_theme(a, NULL);
    if (!olui_load(w->ui, markup, css, err, sizeof err)) goto out;
    olui_on(w->ui, "files", OLUI_SELECT, on_select, w);
    olui_on(w->ui, "files", OLUI_ACTIVATE, on_back, w);
    olui_on(w->ui, "back", OLUI_CLICK, on_back, w);
    olui_on(w->ui, "close", OLUI_CLICK, on_close, w);
    set_title(wn, w);
    fill(w);
    if (!olui_amiga_layout(a)) goto out;
    while (!w->done) {
        struct IntuiMessage *m;
        ULONG got = Wait((1UL << wn->UserPort->mp_SigBit) | SIGBREAKF_CTRL_C | SIG_RELOAD | SIG_BACK);
        if (got & SIGBREAKF_CTRL_C) break;
        if (got & SIG_RELOAD) {
            ObtainSemaphore(&rw.sem);
            of_strlcpy(w->vol, rw.vol, sizeof w->vol);
            ReleaseSemaphore(&rw.sem);
            set_title(wn, w);
            fill(w);
            WindowToFront(wn);
            ActivateWindow(wn);
        }
        if (got & SIG_BACK) put_back(w);
        while ((m = (struct IntuiMessage *)GetMsg(wn->UserPort))) {
            if (olui_amiga_event(a, m) == OLUI_AMIGA_CLOSE) w->done = 1;
            ReplyMsg(&m->ExecMessage);
        }
        olui_amiga_draw(a);
    }
out:
    if (a) olui_amiga_free(a);
    if (w && w->ui) olui_free(w->ui);
    if (wn) CloseWindow(wn);
    if (w) FreeVec(w);
    Forbid();                                   /* rw.task cleared as this process ends, so a new one can start */
    rw.task = NULL;
}

int ob_recover_open(const char *vol, struct Screen *scr)
{
    struct Process *p;
    if (!vol || !scr) return 0;
    sem_init();
    ObtainSemaphore(&rw.sem);
    of_strlcpy(rw.vol, vol, sizeof rw.vol);
    rw.scr = scr;
    if (rw.task) {
        Signal(rw.task, SIG_RELOAD);
        ReleaseSemaphore(&rw.sem);
        return 1;
    }
    ReleaseSemaphore(&rw.sem);
    Forbid();
    p = CreateNewProcTags(NP_Entry, (ULONG)recover_entry, NP_StackSize, 32768, NP_Name,
                          (ULONG)"GFOpenBench recover deleted", TAG_DONE);
    rw.task = p ? &p->pr_Task : NULL;
    Permit();
    return p != NULL;
}

int ob_recover_put_back(void)
{
    int ok;
    sem_init();
    Forbid();
    if ((ok = rw.task != NULL)) Signal(rw.task, SIG_BACK);
    Permit();
    return ok;
}

void ob_recover_close(void)
{
    int i;
    sem_init();
    Forbid();
    if (rw.task) Signal(rw.task, SIGBREAKF_CTRL_C);
    Permit();
    for (i = 0; i < 250 && rw.task; i++) Delay(2);      /* up to 10 s */
    if (!rw.task && OpenLayoutBase) {
        CloseLibrary(OpenLayoutBase);
        OpenLayoutBase = NULL;
    }
}
