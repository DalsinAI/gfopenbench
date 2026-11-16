/* C6's judges (CLOSEWB.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "CLOSEWB.h"

int wbgap_closewb_judge(int closed, int reopened, int same_screen, int port, int drawer, char *why, unsigned cap)
{
    const char *r = "";
    int v = WBGAP_FAIL;
    if (closed && !reopened) r = "the screen closed and OpenWorkBench() didn't bring it back";
    else if (!closed && !same_screen) r = "CloseWorkBench() refused, but the Workbench screen changed";
    else if (!port) r = closed ? "the desktop didn't come back after OpenWorkBench()" : "the desktop went though CloseWorkBench() refused";
    else if (!drawer) r = closed ? "RAM:WBGapC's window wasn't opened again" : "RAM:WBGapC's window went though CloseWorkBench() refused";
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}

int wbgap_closewb_over(const char *text)
{
    const char *p, *want = "INFO C6 done";
    size_t n = strlen(want);
    if (!text) return 0;
    for (p = text; (p = strstr(p, want)); p += n)
        if ((p == text || p[-1] == '\n') && (p[n] == '\n' || p[n] == '\r' || p[n] == 0)) return 1;
    return 0;
}

int wbgap_closewb_last_step(const char *text, char *out, unsigned cap)
{
    const char *p, *want = "INFO C6 step: ", *last = NULL;
    size_t n = strlen(want), len = 0;
    if (out && cap) out[0] = 0;
    if (!text || !out || !cap) return 0;
    for (p = text; (p = strstr(p, want)); p += n)
        if (p == text || p[-1] == '\n') last = p + n;
    if (!last) return 0;
    while (last[len] && last[len] != '\n' && last[len] != '\r') len++;
    if (len >= cap) len = cap - 1;
    memcpy(out, last, len);
    out[len] = 0;
    return 1;
}

/* the reason after a FAIL, to CR, LF or the end, cut to cap - 1 */
static void closewb_copy_line(const char *from, char *why, unsigned cap)
{
    size_t len = 0;
    if (!why || !cap) return;
    while (*from == ' ') from++;
    while (from[len] && from[len] != '\n' && from[len] != '\r') len++;
    if (len >= cap) len = cap - 1;
    memcpy(why, from, len);
    why[len] = 0;
}

int wbgap_closewb_row_verdict(const char *text, const char *row, char *why, unsigned cap)
{
    static const char *const word[] = { "PASS", "FAIL", "SKIP" };
    static const int state[] = { WBGAP_PASS, WBGAP_FAIL, WBGAP_SKIP };
    char want[2][40];
    const char *p, *line, *after = NULL;
    int v = WBGAP_NONE, f, k;
    if (why && cap) why[0] = 0;
    if (!text || !row || !row[0] || strlen(row) > 8) return WBGAP_NONE;
    for (line = text; *line; line = p + 1) {
        for (k = 0; k < 3; k++) {
            snprintf(want[0], sizeof want[0], "%s %s", word[k], row);                 /* its stdout line */
            snprintf(want[1], sizeof want[1], "INFO %s verdict: %s", row, word[k]);  /* its steps file line */
            for (f = 0; f < 2; f++) {
                size_t n = strlen(want[f]);
                char c = line[n];
                if (!strncmp(line, want[f], n) && (c == 0 || c == '\n' || c == '\r' || (k && c == ' '))) {
                    v = state[k];
                    after = line + n;
                }
            }
        }
        if (!(p = strchr(line, '\n'))) break;
    }
    if (v == WBGAP_FAIL || v == WBGAP_SKIP) closewb_copy_line(after, why, cap);
    return v;
}

int wbgap_closewb_verdict(const char *text, char *why, unsigned cap)
{
    return wbgap_closewb_row_verdict(text, "C6", why, cap);
}

int wbgap_closewb_obwait(int found, int waiting, unsigned long sigwait, int pub_bit, int rexx_bit)
{
    int pub, rexx, have = 0, hit = 0;
    if (!found) return WBGAP_OBW_NONE;
    if (!waiting) return WBGAP_OBW_RUNNING;
    pub = pub_bit >= 0 && pub_bit < 32;
    rexx = rexx_bit >= 0 && rexx_bit < 32;
    if (pub) { have++; hit += (sigwait >> pub_bit) & 1; }
    if (rexx) { have++; hit += (sigwait >> rexx_bit) & 1; }
    if (!have || !hit) return WBGAP_OBW_ELSEWHERE;
    return hit == have ? WBGAP_OBW_MAIN : WBGAP_OBW_PART;
}

const char *wbgap_closewb_obwait_text(int where)
{
    switch (where) {
    case WBGAP_OBW_MAIN: return "waiting in its main loop";
    case WBGAP_OBW_PART: return "waiting for some of its ports only";
    case WBGAP_OBW_ELSEWHERE: return "waiting inside a call, not for its ports (a requester or a reply)";
    case WBGAP_OBW_RUNNING: return "not waiting (running or ready)";
    default: return "no GFOPENBENCH port";
    }
}
