/* ob_tasks.h: the Workbench task model's bookkeeping (amicradle-teams designs/WORKBENCH-BUILD-PLAN-20261113.md,
 * row 3; designs/WORKBENCH-REDESIGN-20261113.md). Plain C with no Amiga headers, tested on the host
 * (tests/host/test_ob_tasks.c).
 *
 * The desktop only routes. The work runs in tasks of its own: one per drawer window, the launcher (starts programs
 * and never waits for them) and the jobs (copy, delete and the rest: queued, cancellable, with progress). Every
 * task is supervised, and so are the services the desktop starts (OpenTitle, WBStartup, the systray):
 *
 *   - ob_super: who is running. A task that crashes is caught by its own trap code, which tells the desktop; the
 *     desktop asks ob_super_crashed what to do: restart it (a drawer's window reopens on the same drawer), or,
 *     when it has crashed OB_RESTART_MAX times within OB_RESTART_WINDOW seconds, close it for good. The desktop
 *     itself stays up either way (WBGapTest row X4).
 *   - ob_jobq: the jobs. FIFO, at most max_running at once and never two on the same volume (the disk would
 *     thrash). Cancelling a queued job takes it out at once; a running one is told to stop at its next progress.
 *   - ob_reserve: low memory (row X3). A block is set aside at the start. When an allocation fails, the block is
 *     given back so the low-memory requester can open, and the user answers Retry or Cancel; the block is taken
 *     back as soon as memory allows.
 *
 * No global lock: the desktop owns these tables and the tasks reach it by messages only.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_TASKS_H
#define OB_TASKS_H

/* ---- the supervisor ---- */
enum { OB_T_DRAWER = 1, OB_T_LAUNCHER, OB_T_JOBS, OB_T_SERVICE };
enum { OB_S_FREE = 0, OB_S_RUNNING };
enum { OB_ACT_NONE = 0, OB_ACT_RESTART, OB_ACT_CLOSE };

#define OB_SUPER_MAX 64
#define OB_RESTART_MAX 3                /* crashes within the window that are restarted */
#define OB_RESTART_WINDOW 60UL          /* seconds */

typedef struct ob_child {
    unsigned long id;                   /* 0: a free slot */
    int kind;                           /* OB_T_... */
    int state;                          /* OB_S_... */
    int crashes;                        /* within the current window */
    unsigned long first_crash;          /* seconds, the window's start */
    void *user;                         /* the desktop's own record (a drawer's window, say) */
} ob_child;

typedef struct ob_super {
    ob_child c[OB_SUPER_MAX];
    unsigned long next_id;
} ob_super;

void ob_super_init(ob_super *s);
unsigned long ob_super_add(ob_super *s, int kind, void *user);   /* its id; 0 when the table is full */
ob_child *ob_super_find(ob_super *s, unsigned long id);
int ob_super_count(const ob_super *s, int kind);
int ob_super_ended(ob_super *s, unsigned long id);                /* 1: it was there and is gone now */
int ob_super_crashed(ob_super *s, unsigned long id, unsigned long now);   /* OB_ACT_...; CLOSE frees the slot */

/* ---- the jobs ---- */
enum { OB_J_QUEUED = 1, OB_J_RUNNING, OB_J_DONE, OB_J_FAILED, OB_J_CANCELLED };

#define OB_JOBS_MAX 32
#define OB_VOL_LEN 32

typedef struct ob_job {
    unsigned long id;
    int state;                          /* OB_J_... */
    int cancel;                         /* asked to stop while running */
    char vol[OB_VOL_LEN];               /* the volume it works on (its destination for a copy) */
    unsigned long done, total;          /* progress, in whatever unit the job counts (bytes) */
} ob_job;

typedef struct ob_jobq {
    ob_job j[OB_JOBS_MAX];
    int n;                              /* in j[], in the order added */
    int max_running;
    unsigned long next_id;
} ob_jobq;

void ob_jobq_init(ob_jobq *q, int max_running);
unsigned long ob_jobq_add(ob_jobq *q, const char *vol);          /* queued; 0 when full */
ob_job *ob_jobq_find(ob_jobq *q, unsigned long id);
ob_job *ob_jobq_next(ob_jobq *q);                                /* the first queued job that may start, or NULL */
void ob_jobq_start(ob_jobq *q, unsigned long id);
int ob_jobq_progress(ob_jobq *q, unsigned long id, unsigned long done, unsigned long total);   /* 0: stop now */
int ob_jobq_cancel(ob_jobq *q, unsigned long id);                /* 1 taken out, 2 told to stop, 0 nothing */
void ob_jobq_finish(ob_jobq *q, unsigned long id, int ok);
int ob_jobq_reap(ob_jobq *q);                                    /* finished jobs dropped; how many */
int ob_jobq_percent(const ob_job *j);
/* What a job is ordered by when there is no disk to ask: the volume or device name of path, before its ':'
 * ("DH0:a/b" gives "DH0"); "" when path names none (a relative path: the caller's current disk) */
void ob_vol_name(const char *path, char *out, int n);

/* ---- low memory ---- */
typedef struct ob_mem {
    void *(*alloc)(void *user, unsigned long size);
    void (*free)(void *user, void *p, unsigned long size);
    int (*ask)(void *user);             /* the low-memory requester: 1 Retry, 0 Cancel */
    void *user;
} ob_mem;

typedef struct ob_reserve {
    void *block;
    unsigned long size;
    int low;                            /* 1 while the block is given back */
} ob_reserve;

int ob_reserve_init(ob_reserve *r, const ob_mem *m, unsigned long size);   /* 0: couldn't, low from the start */
void *ob_reserve_alloc(ob_reserve *r, const ob_mem *m, unsigned long size);
void ob_reserve_end(ob_reserve *r, const ob_mem *m);
/* The reserve taken back when memory allows (the desktop's tick): 1 when it is held */
int ob_reserve_refill(ob_reserve *r, const ob_mem *m);
/* Something other than ob_reserve_alloc failed (a program's LoadSeg, a drawer's window): no_memory says it was for
 * want of memory (ob_low). Then the reserve is given back and the user asked: 1 Retry; 0 Cancel, or not memory */
int ob_reserve_failed(ob_reserve *r, const ob_mem *m, int no_memory);
/* A failure was for want of memory: dos's ERROR_NO_FREE_STORE, or less than OB_LOW_FLOOR bytes in one piece */
#define OB_LOW_FLOOR 65536UL
#define OB_NO_FREE_STORE 103
int ob_low(long err, unsigned long largest);

/* ---- the launcher's WBStartup order (ob_launch.h) ----
 * SYS:WBStartup is started one at a time, as Workbench starts it (ob_startup.h), without the desktop ever waiting:
 * the desktop opens the next one, the launcher starts it and answers, and tells the desktop again when it has
 * ended or its WAIT=n seconds are over; only then is the next one opened. */
enum { OB_SEQ_DONE = -2, OB_SEQ_WAIT = -1 };

typedef struct ob_seq {
    const int *wait;                    /* each entry's seconds; 0: DONOTWAIT */
    int n, cur;
    int waiting;                        /* on tag's answer, then on id's end */
    unsigned long tag, id;
} ob_seq;

void ob_seq_init(ob_seq *s, const int *wait, int n);
int ob_seq_next(ob_seq *s);                                       /* the entry to open now, or OB_SEQ_WAIT/DONE */
void ob_seq_opened(ob_seq *s, int at, unsigned long tag);         /* tag: the start asked for, 0 none */
int ob_seq_answered(ob_seq *s, unsigned long tag, int ok, unsigned long id);   /* 1: it was the one waited on */
int ob_seq_ended(ob_seq *s, unsigned long id);                    /* 1: it was the one waited on */

/* The names of the programs still running, one to a line ("  name\n"), at most 8 and then "  and N more", for a
 * requester: a '%' becomes '_' (the requester's text is a format). Its length. */
int ob_names_format(char *out, int n, const char *const *names, int count);

/* Strings packed one after another with their NULs into a fixed buffer (the launcher's messages carry their
 * arguments and tool types so). ob_pack: 1 when s fitted (*count one more). ob_unpack: up to max pointers into buf,
 * how many. */
int ob_pack(char *buf, int size, int *count, const char *s);
int ob_unpack(const char *buf, int size, int count, const char **out, int max);

/* ---- a crashed task, held by the task guard ----
 * The host's task guard (amicradle docs/architecture/TASK_GUARD.md), and GFOpenGuard on a real 68040, hold a task
 * that crashes in Wait(0) (tc_SigWait 0) and leave a record 56 bytes below tc_SPUpper (long aligned): 'TGRD', the
 * vector, the PC, the address or opcode, 16 bytes big-endian. The supervisor looks for it (ob_guard.c). */
#define OB_GUARD_MAGIC 0x54475244UL     /* 'TGRD' */
typedef struct ob_held {
    int vector;                         /* 3 address error, 4 illegal instruction, 10 line A */
    unsigned long pc, addr;
} ob_held;

unsigned long ob_guard_rec_at(unsigned long splower, unsigned long spupper);   /* the record's address; 0: none */
int ob_guard_read(const unsigned char *rec, ob_held *h);         /* 1: a guard record */
/* Held: waiting (in exec's TaskWait) on no signal at all, Wait(0), which only the guard's hold does to a task of
 * ours; the record (rec_ok) only says why. Not the record alone: a lab's runtime that moves stacks may leave it
 * where the task's bounds no longer point (#118 review, WBGapF3). */
int ob_guard_held(int waiting, unsigned long sigwait, int rec_ok);

/* ---- a question the desktop asks a drawer's task (ob_dtask.c's ask) ----
 * The desktop withdraws one the task hasn't taken within OB_ASK_TICKS (a drawer busy on a slow disk must not hold
 * the desktop); one the task took is answered, quickly, from what it has in memory, and waited for. The state
 * changes under Forbid. ob_q_take: 1 to answer it, 0 when withdrawn (the task frees it). ob_q_withdraw: 1 when
 * withdrawn (the desktop asks with a new one). */
#define OB_ASK_TICKS 50                 /* 1 second, in DateStamp ticks */
enum { OB_Q_QUEUED = 0, OB_Q_TAKEN, OB_Q_WITHDRAWN };
int ob_q_take(volatile int *state);
int ob_q_withdraw(volatile int *state, unsigned long waited_ticks);

/* ---- a drawer task's answers about its own drawer (ob_dtask.h) ----
 * wb_look's hidden and snapped are asked once for every entry a drawer reads. A drawer task asks the desktop once
 * for everything about its drawer: the entries left out on the desktop (hidden) and those snapshotted in it, and
 * answers the rest itself, until its drawer or the desktop's generation (left outs and snapshots changed) differ. */
#define OB_DP_NAME 108
typedef struct ob_dpent {
    char name[OB_DP_NAME];
    char hidden;                        /* left out on the desktop */
    char snap;                          /* there is a snapshot for it; where says which */
    char where;                         /* OB_SNAP_DESK 'D' or OB_SNAP_DRAWER 'W' */
    int gx, gy;
    long x, y;
} ob_dpent;

typedef struct ob_dpack {
    char dir[256];                      /* the drawer, as the drawer names it */
    unsigned long gen;
    int filled;                         /* 0: nothing asked yet */
    ob_dpent *e;
    int n, cap;
} ob_dpack;

void ob_dpack_init(ob_dpack *p);
void ob_dpack_free(ob_dpack *p);
void ob_dpack_reset(ob_dpack *p, const char *dir, unsigned long gen);   /* empty, for dir at gen */
int ob_dpack_fresh(const ob_dpack *p, const char *dir, unsigned long gen);
int ob_dpack_hide(ob_dpack *p, const char *name);                       /* 0: no memory or name too long */
/* The first snapshot found for a name is the one kept (as ob_snap_find's first match) */
int ob_dpack_snap(ob_dpack *p, const char *name, int where, long x, long y, int gx, int gy);
int ob_dpack_hidden(const ob_dpack *p, const char *name);
int ob_dpack_snapped(const ob_dpack *p, const char *name, int *gx, int *gy, long *x, long *y);  /* 'W' only */
/* The name of an entry directly in dir ("Work:Tools/x" in "Work:Tools", "Work:x" in "Work:", "a/x" in "a", "x" in
 * ""), any case; NULL when path isn't one */
const char *ob_name_in(const char *path, const char *dir);
/* Two names the same in any case (Latin-1's letters too, as ob_snap.c compares) */
int ob_same_name(const char *a, const char *b);

/* ---- the starts sent to the launcher and not answered yet (X4: a held launcher loses none) ---- */
#define OB_OUT_MAX 64
typedef struct ob_out { void *slot[OB_OUT_MAX]; } ob_out;
int ob_out_add(ob_out *o, void *r);              /* 0 when all OB_OUT_MAX are taken: the caller answers it itself */
void ob_out_del(ob_out *o, void *r);
/* The launcher crashed: the starts it had taken, neither still queued on its port (queued, nq) nor answered
 * already (answered(r): its reply is in, not yet read), taken off the list into lost (at most max). How many. */
int ob_out_lost(ob_out *o, void *const *queued, int nq, int (*answered)(void *r), void **lost, int max);

/* ---- ARexx's icon places in a drawer window (window.icons.<kind>.N.left/.top) ----
 * The drawer's task laid its icons out; it hands each over (ob_dt_each's fn) and ob_place_take keeps the place of
 * the one named (any case): 2 (leave its selection) for the others, -1 (stop) once found. */
typedef struct ob_place {
    const char *name;                   /* the icon asked for */
    int found;
    long x, y;                          /* its picture's place in the window */
} ob_place;
int ob_place_take(void *ctx, const char *name, int type, int selected, short x, short y);

/* GETATTR window.icons.* in a drawer window: the icons its task shows, as it shows them (Show All Files' files with no
 * icon too, what is selected, each picture's place), matched to the desktop's own reading of the drawer with every
 * file. ob_shown_take is ob_dt_each's fn: for each icon handed over it finds that name in the reading (any case;
 * from where the last match was, so a list in the same order is one pass) and keeps its index, selection and place
 * in out, up to cap; a name the reading lacks (made since) is counted in missed. Answers 2: selections unchanged.
 * ob_shown_sort puts out in the reading's order (by name, as it was read). */
typedef struct ob_shown_icon {
    int at;                             /* its index in the reading */
    int selected;
    long x, y;
} ob_shown_icon;
typedef struct ob_shown {
    const char *(*name_at)(void *ctx, int i);   /* the reading's names, 0 .. n - 1 */
    void *ctx;
    int n;
    ob_shown_icon *out;
    int cap, count, missed, hint;
} ob_shown;
int ob_shown_take(void *ctx, const char *name, int type, int selected, short x, short y);
void ob_shown_sort(ob_shown *s);

#endif
