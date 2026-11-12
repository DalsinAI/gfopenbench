/* ob_rexx.h: GFOpenBench's ARexx ports (0.1.50; gap rows R1 and R2, amicradle-teams
 * designs/OPENBENCH-GAPS-CLOSE-PLAN-20261112.md item 7). The plain C half, with no Amiga headers, tested on the host
 * (tests/host/test_ob_rexx.c): the commands and their ReadArgs templates, finding a command in the line a script
 * sent, and matching GETATTR's object names. The ports themselves, the messages and what each command does are in
 * ob_arexx.c.
 *
 * GFOpenBench opens two public ports (Dale, 12 November 2026, 16:17: "it should declare itself openbench and
 * workbench"): WORKBENCH, so existing scripts written for OS 3.2.3's Workbench work as they are, and OPENBENCH,
 * GFOpenBench's own name. Both take OS 3.2.3's commands; GFOpenBench's own extra commands are taken only on
 * OPENBENCH (OB_REXX_OWN). A name already in use (a real Workbench running) is left to its owner: that port is not
 * opened, which is said once, and the other still is.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_REXX_H
#define OB_REXX_H

/* the commands, in OS 3.2.3's order (its HELP lists them alphabetically, as here) */
enum {
    OB_RX_ACTIVATEWINDOW = 0,
    OB_RX_CHANGEWINDOW,
    OB_RX_DELETE,
    OB_RX_FAULT,
    OB_RX_GETATTR,
    OB_RX_HELP,
    OB_RX_ICON,
    OB_RX_INFO,
    OB_RX_KEYBOARD,
    OB_RX_LOCKGUI,
    OB_RX_MENU,
    OB_RX_MOVEWINDOW,
    OB_RX_NEWDRAWER,
    OB_RX_RENAME,
    OB_RX_RX,
    OB_RX_SIZEWINDOW,
    OB_RX_UNLOCKGUI,
    OB_RX_UNZOOMWINDOW,
    OB_RX_VIEW,
    OB_RX_WINDOW,
    OB_RX_WINDOWTOBACK,
    OB_RX_WINDOWTOFRONT,
    OB_RX_ZOOMWINDOW,
    /* GFOpenBench's own, on OPENBENCH only */
    OB_RX_OPEN,             /* OPEN NAME/A: anything, as a double-click opens it */
    OB_RX_VERSION,          /* VERSION: GFOpenBench's version, as RESULT */
    OB_RX_COUNT
};

/* a command's flags */
#define OB_REXX_OWN 1       /* GFOpenBench's own: OPENBENCH only */

typedef struct ob_rexx_cmd {
    const char *name;
    const char *template;   /* its ReadArgs template */
    int flags;
} ob_rexx_cmd;

extern const ob_rexx_cmd ob_rexx_cmds[OB_RX_COUNT];

/* the ports, WORKBENCH first */
#define OB_REXX_PORTS 2
extern const char *const ob_rexx_port_names[OB_REXX_PORTS];

/* The command a line starts with (its first word, any case), or -1. *args: the rest of the line, after the
 * spaces that follow the word. own: the line came to OPENBENCH, where GFOpenBench's own commands are known too. */
int ob_rexx_find(const char *line, const char **args, int own);

/* GETATTR's object names. 1 when obj ("Window.Icons.All.3.Name", any case) is pattern, where each '#' in the pattern
 * is a number (0 or more), put into nums[] in order (at most max of them); else 0. */
int ob_rexx_attr(const char *obj, const char *pattern, long *nums, int max);

/* HELP's answer, into out (n bytes): with no command, every command's name, one space between (own: GFOpenBench's
 * too); with one, its template. 0 when the command isn't one. */
int ob_rexx_help(char *out, int n, const char *command, int own);

#endif
