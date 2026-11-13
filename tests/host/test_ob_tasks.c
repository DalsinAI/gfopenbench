/* ob_tasks' host test: the Workbench task model's supervisor, job queue and low-memory reserve (build plan row 3,
 * WBGapTest rows X3 and X4). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ob_tasks.h"

static int fails;
#define CHECK(what, cond) do { if (!(cond)) { printf("FAIL %s\n", what); fails++; } } while (0)

/* ---- a fake allocator with a budget, for the reserve ---- */
static unsigned long budget, live;
static void *fake_alloc(void *u, unsigned long n)
{
    (void)u;
    if (n > budget) return 0;
    budget -= n;
    live++;
    return malloc(n + sizeof(unsigned long));
}
static void fake_free(void *u, void *p, unsigned long n)
{
    (void)u;
    if (!p) return;
    budget += n;
    live--;
    free(p);
}
static int asked, answer_retry, free_on_ask;
static int ask(void *u)
{
    (void)u;
    asked++;
    if (free_on_ask) budget += free_on_ask, free_on_ask = 0;   /* the user closed something */
    return answer_retry-- > 0;
}

static void test_super(void)
{
    ob_super s;
    unsigned long d1, d2, l, extra[OB_SUPER_MAX];
    int i, n;
    ob_super_init(&s);
    d1 = ob_super_add(&s, OB_T_DRAWER, (void *)1);
    d2 = ob_super_add(&s, OB_T_DRAWER, (void *)2);
    l = ob_super_add(&s, OB_T_LAUNCHER, 0);
    CHECK("ids given", d1 && d2 && l && d1 != d2 && d2 != l);
    CHECK("count drawers", ob_super_count(&s, OB_T_DRAWER) == 2);
    CHECK("find user", ob_super_find(&s, d2) && ob_super_find(&s, d2)->user == (void *)2);
    /* a drawer that crashes is restarted (its window reopened on the same drawer), up to OB_RESTART_MAX times
       in OB_RESTART_WINDOW seconds, then closed */
    for (i = 0; i < OB_RESTART_MAX; i++)
        CHECK("drawer crash restarts", ob_super_crashed(&s, d1, 100 + i) == OB_ACT_RESTART);
    CHECK("drawer crashing too often is closed", ob_super_crashed(&s, d1, 104) == OB_ACT_CLOSE);
    CHECK("closed child is gone", !ob_super_find(&s, d1));
    CHECK("other drawer untouched", ob_super_find(&s, d2) && ob_super_find(&s, d2)->state == OB_S_RUNNING);
    /* crashes spread out are each restarted: the count starts again after the window */
    for (i = 0; i < 10; i++)
        CHECK("spread crashes restart", ob_super_crashed(&s, d2, 1000 + i * (OB_RESTART_WINDOW + 1)) == OB_ACT_RESTART);
    /* the launcher is restarted too, and given up (desktop carries on) only when it can't stay up */
    for (i = 0; i < OB_RESTART_MAX; i++) ob_super_crashed(&s, l, 5000);
    CHECK("launcher crashing too often closed", ob_super_crashed(&s, l, 5000) == OB_ACT_CLOSE);
    /* a normal end frees the slot */
    CHECK("ended", ob_super_ended(&s, d2) == 1 && !ob_super_find(&s, d2));
    CHECK("ended twice", ob_super_ended(&s, d2) == 0);
    CHECK("crash of unknown id", ob_super_crashed(&s, 99999, 1) == OB_ACT_NONE);
    /* full table: add answers 0, nothing overwritten */
    for (n = 0; n < OB_SUPER_MAX + 2; n++) extra[n < OB_SUPER_MAX ? n : OB_SUPER_MAX - 1] = ob_super_add(&s, OB_T_SERVICE, 0);
    CHECK("full table refuses", ob_super_add(&s, OB_T_SERVICE, 0) == 0);
    CHECK("full table count", ob_super_count(&s, OB_T_SERVICE) == OB_SUPER_MAX);
    CHECK("ids still unique", extra[0] != extra[1]);
}

static void test_jobq(void)
{
    ob_jobq q;
    unsigned long a, b, c, d;
    ob_job *j;
    ob_jobq_init(&q, 2);
    a = ob_jobq_add(&q, "Work");
    b = ob_jobq_add(&q, "work");      /* the same volume: waits for a (names compare without case) */
    c = ob_jobq_add(&q, "RAM Disk");
    d = ob_jobq_add(&q, "DH1");
    CHECK("job ids", a && b && c && d);
    j = ob_jobq_next(&q);
    CHECK("first is a", j && j->id == a);
    ob_jobq_start(&q, a);
    j = ob_jobq_next(&q);
    CHECK("b waits on its volume, c goes", j && j->id == c);
    ob_jobq_start(&q, c);
    CHECK("two running: none more", ob_jobq_next(&q) == 0);
    CHECK("progress goes on", ob_jobq_progress(&q, a, 50, 200) == 1);
    CHECK("percent", ob_jobq_percent(ob_jobq_find(&q, a)) == 25);
    /* cancelling a queued job takes it out at once; a running one is asked to stop at its next progress */
    CHECK("cancel queued d", ob_jobq_cancel(&q, d) == 1 && ob_jobq_find(&q, d)->state == OB_J_CANCELLED);
    CHECK("cancel running a", ob_jobq_cancel(&q, a) == 2);
    CHECK("a told to stop", ob_jobq_progress(&q, a, 60, 200) == 0);
    ob_jobq_finish(&q, a, 1);
    CHECK("a ends cancelled, not done", ob_jobq_find(&q, a)->state == OB_J_CANCELLED);
    j = ob_jobq_next(&q);
    CHECK("b goes once a is over", j && j->id == b);
    ob_jobq_start(&q, b);
    ob_jobq_finish(&q, c, 0);
    CHECK("c failed", ob_jobq_find(&q, c)->state == OB_J_FAILED);
    ob_jobq_finish(&q, b, 1);
    CHECK("b done", ob_jobq_find(&q, b)->state == OB_J_DONE);
    CHECK("cancel finished does nothing", ob_jobq_cancel(&q, b) == 0);
    CHECK("reap", ob_jobq_reap(&q) == 4 && q.n == 0);
    CHECK("empty", ob_jobq_next(&q) == 0);
    {   /* a full queue refuses */
        int i;
        for (i = 0; i < OB_JOBS_MAX; i++) ob_jobq_add(&q, "X");
        CHECK("full queue", ob_jobq_add(&q, "X") == 0);
    }
    CHECK("percent of unknown total", ob_jobq_percent(0) == 0);
}

static void test_reserve(void)
{
    ob_reserve r;
    ob_mem m = {fake_alloc, fake_free, ask, 0};
    void *p;
    budget = 100000;
    CHECK("reserve taken", ob_reserve_init(&r, &m, 32768) == 1 && budget == 100000 - 32768);
    /* plenty of memory: nothing asked */
    p = ob_reserve_alloc(&r, &m, 1000);
    CHECK("normal alloc", p && !asked && !r.low);
    fake_free(0, p, 1000);
    /* out of memory: the reserve is given back so the requester can open, the user asked; Cancel gives NULL */
    budget = 10;
    answer_retry = 0;
    p = ob_reserve_alloc(&r, &m, 50000);         /* more than the reserve gives back */
    CHECK("low: asked once", asked == 1);
    CHECK("low: reserve released for the requester", r.block == 0 && r.low == 1);
    CHECK("low: cancel gives NULL", p == 0);
    /* the next allocation that fits while low takes the reserve back first, as soon as it can */
    budget = 40000;
    p = ob_reserve_alloc(&r, &m, 1000);
    CHECK("refilled before the next", r.block != 0 && r.low == 0 && p != 0);
    fake_free(0, p, 1000);
    /* out of memory again; the user frees memory and answers Retry */
    budget = 10;
    asked = 0;
    answer_retry = 1;
    free_on_ask = 60000;
    p = ob_reserve_alloc(&r, &m, 50000);
    CHECK("retry after the user freed memory", p != 0 && asked == 1);
    fake_free(0, p, 50000);
    /* Retry with nothing freed asks again, Cancel then ends it */
    budget = 10;
    asked = 0;
    answer_retry = 2;
    p = ob_reserve_alloc(&r, &m, 50000);
    CHECK("retries ask again until cancel", p == 0 && asked == 3);
    ob_reserve_end(&r, &m);
    CHECK("all freed", live == 0 && r.block == 0);
    /* low, with 33,000 bytes free: the 32 KiB reserve is taken back first and a 1,000-byte request then doesn't fit;
       given back, it fits, and no requester is shown (Koenig's case, #102 review) */
    budget = 100000;
    ob_reserve_init(&r, &m, 32768);
    r.low = 1;
    fake_free(0, r.block, 32768);
    r.block = 0;
    budget = 33000;
    asked = 0;
    answer_retry = 0;
    p = ob_reserve_alloc(&r, &m, 1000);
    CHECK("fits once the reserve is back: no requester", p != 0 && asked == 0);
    fake_free(0, p, 1000);
    ob_reserve_end(&r, &m);
    /* a reserve that can't be taken at the start: low from the beginning, still asks rather than crashes */
    budget = 100;
    CHECK("no reserve at start", ob_reserve_init(&r, &m, 32768) == 0 && r.low == 1);
    asked = 0;
    answer_retry = 0;
    CHECK("no reserve, still asks", ob_reserve_alloc(&r, &m, 5000) == 0 && asked == 1);
    ob_reserve_end(&r, &m);
}


/* WBStartup through the launcher: the same order and waiting as before (ob_startup.h), the desktop never blocked */
static void test_seq(void)
{
    static const int wait[5] = {60, 0, 10, 60, 5};   /* b is DONOTWAIT */
    ob_seq s;
    ob_seq_init(&s, wait, 5);
    CHECK("seq a first", ob_seq_next(&s) == 0);
    ob_seq_opened(&s, 0, 11);                        /* a's start asked for */
    CHECK("seq waits for a's answer", ob_seq_next(&s) == OB_SEQ_WAIT);
    CHECK("other answers ignored", ob_seq_answered(&s, 99, 1, 7) == 0 && ob_seq_next(&s) == OB_SEQ_WAIT);
    CHECK("a started", ob_seq_answered(&s, 11, 1, 500) == 1);
    CHECK("seq waits for a's end", ob_seq_next(&s) == OB_SEQ_WAIT);
    CHECK("another program's end ignored", ob_seq_ended(&s, 501) == 0 && ob_seq_next(&s) == OB_SEQ_WAIT);
    CHECK("a ended (or its WAIT is over)", ob_seq_ended(&s, 500) == 1);
    CHECK("then b", ob_seq_next(&s) == 1);
    ob_seq_opened(&s, 1, 12);                        /* DONOTWAIT: started, not waited for */
    CHECK("b not waited for: c", ob_seq_next(&s) == 2);
    ob_seq_opened(&s, 2, 0);                         /* c started nothing (a drawer, refused) */
    CHECK("c nothing: d", ob_seq_next(&s) == 3);
    ob_seq_opened(&s, 3, 13);
    CHECK("d failed to start: e", ob_seq_answered(&s, 13, 0, 0) == 1 && ob_seq_next(&s) == 4);
    ob_seq_opened(&s, 4, 14);
    CHECK("late answer for b ignored", ob_seq_answered(&s, 12, 1, 600) == 0);
    ob_seq_answered(&s, 14, 1, 700);
    CHECK("e waited for", ob_seq_next(&s) == OB_SEQ_WAIT);
    ob_seq_ended(&s, 700);
    CHECK("done", ob_seq_next(&s) == OB_SEQ_DONE && ob_seq_next(&s) == OB_SEQ_DONE);
    ob_seq_init(&s, wait, 0);
    CHECK("empty drawer done", ob_seq_next(&s) == OB_SEQ_DONE);
}

static void test_names(void)
{
    const char *names[10] = {"GFOpenMeters", "100%", "c", "d", "e", "f", "g", "h", "i", "j"};
    char out[400], small[12];
    ob_names_format(out, sizeof out, names, 2);
    CHECK("names", !strcmp(out, "  GFOpenMeters\n  100_\n"));
    ob_names_format(out, sizeof out, names, 10);
    CHECK("names and more", strstr(out, "  h\n  and 2 more\n") && !strstr(out, "  i\n"));
    ob_names_format(out, sizeof out, names, 0);
    CHECK("no names", out[0] == 0);
    CHECK("short buffer ends", ob_names_format(small, sizeof small, names, 10) < (int)sizeof small && strlen(small) < sizeof small);
}

static void test_pack(void)
{
    char buf[16];
    const char *out[4];
    int n = 0;
    memset(buf, 'x', sizeof buf);
    CHECK("pack a", ob_pack(buf, sizeof buf, &n, "SYS:a") && n == 1);
    CHECK("pack empty", ob_pack(buf, sizeof buf, &n, "") && n == 2);
    CHECK("pack b", ob_pack(buf, sizeof buf, &n, "RAM:b") && n == 3);
    CHECK("too long refused", !ob_pack(buf, sizeof buf, &n, "DH0:long") && n == 3);
    CHECK("unpack", ob_unpack(buf, sizeof buf, n, out, 4) == 3 && !strcmp(out[0], "SYS:a") && !strcmp(out[1], "")
          && !strcmp(out[2], "RAM:b"));
    CHECK("unpack max", ob_unpack(buf, sizeof buf, n, out, 2) == 2);
    memset(buf, 'y', sizeof buf);
    CHECK("no NUL: nothing", ob_unpack(buf, sizeof buf, 1, out, 4) == 0);
}

static void test_guard(void)
{
    unsigned char rec[16] = {'T', 'G', 'R', 'D', 0, 0, 0, 4, 0, 0x21, 0xF0, 0x0A, 0, 0, 0x4A, 0xFC};
    ob_held h;
    CHECK("rec at", ob_guard_rec_at(0x1000, 0x2000) == 0x2000 - 56);
    CHECK("rec aligned", ob_guard_rec_at(0x1000, 0x2003) == ((0x2003UL - 56) & ~3UL));
    CHECK("stack too small", ob_guard_rec_at(0x1FF0, 0x2000) == 0);
    CHECK("bad bounds", ob_guard_rec_at(0x2000, 0x1000) == 0 && ob_guard_rec_at(0, 10) == 0);
    CHECK("read", ob_guard_read(rec, &h) == 1 && h.vector == 4 && h.pc == 0x0021F00AUL && h.addr == 0x4AFC);
    rec[7] = 2;
    CHECK("vector not the guard's", ob_guard_read(rec, &h) == 0);
    rec[7] = 3;
    rec[0] = 'X';
    CHECK("no magic", ob_guard_read(rec, &h) == 0);
    CHECK("null", ob_guard_read(0, &h) == 0);
}

int main(void)
{
    test_super();
    test_jobq();
    test_reserve();
    test_seq();
    test_names();
    test_pack();
    test_guard();
    printf("ob_tasks: %s\n", fails ? "FAILED" : "all passed");
    return fails != 0;
}
