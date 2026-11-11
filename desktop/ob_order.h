/* ob_order.h: which disks lead the desktop's column (GFOpenBench 0.1.46). Plain C with no Amiga headers, tested on
 * the host (tests/host/test_ob_order.c).
 *
 * Dale, 11 November 2026, 15:36: "RAM should always come first on the column." Up to 0.1.45 the disks went down the
 * column in the Clean Up order alone (by name by default), so "Games" came before "RAM Disk". Since 0.1.46 the RAM
 * disk's drive comes first and the boot volume (SYS:'s) second, whatever the Clean Up order; the other disks follow in
 * that order. A disk mounted later never comes before them. A disk the user put somewhere (dragged, or Snapshot)
 * stays where it was put, as before.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_ORDER_H
#define OB_ORDER_H

/* 0: the RAM disk's drive (kind "RAM", drive_kind's); 1: the boot volume, named boot (case aside; NULL or "" when
 * not known); 2: any other disk. */
static inline int ob_vol_rank(const char *kind, const char *name, const char *boot)
{
    const char *a, *b;
    if (kind && kind[0] == 'R' && kind[1] == 'A' && kind[2] == 'M' && !kind[3]) return 0;
    if (!boot || !boot[0] || !name) return 2;
    for (a = name, b = boot; *a && *b; a++, b++) {
        char x = *a, y = *b;
        if (x >= 'A' && x <= 'Z') x = (char)(x + 32);
        if (y >= 'A' && y <= 'Z') y = (char)(y + 32);
        if (x != y) return 2;
    }
    return *a || *b ? 2 : 1;
}

#endif
