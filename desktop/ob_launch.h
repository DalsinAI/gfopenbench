/* ob_launch.h: the launcher, the Workbench task model's task that starts programs (build plan row 3, ob_tasks.h).
 *
 * A process of GFOpenBench's own. The desktop sends it what to start and goes on: the launcher loads the program,
 * starts it with its WBStartup message as Workbench does, answers, and unloads it when its WBStartup comes back.
 * The desktop never waits on a program or on its disk. Asked to, the launcher tells the desktop again when a
 * program it started has ended or a number of seconds are over, whichever is first (SYS:WBStartup's WAIT=n,
 * ob_seq in ob_tasks.h).
 *
 * The launcher starts each through gfopenlaunch.library (Team 2's launcher module, launcher/ol_lib.h: STACK, TOOLPRI,
 * any number of arguments) when it opens, and by itself as GFOpenBench 0.1.55 did when it doesn't.
 *
 * Everything goes by messages (ob_lreq); the launcher owns the list of programs running and nobody else reads it.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_LAUNCH_H
#define OB_LAUNCH_H

#include <exec/ports.h>
#include <dos/dos.h>

enum {
    OB_LR_START = 1,        /* start tool (with project): answers ok and the program's id */
    OB_LR_NAMES,            /* answers left (how many are running) and names */
    OB_LR_QUIT,             /* Ctrl-C to every program running, then up to secs for them to end: answers left */
    OB_LR_END,              /* answered once every program has ended; the launcher is gone then */
    OB_LR_ENDED             /* from the launcher, to a START's reply port: id has ended, or its wait is over.
                               Not to be replied: the desktop frees it with FreeVec */
};

#define OB_LR_PATH 300
#define OB_LR_ARGS 4096
#define OB_LR_TT 1024

typedef struct ob_lreq {
    struct Message msg;
    int kind;
    char tool[OB_LR_PATH];
    char args[OB_LR_ARGS];      /* START: the arguments after the tool (a project; Shift-double-click's icons), packed
                                   one after the other with their NULs (ob_pack) */
    int nargs;
    char tt[OB_LR_TT];          /* START: the icon's tool types, packed so (STACK, TOOLPRI) */
    int ntt;
    long stack, min_stack;      /* START: the icon's stack, Workbench's default */
    int wait;                   /* START: seconds before OB_LR_ENDED at the latest; 0 none sent */
    int secs;                   /* QUIT */
    /* the answer */
    int ok;
    unsigned long id;
    int left;
    char names[400];
    long err;                   /* START, not ok: dos's IoErr then (ERROR_NO_FREE_STORE for want of memory) */
    unsigned long largest;      /* START, not ok: the largest piece of memory free then (ob_low, WBGapTest X3) */
    /* the desktop's own, carried through */
    unsigned long tag;
    void *user;
    char fail[160];             /* START: what to tell the user when it couldn't start, "" nothing */
} ob_lreq;

/* The launcher started; 1 when it is there. */
int ob_launch_start(void);
/* r to the launcher (its mn_ReplyPort set by the caller). */
void ob_launch_send(ob_lreq *r);
/* The launcher's process, for the supervisor; NULL when there is none. */
struct Task *ob_launch_task(void);
/* The launcher crashed (the task guard holds it): it is let go and its request port handed back, with the requests
 * it hadn't taken still on it (the caller takes them off and sends them to the next launcher, ob_launch_start).
 * Its ports and its programs' memory stay, as Suspend leaves them; programs it started through gfopenlaunch.library
 * go on being the library's. */
struct MsgPort *ob_launch_abandon(void);
/* A lock on the drawer holding path, and the name in it (as a WBArg wants them); 0 when there is none. */
BPTR ob_parent_and_name(const char *path, char *name, int n);

#endif
