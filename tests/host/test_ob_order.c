/* ob_order's host test: RAM Disk first, the boot volume second, whatever the Clean Up order (GFOpenBench 0.1.46).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ob_order.h"

static int fails;
#define CHECK(what, cond) do { if (cond) printf("ok   %s\n", what); else { printf("FAIL %s\n", what); fails++; } } while (0)

typedef struct { const char *name, *kind; } vol;
static const char *boot;

/* by_order's first step, then name: what openbench.c does for the default order */
static int cmp(const void *pa, const void *pb)
{
    const vol *a = pa, *b = pb;
    int r = ob_vol_rank(a->kind, a->name, boot) - ob_vol_rank(b->kind, b->name, boot);
    return r ? r : strcmp(a->name, b->name);
}

int main(void)
{
    vol v[] = { { "Work", "Harddisk" }, { "System", "Harddisk" }, { "RAM Disk", "RAM" }, { "Games", "Harddisk" } };
    vol w[] = { { "Apps", "Harddisk" }, { "AAA", "Harddisk" }, { "Work", "Harddisk" }, { "Ram Disk", "RAM" },
                { "Workbench", "Harddisk" }, { "CD0", "CDROM" } };

    CHECK("RAM's drive is 0", ob_vol_rank("RAM", "RAM Disk", "System") == 0);
    CHECK("RAM's drive is 0 whatever its name", ob_vol_rank("RAM", "Scratch", "System") == 0);
    CHECK("the boot volume is 1, case aside", ob_vol_rank("Harddisk", "system", "System") == 1);
    CHECK("a name that only starts like the boot volume's is 2", ob_vol_rank("Harddisk", "Sys", "System") == 2);
    CHECK("a longer one is 2", ob_vol_rank("Harddisk", "Systems", "System") == 2);
    CHECK("no boot volume known: 2", ob_vol_rank("Harddisk", "System", NULL) == 2 && ob_vol_rank("Harddisk", "System", "") == 2);
    CHECK("a floppy is 2", ob_vol_rank("Disk", "Empty", "System") == 2);

    boot = "System";
    qsort(v, 4, sizeof v[0], cmp);
    CHECK("the card's four: RAM Disk, System, Games, Work",
          !strcmp(v[0].name, "RAM Disk") && !strcmp(v[1].name, "System") && !strcmp(v[2].name, "Games") && !strcmp(v[3].name, "Work"));

    boot = "Workbench";
    qsort(w, 6, sizeof w[0], cmp);
    CHECK("names before R and a boot volume after them: RAM first, then the boot volume, then the rest",
          !strcmp(w[0].name, "Ram Disk") && !strcmp(w[1].name, "Workbench") && !strcmp(w[2].name, "AAA") &&
          !strcmp(w[3].name, "Apps") && !strcmp(w[4].name, "CD0") && !strcmp(w[5].name, "Work"));

    printf(fails ? "FAIL ob_order\n" : "ok   ob_order: all cases pass\n");
    return fails != 0;
}
