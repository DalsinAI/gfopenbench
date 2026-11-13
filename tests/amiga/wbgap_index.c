/* wbgap_index: reading the ACFS index's "::wb" answers (wbgap_index.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_index.h"

static const char *const keys[] = {"added", "trash", "trashed", "final", "reclaim", "compat", "sort", "show", "arrange"};

static int lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

/* " key=" of a known key starts at p (p just after a blank) */
static int key_at(const char *p)
{
    size_t k;
    for (k = 0; k < sizeof keys / sizeof keys[0]; k++) {
        size_t n = strlen(keys[k]);
        if (!strncmp(p, keys[k], n) && p[n] == '=') return 1;
    }
    return 0;
}

int wbgap_wb_field(const char *line, const char *key, char *out, int n)
{
    size_t kl = strlen(key);
    const char *p = line, *e;
    int k = 0;
    if (n > 0) out[0] = 0;
    for (; *p; p++) {
        if (p != line && p[-1] != ' ') continue;
        if (strncmp(p, key, kl) || p[kl] != '=') continue;
        p += kl + 1;
        for (e = p; *e && *e != '\n' && *e != '\r'; e++)
            if (*e == ' ' && key_at(e + 1)) break;
        while (p < e && k < n - 1) out[k++] = *p++;
        if (n > 0) out[k] = 0;
        return 1;
    }
    return 0;
}

int wbgap_wb_setting(const char *value, int *on, char *val, int n)
{
    const char *rest;
    if (!strncmp(value, "on", 2) && (value[2] == ':' || !value[2])) { *on = 1; rest = value + 2; }
    else if (!strncmp(value, "off", 3) && (value[3] == ':' || !value[3])) { *on = 0; rest = value + 3; }
    else return 0;
    if (*rest == ':') rest++;
    if (n > 0) {
        int k = 0;
        while (*rest && k < n - 1) val[k++] = *rest++;
        val[k] = 0;
    }
    return 1;
}

int wbgap_compat_ok(const char *name)
{
    size_t n = strlen(name), i;
    if (n < 17 || n > 107 || name[n - 17] != '~') return 0;
    for (i = n - 16; i < n; i++) {
        int c = lower(name[i]);
        if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))) return 0;
    }
    return 1;
}

const char *wbgap_order_name(const char *line)
{
    const char *p = line;
    if (*p < '0' || *p > '9') return NULL;
    while (*p >= '0' && *p <= '9') p++;
    if (*p != ' ' || !p[1]) return NULL;
    return p + 1;
}

int wbgap_same(const char *a, const char *b)
{
    for (; *a && *b; a++, b++)
        if (lower(*a) != lower(*b)) return 0;
    return !*a && !*b;
}
