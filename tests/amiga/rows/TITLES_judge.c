/* W3's judge (TITLES.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "TITLES.h"

int wbgap_title_judge(const char *win_title, const char *name, const char *screen_title, char *why, unsigned cap)
{
    char r[200] = "";
    size_t n = name ? strlen(name) : 0;
    int v = WBGAP_FAIL;
    if (!win_title || !n) { snprintf(r, sizeof r, "the drawer's window not read"); v = WBGAP_SKIP; }
    else if (strncmp(win_title, name, n) || (win_title[n] && win_title[n] != ' '))
        snprintf(r, sizeof r, "the drawer's window is titled \"%.80s\", not with its name \"%.40s\"", win_title, name);
    else if (!screen_title || !*screen_title) snprintf(r, sizeof r, "the Workbench screen has no title");
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
