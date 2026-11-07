/* GFOpenBenchmark: one benchmark for an Amiga, AmiCradle's or any 68040 with an FPU: Dhrystone MIPS,
 * MFLOPS (Whetstone, LINPACK, single precision), memory bandwidth, a few RTG drawing figures and
 * GL through ogles3.library (GFOpenGPU's OpenGL ES 3.1). The same fixed work each time, so runs
 * compare across days and machines; each test is timed three times for at least two seconds and
 * the median reported. The report is plain text, and its last line (GFB-JSON) is one JSON object
 * with every figure, to compare and chart.
 *
 *   GFOpenBenchmark [CPU] [FPU] [MEM] [2D] [GL] [QUICK] [NOFORBID] [HOST] [LABEL=<text>]
 *
 * No test named: all of them. QUICK: a quarter of a second a run, to try it out (its figures aren't
 * for comparing). NOFORBID: let other tasks run during the CPU, FPU and memory tests (they hold
 * them off by default, as SysInfo and ACInfo do). HOST: print the AmiCradle host's status as it
 * comes. LABEL: a note carried into the JSON line (a lab's name, a runtime's build).
 *
 * MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <exec/execbase.h>
#include <exec/memory.h>
#include <devices/timer.h>
#include <dos/dos.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/timer.h>
#include <resources/battclock.h>
#include <proto/battclock.h>

#include "gfb.h"
#include "sysprobe.h"

#ifndef GFB_CFLAGS
#define GFB_CFLAGS "?"
#endif

static const char version[] __attribute__((used)) = "$VER: GFOpenBenchmark " GFB_VERSION " (07.11.2026) Dalsin Limited / the AmiCradle Team";

extern struct ExecBase *SysBase;
struct Device *TimerBase;
struct Library *BattClockBase;
static ULONG rtc0;
static struct timerequest treq;
static ULONG ehz;
static struct EClockVal e0;
static double min_seconds = GFB_MIN_SECONDS;
static int allow_forbid = 1;
char gfb_gl_renderer[160] = "not run";
static void copy_text(char *d, const char *s, unsigned size);

/* ---- the clock ---------------------------------------------------------------------------------------- */

int gfb_clock_open(void)
{
    if (OpenDevice((CONST_STRPTR)TIMERNAME, UNIT_ECLOCK, (struct IORequest *)&treq, 0)) return 0;
    TimerBase = treq.tr_node.io_Device;
    ehz = ReadEClock(&e0);
    BattClockBase = (struct Library *)OpenResource((CONST_STRPTR)BATTCLOCKNAME);
    if (BattClockBase) rtc0 = ReadBattClock();
    return 1;
}

void gfb_clock_close(void)
{
    if (TimerBase) CloseDevice((struct IORequest *)&treq);
    TimerBase = NULL;
}

ULONG gfb_eclock_hz(void) { return ehz; }

double gfb_now(void)
{
    struct EClockVal e;
    ReadEClock(&e);
    return ((double)(e.ev_hi - e0.ev_hi) * 4294967296.0 + ((double)e.ev_lo - (double)e0.ev_lo)) / (double)ehz;
}

/* ---- timing ------------------------------------------------------------------------------------------- */

static volatile double sink;

static double timed(gfb_work work, void *user, unsigned long n, int forbid)
{
    double t0, t1;
    if (forbid) Forbid();
    t0 = gfb_now();
    sink += work(user, n);
    t1 = gfb_now();
    if (forbid) Permit();
    return t1 - t0;
}

void gfb_fail(struct gfb_result *r, const char *why)
{
    r->ok = 0;
    copy_text(r->note, why, sizeof r->note);
}

void gfb_time(struct gfb_result *r, gfb_work work, void *user, double scale, int forbid)
{
    unsigned long n = 1;
    double t, want = min_seconds * 1.1, rate[GFB_RUNS], secs[GFB_RUNS], x;
    int i, j;
    forbid = forbid && allow_forbid;
    /* grow n until a call lasts a quarter of a second (or an eighth of the run, for QUICK) */
    for (;;) {
        t = timed(work, user, n, 0);
        if (t >= want / 8 || n >= 0x40000000UL) break;
        n = t < want / 800 ? n * 16 : n * 2;
    }
    if (t > 0) {
        x = (double)n * want / t;
        n = x > 4.0e9 ? 4000000000UL : x < 1 ? 1 : (unsigned long)x;
    }
    for (i = 0; i < GFB_RUNS; i++) {
        int tries = 0;
        /* a run that came out short (the calibration ran warmer or colder) is made longer and run again */
        while ((secs[i] = timed(work, user, n, forbid)) < min_seconds && ++tries < 4 && n < 4000000000UL) {
            x = (double)n * want / (secs[i] > 0 ? secs[i] : want / 1000);
            n = x > 4.0e9 ? 4000000000UL : (unsigned long)x + 1;
        }
        rate[i] = secs[i] > 0 ? (double)n / secs[i] * scale : 0;
        r->runs[i] = rate[i];
    }
    for (i = 1; i < GFB_RUNS; i++)               /* sort by rate, each run's length with it */
        for (j = i; j > 0 && rate[j] < rate[j - 1]; j--) {
            x = rate[j]; rate[j] = rate[j - 1]; rate[j - 1] = x;
            x = secs[j]; secs[j] = secs[j - 1]; secs[j - 1] = x;
        }
    r->value = rate[GFB_RUNS / 2];
    r->seconds = secs[GFB_RUNS / 2];
    r->n = n;
    r->ok = 1;
}

/* the host-clocked runs (gfb.h) */
struct step_work { gfb_step step, settle; void *user; };

static double step_n(void *u, unsigned long n)
{
    struct step_work *w = u;
    while (n--) w->step(w->user);
    if (w->settle) w->settle(w->user);
    return 0;
}

void gfb_time_host(struct gfb_result *r, gfb_step step, gfb_step settle, void *user, double scale)
{
    ULONG k = min_seconds < 1.5 ? 1 : (ULONG)(min_seconds + 0.5), s0, s, count;
    double rate[GFB_RUNS], x;
    int i, j;
    if (!BattClockBase) {
        struct step_work w = { step, settle, user };
        gfb_time(r, step_n, &w, scale, 0);
        strcpy(r->note, "(E clock: no battery clock)");
        return;
    }
    for (i = 0; i < GFB_RUNS; i++) {
        s = ReadBattClock();
        while ((s0 = ReadBattClock()) == s) step(user);     /* working up to the second's turn */
        count = 0;
        while (ReadBattClock() - s0 < k) {
            step(user);
            count++;
        }
        if (settle) settle(user);
        rate[i] = r->runs[i] = (double)count / k * scale;
        if (i == GFB_RUNS / 2) r->n = count;
    }
    for (i = 1; i < GFB_RUNS; i++)
        for (j = i; j > 0 && rate[j] < rate[j - 1]; j--) { x = rate[j]; rate[j] = rate[j - 1]; rate[j - 1] = x; }
    r->value = rate[GFB_RUNS / 2];
    r->seconds = k;
    r->ok = 1;
    strcpy(r->note, "(host clock)");
}

/* ---- the report ---------------------------------------------------------------------------------------- */

#define MAX_RESULTS 48
#define MAX_INFO 24
static struct { const char *key; double value; int ok; } kept[MAX_RESULTS];
static int nkept;
static struct { const char *key; char value[256]; } facts[MAX_INFO];
static int nfacts;

/* a bounded copy that always ends the string */
static void copy_text(char *d, const char *s, unsigned size)
{
    unsigned n = strlen(s);
    if (n > size - 1) n = size - 1;
    memcpy(d, s, n);
    d[n] = 0;
}

static const char *num(double v, char *buf)
{
    if (v >= 100000) sprintf(buf, "%.0f", v);
    else if (v >= 100) sprintf(buf, "%.1f", v);
    else if (v >= 1) sprintf(buf, "%.2f", v);
    else sprintf(buf, "%.4f", v);
    return buf;
}

void gfb_section(const char *title) { printf("\n%s\n", title); }

void gfb_report(struct gfb_result *r)
{
    char a[32], b[32], c[32], d[32];
    if (!r->ok) printf("  %-34s not measured: %s\n", r->label, r->note);
    else {
        printf("  %-34s %12s %-14s (runs %s %s %s; %.2f s each)%s%s\n", r->label, num(r->value, a), r->unit,
               num(r->runs[0], b), num(r->runs[1], c), num(r->runs[2], d), r->seconds, r->note[0] ? " " : "", r->note);
    }
    if (nkept < MAX_RESULTS) {
        kept[nkept].key = r->key;
        kept[nkept].value = r->value;
        kept[nkept].ok = r->ok;
        nkept++;
    }
    fflush(stdout);
}

void gfb_info(const char *key, const char *label, const char *value)
{
    printf("  %-22s %s\n", label, value);
    if (nfacts < MAX_INFO) {
        facts[nfacts].key = key;
        copy_text(facts[nfacts].value, value, sizeof facts[nfacts].value);
        nfacts++;
    }
}

static void json_str(const char *s)
{
    putchar('"');
    for (; *s; s++) {
        if (*s == '"' || *s == '\\') printf("\\%c", *s);
        else if ((unsigned char)*s < 32) putchar(' ');
        else putchar(*s);
    }
    putchar('"');
}

static void json_line(const char *label)
{
    int i;
    char buf[48];
    printf("\nGFB-JSON {\"tool\":\"GFOpenBenchmark\",\"version\":\"" GFB_VERSION "\",\"label\":");
    json_str(label);
    printf(",\"min_seconds\":%.2f,\"runs\":%d,\"info\":{", min_seconds, GFB_RUNS);
    for (i = 0; i < nfacts; i++) {
        if (i) putchar(',');
        json_str(facts[i].key);
        putchar(':');
        json_str(facts[i].value);
    }
    printf("},\"results\":{");
    for (i = 0; i < nkept; i++) {
        if (i) putchar(',');
        json_str(kept[i].key);
        if (kept[i].ok) {
            sprintf(buf, "%.6g", kept[i].value);
            printf(":%s", buf);
        } else
            printf(":null");
    }
    printf("}}\n");
}

/* ---- the machine ---------------------------------------------------------------------------------------- */

/* GFOpenUp's version: its manifest's "part<TAB>GFOpenUp<TAB>x.y.z" line */
static void gfopenup_version(char *out, int size)
{
    BPTR f = Open((CONST_STRPTR) "ENVARC:GFOpenAmiga/GFOpenUp.manifest", MODE_OLDFILE);
    char line[200];
    strcpy(out, "none (no GFOpenUp manifest)");
    if (!f) return;
    while (FGets(f, (STRPTR)line, sizeof line)) {
        if (!strncmp(line, "part\tGFOpenUp\t", 14)) {
            char *p = line + 14, *e = p;
            while (*e && *e != '\t' && *e != '\n' && *e != '\r') e++;
            *e = 0;
            copy_text(out, p, size);
            break;
        }
    }
    Close(f);
}

static void lib_version(const char *name, char *out)
{
    struct Library *l = OpenLibrary((CONST_STRPTR)name, 0);
    if (!l) { strcpy(out, "none"); return; }
    sprintf(out, "%u.%u", l->lib_Version, l->lib_Revision);
    CloseLibrary(l);
}

static void machine(int host)
{
    UWORD f = SysBase->AttnFlags;
    char buf[200], v[96];
    ULONG c = CacheControl(0, 0);
    struct sp_ac ac;
    ULONG size = 16384;
    char *status = AllocMem(size + 1, MEMF_PUBLIC | MEMF_CLEAR);

    gfb_section("Machine");
    sprintf(buf, "%s, FPU %s, MMU %s", sp_cpu(f), sp_fpu(f), sp_mmu(f));
    gfb_info("cpu", "CPU", buf);
    sprintf(buf, "instruction %s, data %s%s", (c & CACRF_EnableI) ? "on" : "off", (c & CACRF_EnableD) ? "on" : "off",
            (c & CACRF_CopyBack) ? ", copyback" : "");
    gfb_info("caches", "Caches", buf);
    sprintf(buf, "exec %u.%u, Kickstart %u", SysBase->LibNode.lib_Version, SysBase->LibNode.lib_Revision, SysBase->SoftVer);
    gfb_info("kickstart", "ROM", buf);
    gfopenup_version(buf, sizeof buf);
    gfb_info("gfopenup", "GFOpenUp", buf);
    lib_version("gfopengpu.library", v);
    sprintf(buf, "gfopengpu.library %s, ", v);
    lib_version("ogles3.library", v);
    strcat(buf, "ogles3.library ");
    strcat(buf, v);
    gfb_info("gpu_libs", "GFOpenGPU", buf);
    sprintf(buf, "%lu K chip, %lu K fast", (unsigned long)(AvailMem(MEMF_CHIP | MEMF_TOTAL) >> 10),
            (unsigned long)(AvailMem(MEMF_FAST | MEMF_TOTAL) >> 10));
    gfb_info("memory", "Memory", buf);
    sprintf(buf, "%lu Hz", (unsigned long)ehz);
    gfb_info("eclock", "E clock", buf);
    if (sp_ac(&ac, status, status ? size + 1 : 0)) {
        if (ac.pace_known) {
            if (ac.pace) sprintf(buf, "%lu.%02lu MHz", (unsigned long)(ac.pace / 1000000), (unsigned long)((ac.pace / 10000) % 100));
            else strcpy(buf, "hyper (as fast as the host)");
        } else
            strcpy(buf, "unknown");
        gfb_info("pace", "CPU pace", buf);
        if (status && ac.status_len) {
            static const char *const keys[][3] = {
                /* key, label, JSON key (searched after the anchor, when there is one) */
                { "runtime", "Runtime build", "buildId" },
                { "timing", "Timing", "timingMode" },
                { "backend", "CPU backend", "backendResolved" },
                { "machine_board", "Motherboard", "id" },
            };
            static const char *const anchors[] = { NULL, NULL, NULL, "\"motherboard\"" };
            const char *cpu = strstr(status, "\"cpu\":{");
            unsigned i;
            for (i = 0; i < sizeof keys / sizeof keys[0]; i++)
                if (sp_json(status, anchors[i], keys[i][2], buf, 160)) gfb_info(keys[i][0], keys[i][1], buf);
            /* the CPU's own name follows its provider's, after "engine" */
            if (cpu && sp_json(cpu, "\"engine\"", "name", buf, 160)) gfb_info("cpu_name", "CPU personality", buf);
            /* the ROM: the standard's name (up to its notes in brackets) and whether it matches */
            if (sp_json(status, "\"standard\"", "id", buf, 160)) {
                char *b = strstr(buf, " (");
                if (b) *b = 0;
                if (sp_json(status, "\"romCheck\"", "status", v, sizeof v)) { strcat(buf, ", "); strcat(buf, v); }
                gfb_info("rom", "ROM standard", buf);
            }
            if (host) {
                printf("\nHost status\n%s\n", status);
            }
        } else
            gfb_info("runtime", "Runtime build", "the host didn't say");
    } else
        gfb_info("pace", "AmiCradle", "not an AmiCradle machine (no accontrol.device or ACBridge)");
    gfb_info("build", "Built with", GFB_CFLAGS);
    if (status) FreeMem(status, size + 1);
}

/* ---- main ------------------------------------------------------------------------------------------------ */

#define TEMPLATE "CPU/S,FPU/S,MEM/S,2D/S,GL/S,QUICK/S,NOFORBID/S,HOST/S,LABEL/K"
enum { A_CPU, A_FPU, A_MEM, A_2D, A_GL, A_QUICK, A_NOFORBID, A_HOST, A_LABEL, A_N };

int main(void)
{
    LONG a[A_N];
    struct RDArgs *rd;
    int i, all;
    double t0;
    for (i = 0; i < A_N; i++) a[i] = 0;
    if (!(rd = ReadArgs((CONST_STRPTR)TEMPLATE, a, NULL))) {
        PrintFault(IoErr(), (CONST_STRPTR) "GFOpenBenchmark");
        return RETURN_FAIL;
    }
    if (!(SysBase->AttnFlags & AFF_68881)) {
        printf("GFOpenBenchmark needs an FPU (it is built for the 68040 with its FPU).\n");
        FreeArgs(rd);
        return RETURN_FAIL;
    }
    if (!gfb_clock_open()) {
        printf("GFOpenBenchmark: no timer.device\n");
        FreeArgs(rd);
        return RETURN_FAIL;
    }
    if (a[A_QUICK]) min_seconds = 0.25;
    if (a[A_NOFORBID]) allow_forbid = 0;
    all = !a[A_CPU] && !a[A_FPU] && !a[A_MEM] && !a[A_2D] && !a[A_GL];
    printf("GFOpenBenchmark " GFB_VERSION ", Dalsin Limited / the AmiCradle Team\n"
           "Each figure: the median of %d runs of at least %.2f s%s.\n",
           GFB_RUNS, min_seconds, allow_forbid ? " (CPU, FPU and memory under Forbid)" : "");
    machine(a[A_HOST] != 0);
    t0 = gfb_now();
    if (all || a[A_CPU]) gfb_cpu_tests();
    if (all || a[A_FPU]) gfb_fpu_tests();
    if (all || a[A_MEM]) gfb_mem_tests();
    if (all || a[A_2D]) gfb_2d_tests();
    if (all || a[A_GL]) gfb_gl_tests();
    gfb_section("Summary");
    printf("  %-22s %s\n", "GL renderer", gfb_gl_renderer);
    printf("  %-22s %.1f s by the E clock", "Took", gfb_now() - t0);
    if (BattClockBase) printf(", %lu s by the host's (battery) clock since the start", (unsigned long)(ReadBattClock() - rtc0));
    printf("\n");
    json_line(a[A_LABEL] ? (const char *)a[A_LABEL] : "");
    gfb_clock_close();
    FreeArgs(rd);
    return RETURN_OK;
}
