/* ---- row X3: low memory ----
 * WBGapTest LOWMEM secs path, run by WBGapX3.rexx (which prints X3): nearly all memory taken, then GFOpenBench asked
 * to open path; its low-memory requester (from the reserve pool) must come up, and its Retry is clicked once the
 * memory is given back. A hand row (H): the mode prints X3's FAIL lines only, the rexx its verdict.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/memory.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <dos/dostags.h>
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "LOWMEM.h"

WBGAP_MODE(LOWMEM, wbt_lowmem, SCRIPT)

extern struct IntuitionBase *IntuitionBase;

struct eaten { struct eaten *next; ULONG size; };

/* The front screen's low-memory requester (EasyRequest's window, titled "GFOpenBench" and holding "Retry"):
   where its left-hand button is, in screen pixels; 0 when there is none */
static int find_retry(int *x, int *y)
{
    struct Screen *s;
    struct Window *w;
    int found = 0;
    ULONG lock = LockIBase(0);
    for (s = IntuitionBase->FirstScreen; s && !found; s = s->NextScreen)
        for (w = s->FirstWindow; w && !found; w = w->NextWindow) {
            struct Gadget *g, *left = NULL;
            if (!w->Title || strcmp((const char *)w->Title, "GFOpenBench")) continue;
            for (g = w->FirstGadget; g; g = g->NextGadget)
                if ((g->GadgetType & GTYP_GTYPEMASK) == GTYP_BOOLGADGET && !(g->GadgetType & GTYP_SYSGADGET)
                    && (g->Activation & GACT_RELVERIFY) && !(g->Flags & (GFLG_RELRIGHT | GFLG_RELBOTTOM))
                    && (!left || g->LeftEdge < left->LeftEdge))
                    left = g;
            if (!left) continue;
            *x = w->LeftEdge + left->LeftEdge + left->Width / 2;
            *y = w->TopEdge + left->TopEdge + left->Height / 2;
            found = 1;
        }
    UnlockIBase(lock);
    return found;
}

/* GFOpenBench's message (gfopenprefs Common/obmsg.h: keep the layout the same, as DragTest's) */
struct ob_msg {
    struct Message m;
    ULONG magic;
    LONG ok;
    char path[256];
};

int wbt_lowmem(int argc, char **argv)
{
    int secs = argc > 3 ? atoi(argv[2]) : 0;
    const char *path = argc > 3 ? argv[3] : "";
    struct eaten *list = NULL, *e, **pp;
    ULONG n, got = 0, kept = 0;
    int x = 0, y = 0, seen, sent = 0, i;
    char cmd[300];
    struct MsgPort *reply = CreateMsgPort(), *ob;
    struct ob_msg *m = AllocVec(sizeof *m, MEMF_PUBLIC | MEMF_CLEAR);   /* before the memory is taken */
    if (argc < 4) {
        printf("usage: WBGapTest LOWMEM secs path\n");
        if (m) FreeVec(m);
        if (reply) DeleteMsgPort(reply);
        return 20;
    }
    if (!reply || !m) { printf("FAIL X3 setup\n"); if (m) FreeVec(m); if (reply) DeleteMsgPort(reply); return 20; }
    m->m.mn_ReplyPort = reply;
    m->m.mn_Length = sizeof *m;
    m->magic = 0x4F424D31UL;
    strncpy(m->path, path, sizeof m->path - 1);
    printf("INFO X3 taking the memory for %d s, opening %s\n", secs, path);
    fflush(stdout);
    for (n = 1UL << 30; n >= 256;) {                /* the biggest pieces first, down to 256 bytes */
        if (AvailMem(MEMF_ANY | MEMF_LARGEST) < n || !(e = AllocMem(n, MEMF_ANY))) { n >>= 1; continue; }
        e->size = n;
        e->next = list;
        list = e;
        got += n;
    }
    /* 48 KiB back for the system, in pieces of 4 KiB at most: no program's code fits in one. Every other piece only,
       so that no two freed pieces lie side by side and join into a bigger one (the first lab run: they did, and the
       program loaded) */
    for (pp = &list, i = 0; *pp && kept < WBGAP_LOWMEM_GIVE;) {
        e = *pp;
        if (!wbgap_lowmem_give_back(e->size, e->size <= WBGAP_LOWMEM_PIECE ? i++ : 0)) { pp = &e->next; continue; }
        *pp = e->next;
        kept += e->size;
        FreeMem(e, e->size);
    }
    printf("INFO X3 largest free piece now %lu bytes\n", (unsigned long)AvailMem(MEMF_ANY | MEMF_LARGEST));
    {   /* can a program load now at all: it mustn't, for the check to mean anything */
        BPTR seg = LoadSeg((CONST_STRPTR)path);
        printf("INFO X3 LoadSeg of %s here: %s\n", path, seg ? "loaded (memory not low enough)" : "failed, as it should");
        if (seg) UnLoadSeg(seg);
    }
    Forbid();
    if ((ob = FindPort((CONST_STRPTR)"GFOPENBENCH"))) { PutMsg(ob, &m->m); sent = 1; }
    Permit();
    /* hold it: whatever another task gives back meanwhile (more than 4 KiB in one piece) is taken too, every half
       second (the second lab run: something freed enough while it waited, and the program loaded) */
    for (i = 0; i < secs * 2; i++) {
        ULONG big;
        Delay(25);
        while ((big = AvailMem(MEMF_ANY | MEMF_LARGEST)) > 4096 && (e = AllocMem(big & ~7UL, MEMF_ANY))) {
            e->size = big & ~7UL;
            e->next = list;
            list = e;
            got += e->size;
        }
    }
    seen = find_retry(&x, &y);
    {   /* had the drawer opened already, with the memory taken (then there was nothing to ask) */
        struct Screen *s;
        struct Window *w;
        const char *name = (const char *)FilePart((CONST_STRPTR)path);
        int open = 0;
        ULONG lock = LockIBase(0);
        for (s = IntuitionBase->FirstScreen; s; s = s->NextScreen)
            for (w = s->FirstWindow; w; w = w->NextWindow)
                if (w->Title && *name && strstr((const char *)w->Title, name)) open = 1;
        UnlockIBase(lock);
        printf("INFO X3 %s's window open while the memory was taken: %s\n", name, open ? "yes" : "no");
    }
    while ((e = list)) {
        list = e->next;
        FreeMem(e, e->size);
    }
    printf("INFO X3 took %lu KiB, largest free piece then %lu bytes\n", (unsigned long)(got >> 10),
           (unsigned long)AvailMem(MEMF_ANY | MEMF_LARGEST));
    if (seen) {
        printf("INFO X3 low-memory requester open, Retry at %d,%d\n", x, y);
        snprintf(cmd, sizeof cmd, "%s CLICK %d %d", wbt_dragtest(), x, y);
        SystemTags((CONST_STRPTR)cmd, TAG_DONE);
    } else printf("FAIL X3 no low-memory requester\n");
    for (i = 0; sent && i < 100 && !GetMsg(reply); i++) Delay(10);   /* the desktop's answer, 20 s at most */
    if (!sent) printf("FAIL X3 no GFOPENBENCH port\n");
    else if (i < 100) printf("INFO X3 the desktop answered the open: ok %ld\n", (long)m->ok);
    else printf("FAIL X3 the desktop didn't answer the open\n");
    if (!sent || i < 100) { FreeVec(m); DeleteMsgPort(reply); }   /* else left: the desktop may still reply */
    return seen ? 0 : 5;
}

