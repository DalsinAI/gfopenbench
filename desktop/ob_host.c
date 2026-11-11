/* ob_host.c: the host's cores in the screen's title (GFOpenBench 0.1.45; ob_host.h). Plain C, tested on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "ob_host.h"

static int starts(const char *s, const char *p)
{
    return !strncmp(s, p, strlen(p));
}

void ob_host_isa(const char *machine, char out[16])
{
    const char *n;
    if (!machine || !*machine) n = "";
    else if (starts(machine, "aarch64") || starts(machine, "arm")) n = "ARM";
    else if (!strcmp(machine, "x86_64") || !strcmp(machine, "amd64") || !strcmp(machine, "AMD64")) n = "x64";
    else if (machine[0] == 'i' && machine[1] >= '3' && machine[1] <= '6' && !strcmp(machine + 2, "86")) n = "x86";
    else if (starts(machine, "ppc") || starts(machine, "powerpc")) n = "PPC";
    else n = machine;
    strncpy(out, n, 15);
    out[15] = 0;
}

/* The value of "key" inside the object that starts at obj ('{'), not in an object or array nested in it: a string's
 * text or a number's digits, into out. 0 when not there. */
static int value_in(const char *obj, const char *key, char *out, int size)
{
    const char *p = obj + 1;
    int depth = 0, klen = (int)strlen(key), i = 0;
    while (*p) {
        if (*p == '"') {
            const char *s = ++p;
            while (*p && *p != '"') { if (*p == '\\' && p[1]) p++; p++; }
            if (!*p) return 0;
            if (depth == 0 && p - s == klen && !strncmp(s, key, klen)) {
                p++;
                while (*p == ' ') p++;
                if (*p != ':') continue;
                p++;
                while (*p == ' ') p++;
                if (*p == '"') {
                    for (p++; *p && *p != '"' && i < size - 1; p++) out[i++] = *p == '\\' && p[1] ? *++p : *p;
                } else
                    for (; *p && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && i < size - 1; p++) out[i++] = *p;
                out[i] = 0;
                return 1;
            }
            p++;
            continue;
        }
        if (*p == '{' || *p == '[') depth++;
        else if (*p == '}' || *p == ']') { if (!depth--) return 0; }
        p++;
    }
    return 0;
}

unsigned ob_host_cores(const char *status, char isa[16])
{
    const char *h = status;
    char v[32];
    unsigned n = 0;
    int i;
    isa[0] = 0;
    if (!status) return 0;
    /* the top level's "host": an object; a "host" elsewhere that is a string ("clock" has none) is passed over */
    while ((h = strstr(h, "\"host\""))) {
        h += 6;
        while (*h == ' ') h++;
        if (*h != ':') continue;
        h++;
        while (*h == ' ') h++;
        if (*h == '{') break;
    }
    if (!h) return 0;
    if (!value_in(h, "logicalCores", v, sizeof v)) return 0;
    for (i = 0; v[i] >= '0' && v[i] <= '9' && n < 100000; i++) n = n * 10 + (unsigned)(v[i] - '0');
    if (v[i] || !n) return 0;
    if (value_in(h, "machine", v, sizeof v)) ob_host_isa(v, isa);
    else if (value_in(h, "architecture", v, sizeof v)) ob_host_isa(v, isa);
    return n;
}
