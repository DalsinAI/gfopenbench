/* GFOpenBenchmark: memory bandwidth in Chip and Fast RAM: a 1 MiB block copied by a plain long-word
 * loop, copied by exec's CopyMemQuick (the system's own, which AmiCradle runs natively), filled,
 * and read. MB/s are MiB (1,048,576 bytes) a second of the bytes copied, written or read.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <exec/memory.h>
#include <proto/exec.h>
#include "gfb.h"

#define BLOCK (1024UL * 1024UL)

struct blk { ULONG *src, *dst; };

static double w_copy(void *u, unsigned long n)
{
    struct blk *b = u;
    while (n--) {
        const ULONG *s = b->src;
        ULONG *d = b->dst, i;
        for (i = 0; i < BLOCK / 32; i++) {
            d[0] = s[0]; d[1] = s[1]; d[2] = s[2]; d[3] = s[3];
            d[4] = s[4]; d[5] = s[5]; d[6] = s[6]; d[7] = s[7];
            d += 8; s += 8;
        }
    }
    return b->dst[5];
}

static double w_copymem(void *u, unsigned long n)
{
    struct blk *b = u;
    while (n--) CopyMemQuick(b->src, b->dst, BLOCK);
    return b->dst[7];
}

static double w_fill(void *u, unsigned long n)
{
    struct blk *b = u;
    ULONG v = 0;
    while (n--) {
        ULONG *d = b->dst, i;
        v += 0x01010101UL;
        for (i = 0; i < BLOCK / 32; i++) {
            d[0] = v; d[1] = v; d[2] = v; d[3] = v; d[4] = v; d[5] = v; d[6] = v; d[7] = v;
            d += 8;
        }
    }
    return b->dst[3];
}

static double w_read(void *u, unsigned long n)
{
    struct blk *b = u;
    ULONG sum = 0;
    while (n--) {
        const volatile ULONG *s = b->src;
        ULONG i;
        for (i = 0; i < BLOCK / 32; i++) {
            sum += s[0] + s[1] + s[2] + s[3] + s[4] + s[5] + s[6] + s[7];
            s += 8;
        }
    }
    return sum;
}

static void kind(const char *name, ULONG type, const char *k_copy, const char *k_cmq, const char *k_fill, const char *k_read)
{
    struct blk b;
    struct gfb_result r;
    char label[4][40];
    const char *keys[4] = { k_copy, k_cmq, k_fill, k_read };
    static const char *const what[4] = { "copy (long loop)", "copy (CopyMemQuick)", "fill", "read" };
    static gfb_work const work[4] = { w_copy, w_copymem, w_fill, w_read };
    int i;
    b.src = AllocMem(BLOCK, type | MEMF_CLEAR);
    b.dst = AllocMem(BLOCK, type | MEMF_CLEAR);
    for (i = 0; i < 4; i++) {
        sprintf(label[i], "%s %s", name, what[i]);
        memset(&r, 0, sizeof r);
        r.key = keys[i];
        r.label = label[i];
        r.unit = "MB/s";
        if (!b.src || !b.dst) gfb_fail(&r, "no 2 MiB of it free");
        else gfb_time(&r, work[i], &b, (double)BLOCK / 1048576.0, 1);
        gfb_report(&r);
    }
    if (b.src) FreeMem(b.src, BLOCK);
    if (b.dst) FreeMem(b.dst, BLOCK);
}

void gfb_mem_tests(void)
{
    gfb_section("Memory (1 MiB blocks)");
    kind("Fast", MEMF_FAST, "fast_copy_mbs", "fast_copymem_mbs", "fast_fill_mbs", "fast_read_mbs");
    kind("Chip", MEMF_CHIP, "chip_copy_mbs", "chip_copymem_mbs", "chip_fill_mbs", "chip_read_mbs");
}
