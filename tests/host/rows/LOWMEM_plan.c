/* X3: which pieces the low-memory mode gives back (tests/amiga/rows/LOWMEM_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "LOWMEM.h"

void lowmem_plan(void);
WBGAP_PLAN(lowmem_plan)

void lowmem_plan(void)
{
    /* the pieces as the mode takes them, biggest first (1 GiB halving to 256 bytes), then the walk it makes */
    unsigned long sizes[] = {1UL << 20, 65536, 8192, 4097, 4096, 4096, 2048, 1024, 512, 256, 256};
    unsigned long kept = 0;
    int i, nth = 0, last = -2, apart = 1, n = (int)(sizeof sizes / sizeof sizes[0]);
    CHECK(!wbgap_lowmem_give_back(4097, 0) && !wbgap_lowmem_give_back(1UL << 20, 0), "over 4 KiB: kept taken");
    CHECK(wbgap_lowmem_give_back(4096, 0) && wbgap_lowmem_give_back(256, 2), "4 KiB or less, an even one: given back");
    CHECK(!wbgap_lowmem_give_back(4096, 1) && !wbgap_lowmem_give_back(256, 3), "an odd one: kept, so no two freed touch");
    for (i = 0; i < n && kept < WBGAP_LOWMEM_GIVE; i++) {
        if (wbgap_lowmem_give_back(sizes[i], sizes[i] <= WBGAP_LOWMEM_PIECE ? nth++ : 0)) {
            if (last == i - 1) apart = 0;
            last = i;
            kept += sizes[i];
        }
    }
    CHECK(apart && kept == 4096 + 2048 + 512 + 256, "the walk: 4096, 2048, 512, 256 given back, none side by side (%lu)", kept);
}
