/* W2's judges (SWITCH.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "SWITCH.h"

int wbgap_switcher_on(const char *env, char *key, unsigned cap)
{
    const char *p = env;
    int on = 1;
    if (cap) snprintf(key, cap, "lcommand tab");
    while (p && *p) {
        const char *e = p, *v;
        size_t n;
        while (*e && *e != '\n') e++;
        n = (size_t)(e - p);
        while (n && (p[n - 1] == '\r' || p[n - 1] == ' ' || p[n - 1] == '\t')) n--;
        while (n && (*p == ' ' || *p == '\t')) { p++; n--; }
        if (n > 13 && !strncmp(p, "switcher.key", 12) && (p[12] == ' ' || p[12] == '\t')) {
            for (v = p + 13; v < p + n && (*v == ' ' || *v == '\t'); v++) {}
            if (cap) snprintf(key, cap, "%.*s", (int)(p + n - v), v);
        } else if (n > 9 && !strncmp(p, "switcher", 8) && (p[8] == ' ' || p[8] == '\t')) {
            for (v = p + 9; v < p + n && (*v == ' ' || *v == '\t'); v++) {}
            if ((size_t)(p + n - v) == 3 && !strncmp(v, "off", 3)) on = 0;
            else if ((size_t)(p + n - v) == 2 && !strncmp(v, "on", 2)) on = 1;
        }
        p = *e ? e + 1 : e;
    }
    return on;
}

int wbgap_switch_offers(unsigned long flags, int width, int height)
{
    if ((flags & 0x100UL) || width < 8 || height < 8) return 0;            /* WFLG_BACKDROP, tiny */
    if ((flags & 0x2000000UL) && !(flags & 0x8UL)) return 0;               /* WFLG_WBENCHWINDOW without CLOSEGADGET */
    return (flags & 0xFUL) != 0;                                            /* SIZE, DRAGBAR, DEPTH or CLOSE gadget */
}

int wbgap_switch_judge(int back, const unsigned long *before, int nb, const unsigned long *after, int na,
                       unsigned long active, char *why, unsigned cap)
{
    unsigned long want_top, want_active;
    if (!before || !after || nb < 2) {
        snprintf(why, cap, "fewer than two windows to cycle");
        return WBGAP_SKIP;
    }
    if (na != nb) {
        snprintf(why, cap, "%d windows before, %d after: a window came or went", nb, na);
        return WBGAP_FAIL;
    }
    if (!back) {
        want_top = before[nb - 1];
        want_active = before[nb - 1];
        if (after[0] != want_top) {
            snprintf(why, cap, "Amiga+Tab: the window at the back didn't come to the front");
            return WBGAP_FAIL;
        }
    } else {
        want_active = before[1];
        if (after[na - 1] != before[0]) {
            snprintf(why, cap, "Amiga+Shift+Tab: the front window didn't go to the back");
            return WBGAP_FAIL;
        }
    }
    if (active != want_active) {
        snprintf(why, cap, "%s: the window now %s isn't the active one", back ? "Amiga+Shift+Tab" : "Amiga+Tab",
                 back ? "in front" : "brought to the front");
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}
