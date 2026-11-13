/* ol_core.c: the launcher's rules (ol_core.h). Plain C, no Amiga headers.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ol_core.h"

static int ol_lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

const char *ol_tt(const char *const *tt, const char *key)
{
    for (; tt && *tt; tt++) {
        const char *a = *tt, *b = key;
        while (*b && ol_lower((unsigned char)*a) == ol_lower((unsigned char)*b)) a++, b++;
        if (*b) continue;
        if (!*a) return a;
        if (*a == '=') return a + 1;
    }
    return 0;
}

long ol_num(const char *s, long def)
{
    long n = 0;
    int neg = 0, any = 0;
    if (!s) return def;
    while (*s == ' ') s++;
    if (*s == '-' || *s == '+') neg = *s++ == '-';
    for (; *s >= '0' && *s <= '9'; s++, any = 1)
        if (n < 100000000L) n = n * 10 + (*s - '0');
    return any ? (neg ? -n : n) : def;
}

static int ol_clamp(long v, int lo, int hi) { return v < lo ? lo : v > hi ? hi : (int)v; }

void ol_start_read(ol_start *e, const char *const *tt)
{
    e->pri = ol_clamp(ol_num(ol_tt(tt, "STARTPRI"), 0), -128, 127);
    if (ol_tt(tt, "DONOTWAIT")) e->wait = 0;
    else e->wait = ol_clamp(ol_num(ol_tt(tt, "WAIT"), OL_WAIT_MAX), 0, 3600);
}

int ol_start_cmp(const void *x, const void *y)
{
    const ol_start *a = (const ol_start *)x, *b = (const ol_start *)y;
    const char *p = a->name, *q = b->name;
    if (a->pri != b->pri) return b->pri - a->pri;
    if (a->added != b->added) {
        if (!a->added) return 1;                 /* a date not known goes after every known one */
        if (!b->added) return -1;
        return a->added < b->added ? -1 : 1;
    }
    while (*p && ol_lower((unsigned char)*p) == ol_lower((unsigned char)*q)) p++, q++;
    return ol_lower((unsigned char)*p) - ol_lower((unsigned char)*q);
}

unsigned long ol_stamp_secs(long days, long mins, long ticks)
{
    if (days < 0 || mins < 0 || ticks < 0) return 0;
    return (unsigned long)days * 86400UL + (unsigned long)mins * 60UL + (unsigned long)ticks / 50UL;
}

unsigned long ol_added(unsigned long prog, unsigned long info)
{
    if (!prog) return info;
    if (!info) return prog;
    return prog < info ? prog : info;
}

void ol_opts_read(ol_opts *o, const char *const *tt, long icon_stack, long wb_stack)
{
    long st = ol_num(ol_tt(tt, "STACK"), 0);
    if (st > OL_STACK_MAX) st = OL_STACK_MAX;
    if (icon_stack > OL_STACK_MAX) icon_stack = OL_STACK_MAX;   /* the icon's own too (a corrupt or odd icon) */
    o->stack = icon_stack;
    if (st > o->stack) o->stack = st;
    if (wb_stack > o->stack) o->stack = wb_stack;
    o->pri = ol_clamp(ol_num(ol_tt(tt, "TOOLPRI"), 0), -128, 127);
}

int ol_shift_args(const int *sel, int n, int clicked, int *tool, int *args)
{
    int i, k = 0;
    *tool = -1;
    if (clicked < 0 || clicked >= n) return -1;
    if (sel[clicked] == OL_K_TOOL) *tool = clicked;
    else if (sel[clicked] == OL_K_PROJECT) {
        for (i = 0; i < n; i++)
            if (sel[i] == OL_K_TOOL) { *tool = i; break; }
    } else
        return -1;
    for (i = 0; i < n; i++)
        if (i != *tool) args[k++] = i;
    return k;
}

static const char *const ol_words[] = { "-DEBUG", "DELAY", "CLEANUP", "NEWPATH", "SKIPWBSTARTUP", "SIMPLEGELS" };
#define OL_NWORDS (int)(sizeof ol_words / sizeof ol_words[0])

int ol_switch_words(unsigned long flags, char *buf, int n)
{
    int i, k = 0;
    if (n <= 0) return 0;
    buf[0] = 0;
    for (i = 0; i < OL_NWORDS; i++) {
        const char *w = ol_words[i];
        if (!(flags & (1UL << i))) continue;
        if (k && k < n - 1) buf[k++] = ' ';
        while (*w && k < n - 1) buf[k++] = *w++;
    }
    buf[k] = 0;
    return k;
}

/* 1 when word (len characters) is w, case aside */
static int ol_is(const char *word, int len, const char *w)
{
    int i;
    for (i = 0; i < len && w[i]; i++)
        if (ol_lower((unsigned char)word[i]) != ol_lower((unsigned char)w[i])) return 0;
    return i == len && !w[i];
}

unsigned long ol_switch_parse(const char *line)
{
    unsigned long f = 0;
    int i;
    while (line && *line) {
        const char *w;
        int len;
        while (*line == ' ' || *line == '\t' || *line == '\n') line++;
        w = line;
        while (*line && *line != ' ' && *line != '\t' && *line != '\n') line++;
        len = (int)(line - w);
        if (!len) break;
        for (i = 0; i < OL_NWORDS; i++)
            if (ol_is(w, len, ol_words[i])) f |= 1UL << i;
        if (ol_is(w, len, "SKIP")) f |= OL_SW_SKIPSTARTUP;
    }
    return f;
}
