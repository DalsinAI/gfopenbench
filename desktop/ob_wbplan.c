/* ob_wbplan: see ob_wbplan.h. Plain C, no Amiga headers.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "ob_wbplan.h"

#define ROLE_PARENT 1
#define ROLE_TITLE  2

const char *ob_wbp_split(const char *path, char *dir, int n)
{
    const char *part = path, *c;
    int len;
    for (c = path; *c; c++)
        if (*c == ':' || *c == '/') part = c + 1;
    len = (int)(part - path);
    if (len > 0 && path[len - 1] == '/') len--;
    if (n <= 0) return part;
    if (len >= n) len = n - 1;
    memcpy(dir, path, (size_t)len);
    dir[len] = 0;
    return part;
}

int ob_wbp_hook_answer(long r)
{
    switch (r) {
    case OB_WBP_UNSELECT: return 0;
    case OB_WBP_SELECT: return 1;
    case OB_WBP_STOP: return -1;
    default: return 2;
    }
}

int ob_wbp_join(const char *dir, const char *name, char *out, int n)
{
    size_t dl, nl;
    int sep;
    if (n <= 0) return 0;
    out[0] = 0;
    if (!dir) dir = "";
    if (!name) name = "";
    dl = strlen(dir);
    nl = strlen(name);
    if (!dl && nl) return 0;
    sep = nl && dir[dl - 1] != ':' && dir[dl - 1] != '/';
    if (dl + (size_t)sep + nl >= (size_t)n) return 0;
    memcpy(out, dir, dl);
    if (sep) out[dl] = '/';
    memcpy(out + dl + (size_t)sep, name, nl + 1);
    return 1;
}

static int add(ob_wbp_row *out, int k, int max, int index, int level)
{
    if (k < max) {
        out[k].index = index;
        out[k].level = level;
    }
    return k + 1;
}

static int items(const ob_wbp_menu *m, int n, const void *under, ob_wbp_row *out, int k, int max)
{
    int i, j;
    for (i = 0; i < n; i++) {
        if (m[i].parent != under || m[i].role == ROLE_TITLE || !m[i].visible) continue;
        k = add(out, k, max, i, 1);
        if (m[i].role == ROLE_PARENT)
            for (j = 0; j < n; j++)
                if (m[j].parent == m[i].handle && m[j].role != ROLE_TITLE && m[j].visible) k = add(out, k, max, j, 2);
    }
    return k;
}

int ob_wbp_items(const ob_wbp_menu *m, int n, const void *under, ob_wbp_row *out, int max)
{
    int k = items(m, n, under, out, 0, max);
    return k < max ? k : max;
}

int ob_wbp_own(const ob_wbp_menu *m, int n, ob_wbp_row *out, int max)
{
    int i, k = 0;
    for (i = 0; i < n; i++)
        if (m[i].role == ROLE_TITLE && m[i].visible) {
            k = add(out, k, max, i, 0);
            k = items(m, n, m[i].handle, out, k, max);
        }
    return k < max ? k : max;
}

int ob_wbp_any_tools(const ob_wbp_menu *m, int n)
{
    int i;
    for (i = 0; i < n; i++)
        if (m[i].visible && m[i].role != ROLE_TITLE && !m[i].parent) return 1;
    return 0;
}

int ob_wbp_tools_at(const char *const *titles, int nt, int any, int *own)
{
    int i;
    *own = 0;
    for (i = 0; i < nt; i++)
        if (titles[i] && !strcmp(titles[i], "Tools")) {
            *own = 1;
            return i;
        }
    if (!any) return -1;
    for (i = 0; i < nt; i++)
        if (titles[i] && !strcmp(titles[i], "Icons")) return i + 1;
    return nt;
}
