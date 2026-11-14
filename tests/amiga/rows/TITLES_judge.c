/* W3's judge (TITLES.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "TITLES.h"

/* "<path ending ':' or '/'><name> - <n> item[s]", the path one word: GFOpenBench's drawer title */
static int path_items(const char *t, const char *name, size_t n)
{
    const char *p = t;
    while (*p && *p != ' ') p++;                          /* the path and name: one word */
    if ((size_t)(p - t) <= n || strncmp(p - n, name, n) || (p[-n - 1] != ':' && p[-n - 1] != '/')) return 0;
    if (strncmp(p, " - ", 3)) return 0;
    p += 3;
    if (*p < '0' || *p > '9') return 0;
    while (*p >= '0' && *p <= '9') p++;
    return !strcmp(p, " item") || !strcmp(p, " items");
}

int wbgap_title_judge(const char *win_title, const char *name, const char *screen_title, char *why, unsigned cap)
{
    char r[320] = "";
    size_t n = name ? strlen(name) : 0;
    int v = WBGAP_FAIL;
    if (!win_title || !n) { snprintf(r, sizeof r, "the drawer's window not read"); v = WBGAP_SKIP; }
    else if (path_items(win_title, name, n)) {
        if (!screen_title || !*screen_title) snprintf(r, sizeof r, "the Workbench screen has no title");
        else {
            snprintf(r, sizeof r, "chosen difference: titled \"%.80s\" (path - N items), kept by Dale's decision "
                     WBGAP_TITLE_DECISION, win_title);
            v = WBGAP_PASS;
        }
    } else if (strncmp(win_title, name, n) || (win_title[n] && win_title[n] != ' '))
        snprintf(r, sizeof r, "the drawer's window is titled \"%.80s\", not with its name \"%.40s\"", win_title, name);
    else if (!screen_title || !*screen_title) snprintf(r, sizeof r, "the Workbench screen has no title");
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
