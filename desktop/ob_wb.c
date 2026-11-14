/* ob_wb: GFOpenBench as the Workbench that workbench.library knows (GFOpenBench gap item 4, rows A3, A4, A6 and A8;
 * Dale, 12 November 2026: "we need to close this").
 *
 * On the open ROM, workbench.library is a library beside GFOpenBench (amicradle-guest libraries/openworkbench).
 * Until a desktop registers with it, it answers as OS 3.2.3's does with no Workbench running: AddAppIconA and
 * AddAppMenuItemA give NULL, and OpenWorkbenchObjectA and the other calls that need one give FALSE, IoErr 103. So
 * GFOpenBench registers its port (the Team's OWBCTRLA_Desktop, ob_wb.h) and then:
 *   - answers OpenWorkbenchObjectA, CloseWorkbenchObjectA, MakeWorkbenchObjectVisibleA, ChangeWorkbenchSelectionA
 *     and WhichWorkbenchObjectA, which come to its port while the program waits;
 *   - shows programs' AppIcons on the desktop and their AppMenuItems in the Tools menu (openbench.c reads them here
 *     when the library signals a change), and sends their AppMessages: a double-click or Open, icons let go on an
 *     AppIcon, a Tools item picked.
 * Every read of the library's AppIcons and AppMenuItems, and every message to one, is made while the library holds
 * them (OWBCTRLA_HoldAppObjects), so a program removing its object waits and nothing is freed under GFOpenBench.
 *
 * Clean room: from the NDK 3.2 autodoc (wb.doc) and workbench/workbench.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <workbench/icon.h>
#include <utility/hooks.h>
#include <utility/tagitem.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/wb.h>
#include <proto/icon.h>

#include "ob_wb.h"
#include "ob_port.h"
#include "ob_wbplan.h"

/* ob_wbplan's numbers are workbench.h's */
typedef char check_ism[ISMACTION_Unselect == OB_WBP_UNSELECT && ISMACTION_Select == OB_WBP_SELECT &&
                       ISMACTION_Ignore == OB_WBP_IGNORE && ISMACTION_Stop == OB_WBP_STOP ? 1 : -1];

static ob_wb_env env;
static struct MsgPort *req_port;                /* registered with workbench.library: its calls come here */
static struct MsgPort *app_port;                /* the AppMessages we send, back when their programs reply */
static int apps_out;
static int registered;

static int wb_ok(void)
{
    return WorkbenchBase && WorkbenchBase->lib_Version >= 44;
}

static void control(ULONG tag, ULONG data)
{
    struct TagItem t[2];
    t[0].ti_Tag = tag;
    t[0].ti_Data = data;
    t[1].ti_Tag = TAG_DONE;
    t[1].ti_Data = 0;
    WorkbenchControlA(NULL, t);
}

/* ---- the calls programs make ---- */

/* A program's hook, as utility.library's CallHookPkt calls it: a0 the hook, a2 the object, a1 the message, through
 * h_Entry (without needing utility.library open here) */
typedef char check_hook_entry[offsetof(struct Hook, h_Entry) == 8 ? 1 : -1];

static LONG call_hook(struct Hook *h, APTR obj, APTR msg)
{
    register LONG d0 __asm("d0");
    register struct Hook *a0 __asm("a0") = h;
    register APTR a1 __asm("a1") = msg;
    register APTR a2 __asm("a2") = obj;
    __asm__ __volatile__("move.l 8(%%a0),%%a3\n\tjsr (%%a3)"
                         : "=r"(d0), "+r"(a0), "+r"(a1) : "r"(a2) : "d1", "a3", "cc", "memory");
    return d0;
}

typedef struct sel_ctx {
    struct Hook *hook;
    struct IconSelectMsg ism;
} sel_ctx;

static int select_one(void *c, const char *name, int type, int selected, WORD x, WORD y)
{
    sel_ctx *s = c;
    LONG r;
    s->ism.ism_Name = (STRPTR)name;
    s->ism.ism_Type = (UWORD)type;
    s->ism.ism_Selected = selected ? TRUE : FALSE;
    s->ism.ism_Left = x;
    s->ism.ism_Top = y;
    r = call_hook(s->hook, NULL, &s->ism);
    return ob_wbp_hook_answer(r);
}

/* OpenWorkbenchObjectA's arguments (row I11 through LAUNCH; Kill Crazy's and Ivanova's 0.1.60 runs, 13 November
 * 2026: LaunchT got 1 argument, not LaunchT, p1, p2): each WBOPENA_ArgName, in the drawer of the WBOPENA_ArgLock
 * before it, as one path, in the order given. The tags are the caller's, read while it waits. How many (at most
 * max), each path in its own OB_WB_ARGPATH of buf. */
#define OB_WB_ARGPATH 512
static int open_args(struct TagItem *t, char *buf, const char **args, int max)
{
    char dir[OB_WB_ARGPATH];
    int n = 0;
    dir[0] = 0;
    while (t && n < max) {
        switch (t->ti_Tag) {
        case TAG_DONE: t = NULL; continue;
        case TAG_MORE: t = (struct TagItem *)t->ti_Data; continue;
        case TAG_SKIP: t += 1 + t->ti_Data; continue;
        case WBOPENA_ArgLock:
            if (!t->ti_Data || !NameFromLock((BPTR)t->ti_Data, (STRPTR)dir, sizeof dir)) dir[0] = 0;
            break;
        case WBOPENA_ArgName:
            if (ob_wbp_arg_path(dir, (const char *)t->ti_Data, buf + n * OB_WB_ARGPATH, OB_WB_ARGPATH)) {
                args[n] = buf + n * OB_WB_ARGPATH;
                n++;
            }
            break;
        }
        t++;
    }
    return n;
}

#define OB_WB_MAXARGS 32
static int open_with_args(struct OWBRequest *rq)
{
    const char *args[OB_WB_MAXARGS];
    char *buf;
    int n, ok = 0;
    if (!rq->or_Tags || !env.open_args) return -1;
    if (!(buf = AllocVec(OB_WB_MAXARGS * OB_WB_ARGPATH, MEMF_ANY))) return -1;
    n = open_args(rq->or_Tags, buf, args, OB_WB_MAXARGS);
    if (n > 0) ok = env.open_args((const char *)rq->or_Name, args, n);
    FreeVec(buf);
    return n > 0 ? ok : -1;                         /* -1: no arguments, opened as usual */
}

/* WhichWorkbenchObjectA's tags, the caller's (it waits): WBOBJA_Type gets the type, WBOBJA_Name the name, at
 * most WBOBJA_NameSize - 1 characters (64 when the size isn't given). Only the tags workbench.h names. */
static void which_tags(struct TagItem *t, ULONG type, const char *name)
{
#ifdef WBOBJA_Type
    ULONG size = 64;
    struct TagItem *s;
#ifdef WBOBJA_NameSize
    for (s = t; s; ) {
        if (s->ti_Tag == TAG_DONE) break;
        if (s->ti_Tag == TAG_MORE) { s = (struct TagItem *)s->ti_Data; continue; }
        if (s->ti_Tag == TAG_SKIP) { s += 1 + s->ti_Data; continue; }
        if (s->ti_Tag == WBOBJA_NameSize) size = s->ti_Data;
        s++;
    }
#endif
    for (s = t; s; ) {
        if (s->ti_Tag == TAG_DONE) break;
        if (s->ti_Tag == TAG_MORE) { s = (struct TagItem *)s->ti_Data; continue; }
        if (s->ti_Tag == TAG_SKIP) { s += 1 + s->ti_Data; continue; }
        if (s->ti_Tag == WBOBJA_Type && s->ti_Data) *(ULONG *)s->ti_Data = type;
#ifdef WBOBJA_Name
        if (s->ti_Tag == WBOBJA_Name && s->ti_Data && size > 0) {
            ULONG k = strlen(name);
            if (k >= size) k = size - 1;
            memcpy((void *)s->ti_Data, name, k);
            ((char *)s->ti_Data)[k] = 0;
        }
#endif
        s++;
    }
#else
    (void)t; (void)type; (void)name;
#endif
}

static void answer(struct OWBRequest *rq)
{
    char dir[512];
    const char *name;
    struct Window *w;
    rq->or_Result = FALSE;
    rq->or_IoErr = ERROR_OBJECT_NOT_FOUND;
    switch (rq->or_Call) {
    case OWBREQ_OPEN: {
        int r;
        if (!rq->or_Name) break;
        if ((r = open_with_args(rq)) < 0) r = env.open && env.open((const char *)rq->or_Name);
        if (r) rq->or_Result = TRUE;
        break;
    }
    case OWBREQ_CLOSE:
        if (rq->or_Name && env.close && env.close((const char *)rq->or_Name)) rq->or_Result = TRUE;
        break;
    case OWBREQ_VISIBLE:
        /* an icon in an open drawer's window, scrolled to; a disk's is always on the desktop */
        if (!rq->or_Name) break;
        name = ob_wbp_split((const char *)rq->or_Name, dir, sizeof dir);
        if (!*name) {
            BPTR l = Lock(rq->or_Name, ACCESS_READ);
            if (l) { UnLock(l); rq->or_Result = TRUE; }
        } else if (env.show_name && env.show_name(dir, name))
            rq->or_Result = TRUE;
        break;
    case OWBREQ_SELECT: {
        /* each icon of an open drawer's window handed to the program's hook, which says what becomes of it */
        sel_ctx s;
        if (!rq->or_Name || !rq->or_Hook || !env.window || !env.each) break;
        if (!(w = env.window((const char *)rq->or_Name))) break;
        memset(&s, 0, sizeof s);
        s.hook = rq->or_Hook;
        s.ism.ism_Length = sizeof s.ism;
        s.ism.ism_Drawer = Lock(rq->or_Name, ACCESS_READ);
        s.ism.ism_Tags = rq->or_Tags;
        s.ism.ism_DrawerWindow = s.ism.ism_ParentWindow = w;
        if (s.ism.ism_DrawerWindow) {
            s.ism.ism_Width = s.ism.ism_DrawerWindow->Width;
            s.ism.ism_Height = s.ism.ism_DrawerWindow->Height;
        }
        env.each((const char *)rq->or_Name, select_one, &s);
        if (s.ism.ism_Drawer) UnLock(s.ism.ism_Drawer);
        rq->or_Result = TRUE;
        break;
    }
    case OWBREQ_UPDATE: {
        /* UpdateWorkbench: the drawer read again before the program goes on (gap row A7); no lock: the disks */
        dir[0] = 0;
        if (rq->or_Lock && !NameFromLock(rq->or_Lock, (STRPTR)dir, sizeof dir)) break;
        if (env.reread) env.reread(dir);
        rq->or_Result = TRUE;
        break;
    }
    case OWBREQ_INFO: {
        /* WBInfo: the Information window for name in the lock's drawer (gap rows A9, N8) */
        char path[512];
        if (!rq->or_Lock || !NameFromLock(rq->or_Lock, (STRPTR)dir, sizeof dir)) break;
        if (!ob_wbp_join(dir, rq->or_Name ? (const char *)rq->or_Name : "", path, sizeof path)) break;
        if (env.info && env.info(path)) rq->or_Result = TRUE;
        break;
    }
    case OWBREQ_WHICH: {
        /* gap row A8c: the icon under the place (WBO_ICON), the drawer's own space (WBO_DRAWER), else WBO_NONE, as
         * OS 3.2.3 answers for a place with no icon; the type and the name also go where the caller's tags say */
        char path[512], nm[108];
        int at = rq->or_Window && env.which ? env.which(rq->or_Window, rq->or_X, rq->or_Y, path, sizeof path)
                                            : OB_WBP_AT_NONE;
        rq->or_Result = ob_wbp_which(at, path, nm, sizeof nm);
        which_tags(rq->or_Tags, (ULONG)rq->or_Result, nm);
        rq->or_IoErr = 0;
        break;
    }
    }
}

/* ---- start, signals, stop ---- */

int ob_wb_start(const ob_wb_env *e)
{
    env = *e;
    if (!wb_ok()) return 0;
    if (!(req_port = ob_port_new())) return 0;
    if (!(app_port = ob_port_new())) {
        ob_port_free(req_port);
        req_port = NULL;
        return 0;
    }
    control(OWBCTRLA_Desktop, (ULONG)req_port);
    registered = 1;
    return 1;
}

ULONG ob_wb_sigmask(void)
{
    return (req_port ? 1UL << req_port->mp_SigBit : 0) | (app_port ? 1UL << app_port->mp_SigBit : 0);
}

static void app_free(struct AppMessage *am)
{
    LONG i;
    for (i = 0; i < am->am_NumArgs; i++)
        if (am->am_ArgList[i].wa_Lock) UnLock(am->am_ArgList[i].wa_Lock);
    FreeVec(am);
}

void ob_wb_handle(void)
{
    struct Message *m;
    ULONG notes = 0;
    while (req_port && (m = GetMsg(req_port))) {
        answer((struct OWBRequest *)m);
        ReplyMsg(m);
    }
    while (app_port && (m = GetMsg(app_port))) {
        app_free((struct AppMessage *)m);
        apps_out--;
    }
    if (registered) {
        control(OWBCTRLA_TakeNotes, (ULONG)&notes);
        if (notes && env.changed) env.changed(notes);
    }
}

void ob_wb_stop(void)
{
    struct Message *m;
    int i;
    if (!req_port) return;
    if (registered) control(OWBCTRLA_Desktop, 0);   /* no call is put after this */
    registered = 0;
    while ((m = GetMsg(req_port))) {                /* those put before: no Workbench any more */
        struct OWBRequest *rq = (struct OWBRequest *)m;
        rq->or_Result = FALSE;
        rq->or_IoErr = ERROR_OBJECT_NOT_FOUND;
        ReplyMsg(m);
    }
    ob_port_free(req_port);
    req_port = NULL;
    for (i = 0; i < 500 && apps_out; i++) {         /* the programs get ten seconds to answer */
        while ((m = GetMsg(app_port))) {
            app_free((struct AppMessage *)m);
            apps_out--;
        }
        if (apps_out) Delay(1);
    }
    if (!apps_out) ob_port_free(app_port);
    else app_port->mp_Flags = PA_IGNORE;            /* left: a late reply goes to a port still there */
    app_port = NULL;
}

/* ---- AppIcons and AppMenuItems ---- */

static void copy_label(char *out, int n, CONST_STRPTR s)
{
    snprintf(out, (size_t)n, "%s", s ? (const char *)s : "");
}

int ob_wb_icons(ob_wb_icon *out, int max)
{
    struct OWBAppObject q;
    int n = 0;
    if (!registered) return 0;
    control(OWBCTRLA_HoldAppObjects, TRUE);
    memset(&q, 0, sizeof q);
    q.ao_Kind = OWB_AO_APPICON;
    for (;;) {
        control(OWBCTRLA_NextAppObject, (ULONG)&q);
        if (!q.ao_Handle || n >= max) break;
        out[n].handle = q.ao_Handle;
        copy_label(out[n].label, sizeof out[n].label, q.ao_Text);
        /* the autodoc: Workbench copies the DiskObject's imagery, so the program may free its own */
        out[n].dobj = q.ao_DiskObj && IconBase->lib_Version >= 44
                    ? DupDiskObjectA((struct DiskObject *)q.ao_DiskObj, NULL) : NULL;
        n++;
    }
    control(OWBCTRLA_HoldAppObjects, FALSE);
    return n;
}

int ob_wb_menus(ob_wb_menu *out, int max)
{
    struct OWBAppObject q;
    int n = 0;
    if (!registered) return 0;
    control(OWBCTRLA_HoldAppObjects, TRUE);
    memset(&q, 0, sizeof q);
    q.ao_Kind = OWB_AO_APPMENU;
    for (;;) {
        control(OWBCTRLA_NextAppObject, (ULONG)&q);
        if (!q.ao_Handle || n >= max) break;
        out[n].handle = q.ao_Handle;
        out[n].parent = q.ao_Parent;
        copy_label(out[n].label, sizeof out[n].label, q.ao_Text);
        out[n].key[0] = (char)q.ao_CommandKey;
        out[n].key[1] = 0;
        out[n].role = q.ao_Role;
        out[n].separator = q.ao_Separator;
        out[n].visible = q.ao_Visible;
        n++;
    }
    control(OWBCTRLA_HoldAppObjects, FALSE);
    return n;
}

static int is_dir(const char *path)
{
    struct FileInfoBlock *fib;
    BPTR l;
    int r = 0;
    if (!(l = Lock((CONST_STRPTR)path, ACCESS_READ))) return 0;
    if ((fib = AllocDosObject(DOS_FIB, NULL))) {
        if (Examine(l, fib)) r = fib->fib_DirEntryType > 0;
        FreeDosObject(DOS_FIB, fib);
    }
    UnLock(l);
    return r;
}

int ob_wb_send(APTR handle, int kind, const char *const *paths, int n, WORD mx, WORD my, UWORD amclass)
{
    struct OWBAppObject q;
    struct AppMessage *am;
    struct WBArg *args;
    char *names, parent[512];
    ULONG size;
    int i, k = 0, sent = 0;
    if (!registered || !app_port || !handle) return 0;
    if (n < 0) n = 0;
    size = sizeof *am + (ULONG)n * sizeof *args;
    for (i = 0; i < n; i++) size += strlen((const char *)FilePart((CONST_STRPTR)paths[i])) + 1;
    if (!(am = AllocVec(size, MEMF_PUBLIC | MEMF_CLEAR))) return 0;
    args = (struct WBArg *)(am + 1);
    names = (char *)(args + n);
    am->am_ArgList = n ? args : NULL;
    for (i = 0; i < n; i++) {
        const char *name = is_dir(paths[i]) ? "" : (const char *)FilePart((CONST_STRPTR)paths[i]);
        BPTR l;
        if (*name) {
            ob_wbp_split(paths[i], parent, sizeof parent);
            l = Lock((CONST_STRPTR)parent, ACCESS_READ);
        } else
            l = Lock((CONST_STRPTR)paths[i], ACCESS_READ);
        if (!l) continue;
        strcpy(names, name);
        args[k].wa_Lock = l;
        args[k].wa_Name = (APTR)names;
        names += strlen(name) + 1;
        am->am_NumArgs = ++k;
    }
    am->am_Message.mn_Node.ln_Type = NT_MESSAGE;
    am->am_Message.mn_ReplyPort = app_port;
    am->am_Message.mn_Length = sizeof *am;
    am->am_Type = kind == OWB_AO_APPMENU ? AMTYPE_APPMENUITEM : AMTYPE_APPICON;
    am->am_Version = AM_VERSION;
    am->am_Class = amclass;
    am->am_MouseX = mx;
    am->am_MouseY = my;
    CurrentTime(&am->am_Seconds, &am->am_Micros);
    /* looked up and sent while the library holds its objects, so the program can't take it (and its port) away
     * between the two */
    control(OWBCTRLA_HoldAppObjects, TRUE);
    memset(&q, 0, sizeof q);
    q.ao_Kind = (ULONG)kind;
    q.ao_Handle = handle;
    q.ao_Find = 1;
    control(OWBCTRLA_NextAppObject, (ULONG)&q);
    if (q.ao_Handle && q.ao_Port) {
        am->am_ID = q.ao_ID;
        am->am_UserData = q.ao_UserData;
        PutMsg(q.ao_Port, &am->am_Message);
        apps_out++;
        sent = 1;
    }
    control(OWBCTRLA_HoldAppObjects, FALSE);
    if (!sent) app_free(am);
    return sent;
}
