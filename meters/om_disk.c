/* om_disk.c: the host disk a drive shares (GFOpenMeters 0.4; om_disk.h). Plain C, tested on the host.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "om_disk.h"

static const char *blanks(const char *p)
{
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    return p;
}

/* Past the string that starts at p ('"'): at its closing quote, or 0 when the text ends first. */
static const char *string_end(const char *p)
{
    for (p++; *p && *p != '"'; p++) if (*p == '\\' && p[1]) p++;
    return *p ? p : 0;
}

/* The value of "key" inside the object that starts at obj ('{'), not in an object or array nested in it: a string's
 * text or a number's digits, into out. 0 when not there, or when the text ends before it. */
static int value_in(const char *obj, const char *key, char *out, int size)
{
    const char *p = obj + 1;
    int depth = 0, klen = (int)strlen(key), i = 0;
    while (*p) {
        if (*p == '"') {
            const char *s = p + 1, *e = string_end(p);
            if (!e) return 0;
            p = e + 1;
            if (depth == 0 && e - s == klen && !strncmp(s, key, (size_t)klen)) {
                p = blanks(p);
                if (*p != ':') continue;
                p = blanks(p + 1);
                if (*p == '"') {
                    for (p++; *p && *p != '"' && i < size - 1; p++) out[i++] = *p == '\\' && p[1] ? *++p : *p;
                    if (!*p) return 0;
                } else {
                    for (; *p && *p != ',' && *p != '}' && *p != ']' && *p != ' ' && *p != '\n' && i < size - 1; p++) out[i++] = *p;
                    if (!*p) return 0;                  /* cut short in the number: not a number to show */
                }
                out[i] = 0;
                return 1;
            }
            continue;
        }
        if (*p == '{' || *p == '[') depth++;
        else if (*p == '}' || *p == ']') { if (!depth--) return 0; }
        p++;
    }
    return 0;
}

/* Digits only, to a number; 0 when it is anything else (a null, a fraction, a minus). */
static int number(const char *v, unsigned long long *n)
{
    unsigned long long x = 0;
    int i;
    if (!v[0]) return 0;
    for (i = 0; v[i]; i++) {
        if (v[i] < '0' || v[i] > '9' || i >= 19) return 0;
        x = x * 10 + (unsigned)(v[i] - '0');
    }
    *n = x;
    return 1;
}

int om_disk_read(const char *status, struct om_disk *out, int max)
{
    const char *p = status;
    int n = 0, depth = 0;
    if (!status) return 0;
    /* the "volumes" that is a list: a "volumes" elsewhere that is not is passed over */
    while ((p = strstr(p, "\"volumes\""))) {
        p = blanks(p + 9);
        if (*p != ':') continue;
        p = blanks(p + 1);
        if (*p == '[') break;
    }
    if (!p) return 0;
    for (p++; *p && n < max; p++) {
        if (*p == '"') { if (!(p = string_end(p))) break; continue; }
        if (*p == '{' && depth++ == 0) {
            char v[32];
            unsigned long long host_free, shared;
            struct om_disk *d = &out[n];
            if (value_in(p, "hostFreeBytes", v, sizeof v) && number(v, &host_free) &&
                value_in(p, "hostSharedBy", v, sizeof v) && number(v, &shared) && shared > 1 && shared < 1000 &&
                value_in(p, "label", d->label, sizeof d->label) && d->label[0]) {
                d->host_free = host_free;
                d->shared_by = (unsigned)shared;
                n++;
            }
        } else if (*p == '[') depth++;
        else if (*p == '}' || *p == ']') { if (!depth--) break; }   /* the list's own end */
    }
    return n;
}

static int lower(int c) { return c >= 'A' && c <= 'Z' ? c + 32 : c; }

const struct om_disk *om_disk_find(const struct om_disk *d, int n, const char *volume)
{
    int i, k;
    if (!d || !volume) return 0;
    for (i = 0; i < n; i++) {
        for (k = 0; d[i].label[k] && lower((unsigned char)d[i].label[k]) == lower((unsigned char)volume[k]); k++) ;
        if (!d[i].label[k] && !volume[k]) return &d[i];
    }
    return 0;
}

void om_disk_bytes(char *out, int size, unsigned long long bytes)
{
    char tmp[32];
    const char *unit = "MBytes";
    unsigned long long v;
    int len, i, k = 0;
    if (size < 1) return;
    if (bytes >= (10ULL << 30)) { v = (bytes + (1ULL << 29)) >> 30; unit = "GBytes"; }
    else v = (bytes + (1ULL << 19)) >> 20;
    snprintf(tmp, sizeof tmp, "%llu", v);
    len = (int)strlen(tmp);
    for (i = 0; i < len && k < size - 1; i++) {
        if (i && (len - i) % 3 == 0 && k < size - 2) out[k++] = ',';
        out[k++] = tmp[i];
    }
    out[k] = 0;
    snprintf(out + k, (size_t)(size - k), " %s", unit);
}

void om_disk_value(char *out, int size, unsigned long long freeb, unsigned long long host_free)
{
    char own[32], whole[32];
    om_disk_bytes(own, sizeof own, freeb);
    om_disk_bytes(whole, sizeof whole, host_free);
    /* brackets only when they say more: the whole disk has more free than the share, as the two are written */
    if (host_free > freeb && strcmp(own, whole)) snprintf(out, (size_t)size, "%s (%s)", own, whole);
    else snprintf(out, (size_t)size, "%s", own);
}
