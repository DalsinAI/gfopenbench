/* ob_sigplan: GFOpenBench's signals (0.1.62, WBGapTest M2, M6, M8 and X3), the plain-C part, kept apart so the host
 * tests reach it. A task has 16 signals of its own (bits 16 to 31). GFOpenBench 0.1.60's main task had none left, so
 * every window it opened with an IDCMP port was refused: About's and the low-memory EasyRequest, Execute Command's and
 * New drawer's name requester (lab t3-wbgap-i32, 14 November 2026: BuildEasyRequestArgs gave NULL, with a window and
 * without one). Its quiet ports now share one signal (ob_port.h), and OB_SIG_KEEP stay free for those windows.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_SIGPLAN_H
#define OB_SIGPLAN_H

/* free for the windows GFOpenBench's own task opens: EasyRequest's, and GFOpenFiles' requester's (GadTools) */
#define OB_SIG_KEEP 2

/* One signal shared by several ports: sig -1 until the first takes it, users how many hold it */
typedef struct ob_share {
    int sig;
    int users;
} ob_share;

#define OB_SHARE_INIT { -1, 0 }

/* The shared signal for one more user: the one held, or alloc(ctx)'s (a signal number, -1 when none is free) for
 * the first. -1 when there is none. */
int ob_share_take(ob_share *s, int (*alloc)(void *ctx), void *ctx);
/* One user less: the signal to free when it was the last (s back as OB_SHARE_INIT), else -1 */
int ob_share_give(ob_share *s);

/* How many of a task's own signals (bits 16 to 31) tc_SigAlloc leaves free */
int ob_sig_free(unsigned long sigalloc);

#endif
