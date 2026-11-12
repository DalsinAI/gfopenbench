/* ob_startup's host test: SYS:WBStartup's order and waiting (GFOpenBench 0.1.50, WBGapTest rows S2, S3).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ob_startup.h"

static int fails;
#define CHECK(what, cond) do { if (!(cond)) { printf("FAIL %s\n", what); fails++; } } while (0)

static ob_start make(const char *name, const char *const *tt)
{
    ob_start e;
    memset(&e, 0, sizeof e);
    snprintf(e.name, sizeof e.name, "%s", name);
    ob_start_read(&e, tt);
    return e;
}

int main(void)
{
    /* WBGapTest STARTUP SETUP's three icons */
    static const char *const ta[] = {"STARTPRI=-10", "STAMPSECS=1", 0};
    static const char *const tb[] = {"STARTPRI=10", "DONOTWAIT", "STAMPSECS=4", 0};
    static const char *const tc[] = {"STAMPSECS=1", 0};
    static const char *const td[] = {"startpri=10", "Wait=5", 0};
    static const char *const te[] = {"STARTPRI=500", "WAIT=-3", "DONOTWAITX", 0};
    static const char *const tf[] = {"(DONOTWAIT)", "STARTPRI", 0};
    ob_start s[6];
    s[0] = make("WBGapA", ta);
    s[1] = make("WBGapC", tc);
    s[2] = make("WBGapB", tb);
    CHECK("A: STARTPRI -10, waited for", s[0].pri == -10 && s[0].wait == OB_START_WAIT_MAX);
    CHECK("B: STARTPRI 10, DONOTWAIT", s[2].pri == 10 && s[2].wait == 0);
    CHECK("C: no STARTPRI is 0, waited for", s[1].pri == 0 && s[1].wait == OB_START_WAIT_MAX);
    qsort(s, 3, sizeof s[0], ob_start_cmp);
    CHECK("order B, C, A (WBGapTest S2)", !strcmp(s[0].name, "WBGapB") && !strcmp(s[1].name, "WBGapC") && !strcmp(s[2].name, "WBGapA"));
    s[0] = make("beta", td);
    s[1] = make("Alpha", td);
    s[2] = make("Zed", te);
    s[3] = make("plain", 0);
    s[4] = make("Commented", tf);
    CHECK("keys are case-blind; WAIT=5", s[0].pri == 10 && s[0].wait == 5);
    CHECK("STARTPRI kept to 127; WAIT below 0 is 0; DONOTWAITX is not DONOTWAIT", s[2].pri == 127 && s[2].wait == 0);
    CHECK("no icon tool types: 0, waited for", s[3].pri == 0 && s[3].wait == OB_START_WAIT_MAX);
    CHECK("(DONOTWAIT) is a comment; a bare STARTPRI is 0", s[4].pri == 0 && s[4].wait == OB_START_WAIT_MAX);
    qsort(s, 5, sizeof s[0], ob_start_cmp);
    CHECK("same STARTPRI: by name, case aside", !strcmp(s[0].name, "Zed") && !strcmp(s[1].name, "Alpha") && !strcmp(s[2].name, "beta") &&
          !strcmp(s[3].name, "Commented") && !strcmp(s[4].name, "plain"));
    printf(fails ? "FAIL ob_startup\n" : "ok   ob_startup: all cases pass\n");
    return fails != 0;
}
