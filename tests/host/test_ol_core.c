/* ol_core's host test: the launcher's rules (Workbench build plan row 6; WBGapTest rows S2, S2d, S3, I11, I12, A10).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ol_core.h"

static int fails, checks;
#define CHECK(what, cond) do { checks++; if (!(cond)) { printf("FAIL %s\n", what); fails++; } } while (0)

static ol_start make(const char *name, const char *const *tt, unsigned long added)
{
    ol_start e;
    memset(&e, 0, sizeof e);
    snprintf(e.name, sizeof e.name, "%s", name);
    ol_start_read(&e, tt);
    e.added = added;
    return e;
}

static int order_is(const ol_start *s, int n, const char *const *want)
{
    int i;
    for (i = 0; i < n; i++)
        if (strcmp(s[i].name, want[i])) return 0;
    return 1;
}

static void test_startup(void)
{
    /* WBGapTest STARTUP SETUP's three icons (S2, S3) */
    static const char *const ta[] = {"STARTPRI=-10", "STAMPSECS=1", 0};
    static const char *const tb[] = {"STARTPRI=10", "DONOTWAIT", "STAMPSECS=4", 0};
    static const char *const tc[] = {"STAMPSECS=1", 0};
    static const char *const td[] = {"startpri=10", "Wait=5", 0};
    static const char *const te[] = {"STARTPRI=500", "WAIT=-3", "DONOTWAITX", 0};
    static const char *const tf[] = {"(DONOTWAIT)", "STARTPRI", 0};
    static const char *const tg[] = {"STARTPRI=-999", "WAIT=99999", 0};
    static const char *const s2[] = {"WBGapB", "WBGapC", "WBGapA"};
    static const char *const s2d[] = {"Old", "Middle", "New", "Unknown"};
    static const char *const tie[] = {"Zed", "alpha", "Beta", "Commented", "plain"};
    ol_start s[6];

    s[0] = make("WBGapA", ta, 300);
    s[1] = make("WBGapC", tc, 200);
    s[2] = make("WBGapB", tb, 100);
    CHECK("S3: A, STARTPRI -10, waited for", s[0].pri == -10 && s[0].wait == OL_WAIT_MAX);
    CHECK("S3: B, STARTPRI 10, DONOTWAIT", s[2].pri == 10 && s[2].wait == 0);
    CHECK("S3: C, no STARTPRI is 0, waited for", s[1].pri == 0 && s[1].wait == OL_WAIT_MAX);
    qsort(s, 3, sizeof s[0], ol_start_cmp);
    CHECK("S2: STARTPRI first, whatever the dates: B, C, A", order_is(s, 3, s2));

    /* S2d: no STARTPRI anywhere, so the date added decides; a date not known goes last */
    s[0] = make("New", 0, 3000);
    s[1] = make("Unknown", 0, 0);
    s[2] = make("Old", 0, 1000);
    s[3] = make("Middle", 0, 2000);
    qsort(s, 4, sizeof s[0], ol_start_cmp);
    CHECK("S2d: by date added, oldest first, an unknown date last", order_is(s, 4, s2d));

    /* same STARTPRI and the same date: by name, case aside */
    s[0] = make("plain", 0, 0);
    s[1] = make("Beta", td, 7);
    s[2] = make("Zed", te, 7);
    s[3] = make("alpha", td, 7);
    s[4] = make("Commented", tf, 0);
    CHECK("tool type keys are case-blind; WAIT=5", s[1].pri == 10 && s[1].wait == 5);
    CHECK("STARTPRI kept to 127; WAIT below 0 is 0; DONOTWAITX is not DONOTWAIT", s[2].pri == 127 && s[2].wait == 0);
    CHECK("no tool types: 0, waited for", s[0].pri == 0 && s[0].wait == OL_WAIT_MAX);
    CHECK("(DONOTWAIT) is switched off; a bare STARTPRI is 0", s[4].pri == 0 && s[4].wait == OL_WAIT_MAX);
    s[5] = make("Low", tg, 0);
    CHECK("STARTPRI kept to -128; WAIT kept to an hour", s[5].pri == -128 && s[5].wait == 3600);
    qsort(s, 5, sizeof s[0], ol_start_cmp);
    CHECK("Zed (127) first; then 10s by name at the same date; then 0s by name", order_is(s, 5, tie));

    CHECK("DateStamp in seconds", ol_stamp_secs(1, 2, 150) == 86400UL + 120UL + 3UL);
    CHECK("a bad DateStamp is not known", ol_stamp_secs(-1, 0, 0) == 0);
    CHECK("date added: the older of program and icon", ol_added(500, 400) == 400 && ol_added(400, 500) == 400);
    CHECK("date added: whichever is there", ol_added(0, 400) == 400 && ol_added(500, 0) == 500 && ol_added(0, 0) == 0);
}

static void test_opts(void)
{
    static const char *const big[] = {"STACK=65536", "TOOLPRI=3", 0};
    static const char *const small[] = {"STACK=1000", "TOOLPRI=-5", 0};
    static const char *const huge[] = {"stack=99999999", "toolpri=300", 0};
    static const char *const off[] = {"(STACK=65536)", "(TOOLPRI=4)", "TOOLPRIX=2", 0};
    static const char *const neg[] = {"TOOLPRI=-200", "STACK=-5", 0};
    ol_opts o;
    ol_opts_read(&o, big, 4096, 8192);
    CHECK("I12a: STACK above the icon's and the default wins", o.stack == 65536);
    CHECK("I12b: TOOLPRI=3", o.pri == 3);
    ol_opts_read(&o, small, 20000, 8192);
    CHECK("I12a: a smaller STACK never lowers the icon's", o.stack == 20000);
    CHECK("I12b: TOOLPRI=-5", o.pri == -5);
    ol_opts_read(&o, small, 0, 8192);
    CHECK("I12a: Workbench's default at the least", o.stack == 8192);
    ol_opts_read(&o, huge, 4096, 8192);
    CHECK("STACK kept to 1 MB, keys case-blind", o.stack == OL_STACK_MAX);
    CHECK("TOOLPRI kept to 127", o.pri == 127);
    ol_opts_read(&o, off, 4096, 8192);
    CHECK("tool types in brackets are off", o.stack == 8192 && o.pri == 0);
    ol_opts_read(&o, neg, 4096, 8192);
    CHECK("TOOLPRI kept to -128; a STACK below 0 is ignored", o.pri == -128 && o.stack == 8192);
    ol_opts_read(&o, 0, 99999999L, 8192);
    CHECK("the icon's own stack kept to 1 MB too", o.stack == OL_STACK_MAX);
    ol_opts_read(&o, 0, 30000, 8192);
    CHECK("no tool types: the icon's stack, priority 0", o.stack == 30000 && o.pri == 0);
}

static void test_shift(void)
{
    int tool, args[8], n;
    {   /* a tool selected first, two projects after, the last double-clicked (the usual way) */
        static const int sel[] = {OL_K_TOOL, OL_K_PROJECT, OL_K_PROJECT};
        n = ol_shift_args(sel, 3, 2, &tool, args);
        CHECK("I11: the tool selected runs with every project, in order", tool == 0 && n == 2 && args[0] == 1 && args[1] == 2);
    }
    {   /* the tool double-clicked last */
        static const int sel[] = {OL_K_PROJECT, OL_K_DRAWER, OL_K_PROJECT, OL_K_TOOL};
        n = ol_shift_args(sel, 4, 3, &tool, args);
        CHECK("I11: the tool clicked gets every other icon, a drawer too", tool == 3 && n == 3 && args[0] == 0 && args[1] == 1 && args[2] == 2);
    }
    {   /* two tools: the one clicked runs, the other is an argument */
        static const int sel[] = {OL_K_TOOL, OL_K_PROJECT, OL_K_TOOL};
        n = ol_shift_args(sel, 3, 2, &tool, args);
        CHECK("I11: of two tools, the clicked one runs", tool == 2 && n == 2 && args[0] == 0 && args[1] == 1);
    }
    {   /* projects only: the clicked one's default tool, with all of them */
        static const int sel[] = {OL_K_PROJECT, OL_K_PROJECT, OL_K_PROJECT};
        n = ol_shift_args(sel, 3, 1, &tool, args);
        CHECK("I11: projects alone go to the clicked one's default tool", tool == -1 && n == 3 && args[0] == 0 && args[1] == 1 && args[2] == 2);
    }
    {   /* one icon: an ordinary double-click */
        static const int sel[] = {OL_K_TOOL};
        n = ol_shift_args(sel, 1, 0, &tool, args);
        CHECK("I11: one tool alone, no arguments", tool == 0 && n == 0);
    }
    {
        static const int sel[] = {OL_K_PROJECT, OL_K_DRAWER};
        CHECK("a drawer clicked opens as itself", ol_shift_args(sel, 2, 1, &tool, args) == -1);
        CHECK("a click outside the selection is nothing", ol_shift_args(sel, 2, 5, &tool, args) == -1);
    }
}

static void test_switches(void)
{
    char buf[80];
    unsigned long f;
    CHECK("A10: no switches, no words", ol_switch_words(0, buf, sizeof buf) == 0 && !buf[0]);
    ol_switch_words(OL_SW_DELAY | OL_SW_CLEANUP, buf, sizeof buf);
    CHECK("A10: DELAY CLEANUP", !strcmp(buf, "DELAY CLEANUP"));
    ol_switch_words(OL_SW_ALL, buf, sizeof buf);
    CHECK("A10: every switch, in LoadWB's order", !strcmp(buf, "-DEBUG DELAY CLEANUP NEWPATH SKIPWBSTARTUP SIMPLEGELS"));
    CHECK("A10: the words read back", ol_switch_parse(buf) == OL_SW_ALL);
    f = ol_switch_parse("  skip\tdelay LEGACY nonsense -debug\n");
    CHECK("A10: SKIP is SKIPWBSTARTUP; LEGACY and others give none", f == (OL_SW_SKIPSTARTUP | OL_SW_DELAY | OL_SW_DEBUG));
    CHECK("A10: a prefix is not the word", ol_switch_parse("DEL CLEAN SKIPWB") == 0);
    CHECK("A10: d0's bits as LoadWB 3.2.3 gives them", OL_SW_DEBUG == 1 && OL_SW_DELAY == 2 && OL_SW_CLEANUP == 4 &&
          OL_SW_NEWPATH == 8 && OL_SW_SKIPSTARTUP == 0x10 && OL_SW_SIMPLEGELS == 0x20);
    ol_switch_words(OL_SW_ALL, buf, 12);
    CHECK("A10: a short buffer is cut, still ended", strlen(buf) == 11);
    CHECK("A10: a buffer of nothing", ol_switch_words(OL_SW_ALL, buf, 0) == 0);
}

int main(void)
{
    test_startup();
    test_opts();
    test_shift();
    test_switches();
    if (fails) printf("FAIL ol_core: %d of %d\n", fails, checks);
    else printf("ok   ol_core: all %d cases pass\n", checks);
    return fails != 0;
}
