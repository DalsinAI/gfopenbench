/* ob_tasks.c: the Workbench task model's bookkeeping (ob_tasks.h). Plain C, tested on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_tasks.h"

static void zero(void *p, unsigned long n)
{
    unsigned char *b = p;
    while (n--) *b++ = 0;
}

/* ---- the supervisor ---- */
void ob_super_init(ob_super *s)
{
    zero(s, sizeof *s);
    s->next_id = 1;
}

unsigned long ob_super_add(ob_super *s, int kind, void *user)
{
    int i;
    for (i = 0; i < OB_SUPER_MAX; i++)
        if (!s->c[i].id) {
            ob_child *c = &s->c[i];
            zero(c, sizeof *c);
            c->id = s->next_id++;
            if (!s->next_id) s->next_id = 1;
            c->kind = kind;
            c->state = OB_S_RUNNING;
            c->user = user;
            return c->id;
        }
    return 0;
}

ob_child *ob_super_find(ob_super *s, unsigned long id)
{
    int i;
    if (!id) return 0;
    for (i = 0; i < OB_SUPER_MAX; i++)
        if (s->c[i].id == id) return &s->c[i];
    return 0;
}

int ob_super_count(const ob_super *s, int kind)
{
    int i, n = 0;
    for (i = 0; i < OB_SUPER_MAX; i++)
        if (s->c[i].id && s->c[i].kind == kind) n++;
    return n;
}

int ob_super_ended(ob_super *s, unsigned long id)
{
    ob_child *c = ob_super_find(s, id);
    if (!c) return 0;
    zero(c, sizeof *c);
    return 1;
}

int ob_super_crashed(ob_super *s, unsigned long id, unsigned long now)
{
    ob_child *c = ob_super_find(s, id);
    if (!c) return OB_ACT_NONE;
    if (!c->crashes || now - c->first_crash > OB_RESTART_WINDOW) {
        c->crashes = 0;
        c->first_crash = now;
    }
    if (++c->crashes > OB_RESTART_MAX) {
        zero(c, sizeof *c);
        return OB_ACT_CLOSE;
    }
    return OB_ACT_RESTART;
}

/* ---- the jobs ---- */
static int same_vol(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a, y = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (x != y) return 0;
        if (!x) return 1;
    }
}

void ob_jobq_init(ob_jobq *q, int max_running)
{
    zero(q, sizeof *q);
    q->max_running = max_running > 0 ? max_running : 1;
    q->next_id = 1;
}

unsigned long ob_jobq_add(ob_jobq *q, const char *vol)
{
    ob_job *j;
    int i;
    if (q->n >= OB_JOBS_MAX) return 0;
    j = &q->j[q->n++];
    zero(j, sizeof *j);
    j->id = q->next_id++;
    if (!q->next_id) q->next_id = 1;
    j->state = OB_J_QUEUED;
    for (i = 0; vol && vol[i] && i < OB_VOL_LEN - 1; i++) j->vol[i] = vol[i];
    return j->id;
}

ob_job *ob_jobq_find(ob_jobq *q, unsigned long id)
{
    int i;
    for (i = 0; i < q->n; i++)
        if (q->j[i].id == id) return &q->j[i];
    return 0;
}

ob_job *ob_jobq_next(ob_jobq *q)
{
    int i, k, running = 0;
    for (i = 0; i < q->n; i++)
        if (q->j[i].state == OB_J_RUNNING) running++;
    if (running >= q->max_running) return 0;
    for (i = 0; i < q->n; i++) {
        int busy = 0;
        if (q->j[i].state != OB_J_QUEUED) continue;
        for (k = 0; k < q->n && !busy; k++)
            busy = q->j[k].state == OB_J_RUNNING && same_vol(q->j[k].vol, q->j[i].vol);
        if (!busy) return &q->j[i];
    }
    return 0;
}

void ob_jobq_start(ob_jobq *q, unsigned long id)
{
    ob_job *j = ob_jobq_find(q, id);
    if (j && j->state == OB_J_QUEUED) j->state = OB_J_RUNNING;
}

int ob_jobq_progress(ob_jobq *q, unsigned long id, unsigned long done, unsigned long total)
{
    ob_job *j = ob_jobq_find(q, id);
    if (!j || j->state != OB_J_RUNNING) return 0;
    j->done = done;
    j->total = total;
    return !j->cancel;
}

int ob_jobq_cancel(ob_jobq *q, unsigned long id)
{
    ob_job *j = ob_jobq_find(q, id);
    if (!j) return 0;
    if (j->state == OB_J_QUEUED) { j->state = OB_J_CANCELLED; return 1; }
    if (j->state == OB_J_RUNNING) { j->cancel = 1; return 2; }
    return 0;
}

void ob_jobq_finish(ob_jobq *q, unsigned long id, int ok)
{
    ob_job *j = ob_jobq_find(q, id);
    if (!j || j->state != OB_J_RUNNING) return;
    j->state = j->cancel ? OB_J_CANCELLED : ok ? OB_J_DONE : OB_J_FAILED;
}

int ob_jobq_reap(ob_jobq *q)
{
    int i, k = 0, gone = 0;
    for (i = 0; i < q->n; i++) {
        int st = q->j[i].state;
        if (st == OB_J_DONE || st == OB_J_FAILED || st == OB_J_CANCELLED) { gone++; continue; }
        if (k != i) q->j[k] = q->j[i];
        k++;
    }
    q->n = k;
    return gone;
}

int ob_jobq_percent(const ob_job *j)
{
    if (!j || !j->total) return 0;
    if (j->done >= j->total) return 100;
    /* without overflow for large totals */
    return j->total > 0xFFFFFFUL ? (int)(j->done / (j->total / 100)) : (int)(j->done * 100 / j->total);
}

/* ---- low memory ---- */
static int refill(ob_reserve *r, const ob_mem *m)
{
    if (r->block || !r->size) return 1;
    r->block = m->alloc(m->user, r->size);
    r->low = !r->block;
    return !r->low;
}

int ob_reserve_init(ob_reserve *r, const ob_mem *m, unsigned long size)
{
    r->block = 0;
    r->size = size;
    r->low = 1;
    return refill(r, m);
}

void *ob_reserve_alloc(ob_reserve *r, const ob_mem *m, unsigned long size)
{
    for (;;) {
        void *p;
        if (r->low) refill(r, m);           /* the reserve first: the next requester needs it */
        if ((p = m->alloc(m->user, size))) return p;
        if (r->block) {                     /* give it back, so the requester can open */
            m->free(m->user, r->block, r->size);
            r->block = 0;
            r->low = 1;
            /* the request may fit now the reserve is back: no requester then (Koenig, Team 1, #102 review) */
            if ((p = m->alloc(m->user, size))) return p;
        }
        r->low = 1;
        if (!m->ask(m->user)) return 0;
    }
}

void ob_reserve_end(ob_reserve *r, const ob_mem *m)
{
    if (r->block) m->free(m->user, r->block, r->size);
    r->block = 0;
}

/* ---- the launcher's WBStartup order ---- */
void ob_seq_init(ob_seq *s, const int *wait, int n)
{
    zero(s, sizeof *s);
    s->wait = wait;
    s->n = n > 0 ? n : 0;
}

int ob_seq_next(ob_seq *s)
{
    if (s->waiting) return OB_SEQ_WAIT;
    if (s->cur >= s->n) return OB_SEQ_DONE;
    return s->cur++;
}

void ob_seq_opened(ob_seq *s, int at, unsigned long tag)
{
    if (!tag || at < 0 || at >= s->n || s->wait[at] <= 0) return;   /* nothing started, or DONOTWAIT: the next now */
    s->waiting = 1;
    s->tag = tag;
    s->id = 0;
}

int ob_seq_answered(ob_seq *s, unsigned long tag, int ok, unsigned long id)
{
    if (!s->waiting || s->id || tag != s->tag) return 0;
    if (!ok || !id) s->waiting = 0;                 /* it didn't start: the next */
    else s->id = id;
    return 1;
}

int ob_seq_ended(ob_seq *s, unsigned long id)
{
    if (!s->waiting || !id || id != s->id) return 0;
    s->waiting = 0;
    return 1;
}

int ob_names_format(char *out, int n, const char *const *names, int count)
{
    int k = 0, i;
    if (n <= 0) return 0;
    out[0] = 0;
    for (i = 0; i < count && i < 8; i++) {
        const char *p = names[i] ? names[i] : "";
        if (k + 3 >= n) break;
        out[k++] = ' ';
        out[k++] = ' ';
        while (*p && k + 1 < n - 1) out[k++] = *p == '%' ? '_' : *p, p++;
        out[k++] = '\n';
    }
    if (count > 8) {
        char tail[32];
        int t = 0, m = count - 8, d = 1;
        const char *a = "  and ", *b = " more\n";
        while (*a) tail[t++] = *a++;
        while (m / d >= 10) d *= 10;
        for (; d; d /= 10) tail[t++] = (char)('0' + m / d % 10);
        while (*b) tail[t++] = *b++;
        for (i = 0; i < t && k < n - 1; i++) out[k++] = tail[i];
    }
    out[k] = 0;
    return k;
}

/* ---- packed strings ---- */
static int used(const char *buf, int size, int count)
{
    int k = 0;
    while (count-- > 0 && k < size) {
        while (k < size && buf[k]) k++;
        k++;
    }
    return k;
}

int ob_pack(char *buf, int size, int *count, const char *s)
{
    int k = used(buf, size, *count), n = 0;
    while (s[n]) n++;
    if (k + n + 1 > size) return 0;
    for (n = 0; s[n]; n++) buf[k + n] = s[n];
    buf[k + n] = 0;
    (*count)++;
    return 1;
}

int ob_unpack(const char *buf, int size, int count, const char **out, int max)
{
    int k = 0, i;
    for (i = 0; i < count && i < max && k < size; i++) {
        int e = k;
        while (e < size && buf[e]) e++;
        if (e >= size) break;           /* no NUL: not one of ours */
        out[i] = buf + k;
        k = e + 1;
    }
    return i;
}

/* ---- a crashed task, held by the task guard ---- */
unsigned long ob_guard_rec_at(unsigned long splower, unsigned long spupper)
{
    unsigned long r;
    if (spupper < 56 || spupper <= splower) return 0;
    r = (spupper - 56) & ~3UL;
    if (r < splower || r + 16 > spupper) return 0;
    return r;
}

static unsigned long be32(const unsigned char *p)
{
    return (unsigned long)p[0] << 24 | (unsigned long)p[1] << 16 | (unsigned long)p[2] << 8 | p[3];
}

int ob_guard_read(const unsigned char *rec, ob_held *h)
{
    unsigned long v;
    if (!rec || be32(rec) != OB_GUARD_MAGIC) return 0;
    v = be32(rec + 4);
    if (v != 3 && v != 4 && v != 10) return 0;      /* the only ones the guard holds */
    if (h) {
        h->vector = (int)v;
        h->pc = be32(rec + 8);
        h->addr = be32(rec + 12);
    }
    return 1;
}
