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

#endif
