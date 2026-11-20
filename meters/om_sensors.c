/* om_sensors.c: the host's sensors for GFOpenMeters' Temp row (0.5; om_sensors.h). Plain C, tested on the host.
 * The small JSON reading is om_disk.c's: a key's value in one object, not in an object or array nested in it.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "om_sensors.h"

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

/* The value of "key" inside the object that starts at obj ('{'), not in one nested in it: a string's text, or a
 * number's or a true's or false's characters, into out. 0 when not there, or when the text ends before it. */
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
                    if (!*p) return 0;                  /* cut short in the value */
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

/* Digits only, to a number; 0 for anything else (a null, a fraction, a minus). */
static int digits(const char *v, unsigned long *n)
{
    unsigned long x = 0;
    if (!*v) return 0;
    for (; *v; v++) {
        if (*v < '0' || *v > '9') return 0;
        x = x * 10 + (unsigned long)(*v - '0');
    }
    *n = x;
    return 1;
}

/* "0x50005" to a number; 0 for anything else. */
static int hex(const char *v, unsigned long *n)
{
    unsigned long x = 0;
    if (v[0] != '0' || (v[1] != 'x' && v[1] != 'X') || !v[2]) return 0;
    for (v += 2; *v; v++) {
        int d = *v >= '0' && *v <= '9' ? *v - '0' : *v >= 'a' && *v <= 'f' ? *v - 'a' + 10
              : *v >= 'A' && *v <= 'F' ? *v - 'A' + 10 : -1;
        if (d < 0) return 0;
        x = x * 16 + (unsigned long)d;
    }
    *n = x;
    return 1;
}

/* The "sensors" object: where the key is followed by ':' and '{' (host_power.py's sensors(), only under "host"). */
static const char *sensors_object(const char *status)
{
    const char *p = status;
    while ((p = strstr(p, "\"sensors\""))) {
        const char *q = blanks(p + 9);
        if (*q == ':' && *(q = blanks(q + 1)) == '{') return q;
        p += 9;
    }
    return 0;
}

int om_sensors_read(const char *status, struct om_sensors *s)
{
    const char *o;
    char v[32];
    unsigned long n;
    memset(s, 0, sizeof *s);
    if (!status || !(o = sensors_object(status))) return 0;
    if (value_in(o, "tempMilliC", v, sizeof v) && digits(v, &n)) { s->have_temp = 1; s->temp_milli = n; }
    if (value_in(o, "clockKHz", v, sizeof v) && digits(v, &n) && n) { s->have_clock = 1; s->clock_khz = n; }
    if (value_in(o, "throttled", v, sizeof v) && hex(v, &n)) { s->have_throttled = 1; s->throttled = n; }
    if (value_in(o, "underVoltage", v, sizeof v) && (!strcmp(v, "true") || !strcmp(v, "false"))) {
        s->have_uv = 1;
        s->under_voltage = v[0] == 't';
    }
    if (value_in(o, "fanRPM", v, sizeof v) && digits(v, &n)) { s->have_fan = 1; s->fan_rpm = n; }
    return s->have_temp || s->have_clock || s->have_throttled || s->have_uv || s->have_fan;
}

void om_sensors_value(char *out, int size, const struct om_sensors *s)
{
    int len = 0;
    if (size <= 0) return;
    out[0] = 0;
#define ADD(...) do { if (len < size) { int w_ = snprintf(out + len, (size_t)(size - len), __VA_ARGS__); \
    if (w_ > 0) len += w_; } } while (0)
    if (s->have_temp) ADD("%lu\xb0" "C", (s->temp_milli + 500) / 1000);
    if (s->have_clock) {
        if (s->clock_khz >= 1000000) {
            unsigned long tenths = (s->clock_khz + 50000) / 100000;
            ADD("%s%lu.%lu GHz", s->have_temp ? " at " : "", tenths / 10, tenths % 10);
        } else {
            ADD("%s%lu MHz", s->have_temp ? " at " : "", (s->clock_khz + 500) / 1000);
        }
    }
    if (s->have_throttled && (s->throttled & (OM_THR_CAPPED | OM_THR_THROTTLED | OM_THR_SOFTTEMP)))
        ADD("%sthrottled", len ? ", " : "");
    if ((s->have_throttled && (s->throttled & OM_THR_UNDERVOLT)) || (s->have_uv && s->under_voltage))
        ADD("%slow volts", len ? ", " : "");
    if (s->have_fan) ADD("%s%lu rpm", len ? ", " : "", s->fan_rpm);    /* no comma: the whole row fits 47 */
#undef ADD
}

unsigned long om_sensors_fill(const struct om_sensors *s)
{
    if (!s->have_temp) return (unsigned long)-1;
    if (s->temp_milli >= OM_SENSORS_HOT_MILLIC) return 1000;
    return s->temp_milli * 1000 / OM_SENSORS_HOT_MILLIC;
}
