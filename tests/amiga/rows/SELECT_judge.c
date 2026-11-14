/* I1 and I2's judge (SELECT.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "SELECT.h"

#define SEL_MAX 16
#define SEL_LEN 32

/* the names in s, split at any of seps, blanks and CRs trimmed, empty ones left out; how many (at most SEL_MAX) */
static int sel_split(const char *s, const char *seps, char out[SEL_MAX][SEL_LEN])
{
    int n = 0;
    while (s && *s) {
        size_t len = strcspn(s, seps), a = 0, b = len;
        while (a < b && (s[a] == ' ' || s[a] == '\t' || s[a] == '\r')) a++;
        while (b > a && (s[b - 1] == ' ' || s[b - 1] == '\t' || s[b - 1] == '\r')) b--;
        if (b > a && n < SEL_MAX) {
            if (b - a >= SEL_LEN) b = a + SEL_LEN - 1;
            memcpy(out[n], s + a, b - a);
            out[n][b - a] = 0;
            n++;
        }
        s += len;
        if (*s) s++;
    }
    return n;
}

static int sel_in(char set[SEL_MAX][SEL_LEN], int n, const char *name)
{
    int i;
    for (i = 0; i < n; i++)
        if (!strcmp(set[i], name)) return 1;
    return 0;
}

/* the names joined with blanks, "nothing" for none */
static void sel_join(char set[SEL_MAX][SEL_LEN], int n, char *buf, unsigned cap)
{
    int i;
    size_t at = 0;
    buf[0] = 0;
    if (!n) { snprintf(buf, cap, "nothing"); return; }
    for (i = 0; i < n && at + 1 < cap; i++) {
        int k = snprintf(buf + at, cap - at, "%s%s", i ? " " : "", set[i]);
        if (k < 0) break;
        at += (size_t)k;
    }
}

int wbgap_select_judge(const char *list, const char *want, const char *step, char *why, unsigned cap)
{
    char got[SEL_MAX][SEL_LEN], wnt[SEL_MAX][SEL_LEN], g[200], e[200], r[300] = "";
    int ng, ne, i, ok, v = WBGAP_FAIL;
    if (!step) step = "the clicks";
    if (!list) {
        snprintf(r, sizeof r, "the selected icons weren't read after %.60s", step);
        v = WBGAP_SKIP;
    } else {
        ng = sel_split(list, "\n", got);
        ne = sel_split(want, " ", wnt);
        ok = ng == ne;
        for (i = 0; ok && i < ng; i++) ok = sel_in(wnt, ne, got[i]);
        for (i = 0; ok && i < ne; i++) ok = sel_in(got, ng, wnt[i]);
        if (ok) v = WBGAP_PASS;
        else {
            sel_join(got, ng, g, sizeof g);
            sel_join(wnt, ne, e, sizeof e);
            snprintf(r, sizeof r, "after %.60s: %.100s selected, not %.100s", step, g, e);
        }
    }
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
