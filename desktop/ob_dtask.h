/* ob_dtask.h: one task per drawer window (Workbench build plan row 3, designs/WORKBENCH-REDESIGN-20261113.md).
 *
 * Each drawer window runs in a process of its own: it reads its drawer, draws its icons, takes its window's
 * IntuiMessages on a port of its own and watches its drawer (DOS notify, 200 ms). A slow disk holds up its own
 * window and nothing else; the desktop only routes.
 *
 * Everything between the desktop and a drawer goes by messages; there is no lock between them:
 *   - the desktop to a drawer: notes (read again, draw again, thumbnails ready, search hits, find, close), bits that
 *     the drawer takes in its own loop, and questions (wb_drawer_at: what is at a place, for a drop; and
 *     workbench.library's show-a-name and each-entry calls), answered at once, even while the drawer itself is
 *     waiting on the desktop;
 *   - a drawer to the desktop: the look's calls (open a program, Information, a drop, Snapshot, Leave Out, Rename,
 *     Delete, Copy, New drawer, Empty trash), which run in the desktop while the drawer waits, and events (opened,
 *     could not open, closed, made active, clicked, search hits taken in), which it doesn't wait for.
 *   - wb_look's hidden and snapped, asked for every entry a drawer reads, are answered in the drawer from what the
 *     desktop told it once about its drawer (ob_dpack in ob_tasks.h), until the desktop's left outs or snapshots
 *     change (ob_dt_set_gen).
 *
 * A drawer task that crashes is held by the task guard; the supervisor finds it (ob_dt_task, ob_guard.h), closes its
 * window (ob_dt_abandon) and opens the drawer again (ob_super_crashed).
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_DTASK_H
#define OB_DTASK_H

#include <exec/ports.h>
#include <intuition/intuition.h>
#include "wb_drawer.h"
#include "ob_tasks.h"

typedef struct ob_dt ob_dt;

/* notes, the desktop to a drawer */
#define OB_DT_REREAD      0x01UL
#define OB_DT_REDRAW      0x02UL
#define OB_DT_THUMBS      0x04UL
#define OB_DT_FIND_UPDATE 0x08UL

/* events, a drawer to the desktop */
enum { OB_DTE_OPENED = 1,   /* its window is open (ob_dt_window) */
       OB_DTE_FAILED,       /* it couldn't open: err (IoErr) and the largest piece of memory then (ob_low); the
                               handle goes when the event's call returns */
       OB_DTE_CLOSED,       /* its window closed and its task is gone; the handle goes when the event's call returns */
       OB_DTE_ACTIVE,       /* its window was made active */
       OB_DTE_CLICK,        /* a mouse button went down in it */
       OB_DTE_FOUND };      /* it took in its search's hits: the other drawers are told (OB_DT_FIND_UPDATE) */

typedef struct ob_dt_env {
    /* the desktop's look: the drawers' windows open on its screen in its theme, and its calls run in the desktop
     * when a drawer calls them. Its port isn't used: each drawer has its own. */
    const wb_look *look;
    void (*event)(ob_dt *t, int kind, long err, unsigned long largest);
    /* everything about drawer dir for wb_look's hidden and snapped: ob_dpack_hide and ob_dpack_snap into p, which
     * is reset for dir already */
    void (*pack)(const char *dir, ob_dpack *p);
    /* what pack's answers depend on, as it is now: asked after each call a drawer makes, which may change it */
    unsigned long (*gen)(void);
} ob_dt_env;

/* The desktop's side (its port for the drawers' calls and events); 1 when ready. env is kept. */
int ob_dt_init(const ob_dt_env *env);
/* Every drawer closed (ob_dt_close_all) first. */
void ob_dt_end(void);
/* The signal the drawers' calls and events come on, and what to do with them (in the desktop's loop) */
ULONG ob_dt_sigmask(void);
/* A port of the desktop's on that signal, for another of its ports when no signal is left (the 2-second timer's: the
 * desktop's own signals are all taken, so its CreateMsgPort failed and nothing ran every two seconds: #118 review,
 * WBGapF3). FreeVec it; it has no signal of its own. NULL before ob_dt_init. */
struct MsgPort *ob_dt_port(void);
void ob_dt_service(void);
/* Left outs or snapshots changed: the drawers ask again before they next answer hidden or snapped */
void ob_dt_set_gen(unsigned long gen);

/* A drawer window on path, opened by a task of its own; box: where (NULL: as its icon's DrawerData says). NULL when
 * the task couldn't start; else OB_DTE_OPENED or OB_DTE_FAILED follows. */
ob_dt *ob_dt_open(const char *path, const struct IBox *box);
/* For ARexx's OPEN, whose answer says the window is there: its events only (its calls wait) until it is open, it
 * couldn't open, or secs have gone. 1 when it is open. The desktop otherwise never waits on a drawer. */
int ob_dt_wait_open(ob_dt *t, int secs);
/* closed: OB_DTE_CLOSED follows */
void ob_dt_close(ob_dt *t);
/* every drawer closed and waited for (the screen is about to close, GFOpenBench is quitting); their calls in the
 * meantime are answered 0 */
void ob_dt_close_all(void);
void ob_dt_note(ob_dt *t, ULONG notes);
/* Window > Find...: the search of every drive, from this drawer (its window to the front first) */
void ob_dt_find(ob_dt *t, const char *words);
/* wb_drawer_at, answered by the drawer's task (WB_AT_NONE when it is gone) */
int ob_dt_at(ob_dt *t, WORD sx, WORD sy, char *out, int n);
/* wb_drawer_show_name and wb_drawer_each (workbench.library's MakeWorkbenchObjectVisibleA and
 * ChangeWorkbenchSelectionA, ob_wb.h), done by the drawer's task while the desktop waits: fn runs in that task.
 * 0 and -1 when it is gone. */
int ob_dt_show_name(ob_dt *t, const char *name);
int ob_dt_each(ob_dt *t, int (*fn)(void *ctx, const char *name, int type, int selected, WORD x, WORD y), void *ctx);

struct Window *ob_dt_window(const ob_dt *t);    /* NULL until it is open */
const char *ob_dt_path(const ob_dt *t);
wb_drawer *ob_dt_drawer(const ob_dt *t);        /* what the look's calls name as from; NULL until it is open */
struct Task *ob_dt_task(const ob_dt *t);
/* The task guard holds its task: its window closed from here, and the handle let go (its task's memory stays, as
 * Suspend leaves it). No event follows. */
void ob_dt_abandon(ob_dt *t);
/* the user's own pointer, the desktop's */
void ob_dt_set_user(ob_dt *t, unsigned long user);
unsigned long ob_dt_user(const ob_dt *t);

#endif
