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

void ob_vol_name(const char *path, char *out, int n)
{
    int i;
    if (n <= 0) return;
    out[0] = 0;
    if (!path) return;
    for (i = 0; path[i] && path[i] != ':'; i++) ;
    if (path[i] != ':') return;
    if (i > n - 1) i = n - 1;
    for (n = 0; n < i; n++) out[n] = path[n];
    out[i] = 0;
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

int ob_reserve_refill(ob_reserve *r, const ob_mem *m)
{
    return refill(r, m);
}

int ob_reserve_failed(ob_reserve *r, const ob_mem *m, int no_memory)
{
    if (!no_memory) return 0;
    if (r->block) m->free(m->user, r->block, r->size);   /* so the requester can open */
    r->block = 0;
    r->low = 1;
    return m->ask(m->user) != 0;
}

int ob_low(long err, unsigned long largest)
{
    return err == OB_NO_FREE_STORE || largest < OB_LOW_FLOOR;
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

int ob_guard_held(int waiting, unsigned long sigwait, int rec_ok)
{
    (void)rec_ok;
    return waiting && !sigwait;
}

int ob_q_take(volatile int *state)
{
    if (*state == OB_Q_WITHDRAWN) return 0;
    *state = OB_Q_TAKEN;
    return 1;
}

int ob_q_withdraw(volatile int *state, unsigned long waited_ticks)
{
    if (*state != OB_Q_QUEUED || waited_ticks < OB_ASK_TICKS) return 0;
    *state = OB_Q_WITHDRAWN;
    return 1;
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

/* ---- a drawer task's answers about its own drawer ---- */
#include <stdlib.h>
#include <string.h>

static int fold(int c)                  /* as ob_snap.c: Latin-1's letters, any case */
{
    c &= 0xFF;
    if ((c >= 'a' && c <= 'z') || (c >= 0xE0 && c <= 0xFE && c != 0xF7)) return c - 32;
    return c;
}

int ob_same_name(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (fold((unsigned char)*a) != fold((unsigned char)*b)) return 0;
    return !*a && !*b;
}

void ob_dpack_init(ob_dpack *p)
{
    zero(p, sizeof *p);
}

void ob_dpack_free(ob_dpack *p)
{
    free(p->e);
    ob_dpack_init(p);
}

void ob_dpack_reset(ob_dpack *p, const char *dir, unsigned long gen)
{
    size_t n = strlen(dir);
    if (n >= sizeof p->dir) n = sizeof p->dir - 1;
    memcpy(p->dir, dir, n);
    p->dir[n] = 0;
    p->gen = gen;
    p->n = 0;
    p->filled = 1;
}

int ob_dpack_fresh(const ob_dpack *p, const char *dir, unsigned long gen)
{
    return p->filled && p->gen == gen && !strcmp(p->dir, dir);
}

static ob_dpent *find(const ob_dpack *p, const char *name)
{
    int i;
    for (i = 0; i < p->n; i++)
        if (ob_same_name(p->e[i].name, name)) return &p->e[i];
    return NULL;
}

static ob_dpent *entry(ob_dpack *p, const char *name)
{
    ob_dpent *e = find(p, name);
    size_t n = strlen(name);
    if (e) return e;
    if (n >= OB_DP_NAME) return NULL;
    if (p->n == p->cap) {
        int cap = p->cap ? p->cap * 2 : 16;
        ob_dpent *ne = malloc((size_t)cap * sizeof *ne);
        if (!ne) return NULL;
        if (p->n) memcpy(ne, p->e, (size_t)p->n * sizeof *ne);
        free(p->e);
        p->e = ne;
        p->cap = cap;
    }
    e = &p->e[p->n++];
    zero(e, sizeof *e);
    memcpy(e->name, name, n + 1);
    return e;
}

int ob_dpack_hide(ob_dpack *p, const char *name)
{
    ob_dpent *e = entry(p, name);
    if (!e) return 0;
    e->hidden = 1;
    return 1;
}

int ob_dpack_snap(ob_dpack *p, const char *name, int where, long x, long y, int gx, int gy)
{
    ob_dpent *e = entry(p, name);
    if (!e) return 0;
    if (e->snap) return 1;              /* the first one found stands */
    e->snap = 1;
    e->where = (char)where;
    e->x = x; e->y = y; e->gx = gx; e->gy = gy;
    return 1;
}

int ob_dpack_hidden(const ob_dpack *p, const char *name)
{
    const ob_dpent *e = find(p, name);
    return e && e->hidden;
}

int ob_dpack_snapped(const ob_dpack *p, const char *name, int *gx, int *gy, long *x, long *y)
{
    const ob_dpent *e = find(p, name);
    if (!e || !e->snap || e->where != 'W') return 0;
    *gx = e->gx; *gy = e->gy; *x = e->x; *y = e->y;
    return 1;
}

const char *ob_name_in(const char *path, const char *dir)
{
    size_t k, dl = strlen(dir);
    const char *rest;
    for (k = 0; k < dl; k++)
        if (!path[k] || fold((unsigned char)path[k]) != fold((unsigned char)dir[k])) return NULL;
    rest = path + dl;
    if (dl && dir[dl - 1] != ':') {     /* "Work:Tools" + "/x" */
        if (*rest != '/') return NULL;
        rest++;
    }
    if (!*rest || strchr(rest, '/') || strchr(rest, ':')) return NULL;
    return rest;
}

/* ---- the starts sent to the launcher and not answered yet ---- */

int ob_out_add(ob_out *o, void *r)
{
    int i;
    for (i = 0; i < OB_OUT_MAX; i++)
        if (!o->slot[i]) {
            o->slot[i] = r;
            return 1;
        }
    return 0;
}

void ob_out_del(ob_out *o, void *r)
{
    int i;
    for (i = 0; i < OB_OUT_MAX; i++) if (o->slot[i] == r) o->slot[i] = 0;
}

int ob_out_lost(ob_out *o, void *const *queued, int nq, int (*answered)(void *r), void **lost, int max)
{
    int i, k, n = 0;
    for (i = 0; i < OB_OUT_MAX && n < max; i++) {
        void *r = o->slot[i];
        int q = 0;
        if (!r) continue;
        for (k = 0; k < nq; k++) q |= queued[k] == r;
        if (q || (answered && answered(r))) continue;   /* the next launcher's, or launch_answers' to reply */
        o->slot[i] = 0;
        lost[n++] = r;
    }
    return n;
}

/* ---- ARexx's icon places in a drawer window ---- */
static int same_name(const char *a, const char *b)
{
    for (;; a++, b++) {
        int x = *a >= 'A' && *a <= 'Z' ? *a + 32 : *a, y = *b >= 'A' && *b <= 'Z' ? *b + 32 : *b;
        if (x != y) return 0;
        if (!x) return 1;
    }
}

int ob_place_take(void *ctx, const char *name, int type, int selected, short x, short y)
{
    ob_place *p = ctx;
    (void)type;
    (void)selected;
    if (!p || !p->name || !name || !same_name(p->name, name)) return 2;
    p->found = 1;
    p->x = x;
    p->y = y;
    return -1;
}

int ob_shown_take(void *ctx, const char *name, int type, int selected, short x, short y)
{
    ob_shown *s = ctx;
    int k, i;
    (void)type;
    if (!s || !name || !s->name_at) return -1;
    if (s->hint < 0 || s->hint >= s->n) s->hint = 0;
    for (k = 0; k < s->n; k++) {
        const char *at;
        i = (s->hint + k) % s->n;
        if (!(at = s->name_at(s->ctx, i)) || !same_name(at, name)) continue;
        if (s->count < s->cap && s->out) {
            s->out[s->count].at = i;
            s->out[s->count].selected = selected != 0;
            s->out[s->count].x = x;
            s->out[s->count].y = y;
            s->count++;
        }
        s->hint = i + 1;
        return 2;
    }
    s->missed++;
    return 2;
}

void ob_shown_sort(ob_shown *s)
{
    int i, j;
    if (!s || !s->out) return;
    for (i = 1; i < s->count; i++) {            /* insertion: the drawer's own order is nearly the reading's */
        ob_shown_icon v = s->out[i];
        for (j = i; j > 0 && s->out[j - 1].at > v.at; j--) s->out[j] = s->out[j - 1];
        s->out[j] = v;
    }
}
