/* ob_menustate's host test (GF-127): each kind of item enabled only on the desktop states where it can act.
 *   test_ob_menustate
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_menustate.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static const int needs[] = { OB_MS_SEL, OB_MS_SEL_FILE, OB_MS_DISK, OB_MS_LOUT, OB_MS_LOUT_TEMP, OB_MS_RECOVER,
                             OB_MS_PUTBACK, OB_MS_SHOWN, OB_MS_FILES, OB_MS_ANY_DISK };
#define NNEEDS (int)(sizeof needs / sizeof needs[0])

/* The pointer over the backdrop, nothing selected, No icons on: only the desktop's own items */
static void nothing(void)
{
    ob_ms_facts f;
    int i;
    memset(&f, 0, sizeof f);
    for (i = 0; i < NNEEDS; i++) CHECK(!ob_ms_enabled(&f, needs[i]), "need %d enabled with nothing there", needs[i]);
    CHECK(ob_ms_enabled(&f, OB_MS_ALWAYS), "the desktop's own items always enabled");
    CHECK(ob_ms_enabled(NULL, OB_MS_DISK), "no facts: as before, enabled");
    CHECK(ob_ms_enabled(&f, 999), "an unknown need: enabled");
}

/* A disk selected (Work:) */
static void disk(void)
{
    ob_ms_facts f;
    memset(&f, 0, sizeof f);
    f.sel = f.sel_file = f.disks = f.recover = 1;
    f.shown = f.files = f.all_disks = 3;
    CHECK(ob_ms_enabled(&f, OB_MS_SEL) && ob_ms_enabled(&f, OB_MS_SEL_FILE), "Open, Information");
    CHECK(ob_ms_enabled(&f, OB_MS_DISK), "Eject, Format");
    CHECK(ob_ms_enabled(&f, OB_MS_RECOVER), "Recover deleted");
    CHECK(!ob_ms_enabled(&f, OB_MS_LOUT), "Put Away, Delete: a disk isn't left out");
    CHECK(!ob_ms_enabled(&f, OB_MS_LOUT_TEMP), "Leave Out: a disk can't be");
    CHECK(!ob_ms_enabled(&f, OB_MS_PUTBACK), "Put back: no Recover window");
}

/* An icon left out for this run, then for good */
static void left_out(void)
{
    ob_ms_facts f;
    memset(&f, 0, sizeof f);
    f.sel = f.sel_file = f.louts = f.louts_temp = 1;
    f.shown = f.files = 2;
    CHECK(ob_ms_enabled(&f, OB_MS_LOUT) && ob_ms_enabled(&f, OB_MS_LOUT_TEMP), "Put Away, Delete, Leave Out");
    CHECK(!ob_ms_enabled(&f, OB_MS_DISK), "Eject, Format: not a disk");
    CHECK(!ob_ms_enabled(&f, OB_MS_ANY_DISK), "Open volume: no disk on the desktop");
    f.louts_temp = 0;
    CHECK(!ob_ms_enabled(&f, OB_MS_LOUT_TEMP), "Leave Out: left out for good already");
    CHECK(ob_ms_enabled(&f, OB_MS_LOUT), "Put Away still");
}

/* A program's AppIcon: Open only (it has no file) */
static void appicon(void)
{
    ob_ms_facts f;
    memset(&f, 0, sizeof f);
    f.sel = 1;
    f.shown = 1;
    CHECK(ob_ms_enabled(&f, OB_MS_SEL), "Open: the program's AppMessage");
    CHECK(!ob_ms_enabled(&f, OB_MS_SEL_FILE), "Information, Rename, Snapshot: no file");
    CHECK(!ob_ms_enabled(&f, OB_MS_FILES), "Snapshot > All: nothing to keep");
}

/* The Recover deleted window open, nothing selected */
static void recover_window(void)
{
    ob_ms_facts f;
    memset(&f, 0, sizeof f);
    f.recover_open = 1;
    CHECK(ob_ms_enabled(&f, OB_MS_PUTBACK), "Put back");
    CHECK(!ob_ms_enabled(&f, OB_MS_RECOVER), "Recover deleted: nothing selected");
}

int main(void)
{
    nothing();
    disk();
    left_out();
    appicon();
    recover_window();
    printf("test_ob_menustate: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
