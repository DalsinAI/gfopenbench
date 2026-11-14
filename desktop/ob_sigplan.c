/* ob_sigplan: see ob_sigplan.h. Plain C, no Amiga headers.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_sigplan.h"

int ob_share_take(ob_share *s, int (*alloc)(void *ctx), void *ctx)
{
    if (s->users <= 0) {
        int sig = alloc ? alloc(ctx) : -1;
        if (sig < 0 || sig > 31) return -1;
        s->sig = sig;
        s->users = 0;
    }
    s->users++;
    return s->sig;
}

int ob_share_give(ob_share *s)
{
    int sig;
    if (s->users <= 0) return -1;
    if (--s->users > 0) return -1;
    sig = s->sig;
    s->sig = -1;
    return sig;
}

int ob_sig_free(unsigned long sigalloc)
{
    int b, n = 0;
    for (b = 16; b < 32; b++)
        if (!(sigalloc & (1UL << b))) n++;
    return n;
}
