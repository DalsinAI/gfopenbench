/* The STARTUP rows' judges (STARTUP.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "STARTUP.h"

static const char *const loadwb_switches[] = {"DEBUG", "-DEBUG", "DELAY", "CLEANUP", "NEWPATH", "SKIPWBSTARTUP", "SKIP",
                                              "SIMPLEGELS", "LEGACY"};

static int stricmp_n(const char *a, const char *b, int n)
{
    for (; n > 0; n--, a++, b++) {
        char x = (*a >= 'a' && *a <= 'z') ? *a - 32 : *a, y = (*b >= 'a' && *b <= 'z') ? *b - 32 : *b;
        if (x != y) return 1;
        if (!x) return 0;
    }
    return 0;
}

int wbgap_startup_order(const char *order, const char *row, int gfos, char *why, unsigned cap)
{
    const char *sa, *sb, *sc, *eb, *ec, *r = "";
    int v;
    if (!order) order = "";
    sa = strstr(order, "start WBGapA"); sb = strstr(order, "start WBGapB"); sc = strstr(order, "start WBGapC");
    eb = strstr(order, "end WBGapB"); ec = strstr(order, "end WBGapC");
    if (!strcmp(row, "S1"))
        v = sa && sb && sc ? WBGAP_PASS : (r = "not all three started", WBGAP_FAIL);
    else if (!strcmp(row, "S2"))
        v = sa && sb && sc && sb < sc && sc < sa ? WBGAP_PASS : (r = "not in STARTPRI order B, C, A", WBGAP_FAIL);
    else if (!strcmp(row, "S3"))
        v = sc && eb && ec && sa && sc < eb && ec < sa ? WBGAP_PASS
                                                       : (r = "DONOTWAIT or the wait for a program not honoured", WBGAP_FAIL);
    else if (!strcmp(row, "S2d")) {
        const char *sd = strstr(order, "start WBGapD"), *se = strstr(order, "start WBGapE");
        if (!sd || !se) v = WBGAP_SKIP, r = "WBGapD or WBGapE didn't start (not set up?)";
        else if (!gfos) v = WBGAP_SKIP, r = "a GFOS decision; 3.2.3 orders by name";
        else if (sc && se < sc && sc < sd) v = WBGAP_PASS;
        else v = WBGAP_FAIL, r = "not by date added E, C, D";
    } else
        v = WBGAP_SKIP, r = "not a STARTUP order row";
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_startup_switches(const char *text, char *out, int n)
{
    int k, count = 0;
    char up[400], *t, *w;
    if (n > 0) out[0] = 0;
    snprintf(up, sizeof up, "%s", text ? text : "");
    for (t = up; *t; t++) if (*t >= 'a' && *t <= 'z') *t -= 32;
    for (k = 0; k < (int)(sizeof loadwb_switches / sizeof loadwb_switches[0]); k++) {
        const char *sw = loadwb_switches[k];
        char tmp[400];
        snprintf(tmp, sizeof tmp, "%s", up);
        for (w = strtok(tmp, " \t\r\n"); w; w = strtok(NULL, " \t\r\n"))
            if (!strcmp(w, sw)) {
                const char *name = !strcmp(sw, "-DEBUG") ? "DEBUG" : !strcmp(sw, "SKIP") ? "SKIPWBSTARTUP" : sw;
                if (!strstr(out, name) && (int)(strlen(out) + strlen(name) + 2) < n) { strcat(out, " "); strcat(out, name); count++; }
            }
    }
    return count;
}

const char *wbgap_startup_desk_rest(const char *line, int ob)
{
    const char *p = line, *cmd = ob ? "SYS:System/OpenBench" : "LoadWB";
    int cl = (int)strlen(cmd);
    if (!p) return NULL;
    for (;;) {   /* leading blanks, "Run" and redirections */
        while (*p == ' ' || *p == '\t') p++;
        if (!stricmp_n(p, "Run ", 4)) { p += 4; continue; }
        if (*p == '>' || *p == '<') { while (*p && *p != ' ' && *p != '\t') p++; continue; }
        break;
    }
    if (!ob && !stricmp_n(p, "C:LoadWB", 8)) p += 2;
    if (!stricmp_n(p, cmd, cl) && (p[cl] == ' ' || p[cl] == '\t' || p[cl] == '\n' || !p[cl])) return p + cl;
    return NULL;
}
