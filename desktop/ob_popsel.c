/* ob_popsel.c: the menu at the pointer in a drawer window (ob_popsel.h, 0.1.74). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "ob_popsel.h"

int ob_popsel_trap(int at, int left_down, int hits)
{
    return (at == OB_POPSEL_DRAWER || at == OB_POPSEL_FILE) && !left_down && !hits;
}

const char *ob_popsel_leaf(const char *path)
{
    const char *s, *leaf;
    if (!path) return "";
    for (s = leaf = path; *s; s++)
        if (*s == ':' || *s == '/') leaf = s + 1;
    return leaf;
}

int ob_popsel_select(const char *name, const char *leaf, int keep)
{
    if (keep) return 2;
    return name && leaf && !strcmp(name, leaf) ? 1 : 0;
}

int ob_popsel_items(const ob_popsel_item *it, int n, int trash, int *pick, int max)
{
    int i, k = 0;
    for (i = 0; i < n && k < max; i++) {
        if (!it[i].label || !it[i].enabled || it[i].sub || !it[i].id) continue;
        if (!strcmp(it[i].label, "Select all")) continue;
        if (!trash && !strcmp(it[i].label, "Empty trash")) continue;
        pick[k++] = i;
    }
    return k;
}
