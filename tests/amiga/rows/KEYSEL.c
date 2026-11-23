/* ---- I16: keyboard selection in a drawer (cursor keys, Shift adds, Tab and Shift-Tab cycle) ----
 * RAM:WBGapK opened at the top-left, 400x300, with a (snapshotted at 20,10), b at 120,10 and c at 220,10, as SELECT's
 * drawer. Each step of KEYSEL_judge.c's list is a DragTest click on an icon (which also makes the window the active
 * one, so the keys go to it) or a DragTest KEY, and then the window's selected icons are read through the WORKBENCH
 * ARexx port (window.icons.selected) and judged as SELECT's are (wbgap_select_judge). The row passes when every step
 * leaves what it should. Return (open) isn't pressed: a project with no tool would leave a requester up.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <intuition/intuition.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "SELECT.h"
#include "KEYSEL.h"

WBGAP_MODE(KEYSEL, wbt_keysel, NEWROWS)
WBGAP_ROWMODE(I16, KEYSEL)

#define KS_DIR "RAM:WBGapK"
#define KS_TITLE "WBGapK"
#define KS_REXX "RAM:wbgap-ks.rexx"
#define KS_OUT "RAM:wbgap-ks"

static const char ks_rexx[] =
    "/* WBGapTest KEYSEL: the selected icons of window win, a name a line, into out */\n"
    "OPTIONS RESULTS\n"
    "PARSE ARG out win\n"
    "win = STRIP(win)\n"
    "IF ~SHOW('P', 'WORKBENCH') THEN EXIT 5\n"
    "ADDRESS WORKBENCH\n"
    "GETATTR 'window.icons.selected.count' NAME win\n"
    "IF RC ~= 0 THEN EXIT 5\n"
    "n = RESULT\n"
    "IF n = '' | n = 'RESULT' THEN n = 0\n"
    "IF ~OPEN('o', out, 'W') THEN EXIT 5\n"
    "DO i = 0 TO n - 1\n"
    "  GETATTR 'window.icons.selected.'i'.name' NAME win\n"
    "  CALL WRITELN('o', RESULT)\n"
    "END\n"
    "CALL CLOSE('o')\n"
    "EXIT 0\n";

static int ks_rexx_up(void)
{
    BPTR l = Lock((CONST_STRPTR)"SYS:Rexxc/rx", ACCESS_READ);
    int up;
    if (!l) return 0;
    UnLock(l);
    Forbid();
    up = FindPort((CONST_STRPTR)"REXX") != NULL;
    Permit();
    if (!up) {
        wbt_run("Run >NIL: SYS:System/RexxMast >NIL:");
        Delay(50);
    }
    return wbt_write_file(KS_REXX, ks_rexx);
}

static const char *ks_read(char *list, long cap)
{
    char cmd[200];
    long n;
    DeleteFile((CONST_STRPTR)KS_OUT);
    if (!wbgap_rx_line(cmd, sizeof cmd, KS_REXX, KS_OUT " " KS_DIR)) return NULL;
    wbt_run(cmd);
    n = wbt_read_all(KS_OUT, list, cap);
    DeleteFile((CONST_STRPTR)KS_OUT);
    if (n < 0) { printf("INFO I16 the selected icons not read (no WORKBENCH port, or GETATTR refused)\n"); return NULL; }
    return list;
}

static void ks_clean(void)
{
    static const char *const files[] = {KS_DIR "/a.info", KS_DIR "/a", KS_DIR "/b.info", KS_DIR "/b",
                                        KS_DIR "/c.info", KS_DIR "/c", KS_DIR ".info", KS_DIR};
    unsigned i, pass;
    int t;
    CloseWorkbenchObjectA((STRPTR)KS_DIR, NULL);
    for (t = 0; t < 40 && wbt_find_window(KS_TITLE); t++) Delay(5);
    for (pass = 0; pass < 2; pass++) {   /* twice: a desktop may write the drawer's .info after the close */
        if (pass) Delay(25);
        for (i = 0; i < sizeof files / sizeof files[0]; i++) DeleteFile((CONST_STRPTR)files[i]);
    }
    DeleteFile((CONST_STRPTR)KS_REXX);
    DeleteFile((CONST_STRPTR)KS_OUT);
}

int wbt_keysel(int argc, char **argv)
{
    static char list[2048];
    struct Window *w;
    long box[3][4], at[3][2];
    char why[300], cmd[320];
    int how, i, v = WBGAP_PASS;
    (void)argc, (void)argv;
    ks_clean();
    if (!ks_rexx_up()) { wbt_skip("I16", "no rx (SYS:Rexxc/rx), or " KS_REXX " not written"); return 0; }
    wbt_drawer(KS_DIR, "a");
    if (!wbt_open_drawer(KS_DIR, KS_TITLE, 10, &how) || !(w = wbt_place(KS_TITLE, NULL, 400, 300))) {
        wbt_fail("I16", KS_DIR "'s window didn't open, or couldn't be sized and put at the top-left");
        goto out;
    }
    Delay(50);
    if (!wbt_make_icon(w, KS_DIR, "b", WBPROJECT, 120, 10, box[1]) || !(w = wbt_find_window(KS_TITLE)) ||
        !wbt_make_icon(w, KS_DIR, "c", WBPROJECT, 220, 10, box[2]) || !(w = wbt_find_window(KS_TITLE))) {
        wbt_fail("I16", "b or c never showed in " KS_DIR "'s window (5 s after UpdateWorkbench)");
        goto out;
    }
    /* a's box: b's, 100 pixels to the left (a is snapshotted at 20,10, b made at 120,10) */
    box[0][0] = box[1][0] - 100, box[0][1] = box[1][1], box[0][2] = box[1][2], box[0][3] = box[1][3];
    for (i = 0; i < 3; i++) wbgap_grab_point(box[i], at[i]);
    for (i = 0; i < wbgap_keysel_count() && v == WBGAP_PASS; i++) {
        const struct wbgap_keysel_step *s = wbgap_keysel_step(i);
        const char *got;
        if (!(w = wbt_find_window(KS_TITLE))) { v = WBGAP_FAIL; snprintf(why, sizeof why, KS_DIR "'s window went"); break; }
        if (s->click) {
            int k = s->click - 'a';
            snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), w->LeftEdge + w->BorderLeft + at[k][0],
                     w->TopEdge + w->BorderTop + at[k][1]);
        } else snprintf(cmd, sizeof cmd, "%s KEY 0x%x%s", wbt_dragtest(), s->key, s->shift ? " QUAL LSHIFT" : "");
        wbt_run(cmd);
        Delay(75);                                /* a second and a half: no double-click, the desktop caught up */
        got = ks_read(list, sizeof list - 1);
        if (got) list[sizeof list - 1] = 0, printf("INFO I16 after %s: \"%s\" selected\n", s->what, got);
        v = wbgap_select_judge(got, s->want, s->what, why, sizeof why);
    }
    if (v == WBGAP_PASS) snprintf(why, sizeof why, "every step left what it should (%d steps)", wbgap_keysel_count());
    wbt_say("I16", v, why);
    if ((w = wbt_find_window(KS_TITLE))) {   /* nothing left selected */
        snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), (long)(w->LeftEdge + w->Width - 40), (long)(w->TopEdge + w->Height - 40));
        wbt_run(cmd);
        Delay(75);
    }
out:
    ks_clean();
    return wbt_fails() ? 5 : 0;
}
