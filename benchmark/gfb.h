/* GFOpenBenchmark: what its parts share. The clock, the way each test is timed, and the results.
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#ifndef GFB_H
#define GFB_H

#include <exec/types.h>

#define GFB_VERSION "1.0"
#define GFB_RUNS 3              /* each test is timed this many times; the median is reported */
#define GFB_MIN_SECONDS 2.0     /* and each timed run lasts at least this long */

/* the E clock (timer.device), in seconds since gfb_clock_open */
int gfb_clock_open(void);
void gfb_clock_close(void);
double gfb_now(void);
ULONG gfb_eclock_hz(void);

/* A test's work: n units of it. Returns anything the work computed (kept, so it isn't optimised
 * away). */
typedef double (*gfb_work)(void *user, unsigned long n);

struct gfb_result {
    const char *key;            /* the machine-readable name, e.g. "dhry_dps" */
    const char *label;          /* "Dhrystone 2.1" */
    const char *unit;           /* "Dhrystones/s" */
    double value;               /* median of GFB_RUNS: units/s times scale */
    double runs[GFB_RUNS];
    double seconds;             /* the median run's length */
    unsigned long n;            /* units in each timed run */
    int ok;                     /* 0: not measured (why in note) */
    char note[96];
};

/* Times `work`: grows n until one call lasts a quarter of a second, sets n for GFB_MIN_SECONDS
 * (and a tenth more), then times GFB_RUNS calls of n units. value = median(n / seconds) * scale.
 * forbid: hold other tasks off during each timed call (the CPU, FPU and memory tests). */
void gfb_time(struct gfb_result *r, gfb_work work, void *user, double scale, int forbid);
void gfb_fail(struct gfb_result *r, const char *why);

/* For work that waits on the host (GL): in hyper mode AmiCradle's guest clock, the E clock with it,
 * falls behind real time while the CPU waits on a host call and catches up at most 2 ms a slice
 * afterwards, so the E clock can't time it. This times it by the battery clock instead (on
 * AmiCradle it reads the host's own time, in seconds): it runs `step` until the clock's second
 * turns, counts the steps until it has turned min_seconds more times (2 s; QUICK 1 s), then calls
 * settle (glFinish). One step is one unit. Three runs, the median. Without a battery clock it
 * falls back to the E clock. */
typedef void (*gfb_step)(void *user);
void gfb_time_host(struct gfb_result *r, gfb_step step, gfb_step settle, void *user, double scale);

/* the report: each result is printed as it is measured, and kept for the machine-readable line */
void gfb_report(struct gfb_result *r);
void gfb_info(const char *key, const char *label, const char *value);   /* a machine fact */
void gfb_section(const char *title);

/* the parts */
void gfb_cpu_tests(void);
void gfb_fpu_tests(void);
void gfb_mem_tests(void);
void gfb_2d_tests(void);
void gfb_gl_tests(void);

/* GL's driver, for the summary: "virgl (AMD Radeon ...)", "softpipe", or why there is none */
extern char gfb_gl_renderer[160];

#endif
