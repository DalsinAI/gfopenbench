/* ob_rexx.c: GFOpenBench's ARexx commands, the plain C half (ob_rexx.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_rexx.h"

/* The templates are OS 3.2.3's Workbench's (its HELP COMMAND gives them), so scripts written for it parse the same. */
const ob_rexx_cmd ob_rexx_cmds[OB_RX_COUNT] = {
    { "ACTIVATEWINDOW", "WINDOW", 0 },
    { "CHANGEWINDOW", "WINDOW,LEFTEDGE/N,TOPEDGE/N,WIDTH/N,HEIGHT/N", 0 },
    { "DELETE", "NAME/A,ALL/S", 0 },
    { "FAULT", "CODE/A/N", 0 },
    { "GETATTR", "OBJECT/A,NAME/K,STEM/K,VAR/K", 0 },
    { "HELP", "COMMAND/K,MENUS/S,PROMPT/S", 0 },
    { "ICON", "WINDOW,NAMES/M,OPEN/S,MAKEVISIBLE/S,SELECT/S,UNSELECT/S,UP/N,DOWN/N,LEFT/N,RIGHT/N,X/N,Y/N,"
              "ACTIVATE/K,CYCLE/K,MOVE/K", 0 },
    { "INFO", "NAME/A", 0 },
    { "KEYBOARD", "NAME/A,ADD/S,REMOVE/S,KEY,CMD/F", 0 },
    { "LOCKGUI", "", 0 },
    { "MENU", "WINDOW/K,INVOKE,NAME/K,TITLE/K,SHORTCUT/K,ADD/S,REMOVE/S,CMD/K/F", 0 },
    { "MOVEWINDOW", "WINDOW,LEFTEDGE/N,TOPEDGE/N", 0 },
    { "NEWDRAWER", "NAME/A", 0 },
    { "RENAME", "OLDNAME/A,NEWNAME/A", 0 },
    { "RX", "CONSOLE/S,ASYNC/S,CMD/A/F", 0 },
    { "SIZEWINDOW", "WINDOW,WIDTH/N,HEIGHT/N", 0 },
    { "UNLOCKGUI", "", 0 },
    { "UNZOOMWINDOW", "WINDOW", 0 },
    { "VIEW", "WINDOW,PAGE/S,PIXEL/S,UP/S,DOWN/S,LEFT/S,RIGHT/S", 0 },
    { "WINDOW", "WINDOWS/M/A,OPEN/S,CLOSE/S,SNAPSHOT/S,ACTIVATE/S,MIN/S,MAX/S,FRONT/S,BACK/S,CYCLE/K", 0 },
    { "WINDOWTOBACK", "WINDOW", 0 },
    { "WINDOWTOFRONT", "WINDOW", 0 },
    { "ZOOMWINDOW", "WINDOW", 0 },
    { "OPEN", "NAME/A", OB_REXX_OWN },
    { "VERSION", "", OB_REXX_OWN },
};

const char *const ob_rexx_port_names[OB_REXX_PORTS] = { "WORKBENCH", "OPENBENCH" };

static int up(int c)
{
    return c >= 'a' && c <= 'z' ? c - 32 : c;
}

static int space(int c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int ob_rexx_find(const char *line, const char **args, int own)
{
    const char *w, *e;
    int i;
    size_t n;
    if (!line) return -1;
    for (w = line; space((unsigned char)*w); w++)
        ;
    for (e = w; *e && !space((unsigned char)*e); e++)
        ;
    n = (size_t)(e - w);
    if (!n) return -1;
    for (i = 0; i < OB_RX_COUNT; i++) {
        const char *c = ob_rexx_cmds[i].name;
        size_t k;
        if ((ob_rexx_cmds[i].flags & OB_REXX_OWN) && !own) continue;
        if (strlen(c) != n) continue;
        for (k = 0; k < n && up((unsigned char)w[k]) == c[k]; k++)
            ;
        if (k < n) continue;
        while (space((unsigned char)*e)) e++;
        if (args) *args = e;
        return i;
    }
    return -1;
}

int ob_rexx_attr(const char *obj, const char *pattern, long *nums, int max)
{
    const char *o = obj, *p = pattern;
    int got = 0;
    if (!o || !p) return 0;
    while (*p) {
        if (*p == '#') {
            long v = 0;
            int digits = 0;
            while (*o >= '0' && *o <= '9') {
                if (v > 99999999L) return 0;       /* past any list's end */
                v = v * 10 + (*o++ - '0');
                digits++;
            }
            if (!digits) return 0;
            if (got < max && nums) nums[got] = v;
            got++;
            p++;
            continue;
        }
        if (up((unsigned char)*o) != up((unsigned char)*p)) return 0;
        o++;
        p++;
    }
    return *o == 0;
}

int ob_rexx_help(char *out, int n, const char *command, int own)
{
    int i, at = 0;
    if (!out || n < 1) return 0;
    out[0] = 0;
    if (command && *command) {
        const char *rest;
        if ((i = ob_rexx_find(command, &rest, own)) < 0 || *rest) return 0;
        snprintf(out, (size_t)n, "%s", ob_rexx_cmds[i].template);
        return 1;
    }
    for (i = 0; i < OB_RX_COUNT; i++) {
        int w;
        if ((ob_rexx_cmds[i].flags & OB_REXX_OWN) && !own) continue;
        w = snprintf(out + at, (size_t)(n - at), "%s%s", at ? " " : "", ob_rexx_cmds[i].name);
        if (w < 0 || w >= n - at) {
            out[at] = 0;                            /* only whole names */
            break;
        }
        at += w;
    }
    return 1;
}
