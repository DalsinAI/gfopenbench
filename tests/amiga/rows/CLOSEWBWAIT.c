/* ---- CLOSEWBWAIT: C6's watcher (rows/CLOSEWB.c) ----
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/tasks.h>
#include <exec/execbase.h>
#include <dos/dos.h>
#include <dos/dosextens.h>
#include <proto/exec.h>
#include <proto/dos.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "CLOSEWB.h"

WBGAP_MODE(CLOSEWBWAIT, wbt_closewbwait, SCRIPT)

/* CLOSEWBWAIT file secs: the script starts CLOSEWB with Run >file, then this
 * waits up to secs for it to be over (wbgap_closewb_over) and prints what it
 * wrote. A CloseWorkBench() that never returns (lab t2-killcrazy-wbgap, 14
 * Nov, before amicradle-guest #752) is then FAIL C6 here, and the run goes
 * on to REPORT; the CLOSEWB process is left as it is, with its own code.
 * The FAIL names the last "INFO C6 step:" line CLOSEWB wrote (from
 * WBGAP_C6_STEPS, which CLOSEWB closes after each line, else the Run file),
 * so a run shows which step hung (wbgap_closewb_last_step); an IoErr() on
 * opening the Run file is printed too, as Ackerman asked on #222.
 * When CLOSEWB had already decided C6 (wbgap_closewb_verdict) and only its
 * clean-up hung, C6 is that verdict and C6a is a FAIL naming the step.
 * CLOSEWB is over when "INFO C6 done" is in the Run file OR the steps file:
 * on 3.2.3 (15 Nov 00:32 lab) CLOSEWB ended, Status listing no WBGapTest,
 * but none of its stdout reached the Run file, so it looked hung; its
 * verdicts are then taken from the steps file (wbgap_closewb_row_verdict). */
static long read_text(const char *path, char *to, long cap, long *ioerr)
{
    BPTR fh;
    long n = 0;
    to[0] = 0;
    if ((fh = Open((CONST_STRPTR)path, MODE_OLDFILE))) {
        n = Read(fh, to, cap - 1);
        to[n > 0 ? n : 0] = 0;
        Close(fh);
        if (ioerr) *ioerr = 0;
    } else if (ioerr) *ioerr = IoErr();
    return n;
}

/* one of CLOSEWB's rows: its line already printed from buf, else from the steps file, else the FAIL or SKIP given */
static void closewb_row(const char *row, const char *buf, const char *steps, int none_state, const char *none_why)
{
    char said[128];
    int v;
    if (wbgap_closewb_row_verdict(buf, row, said, sizeof said) != WBGAP_NONE) return;   /* printed with buf */
    if ((v = wbgap_closewb_row_verdict(steps, row, said, sizeof said)) != WBGAP_NONE) wbt_say(row, v, said);
    else wbt_say(row, none_state, none_why);
}

/* where GFOpenBench's task is, read under Forbid from its GFOPENBENCH port's task (wbgap_closewb_obwait) */
static void openbench_where(void)
{
    struct MsgPort *pub, *rexx;
    struct Task *t;
    ULONG sigwait = 0, recvd = 0, sp = 0, at_sp = 0;
    int found = 0, waiting = 0, pub_bit = -1, rexx_bit = -1, where;
    Forbid();
    /* mp_SigTask is a task only on a PA_SIGNAL port (a PA_SOFTINT one's is an Interrupt: Ackerman, #223) */
    if ((pub = FindPort((CONST_STRPTR)"GFOPENBENCH")) && (pub->mp_Flags & PF_ACTION) == PA_SIGNAL &&
        (t = pub->mp_SigTask)) {
        found = 1;
        waiting = t->tc_State == TS_WAIT;
        sigwait = t->tc_SigWait;
        recvd = t->tc_SigRecvd;
        sp = (ULONG)t->tc_SPReg;
        if (sp >= (ULONG)t->tc_SPLower && sp + 4 <= (ULONG)t->tc_SPUpper && !(sp & 1)) at_sp = *(ULONG *)sp;
        pub_bit = pub->mp_SigBit;
        if ((rexx = FindPort((CONST_STRPTR)"OPENBENCH")) && (rexx->mp_Flags & PF_ACTION) == PA_SIGNAL &&
            rexx->mp_SigTask == t)
            rexx_bit = rexx->mp_SigBit;
    }
    Permit();
    where = wbgap_closewb_obwait(found, waiting, sigwait, pub_bit, rexx_bit);
    if (!found) printf("INFO C6 GFOpenBench: %s\n", wbgap_closewb_obwait_text(where));
    else printf("INFO C6 GFOpenBench's task is %s: waits for $%08lx, has $%08lx; GFOPENBENCH bit %d, OPENBENCH bit %d; "
                "SP $%08lx: $%08lx\n", wbgap_closewb_obwait_text(where), (unsigned long)sigwait, (unsigned long)recvd,
                pub_bit, rexx_bit, (unsigned long)sp, (unsigned long)at_sp);
}

/* GFOpenBench's drawer tasks (ob_dtask.c names them "GFOpenBench drawer"): each one's state and signals, so a
 * drawer that never takes its close (Ctrl-F, DT_NOTE_SIG) shows up next to the desktop that waits for it */
static void drawers_where(void)
{
    struct { int state; ULONG wait, recvd, sp, at_sp; } d[8];
    struct List *lists[2];
    struct Node *n;
    int i, k = 0;
    lists[0] = &SysBase->TaskWait;
    lists[1] = &SysBase->TaskReady;
    Forbid();
    for (i = 0; i < 2; i++)
        for (n = lists[i]->lh_Head; n->ln_Succ && k < 8; n = n->ln_Succ)
            if (n->ln_Name && !strcmp(n->ln_Name, "GFOpenBench drawer")) {
                struct Task *t = (struct Task *)n;
                d[k].state = t->tc_State;
                d[k].wait = t->tc_SigWait;
                d[k].recvd = t->tc_SigRecvd;
                d[k].sp = (ULONG)t->tc_SPReg;
                d[k].at_sp = d[k].sp >= (ULONG)t->tc_SPLower && d[k].sp + 4 <= (ULONG)t->tc_SPUpper && !(d[k].sp & 1) ?
                             *(ULONG *)d[k].sp : 0;
                k++;
            }
    Permit();
    printf("INFO C6 GFOpenBench drawer tasks: %d\n", k);
    for (i = 0; i < k; i++)
        printf("INFO C6 drawer task %d: tc_State %d, waits for $%08lx, has $%08lx; SP $%08lx: $%08lx\n", i,
               d[i].state, (unsigned long)d[i].wait, (unsigned long)d[i].recvd, (unsigned long)d[i].sp,
               (unsigned long)d[i].at_sp);
}

/* Where in GFOpenBench a task is: OpenBench's segments (its CLI's cli_Module) and, for each of its tasks,
 * the longs on its stack that point into them or into the ROM, so the build's symbols name the calls it is in */
#define OBW_SEGS 8
#define OBW_DEPTH 64
static ULONG obw_seg[OBW_SEGS], obw_len[OBW_SEGS];
static int obw_nseg;

static void openbench_segs(void)
{
    LONG n, max;
    obw_nseg = 0;
    Forbid();
    max = MaxCli();
    for (n = 1; n <= max && !obw_nseg; n++) {
        struct Process *p = FindCliProc((ULONG)n);
        struct CommandLineInterface *cli = p ? (struct CommandLineInterface *)BADDR(p->pr_CLI) : NULL;
        UBYTE *name = cli ? (UBYTE *)BADDR(cli->cli_CommandName) : NULL;
        BPTR seg;
        if (!name || name[0] < 9 || memcmp(name + 1 + name[0] - 9, "OpenBench", 9)) continue;
        for (seg = cli->cli_Module; seg && obw_nseg < OBW_SEGS; seg = *(BPTR *)BADDR(seg)) {
            ULONG *sp = (ULONG *)BADDR(seg);
            obw_seg[obw_nseg] = (ULONG)(sp + 1);
            obw_len[obw_nseg] = sp[-1] - 8;
            obw_nseg++;
        }
    }
    Permit();
    for (n = 0; n < obw_nseg; n++)
        printf("INFO C6 OpenBench hunk %ld at $%08lx, %lu bytes\n", (long)n, (unsigned long)obw_seg[n],
               (unsigned long)obw_len[n]);
}

/* the longs on t's stack that point into OpenBench's hunks or the ROM; under Forbid, copied, printed after */
static void stack_where(const char *who, struct Task *t)   /* t NULL: GFOPENBENCH's */
{
    ULONG at[OBW_DEPTH], v[OBW_DEPTH], sp, lo, hi;
    int n = 0, i, k, there = 0;
    struct Node *nd;
    Forbid();
    if (!t) {                           /* the desktop: GFOPENBENCH's task, alive while its port is there */
        struct MsgPort *pub = FindPort((CONST_STRPTR)"GFOPENBENCH");
        if (pub && (pub->mp_Flags & PF_ACTION) == PA_SIGNAL && (t = pub->mp_SigTask)) there = 1;
    }
    for (nd = SysBase->TaskWait.lh_Head; nd->ln_Succ && !there; nd = nd->ln_Succ) there = (struct Task *)nd == t;
    for (nd = SysBase->TaskReady.lh_Head; nd->ln_Succ && !there; nd = nd->ln_Succ) there = (struct Task *)nd == t;
    if (!there) {
        Permit();
        printf("INFO C6 %s ended\n", who);
        return;
    }
    sp = (ULONG)t->tc_SPReg;
    lo = (ULONG)t->tc_SPLower;
    hi = (ULONG)t->tc_SPUpper;
    if (!(sp & 1) && sp >= lo)
        for (i = 0; i < 1024 && n < OBW_DEPTH && sp + 4 * i + 4 <= hi; i++) {
            ULONG x = ((ULONG *)sp)[i];
            int ours = x >= 0xF80000 && x < 0x1000000;
            for (k = 0; k < obw_nseg && !ours; k++) ours = x >= obw_seg[k] && x < obw_seg[k] + obw_len[k];
            if (ours) { at[n] = (ULONG)i * 4; v[n] = x; n++; }
        }
    Permit();
    for (i = 0; i < n; i++) {
        int h = -1;
        for (k = 0; k < obw_nseg; k++)
            if (v[i] >= obw_seg[k] && v[i] < obw_seg[k] + obw_len[k]) h = k;
        if (h >= 0)
            printf("INFO C6 %s SP+%lu: $%08lx = hunk %d +$%lx\n", who, (unsigned long)at[i], (unsigned long)v[i], h,
                   (unsigned long)(v[i] - obw_seg[h]));
        else
            printf("INFO C6 %s SP+%lu: $%08lx = ROM +$%lx\n", who, (unsigned long)at[i], (unsigned long)v[i],
                   (unsigned long)(v[i] - 0xF80000));
    }
}

/* GFOpenBench's own tasks (the desktop, "GFOpenBench" and its drawers) found by name, each one's stack read */
static void tasks_where(void)
{
    static const char *names[] = { "GFOpenBench", "GFOpenBench drawer" };
    struct List *lists[2];
    struct Task *found[8];
    char label[48];
    struct Node *nd;
    int i, j, nf = 0;
    lists[0] = &SysBase->TaskWait;
    lists[1] = &SysBase->TaskReady;
    stack_where("desktop (GFOPENBENCH's task)", NULL);   /* on no list while the runtime holds it */
    for (j = 0; j < 2; j++) {
        Forbid();
        nf = 0;
        for (i = 0; i < 2; i++)
            for (nd = lists[i]->lh_Head; nd->ln_Succ && nf < 8; nd = nd->ln_Succ)
                if (nd->ln_Name && !strcmp(nd->ln_Name, names[j])) found[nf++] = (struct Task *)nd;
        Permit();
        for (i = 0; i < nf; i++) {
            snprintf(label, sizeof label, "%s %d", j ? "drawer" : "desktop", i);
            stack_where(label, found[i]);   /* looked for again under its own Forbid: one that ended isn't read */
        }
    }
}

static int closewb_wait(const char *path, int secs)
{
    static char buf[4096], steps[2048];
    int t, over = 0;
    long ioerr = 0;
    for (t = 0; t <= secs * 2 && !over; t++) {
        if (t) Delay(25);
        read_text(path, buf, sizeof buf, &ioerr);
        read_text(WBGAP_C6_STEPS, steps, sizeof steps, NULL);
        over = wbgap_closewb_over(buf) || wbgap_closewb_over(steps);
    }
    if (buf[0]) printf("%s%s", buf, buf[strlen(buf) - 1] == '\n' ? "" : "\n");
    if (ioerr) printf("INFO C6 %s couldn't be opened to read (IoErr %ld)\n", path, ioerr);
    if (over) {
        /* ended: its verdicts, from buf or the steps file (the Run file may have none of its stdout) */
        closewb_row("C6", buf, steps, WBGAP_FAIL, "CLOSEWB ended without a C6 line");
        closewb_row("C6a", buf, steps, WBGAP_FAIL, "CLOSEWB ended without a C6a line");
        DeleteFile((CONST_STRPTR)WBGAP_C6_STEPS);    /* a row that ended has no use for its steps; a hung one keeps them */
    } else {
        char why[192], step[96];
        openbench_where();                          /* a CLOSEWB that doesn't end: is GFOpenBench answering? */
        Delay(10);
        openbench_where();                          /* twice: a task that spins shows "running" both times */
        drawers_where();
        openbench_segs();
        tasks_where();
        /* the steps file first: closed after each line, it has every step; buf may be held or short */
        if (!wbgap_closewb_last_step(steps, step, sizeof step)) wbgap_closewb_last_step(buf, step, sizeof step);
        if (wbgap_closewb_verdict(buf, why, sizeof why) != WBGAP_NONE || wbgap_closewb_verdict(steps, why, sizeof why) != WBGAP_NONE) {
            /* C6 decided; its clean-up hung: a real fault, counted as C6a (Ackerman, #222) */
            snprintf(why, sizeof why, "%s didn't end in %d s (left running)", step[0] ? step : "the clean-up", secs);
            closewb_row("C6", buf, steps, WBGAP_FAIL, "");
            closewb_row("C6a", buf, steps, WBGAP_FAIL, why);
        } else {
            if (step[0])
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); last step: %s",
                         secs, step);
            else
                snprintf(why, sizeof why, "CloseWorkBench() or the row after it didn't return in %d s (left running); no step written",
                         secs);
            wbt_fail("C6", why);
            wbt_skip("C6a", "C6 didn't decide");
        }
    }
    return wbt_fails() ? 5 : 0;
}

int wbt_closewbwait(int argc, char **argv)
{
    if (argc < 4) { printf("usage: WBGapTest CLOSEWBWAIT <file> <secs>\n"); return 20; }
    return closewb_wait(argv[2], atoi(argv[3]));
}
