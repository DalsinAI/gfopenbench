/* ob_drop: icons let go on the desktop's windows (W67; Dale, 18 October 2026: "dragging icons 'into'
 * windows"; 24 October 2026: icons dragged between windows, as OS 3.2.3's Workbench does it). See ob_drop.c.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_DROP_H
#define OB_DROP_H

#include <exec/types.h>
#include <intuition/intuition.h>
#include "wb_drawer.h"

/* What a desktop icon is (ob_drop_env's icon_at) */
enum { OB_DESK_NONE = 0,
       OB_DESK_DISK,                            /* a disk: into it */
       OB_DESK_DRAWER,                          /* a drawer left out: into it */
       OB_DESK_TRASH,                           /* a trashcan left out: to the trash */
       OB_DESK_FILE };                          /* a tool or a project left out: the desktop's space */

/* What GFOpenBench shares with it: its statics, read at each drop (the screen and the root window change when
 * the Workbench screen is reset), and the desktop's icons. */
typedef struct ob_drop_env {
    struct Screen **scr;                        /* the Workbench screen */
    struct Window **root;                       /* the desktop's window */
    /* What drawer window w has at (sx, sy) (wb_drawer_at, answered by the drawer's own task: ob_dtask.h), with the
     * drawer's path in dir and the drawer in *who; -1 when w is no drawer's window. */
    int (*drawer_at)(struct Window *w, WORD sx, WORD sy, char *out, int n, char *dir, int dn, wb_drawer **who);
    /* Every open drawer showing path read again, but except (NULL: every one) */
    void (*reread)(const char *path, wb_drawer *except);
    /* The desktop's icon at (sx, sy), not the one being dragged: OB_DESK_*, and its path in out ("Work:", or a
     * left-out "Work:Tools/x"). */
    int (*icon_at)(WORD sx, WORD sy, char *out, int n);
    /* Icons let go on the desktop's own space: left out there for this run, as OS 3.2.3 leaves them out (gone
     * from their drawers, not written to .backdrop). 1 when taken. */
    int (*leave_out)(const char *const *paths, int n, WORD sx, WORD sy);
    /* A job has ended: files may have gone from where icons left out on the desktop say they are. */
    void (*changed)(void);
    /* 0.1.44: words for the screen's title, kept there until the next click, as OS 3.2.3 says an error that asks
     * nothing (a move within a volume onto a name that is taken). NULL: not said. */
    void (*say)(const char *text);
} ob_drop_env;

/* Before the dock and WBStartup: tells workbench.library that drops on AppWindows are delivered (the Team's
 * OWBCTRLA_DesktopDrops), so programs' AddAppWindowA is taken. 1, or 0 with no ports. */
int ob_drop_start(const ob_drop_env *env);
/* At the end: the jobs still running are asked to stop and waited for, AppMessages not yet answered too
 * (for a while). */
void ob_drop_stop(void);
ULONG ob_drop_sigmask(void);
/* Its signals came: finished jobs (their drawers read again), answered AppMessages (freed). */
void ob_drop_handle(void);
/* Icons let go at (sx, sy) on the screen: from a drawer window (wb_look's drop; from is the drawer), or left out
 * on the desktop (from NULL). 1 when something was done. */
int ob_drop(const char *const *paths, int n, WORD sx, WORD sy, UWORD qual, wb_drawer *from);
/* A disk's icon from the desktop let go at (sx, sy), over a window that isn't the desktop's: its contents
 * into a drawer (a new drawer named after the disk), or to a program's AppWindow. 1 when taken. */
int ob_drop_volume(const char *volume, WORD sx, WORD sy, UWORD qual);
/* A drawer window's Window > New drawer... (wb_look's new_drawer): asks for the name (GFOpenFiles' requester), makes
 * the drawer in dir through GFOpenFiles' engine and gives it a drawer icon. 1 when made. */
int ob_new_drawer(const char *dir, wb_drawer *from);
/* GFOpenFiles' one-line requester over parent (Execute Command...): 1 when OK was chosen, buf holding the line */
int ob_req_string(struct Window *parent, const char *title, const char *question, const char *ok, char *buf, int size);
/* A drawer window's Icons > Copy (wb_look's copy): the selected entries copied beside themselves, as a job through
 * GFOpenFiles' engine. 1 when started. */
int ob_copy(const char *dir, const char *const *names, int n, wb_drawer *from);
/* A drawer window's Icons > Empty trash (wb_look's empty_trash): the selected trashcans emptied through GFOpenFiles'
 * engine, each after a requester. 1 when a job started. */
int ob_empty_trash(const char *dir, const char *const *names, int n, wb_drawer *from);
/* Icons > Rename... (CM4 item 12, row M19): path renamed newname in its drawer, its .info with it, by GFOpenFiles'
 * engine (of_rename), and the drawers showing it read again. OF_OK, or the engine's OF_ERR_* (ob_drop_error says it). */
int ob_drop_rename(const char *path, const char *newname);
/* An OF_ERR_* in words, as the drops' requesters say it ("object already exists"). */
const char *ob_drop_error(int err);
/* Icons > Delete... (row M23): OS 3.2.3's question ("OK to delete: n file(s) and m drawer(s) (and their contents)?",
 * OK or Cancel) on window win (NULL: the screen's front), then the engine's delete in a job of its own: to the volume's
 * trash (of_delete; Empty Trash takes them for good), and for good what is in the trash already. Disks and trashcans
 * are left. 1 when something was started. */
int ob_drop_delete(const char *const *paths, int n, struct Window *win);
/* The window whose layer is at (sx, sy) on the screen, or NULL (the screen's bar, say). */
struct Window *ob_drop_window_at(WORD sx, WORD sy);
/* The same, looking through skip (an icon's picture being dragged). */
struct Window *ob_drop_window_under(WORD sx, WORD sy, struct Window *skip);

#endif
