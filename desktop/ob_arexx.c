/* ob_arexx.c: GFOpenBench's ARexx ports, WORKBENCH and OPENBENCH (ob_arexx.h). OS 3.2.3's Workbench commands, with
 * their templates (ob_rexx.c), read from the command line with ReadArgs as Workbench does, so a script written for it
 * is read the same way. What each does:
 *
 *   WINDOW names OPEN/CLOSE/FRONT/BACK/ACTIVATE/MIN/MAX   GFOpenBench's root ("root") and drawer windows (by path)
 *   ACTIVATEWINDOW, WINDOWTOFRONT, WINDOWTOBACK, ZOOMWINDOW, UNZOOMWINDOW, MOVEWINDOW, SIZEWINDOW, CHANGEWINDOW
 *                                                         the window named, or the active one of GFOpenBench's
 *   GETATTR                                               application.*, windows.*, window.* (NAME the window),
 *                                                         window.icons.all/selected/unselected.*, keycommands.*,
 *                                                         menucommands.*; into RESULT, VAR or STEM
 *   NEWDRAWER, RENAME, DELETE                             GFOpenFiles' engine (of_mkdir, of_rename, which takes the
 *                                                         .info along, of_delete_forever), then the drawers read again
 *   INFO, ICON ... OPEN                                   Information, and opening, as on the desktop
 *   ICON ... SELECT/UNSELECT                              the desktop's icons
 *   FAULT, HELP, LOCKGUI, UNLOCKGUI                       as 3.2.3's
 *   RX                                                    SYS:Rexxc/rx, always on its own (see do_rx)
 *   OPEN, VERSION (OPENBENCH only)                        GFOpenBench's own
 *
 * Not yet (each answers RC 10 with ERROR_ACTION_NOT_KNOWN, 209, so a script can tell): MENU's ADD, REMOVE and INVOKE,
 * KEYBOARD, VIEW, WINDOW's SNAPSHOT and CYCLE, ICON's moving, cycling and its selection in drawer windows.
 *
 * A command that fails answers RC 10 and the DOS error in the secondary result, which FAULT turns into words.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/dostags.h>
#include <dos/rdargs.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <graphics/text.h>
#include <rexx/storage.h>
#include <rexx/rxslib.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/icon.h>
#include <proto/rexxsyslib.h>
#include <clib/alib_protos.h>

#include "ob_rexx.h"
#include "ob_arexx.h"
#include "of_ops.h"
#include "of_fs_amiga.h"
#include "of_path.h"

/* rexxsyslib.library, opened while a port is: whatever type its proto gives it */
__typeof__(RexxSysBase) RexxSysBase;

#define RC_OK 0
#define RC_ERROR 10
#define RESULT_MAX 1024
#define NAME_MAX 256

static const ob_rexx_env *env;
static struct MsgPort *ports[OB_REXX_PORTS];
static int gui_locks;                       /* LOCKGUI's count: kept, and answered, as 3.2.3's */

/* ---- a command's answer -------------------------------------------------------------------------------------- */

typedef struct answer {
    LONG rc, err;                           /* err: the DOS error when rc isn't 0 */
    char result[RESULT_MAX];
    int has_result;
} answer;

static void fail(answer *a, LONG err)
{
    a->rc = RC_ERROR;
    a->err = err ? err : ERROR_OBJECT_NOT_FOUND;
}

static void result(answer *a, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
static void result(answer *a, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(a->result, sizeof a->result, fmt, ap);
    va_end(ap);
    a->has_result = 1;
}

static LONG of_to_dos(int e)
{
    switch (e) {
    case OF_OK: return 0;
    case OF_ERR_NOTFOUND: return ERROR_OBJECT_NOT_FOUND;
    case OF_ERR_EXISTS: return ERROR_OBJECT_EXISTS;
    case OF_ERR_FULL: return ERROR_DISK_FULL;
    case OF_ERR_XDEV: return ERROR_RENAME_ACROSS_DEVICES;
    case OF_ERR_NOTEMPTY: return ERROR_DIRECTORY_NOT_EMPTY;
    case OF_ERR_INVALID: return ERROR_INVALID_COMPONENT_NAME;
    case OF_ERR_NOMEM: return ERROR_NO_FREE_STORE;
    case OF_ERR_PROTECTED: return ERROR_DELETE_PROTECTED;
    case OF_ERR_READONLY: return ERROR_DISK_WRITE_PROTECTED;
    default: return ERROR_SEEK_ERROR;
    }
}

/* ---- the windows --------------------------------------------------------------------------------------------- */

static int ci_eq(const char *a, const char *b)
{
    size_t la = strlen(a), lb = strlen(b);
    while (la > 1 && a[la - 1] == '/') la--;            /* "RAM:WBGap/" is "RAM:WBGap" */
    while (lb > 1 && b[lb - 1] == '/') lb--;
    if (la != lb) return 0;
    while (la--) {
        int x = (unsigned char)*a++, y = (unsigned char)*b++;
        if (x >= 'a' && x <= 'z') x -= 32;
        if (y >= 'a' && y <= 'z') y -= 32;
        if (x != y) return 0;
    }
    return 1;
}

/* the window named: "root", "active" (GFOpenBench's active window), or a drawer's path; with no name, the active
 * one, else the root. name_out gets its name. */
static struct Window *window_named(const char *name, char *name_out, int n)
{
    struct Window *w, *active = IntuitionBase->ActiveWindow;
    char wn[NAME_MAX];
    int i;
    if (!name || !*name || ci_eq(name, "active")) {
        for (i = 0; (w = env->window_at(i, wn, sizeof wn)); i++)
            if (w == active) {
                if (name_out) snprintf(name_out, (size_t)n, "%s", wn);
                return w;
            }
        if (name && *name) return NULL;                 /* "active": none of ours is */
        if ((w = env->window_at(0, wn, sizeof wn)) && name_out) snprintf(name_out, (size_t)n, "%s", wn);
        return w;
    }
    for (i = 0; (w = env->window_at(i, wn, sizeof wn)); i++)
        if (ci_eq(wn, name)) {
            if (name_out) snprintf(name_out, (size_t)n, "%s", wn);
            return w;
        }
    /* a drawer named another way (a volume name for its device, an assign): the same lock */
    {
        BPTR l = Lock((CONST_STRPTR)name, ACCESS_READ), l2;
        if (!l) return NULL;
        for (i = 1; (w = env->window_at(i, wn, sizeof wn)); i++)
            if ((l2 = Lock((CONST_STRPTR)wn, ACCESS_READ))) {
                int same = SameLock(l, l2) == LOCK_SAME;
                UnLock(l2);
                if (same) {
                    UnLock(l);
                    if (name_out) snprintf(name_out, (size_t)n, "%s", wn);
                    return w;
                }
            }
        UnLock(l);
    }
    return NULL;
}

static int is_root(struct Window *w)
{
    char wn[8];
    return w && env->window_at(0, wn, sizeof wn) == w;
}

static void box(struct Window *w, LONG x, LONG y, LONG wd, LONG ht)
{
    struct Screen *s = w->WScreen;
    if (wd < w->MinWidth) wd = w->MinWidth;
    if (ht < w->MinHeight) ht = w->MinHeight;
    if (w->MaxWidth && (UWORD)w->MaxWidth != 0xFFFF && wd > w->MaxWidth) wd = w->MaxWidth;
    if (w->MaxHeight && (UWORD)w->MaxHeight != 0xFFFF && ht > w->MaxHeight) ht = w->MaxHeight;
    if (wd > s->Width) wd = s->Width;
    if (ht > s->Height) ht = s->Height;
    if (x + wd > s->Width) x = s->Width - wd;
    if (y + ht > s->Height) y = s->Height - ht;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    ChangeWindowBox(w, x, y, wd, ht);
}

/* ---- ReadArgs over the command's arguments -------------------------------------------------------------------- */

typedef struct parsed {
    struct RDArgs *rda;
    LONG a[16];
    char *buf;
} parsed;

static int parse(parsed *p, const char *template, const char *args, answer *ans)
{
    size_t n = strlen(args);
    memset(p, 0, sizeof *p);
    if (!*template) {
        if (*args) {
            fail(ans, ERROR_TOO_MANY_ARGS);
            return 0;
        }
        return 1;
    }
    if (!(p->buf = AllocVec(n + 2, MEMF_ANY)) || !(p->rda = AllocDosObject(DOS_RDARGS, NULL))) {
        if (p->buf) FreeVec(p->buf);
        p->buf = NULL;
        fail(ans, ERROR_NO_FREE_STORE);
        return 0;
    }
    memcpy(p->buf, args, n);
    p->buf[n] = '\n';                                   /* ReadArgs reads up to the end of the line */
    p->buf[n + 1] = 0;
    p->rda->RDA_Source.CS_Buffer = (UBYTE *)p->buf;
    p->rda->RDA_Source.CS_Length = (LONG)n + 1;
    p->rda->RDA_Source.CS_CurChr = 0;
    p->rda->RDA_Flags |= RDAF_NOPROMPT;
    if (!ReadArgs((CONST_STRPTR)template, p->a, p->rda)) {
        fail(ans, IoErr());
        FreeDosObject(DOS_RDARGS, p->rda);
        FreeVec(p->buf);
        p->rda = NULL;
        p->buf = NULL;
        return 0;
    }
    return 1;
}

static void unparse(parsed *p)
{
    if (p->rda) {
        FreeArgs(p->rda);
        FreeDosObject(DOS_RDARGS, p->rda);
    }
    if (p->buf) FreeVec(p->buf);
}

#define STR(i) ((const char *)p.a[i])
#define NUM(i) (p.a[i] ? *(LONG *)p.a[i] : 0)
#define HAS(i) (p.a[i] != 0)

/* ---- GETATTR ------------------------------------------------------------------------------------------------ */

/* which icons: all, selected or unselected */
enum { ICONS_ALL, ICONS_SEL, ICONS_UNSEL };

/* the k-th icon of window w among the kind wanted; how many there are when k < 0 */
static int icon_of(struct Window *w, int kind, long k, ob_rexx_icon *out)
{
    ob_rexx_icon ic;
    int i, got = 0;
    for (i = 0; env->icon_at(w, i, &ic, 0); i++) {
        if (kind == ICONS_SEL && !ic.selected) continue;
        if (kind == ICONS_UNSEL && ic.selected) continue;
        if (k >= 0 && got == k) {
            env->icon_at(w, i, out, 1);
            return 1;
        }
        got++;
    }
    return k < 0 ? got : 0;
}

static void font_attr(answer *a, struct TextFont *f, const char *what)
{
    if (!f) {
        fail(a, ERROR_OBJECT_NOT_FOUND);
        return;
    }
    if (!strcmp(what, "name"))
        result(a, "%s/%u", f->tf_Message.mn_Node.ln_Name ? f->tf_Message.mn_Node.ln_Name : "", (unsigned)f->tf_YSize);
    else if (!strcmp(what, "width"))
        result(a, "%u", (unsigned)f->tf_XSize);
    else
        result(a, "%u", (unsigned)f->tf_YSize);
}

/* "window." in any case, then something */
static int starts_window(const char *obj)
{
    static const char w[] = "WINDOW.";
    int i;
    for (i = 0; w[i]; i++) {
        int c = (unsigned char)obj[i];
        if (c >= 'a' && c <= 'z') c -= 32;
        if (c != w[i]) return 0;
    }
    return obj[i] != 0;
}

/* one GETATTR object into a->result; a list's names one per line in list[] (STEM) */
static void getattr(answer *a, const char *obj, const char *wname)
{
    static const char *const kinds[] = { "all", "selected", "unselected" };
    struct Screen *scr = env->screen ? env->screen() : NULL;
    struct Window *w;
    char wn[NAME_MAX];
    long num[2];
    int i;

    if (ob_rexx_attr(obj, "application.version", num, 2)) {
        result(a, "%s", env->version);
        return;
    }
    if (ob_rexx_attr(obj, "application.screen", num, 2)) {
        result(a, "Workbench");
        return;
    }
    {
        static const char *const fp[] = { "screen", "icon", "system" };
        static const char *const fw[] = { "name", "width", "height" };
        int k, m;
        for (k = 0; k < 3; k++)
            for (m = 0; m < 3; m++) {
                char pat[48];
                snprintf(pat, sizeof pat, "application.font.%s.%s", fp[k], fw[m]);
                if (ob_rexx_attr(obj, pat, num, 2)) {
                    struct TextFont *f = NULL;
                    if (k == 0 && scr) f = scr->RastPort.Font;
                    else if (k == 1) f = env->icon_font ? env->icon_font() : NULL;
                    if (k == 1 && !f && scr) f = scr->RastPort.Font;
                    if (k == 2) f = GfxBase->DefaultFont;
                    font_attr(a, f, fw[m]);
                    return;
                }
            }
    }
    if (ob_rexx_attr(obj, "windows.count", num, 2)) {
        for (i = 0; env->window_at(i, wn, sizeof wn); i++)
            ;
        result(a, "%d", i);
        return;
    }
    if (ob_rexx_attr(obj, "windows.active", num, 2)) {
        if (window_named("active", wn, sizeof wn)) result(a, "%s", wn);
        else result(a, "%s", "");
        return;
    }
    if (ob_rexx_attr(obj, "windows.#", num, 2)) {
        if (!env->window_at((int)num[0], wn, sizeof wn)) fail(a, ERROR_OBJECT_NOT_FOUND);
        else result(a, "%s", wn);
        return;
    }
    if (ob_rexx_attr(obj, "keycommands.count", num, 2) || ob_rexx_attr(obj, "menucommands.count", num, 2)) {
        result(a, "0");                                 /* none can be added yet (KEYBOARD, MENU ADD) */
        return;
    }
    if (!starts_window(obj)) {
        fail(a, ERROR_OBJECT_NOT_FOUND);
        return;
    }
    if (!(w = window_named(wname, wn, sizeof wn))) {
        fail(a, ERROR_OBJECT_NOT_FOUND);
        return;
    }
    if (ob_rexx_attr(obj, "window.left", num, 2)) result(a, "%d", w->LeftEdge);
    else if (ob_rexx_attr(obj, "window.top", num, 2)) result(a, "%d", w->TopEdge);
    else if (ob_rexx_attr(obj, "window.width", num, 2)) result(a, "%d", w->Width);
    else if (ob_rexx_attr(obj, "window.height", num, 2)) result(a, "%d", w->Height);
    else if (ob_rexx_attr(obj, "window.min.width", num, 2)) result(a, "%d", w->MinWidth);
    else if (ob_rexx_attr(obj, "window.min.height", num, 2)) result(a, "%d", w->MinHeight);
    else if (ob_rexx_attr(obj, "window.max.width", num, 2))
        result(a, "%d", (UWORD)w->MaxWidth == 0xFFFF ? w->WScreen->Width : w->MaxWidth);
    else if (ob_rexx_attr(obj, "window.max.height", num, 2))
        result(a, "%d", (UWORD)w->MaxHeight == 0xFFFF ? w->WScreen->Height : w->MaxHeight);
    else if (ob_rexx_attr(obj, "window.view.left", num, 2) || ob_rexx_attr(obj, "window.view.top", num, 2))
        result(a, "0");
    else if (ob_rexx_attr(obj, "window.screen.name", num, 2)) result(a, "Workbench");
    else if (ob_rexx_attr(obj, "window.screen.width", num, 2)) result(a, "%d", w->WScreen->Width);
    else if (ob_rexx_attr(obj, "window.screen.height", num, 2)) result(a, "%d", w->WScreen->Height);
    else {
        int k;
        for (k = 0; k < 3; k++) {
            char pat[48];
            ob_rexx_icon ic;
            snprintf(pat, sizeof pat, "window.icons.%s.count", kinds[k]);
            if (ob_rexx_attr(obj, pat, num, 2)) {
                result(a, "%d", icon_of(w, k, -1, &ic));
                return;
            }
            snprintf(pat, sizeof pat, "window.icons.%s.#", kinds[k]);
            if (ob_rexx_attr(obj, pat, num, 2)) {     /* an icon itself: its name */
                if (!icon_of(w, k, num[0], &ic)) fail(a, ERROR_OBJECT_NOT_FOUND);
                else result(a, "%s", ic.name);
                return;
            }
            {
                static const char *const f[] = { "name", "left", "top", "width", "height", "type", "status" };
                int m;
                for (m = 0; m < 7; m++) {
                    snprintf(pat, sizeof pat, "window.icons.%s.#.%s", kinds[k], f[m]);
                    if (!ob_rexx_attr(obj, pat, num, 2)) continue;
                    if (!icon_of(w, k, num[0], &ic)) {
                        fail(a, ERROR_OBJECT_NOT_FOUND);
                        return;
                    }
                    switch (m) {
                    case 0: result(a, "%s", ic.name); break;
                    case 1: result(a, "%ld", (long)ic.left); break;
                    case 2: result(a, "%ld", (long)ic.top); break;
                    case 3: result(a, "%ld", (long)ic.width); break;
                    case 4: result(a, "%ld", (long)ic.height); break;
                    case 5: result(a, "%s", ic.type ? ic.type : "PROJECT"); break;
                    default: result(a, "%s", ic.selected ? "SELECTED" : "UNSELECTED"); break;
                    }
                    return;
                }
            }
        }
        fail(a, ERROR_OBJECT_NOT_FOUND);
    }
}

/* STEM's list: windows, window.icons.<kind>; the stem's COUNT and each one's name */
static int stem_list(struct RexxMsg *rm, const char *obj, const char *wname, const char *stem, answer *a)
{
    static const char *const kinds[] = { "window.icons.all", "window.icons.selected", "window.icons.unselected" };
    char var[NAME_MAX + 16], wn[NAME_MAX];
    int i, k, n = 0;
    long num[2];
    if (ob_rexx_attr(obj, "windows", num, 2)) {
        struct Window *w;
        for (i = 0; (w = env->window_at(i, wn, sizeof wn)); i++) {
            snprintf(var, sizeof var, "%s.%d", stem, i);
            SetRexxVar((struct Message *)rm, (CONST_STRPTR)var, (CONST_STRPTR)wn, (LONG)strlen(wn));
        }
        n = i;
    } else {
        struct Window *w;
        for (k = 0; k < 3 && !ob_rexx_attr(obj, kinds[k], num, 2); k++)
            ;
        if (k == 3) return 0;
        if (!(w = window_named(wname, wn, sizeof wn))) {
            fail(a, ERROR_OBJECT_NOT_FOUND);
            return 1;
        }
        {
            ob_rexx_icon ic;
            while (icon_of(w, k, n, &ic)) {
                snprintf(var, sizeof var, "%s.%d", stem, n);
                SetRexxVar((struct Message *)rm, (CONST_STRPTR)var, (CONST_STRPTR)ic.name, (LONG)strlen(ic.name));
                n++;
            }
        }
    }
    snprintf(var, sizeof var, "%s.COUNT", stem);
    snprintf(wn, sizeof wn, "%d", n);
    SetRexxVar((struct Message *)rm, (CONST_STRPTR)var, (CONST_STRPTR)wn, (LONG)strlen(wn));
    return 1;
}

/* ---- the commands --------------------------------------------------------------------------------------------- */

static void changed_around(const char *path)
{
    char dir[OF_PATH_MAX];
    if (env->changed && of_path_parent(dir, path)) env->changed(dir);
}

static void do_newdrawer(const char *name, answer *a)
{
    of_fs fs;
    of_ctx c;
    int rc;
    of_fs_amiga_init(&fs);
    of_ctx_init(&c, &fs);
    if ((rc = of_mkdir(&c, name)) != OF_OK) {
        fail(a, of_to_dos(rc));
        return;
    }
    /* its icon, as Window > New drawer gives one (the default drawer's) */
    {
        struct DiskObject *o = GetDefDiskObject(WBDRAWER);
        if (o) {
            o->do_CurrentX = o->do_CurrentY = NO_ICON_POSITION;
            PutDiskObject((CONST_STRPTR)name, o);
            FreeDiskObject(o);
        }
    }
    changed_around(name);
}

static void do_rename(const char *old, const char *newname, answer *a)
{
    of_fs fs;
    of_ctx c;
    int rc;
    of_fs_amiga_init(&fs);
    of_ctx_init(&c, &fs);
    if ((rc = of_rename(&c, old, newname)) != OF_OK) fail(a, of_to_dos(rc));
    else changed_around(old);
}

static void do_delete(const char *name, int all, answer *a)
{
    of_fs fs;
    of_ctx c;
    of_stat st;
    char icon[OF_PATH_MAX + 8];
    int rc;
    of_fs_amiga_init(&fs);
    of_ctx_init(&c, &fs);
    if (!*of_path_name(name)) {                         /* a volume or an assign itself */
        fail(a, ERROR_OBJECT_WRONG_TYPE);
        return;
    }
    if ((rc = fs.stat(&fs, name, &st)) != OF_OK || st.type == OF_NONE) {
        fail(a, rc != OF_OK ? of_to_dos(rc) : ERROR_OBJECT_NOT_FOUND);
        return;
    }
    if (st.type == OF_DIR && !all) {                    /* 3.2.3: a drawer with something in it needs ALL */
        BPTR l = Lock((CONST_STRPTR)name, ACCESS_READ);
        struct FileInfoBlock *fib = AllocDosObject(DOS_FIB, NULL);
        int empty = 1;
        if (l && fib && Examine(l, fib) && ExNext(l, fib)) empty = 0;
        if (fib) FreeDosObject(DOS_FIB, fib);
        if (l) UnLock(l);
        if (!empty) {
            fail(a, ERROR_DIRECTORY_NOT_EMPTY);
            return;
        }
    }
    if ((rc = of_delete_forever(&c, name)) != OF_OK) {
        fail(a, of_to_dos(rc));
        return;
    }
    snprintf(icon, sizeof icon, "%s.info", name);       /* its icon goes with it */
    if (fs.stat(&fs, icon, &st) == OF_OK && st.type != OF_NONE) of_delete_forever(&c, icon);
    changed_around(name);
}

static void do_window(parsed *pp, answer *a)
{
    parsed p = *pp;
    const char **names = (const char **)p.a[0];
    int i;
    if (HAS(3) || HAS(9)) {                             /* SNAPSHOT, CYCLE: not yet */
        fail(a, ERROR_ACTION_NOT_KNOWN);
        return;
    }
    for (i = 0; names && names[i]; i++) {
        struct Window *w = window_named(names[i], NULL, 0);
        if (HAS(1)) {                                   /* OPEN */
            if (!w && !env->open_drawer(names[i])) {
                fail(a, ERROR_OBJECT_NOT_FOUND);
                return;
            }
            w = window_named(names[i], NULL, 0);
        }
        if (HAS(2)) {                                   /* CLOSE */
            if (!w) continue;                           /* closed already */
            if (!env->close_window(w)) {
                fail(a, ERROR_OBJECT_WRONG_TYPE);       /* the root stays */
                return;
            }
            continue;
        }
        if (!w) {
            fail(a, ERROR_OBJECT_NOT_FOUND);
            return;
        }
        if (HAS(5)) box(w, w->LeftEdge, w->TopEdge, w->MinWidth, w->MinHeight);                   /* MIN */
        if (HAS(6)) box(w, w->LeftEdge, w->TopEdge, w->WScreen->Width, w->WScreen->Height);      /* MAX */
        if (HAS(7)) WindowToFront(w);                                                             /* FRONT */
        if (HAS(8)) WindowToBack(w);                                                              /* BACK */
        if (HAS(4)) ActivateWindow(w);                                                            /* ACTIVATE */
    }
}

static void do_icon(parsed *pp, answer *a)
{
    parsed p = *pp;
    const char **names = (const char **)p.a[1];
    struct Window *w;
    char wn[NAME_MAX];
    int i;
    /* MAKEVISIBLE, UP..Y, ACTIVATE, CYCLE, MOVE: not yet */
    for (i = 3; i <= 14; i++)
        if (i != 4 && i != 5 && HAS(i)) {
            fail(a, ERROR_ACTION_NOT_KNOWN);
            return;
        }
    if (!(w = window_named(STR(0), wn, sizeof wn))) {
        fail(a, ERROR_OBJECT_NOT_FOUND);
        return;
    }
    for (i = 0; names && names[i]; i++) {
        char path[OF_PATH_MAX];
        if (HAS(4) || HAS(5)) {                         /* SELECT, UNSELECT */
            if (!env->icon_select || !env->icon_select(w, names[i], HAS(4) ? 1 : 0)) {
                fail(a, is_root(w) ? ERROR_OBJECT_NOT_FOUND : ERROR_ACTION_NOT_KNOWN);
                return;
            }
        }
        if (HAS(2)) {                                   /* OPEN */
            if (is_root(w) || strchr(names[i], ':')) snprintf(path, sizeof path, "%s", names[i]);
            else of_path_join(path, wn, names[i]);
            if (!env->open_object(path)) {
                fail(a, ERROR_OBJECT_NOT_FOUND);
                return;
            }
        }
    }
}

static void do_rx(parsed *pp, answer *a)
{
    /* Always started on its own (ASYNC or not): a script that talks back to WORKBENCH would otherwise wait for
     * GFOpenBench while GFOpenBench waits for it. RC says whether it started. */
    parsed p = *pp;
    char cmd[RESULT_MAX];
    BPTR in = Open((CONST_STRPTR)"NIL:", MODE_OLDFILE);
    BPTR out = HAS(0) ? Open((CONST_STRPTR)"CON:0/20/640/200/Workbench ARexx/AUTO/CLOSE/WAIT", MODE_NEWFILE)
                      : Open((CONST_STRPTR)"NIL:", MODE_NEWFILE);
    snprintf(cmd, sizeof cmd, "SYS:Rexxc/rx %s", STR(2));
    if (!in || !out) {
        if (in) Close(in);
        if (out) Close(out);
        fail(a, IoErr());
        return;
    }
    if (SystemTags((CONST_STRPTR)cmd, SYS_Input, in, SYS_Output, out, SYS_Asynch, TRUE, NP_StackSize, 16384,
                   TAG_DONE) == -1) {
        Close(in);                                      /* not started: the handles are still ours */
        Close(out);
        fail(a, IoErr());
    }
}

/* the window to act on: WINDOW, or the active one of GFOpenBench's, else the root */
static struct Window *target(const char *name, answer *a)
{
    struct Window *w = window_named(name, NULL, 0);
    if (!w) fail(a, ERROR_OBJECT_NOT_FOUND);
    return w;
}

static void command(struct RexxMsg *rm, const char *line, int own, answer *a)
{
    const char *args = "";
    int cmd = ob_rexx_find(line, &args, own);
    parsed p;
    struct Window *w;
    memset(a, 0, sizeof *a);
    if (cmd < 0) {
        fail(a, ERROR_ACTION_NOT_KNOWN);
        return;
    }
    if (!parse(&p, ob_rexx_cmds[cmd].template, args, a)) return;
    switch (cmd) {
    case OB_RX_ACTIVATEWINDOW:
        if ((w = target(STR(0), a))) ActivateWindow(w);
        break;
    case OB_RX_WINDOWTOFRONT:
        if ((w = target(STR(0), a))) WindowToFront(w);
        break;
    case OB_RX_WINDOWTOBACK:
        if ((w = target(STR(0), a))) WindowToBack(w);
        break;
    case OB_RX_ZOOMWINDOW:
    case OB_RX_UNZOOMWINDOW:
        if ((w = target(STR(0), a))) {
            int zoomed = (w->Flags & WFLG_ZOOMED) != 0;
            if (zoomed == (cmd == OB_RX_UNZOOMWINDOW)) ZipWindow(w);
        }
        break;
    case OB_RX_MOVEWINDOW:
        if ((w = target(STR(0), a)))
            box(w, HAS(1) ? NUM(1) : w->LeftEdge, HAS(2) ? NUM(2) : w->TopEdge, w->Width, w->Height);
        break;
    case OB_RX_SIZEWINDOW:
        if ((w = target(STR(0), a)))
            box(w, w->LeftEdge, w->TopEdge, HAS(1) ? NUM(1) : w->Width, HAS(2) ? NUM(2) : w->Height);
        break;
    case OB_RX_CHANGEWINDOW:
        if ((w = target(STR(0), a)))
            box(w, HAS(1) ? NUM(1) : w->LeftEdge, HAS(2) ? NUM(2) : w->TopEdge, HAS(3) ? NUM(3) : w->Width,
                HAS(4) ? NUM(4) : w->Height);
        break;
    case OB_RX_DELETE:
        do_delete(STR(0), HAS(1), a);
        break;
    case OB_RX_NEWDRAWER:
        do_newdrawer(STR(0), a);
        break;
    case OB_RX_RENAME:
        do_rename(STR(0), STR(1), a);
        break;
    case OB_RX_FAULT: {
        char text[128];
        if (Fault(NUM(0), NULL, (STRPTR)text, sizeof text) <= 0) snprintf(text, sizeof text, "Error %ld", (long)NUM(0));
        result(a, "%s", text);
        break;
    }
    case OB_RX_GETATTR: {
        const char *obj = STR(0), *stem = STR(2), *var = STR(3);
        if (stem && stem_list(rm, obj, STR(1), stem, a)) break;
        getattr(a, obj, STR(1));
        if (a->rc) break;
        if (var || stem) {                              /* the value into the variable, not RESULT */
            SetRexxVar((struct Message *)rm, (CONST_STRPTR)(var ? var : stem), (CONST_STRPTR)a->result,
                       (LONG)strlen(a->result));
            a->has_result = 0;
        }
        break;
    }
    case OB_RX_HELP:
        if (HAS(1)) {                                   /* MENUS: GFOpenBench's menus can't be named yet */
            fail(a, ERROR_ACTION_NOT_KNOWN);
            break;
        }
        if (!ob_rexx_help(a->result, sizeof a->result, STR(0), own)) fail(a, ERROR_OBJECT_NOT_FOUND);
        else a->has_result = 1;
        break;
    case OB_RX_ICON:
        do_icon(&p, a);
        break;
    case OB_RX_INFO:
        if (!env->info(STR(0))) fail(a, ERROR_OBJECT_NOT_FOUND);
        break;
    case OB_RX_LOCKGUI:
        gui_locks++;
        break;
    case OB_RX_UNLOCKGUI:
        if (gui_locks) gui_locks--;
        break;
    case OB_RX_RX:
        do_rx(&p, a);
        break;
    case OB_RX_WINDOW:
        do_window(&p, a);
        break;
    case OB_RX_OPEN:
        if (!env->open_object(STR(0))) fail(a, ERROR_OBJECT_NOT_FOUND);
        break;
    case OB_RX_VERSION:
        result(a, "%s", env->version);
        break;
    default:                                            /* KEYBOARD, MENU, VIEW: not yet */
        fail(a, ERROR_ACTION_NOT_KNOWN);
        break;
    }
    unparse(&p);
}

/* ---- the ports ---------------------------------------------------------------------------------------------- */

static void reply(struct RexxMsg *rm, const answer *a)
{
    rm->rm_Result1 = a->rc;
    rm->rm_Result2 = 0;
    if (a->rc) rm->rm_Result2 = a->err;
    else if ((rm->rm_Action & RXFF_RESULT) && a->has_result)
        rm->rm_Result2 = (LONG)CreateArgstring((CONST_STRPTR)a->result, (ULONG)strlen(a->result));
    ReplyMsg((struct Message *)rm);
}

int ob_arexx_start(const ob_rexx_env *e)
{
    int i, open = 0;
    env = e;
    if (!(RexxSysBase = (__typeof__(RexxSysBase))OpenLibrary((CONST_STRPTR)"rexxsyslib.library", 36))) return 0;
    for (i = 0; i < OB_REXX_PORTS; i++) {
        struct MsgPort *mp;
        Forbid();
        if (FindPort((CONST_STRPTR)ob_rexx_port_names[i])) {
            Permit();
            /* a real Workbench (or another program) has the name: it keeps it */
            Printf((CONST_STRPTR)"GFOpenBench: the ARexx port %s is taken: not opened\n", (LONG)ob_rexx_port_names[i]);
            continue;
        }
        Permit();
        if (!(mp = CreateMsgPort())) continue;
        mp->mp_Node.ln_Name = (char *)ob_rexx_port_names[i];
        mp->mp_Node.ln_Pri = 0;
        Forbid();
        if (FindPort((CONST_STRPTR)ob_rexx_port_names[i])) {    /* taken in between */
            Permit();
            DeleteMsgPort(mp);
            continue;
        }
        AddPort(mp);
        Permit();
        ports[i] = mp;
        open++;
    }
    if (!open) {
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = NULL;
    }
    return open > 0;
}

ULONG ob_arexx_sigmask(void)
{
    ULONG m = 0;
    int i;
    for (i = 0; i < OB_REXX_PORTS; i++)
        if (ports[i]) m |= 1UL << ports[i]->mp_SigBit;
    return m;
}

void ob_arexx_handle(void)
{
    int i;
    for (i = 0; i < OB_REXX_PORTS; i++) {
        struct Message *m;
        if (!ports[i]) continue;
        while ((m = GetMsg(ports[i]))) {
            struct RexxMsg *rm = (struct RexxMsg *)m;
            answer a;
            if (!IsRexxMsg(rm)) {                       /* not ARexx's: answered as it came */
                ReplyMsg(m);
                continue;
            }
            command(rm, rm->rm_Args[0] ? (const char *)rm->rm_Args[0] : "", i == 1, &a);
            reply(rm, &a);
        }
    }
}

void ob_arexx_stop(void)
{
    int i;
    for (i = 0; i < OB_REXX_PORTS; i++) {
        struct Message *m;
        if (!ports[i]) continue;
        Forbid();
        RemPort(ports[i]);
        Permit();
        while ((m = GetMsg(ports[i]))) {                /* those still waiting: GFOpenBench is going */
            struct RexxMsg *rm = (struct RexxMsg *)m;
            if (IsRexxMsg(rm)) {
                rm->rm_Result1 = RC_ERROR;
                rm->rm_Result2 = ERROR_OBJECT_NOT_FOUND;
            }
            ReplyMsg(m);
        }
        DeleteMsgPort(ports[i]);
        ports[i] = NULL;
    }
    if (RexxSysBase) {
        CloseLibrary((struct Library *)RexxSysBase);
        RexxSysBase = NULL;
    }
    env = NULL;
}
