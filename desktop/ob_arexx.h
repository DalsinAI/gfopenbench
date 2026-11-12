/* ob_arexx.h: GFOpenBench's ARexx ports, WORKBENCH and OPENBENCH (0.1.50; gap rows R1 and R2). The commands and
 * their templates are ob_rexx.h's; this is the Amiga half: the ports, the messages, and what each command does,
 * through what GFOpenBench gives it here (ob_rexx_env) and GFOpenFiles' engine (NEWDRAWER, RENAME, DELETE).
 *
 * In use (openbench.c):
 *     ob_arexx_start(&env);                 after the desktop is up; a port whose name is taken is not opened
 *     Wait(... | ob_arexx_sigmask());
 *     if (sigs & ob_arexx_sigmask()) ob_arexx_handle();
 *     ob_arexx_stop();                      before the windows go: every message waiting is answered
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_AREXX_H
#define OB_AREXX_H

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/text.h>

/* an icon in a window, for GETATTR window.icons.* and ICON */
typedef struct ob_rexx_icon {
    char name[108];
    LONG left, top, width, height;          /* in the window, as 3.2.3 gives them */
    const char *type;                       /* "DISK", "DRAWER", "TOOL", "PROJECT", "GARBAGE", "DEVICE", "KICK" */
    int selected;
} ob_rexx_icon;

typedef struct ob_rexx_env {
    const char *version;                    /* "GFOpenBench 0.1.50" */
    struct Screen *(*screen)(void);         /* the Workbench screen, or NULL while it is being reset */
    /* the icons' font, or NULL: the screen's */
    struct TextFont *(*icon_font)(void);
    /* the windows: 0 is the root ("root"), then the drawers, each named by its drawer's path; NULL past the last.
     * name gets the name (n bytes). */
    struct Window *(*window_at)(int i, char *name, int n);
    /* opens a drawer's window (or brings it to the front); 1 when done */
    int (*open_drawer)(const char *path);
    /* closes a drawer's window; 0 for the root */
    int (*close_window)(struct Window *w);
    /* opens anything, as a double-click on its icon does; 1 when done */
    int (*open_object)(const char *path);
    /* its Information window; 1 when done */
    int (*info)(const char *path);
    /* icon i of window w; 0 past the last. detail: its type, place and size too (a drawer's icon is then loaded);
     * else only its name and whether it is selected */
    int (*icon_at)(struct Window *w, int i, ob_rexx_icon *out, int detail);
    /* selects (how 1) or unselects (0) the icon named name in window w (the root's only, for now); 1 when found */
    int (*icon_select)(struct Window *w, const char *name, int how);
    /* something changed in drawer dir ("RAM:WBGap"): its windows read it again */
    void (*changed)(const char *dir);
} ob_rexx_env;

/* Opens the ports whose names are free. 1 when at least one is open. */
int ob_arexx_start(const ob_rexx_env *env);
/* Answers every message waiting, closes the ports. */
void ob_arexx_stop(void);
/* The ports' signals (0 when none is open). */
ULONG ob_arexx_sigmask(void);
/* Every message waiting on either port, done and answered. */
void ob_arexx_handle(void);

#endif
