/* X3's give-back rule (LOWMEM.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "LOWMEM.h"

int wbgap_lowmem_give_back(unsigned long size, int nth)
{
    return size <= WBGAP_LOWMEM_PIECE && !(nth & 1);
}
