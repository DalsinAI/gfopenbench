/* WBGapIndex: the gap rows the ACFS index answers (Workbench build row 8, gfosmotherboards #140), through its
 * "::wb" questions on acscsi.device's ACSCMD_SEARCH, as amicradle-guest's GFOpenIndex asks them (oix_acfs.c).
 * One line per row, as WBGapTest's: "PASS <row>", "FAIL <row> <why>" or "SKIP <row> <why>".
 *
 *   WBGapIndex [VOLUME]     on the ACFS volume VOLUME (no colon; default Work, and only Work: a lab's
 *                           other ACFS units can be shared host paths),
 *                           in the drawer VOLUME:WBGapIdx (made here, emptied first):
 *     M16t  a drawer icon's SORT= is copied to the index, and again when the icon changes
 *     M32t  SORT=NAME,REVERSE kept whole
 *     M15t  SHOW=ALL kept
 *     M34t  (ARRANGE=) kept as off
 *     S2i   date added: A made, then B; A moved out and back; ::wb order lists B before A
 *     M23r  a file trashed, made live again (its bytes still there), then final
 *     M23s  the host-reclaim time (a boot-screen setting) answers as a number of seconds
 *     X2a   a 200-character name is taken (GFOS mode)
 *     X2b   its compat name is 107 characters at most, ending in ~ and 16 hex digits
 *
 * Built against amicradle-guest's tools/openindex (AMICRADLE_GUEST in build-amiga.sh). For lab instances only.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>
#include <exec/types.h>
#include <dos/dos.h>
#include <workbench/workbench.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/icon.h>

#include "oix_acfs.h"
#include "wbgap_index.h"

struct Library *IconBase;
static ULONG unit;
static char vol[40];

static void pass(const char *row) { printf("PASS %s\n", row); }
static void fail(const char *row, const char *why) { printf("FAIL %s %s\n", row, why); }
static void skip(const char *row, const char *why) { printf("SKIP %s %s\n", row, why); }

static const char *const rows[] = {"M16t", "M32t", "M15t", "M34t", "S2i", "M23r", "M23s", "X2a", "X2b"};
static void skip_all(const char *why)
{
    int i;
    for (i = 0; i < (int)(sizeof rows / sizeof rows[0]); i++) skip(rows[i], why);
}

/* ---- the host's answers ---- */
#define MAXORDER 16
struct answer {
    char wb[480];               /* the "WB" hit's line */
    char order[MAXORDER][64];   /* the "ORDER" hits' names, in order */
    int norder, hits;
};

static int on_hit(void *ctx, const char *v, const ACSHit *h)
{
    struct answer *a = ctx;
    (void)v;
    a->hits++;
    if (!strcmp(h->device, "WB")) snprintf(a->wb, sizeof a->wb, "%s", h->path);
    else if (!strcmp(h->device, "ORDER") && a->norder < MAXORDER) {
        const char *n = wbgap_order_name(h->path);
        snprintf(a->order[a->norder++], sizeof a->order[0], "%.63s", n ? n : h->path);
    }
    return 0;
}

/* "::wb <what> <path>": 1 if the host answered with a hit */
static int ask(const char *what, const char *path, struct answer *a)
{
    char q[300];
    ULONG unseen = 0, walking = 0;
    memset(a, 0, sizeof *a);
    if (path) snprintf(q, sizeof q, "::wb %s %s", what, path);
    else snprintf(q, sizeof q, "::wb %s", what);
    if (!oixa_search_dev(unit, q, "", on_hit, a, &unseen, &walking)) return 0;
    return a->hits > 0;
}

static int setting(const struct answer *a, const char *key, int *on, char *val, int n)
{
    char v[200];
    return wbgap_wb_field(a->wb, key, v, sizeof v) && wbgap_wb_setting(v, on, val, n);
}

/* ---- files ---- */
static void path_of(char *out, int n, const char *rel) { snprintf(out, n, "%s:%s", vol, rel); }

static int write_file(const char *rel, const char *text)
{
    char p[300];
    BPTR fh;
    path_of(p, sizeof p, rel);
    if (!(fh = Open((CONST_STRPTR)p, MODE_NEWFILE))) return 0;
    Write(fh, (APTR)text, strlen(text));
    Close(fh);
    return 1;
}

static int read_back(const char *rel, const char *want)
{
    char p[300], buf[64];
    long n;
    BPTR fh;
    path_of(p, sizeof p, rel);
    if (!(fh = Open((CONST_STRPTR)p, MODE_OLDFILE))) return 0;
    n = Read(fh, buf, sizeof buf - 1);
    Close(fh);
    buf[n > 0 ? n : 0] = 0;
    return !strcmp(buf, want);
}

static int make_dir(const char *rel)
{
    char p[300];
    BPTR l;
    path_of(p, sizeof p, rel);
    if ((l = CreateDir((CONST_STRPTR)p))) { UnLock(l); return 1; }
    if ((l = Lock((CONST_STRPTR)p, ACCESS_READ))) { UnLock(l); return 1; }
    return 0;
}

static void remove_rel(const char *rel)
{
    char p[300];
    path_of(p, sizeof p, rel);
    DeleteFile((CONST_STRPTR)p);
}

static int drawer_icon(const char *rel, STRPTR *tooltypes)
{
    char p[300];
    struct DiskObject *d = GetDefDiskObject(WBDRAWER);
    STRPTR *old;
    int ok;
    if (!d) return 0;
    path_of(p, sizeof p, rel);
    old = d->do_ToolTypes;
    d->do_ToolTypes = tooltypes;
    d->do_CurrentX = d->do_CurrentY = NO_ICON_POSITION;
    ok = PutDiskObject((CONST_STRPTR)p, d);   /* through the drive, so the host copies the settings */
    d->do_ToolTypes = old;
    FreeDiskObject(d);
    return ok;
}

/* ---- the rows ---- */
static void check_settings(void)
{
    static STRPTR t1[] = {(STRPTR)"SORT=NAME,REVERSE", (STRPTR)"SHOW=ALL", (STRPTR)"(ARRANGE=)", NULL};
    static STRPTR t2[] = {(STRPTR)"SORT=DATE", (STRPTR)"SHOW=ALL", (STRPTR)"(ARRANGE=)", NULL};
    struct answer a;
    char v[64], why[200];
    int on;
    if (!make_dir("WBGapIdx/sorted") || !drawer_icon("WBGapIdx/sorted", t1)) {
        fail("M16t", "WBGapIdx/sorted or its icon not made");
        skip("M32t", "no drawer"); skip("M15t", "no drawer"); skip("M34t", "no drawer");
        return;
    }
    if (!ask("get", "WBGapIdx/sorted", &a) || !a.wb[0]) {
        fail("M16t", "the host gave no WB line for ::wb get (needs gfosmotherboards #140)");
        skip("M32t", "no WB line"); skip("M15t", "no WB line"); skip("M34t", "no WB line");
        return;
    }
    printf("INFO ::wb get WBGapIdx/sorted: %s\n", a.wb);
    if (setting(&a, "sort", &on, v, sizeof v) && on && wbgap_same(v, "NAME,REVERSE")) pass("M32t");
    else { snprintf(why, sizeof why, "SORT=NAME,REVERSE not kept whole: %.120s", a.wb); fail("M32t", why); }
    if (setting(&a, "show", &on, v, sizeof v) && on && wbgap_same(v, "ALL")) pass("M15t");
    else fail("M15t", "SHOW=ALL not kept");
    if (setting(&a, "arrange", &on, v, sizeof v) && !on) pass("M34t");
    else fail("M34t", "(ARRANGE=) not kept as off");
    /* M16t: the view kept, and copied again when the icon changes */
    if (!drawer_icon("WBGapIdx/sorted", t2) || !ask("get", "WBGapIdx/sorted", &a)) { fail("M16t", "the icon rewritten, no answer"); return; }
    printf("INFO ::wb get after SORT=DATE: %s\n", a.wb);
    if (setting(&a, "sort", &on, v, sizeof v) && on && wbgap_same(v, "DATE")) pass("M16t");
    else fail("M16t", "SORT=DATE not copied when the icon changed");
}

static void check_order(void)
{
    char a1[300], a2[300];
    struct answer a;
    int i, ia = -1, ib = -1;
    remove_rel("WBGapIdx/order/A");
    remove_rel("WBGapIdx/order/B");
    remove_rel("WBGapIdx/A");
    if (!make_dir("WBGapIdx/order") || !write_file("WBGapIdx/order/A", "A\n")) { fail("S2i", "WBGapIdx/order/A not made"); return; }
    Delay(100);
    write_file("WBGapIdx/order/B", "B\n");
    Delay(100);
    path_of(a1, sizeof a1, "WBGapIdx/order/A");
    path_of(a2, sizeof a2, "WBGapIdx/A");
    if (!Rename((CONST_STRPTR)a1, (CONST_STRPTR)a2) || (Delay(100), !Rename((CONST_STRPTR)a2, (CONST_STRPTR)a1))) {
        fail("S2i", "A not moved out and back");
        return;
    }
    if (!ask("order", "WBGapIdx/order", &a)) { fail("S2i", "the host gave no ORDER hits for ::wb order"); return; }
    printf("INFO ::wb order WBGapIdx/order:");
    for (i = 0; i < a.norder; i++) {
        printf(" %s", a.order[i]);
        if (!strcmp(a.order[i], "A")) ia = i;
        if (!strcmp(a.order[i], "B")) ib = i;
    }
    printf("\n");
    if (ia >= 0 && ib >= 0 && ib < ia) pass("S2i");
    else fail("S2i", "not B then A (A was added again after B)");
}

static void check_trash(void)
{
    struct answer a;
    char st[16], why[200];
    int ok = 1;
    if (!write_file("WBGapIdx/t", "keep\n")) { fail("M23r", "WBGapIdx/t not made"); return; }
    if (!ask("trash", "WBGapIdx/t", &a) || !wbgap_wb_field(a.wb, "trash", st, sizeof st) || strcmp(st, "trash")) {
        snprintf(why, sizeof why, "::wb trash didn't give trash=trash: %.120s", a.wb[0] ? a.wb : "no answer");
        fail("M23r", why);
        return;
    }
    if (!ask("live", "WBGapIdx/t", &a) || !wbgap_wb_field(a.wb, "trash", st, sizeof st) || strcmp(st, "live")) {
        snprintf(why, sizeof why, "::wb live didn't give trash=live: %.120s", a.wb[0] ? a.wb : "no answer");
        fail("M23r", why);
        return;
    }
    if (!read_back("WBGapIdx/t", "keep\n")) { fail("M23r", "made live again, but its bytes are gone"); ok = 0; }
    if (ok && (!ask("final", "WBGapIdx/t", &a) || !wbgap_wb_field(a.wb, "trash", st, sizeof st) || strcmp(st, "final"))) {
        snprintf(why, sizeof why, "::wb final didn't give trash=final: %.120s", a.wb[0] ? a.wb : "no answer");
        fail("M23r", why);
        ok = 0;
    }
    if (ok) pass("M23r");
}

static void check_reclaim(void)
{
    struct answer a;
    char v[32];
    const char *line;
    if (!ask("reclaim", NULL, &a)) { fail("M23s", "the host gave no answer to ::wb reclaim"); return; }
    line = a.wb[0] ? a.wb : (a.norder ? a.order[0] : "");
    if (wbgap_wb_field(line, "reclaim", v, sizeof v) && v[0] >= '0' && v[0] <= '9') { printf("INFO reclaim after %s s\n", v); pass("M23s"); }
    else fail("M23s", "no reclaim=SECONDS");
}

static void check_long(void)
{
    char rel[260], c[200], why[300];
    struct answer a;
    int i;
    strcpy(rel, "WBGapIdx/");
    for (i = 0; i < 200; i++) rel[9 + i] = 'a' + i % 26;
    rel[209] = 0;
    remove_rel(rel);
    if (!write_file(rel, "long\n")) {
        snprintf(why, sizeof why, "a 200-character name refused (IoErr %ld)", (long)IoErr());
        fail("X2a", why);
        skip("X2b", "no long name");
        return;
    }
    if (read_back(rel, "long\n")) pass("X2a"); else fail("X2a", "a 200-character name made but not read back");
    if (!ask("get", rel, &a) || !wbgap_wb_field(a.wb, "compat", c, sizeof c)) { fail("X2b", "no compat= for the long name"); return; }
    printf("INFO X2b compat name (%d characters): %s\n", (int)strlen(c), c);
    if (wbgap_compat_ok(c)) pass("X2b");
    else fail("X2b", "not 107 characters at most ending in ~ and 16 hex digits");
    remove_rel(rel);
}

int main(int argc, char **argv)
{
    LONG i;
    if (!(IconBase = OpenLibrary("icon.library", 44))) { skip_all("no icon.library 44"); return 20; }
    if (!oixa_refresh(0, 0)) { skip_all("no ACFS volume"); CloseLibrary(IconBase); return 0; }
    /* Work: unless a VOLUME is given, and never another: a lab's other ACFS units can be shared host paths, and
     * this writes, renames, trashes and deletes files */
    i = oixa_find(argc > 1 ? argv[1] : "Work");
    if (i < 0) {
        skip_all(argc > 1 ? "that volume isn't ACFS" : "Work: isn't ACFS; give VOLUME");
        CloseLibrary(IconBase);
        return 10;
    }
    unit = (ULONG)i;
    snprintf(vol, sizeof vol, "%s", oixa_name(unit));
    printf("INFO WBGapIndex on %s: (ACFS volume %ld of %ld)\n", vol, (long)i + 1, (long)oixa_count());
    if (!make_dir("WBGapIdx")) { skip_all("WBGapIdx not made: the volume is read-only?"); CloseLibrary(IconBase); return 10; }
    check_settings();
    check_order();
    check_trash();
    check_reclaim();
    check_long();
    CloseLibrary(IconBase);
    return 0;
}
