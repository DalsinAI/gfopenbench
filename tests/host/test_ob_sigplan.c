/* ob_sigplan's host test: one signal shared by GFOpenBench's quiet ports, and the signals a task leaves free
 * (0.1.62, WBGapTest M2, M6, M8, X3: 0.1.60's main task had none left for its requesters).
 *   test_ob_sigplan
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "ob_sigplan.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

/* a task's signals as exec hands them out: the highest free one, -1 when none */
static unsigned long alloc_mask;
static int allocs;

static int fake_alloc(void *ctx)
{
    int b;
    (void)ctx;
    allocs++;
    for (b = 31; b >= 16; b--)
        if (!(alloc_mask & (1UL << b))) {
            alloc_mask |= 1UL << b;
            return b;
        }
    return -1;
}

int main(void)
{
    ob_share s = OB_SHARE_INIT;
    int i, sig;

    /* the first port takes a signal, the others share it */
    alloc_mask = 0xffffUL;
    allocs = 0;
    sig = ob_share_take(&s, fake_alloc, NULL);
    CHECK(sig == 31 && allocs == 1, "first user takes 31 (got %d, %d allocs)", sig, allocs);
    for (i = 0; i < 8; i++) CHECK(ob_share_take(&s, fake_alloc, NULL) == 31, "user %d shares 31", i + 2);
    CHECK(allocs == 1 && s.users == 9, "nine ports, one signal (%d allocs, %d users)", allocs, s.users);
    CHECK(ob_sig_free(alloc_mask) == 15, "15 left free (%d)", ob_sig_free(alloc_mask));
    for (i = 0; i < 8; i++) CHECK(ob_share_give(&s) == -1, "not the last: nothing freed (%d)", i);
    CHECK(ob_share_give(&s) == 31 && s.sig == -1 && s.users == 0, "the last frees 31");
    CHECK(ob_share_give(&s) == -1 && s.users == 0, "one give too many does nothing");

    /* taken again after the last went: a new signal */
    alloc_mask = 0xffffUL | (1UL << 31);
    allocs = 0;
    CHECK(ob_share_take(&s, fake_alloc, NULL) == 30 && allocs == 1, "taken again: 30");
    CHECK(ob_share_give(&s) == 30, "and given back");

    /* no signal free: no port, nothing held */
    alloc_mask = 0xffffffffUL;
    CHECK(ob_share_take(&s, fake_alloc, NULL) == -1 && s.users == 0 && s.sig == -1, "none free: -1, nothing held");
    CHECK(ob_share_take(&s, NULL, NULL) == -1 && s.users == 0, "no allocator: -1");

    /* ob_sig_free: bits 16 to 31 only */
    CHECK(ob_sig_free(0x0000ffffUL) == 16, "a new task: 16 free");
    CHECK(ob_sig_free(0xffffffffUL) == 0, "all taken: 0");
    CHECK(ob_sig_free(0x3fffffffUL) == 2, "two left: 2");
    CHECK(ob_sig_free(0x00000000UL) == 16, "the system's bits don't count");

    /* 0.1.60's main task, as counted from its code: 19 wanted of 16; with the 9 quiet ports on one, 11, and
     * OB_SIG_KEEP still free */
    {
        int quiet = 9, others = 10, mask_bits = others + 1;
        unsigned long m = 0xffffUL;
        CHECK(quiet + others > 16, "0.1.60 wanted more than 16 (%d)", quiet + others);
        for (i = 0; i < mask_bits; i++) m |= 1UL << (31 - i);
        CHECK(ob_sig_free(m) >= OB_SIG_KEEP, "0.1.62 leaves %d free, %d kept for requesters", ob_sig_free(m), OB_SIG_KEEP);
    }

    printf("ob_sigplan: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
