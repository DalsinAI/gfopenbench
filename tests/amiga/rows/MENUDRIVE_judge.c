/* M8's judge (MENUDRIVE.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "MENUDRIVE.h"

int wbgap_menudrive_new_dir(const char (*before)[32], int n_before, const char (*after)[32], int n_after)
{
    int i, j;
    for (i = 0; i < n_after; i++) {
        for (j = 0; j < n_before && strcmp(before[j], after[i]); j++) {}
        if (j == n_before) return i;
    }
    return -1;
}
