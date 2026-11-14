/* ---- P7: Workbench prefs, as Prefs/Workbench's Use leaves them ----
 * ENV:Sys/Workbench.prefs is written as the editor's Use writes it, with RAM: among the hidden devices and the default
 * stack PREFSWB_STACK; IPrefs (3.2.3's Workbench) or GFOpenBench itself (it watches the file, 0.1.19) applies it. The
 * RAM Disk's icon must leave the desktop, and workbench.library's default stack (WBCTRLA_GetDefaultStackSize) must be
 * the prefs'. Then the old file is put back (or, if there was none, one with nothing hidden and the old stack, then
 * deleted) and the icon must come back. Both desktops: the icons are read through WBGapRoot.rexx.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <utility/tagitem.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "PREFSWB.h"

WBGAP_MODE(PREFSWB, wbt_prefswb, SCRIPT)
WBGAP_ROWMODE(P7, PREFSWB)

#define PREFSWB_STACK 65536UL
#define PREFSWB_FILE "ENV:Sys/Workbench.prefs"

static unsigned char was_buf[4096], new_buf[4096 + 256];

static int put_file(const char *path, const unsigned char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG w;
    if (!fh) return 0;
    w = Write(fh, (APTR)buf, n);
    Close(fh);
    return w == n;
}

static unsigned long stack_now(void)
{
    ULONG s = 0;
    if (!WorkbenchControl(NULL, WBCTRLA_GetDefaultStackSize, (ULONG)&s, TAG_END)) return 0;
    return s;
}

/* the disk icon of vol is (want 0) gone or (want 1) back within secs: what was last read (1 there, 0 not, -1 unread) */
static int icon_wait(const char *vol, int want, int secs, const char *when)
{
    int t, has = -1;
    for (t = 0; t < secs; t++) {
        has = wbt_desktop_has_disk("P7", vol, when);
        if (has < 0 || has == want) break;
        Delay(50);
    }
    return has;
}

int wbt_prefswb(int argc, char **argv)
{
    char vol[64], why[160];
    unsigned long old_stack, got = 0;
    long was_n, n;
    int had, before, gone = -1, back = -1, v, t, restored;
    BPTR l;
    (void)argc;
    (void)argv;
    vol[0] = 0;
    if ((l = Lock((CONST_STRPTR)"RAM:", ACCESS_READ))) {   /* "Ram Disk", or the name it was given */
        if (NameFromLock(l, (STRPTR)vol, sizeof vol)) { char *c = strchr(vol, ':'); if (c) *c = 0; }
        UnLock(l);
    }
    if (!vol[0]) { wbt_skip("P7", "RAM: has no volume name here"); return 0; }
    if ((before = wbt_desktop_has_disk("P7", vol, "before")) <= 0) {
        wbt_skip("P7", before < 0 ? "the desktop's icons not read (WBGapRoot.rexx: no WORKBENCH port, or no rx)"
                                  : "no RAM Disk icon on the desktop to hide");
        return 0;
    }
    was_n = wbt_read_all(PREFSWB_FILE, (char *)was_buf, sizeof was_buf);
    had = was_n >= 0;
    if (had && !wbgap_wbprefs_whole(was_n, (long)sizeof was_buf)) { wbt_skip("P7", "ENV:Sys/Workbench.prefs is bigger than this row reads; left as it is"); return 0; }
    old_stack = stack_now();
    printf("INFO P7 %s %s (%ld bytes), default stack %lu, \"%s\" on the desktop\n", PREFSWB_FILE, had ? "there" : "not there",
           had ? was_n : 0L, old_stack, vol);
    n = wbgap_wbprefs_make(had ? was_buf : NULL, had ? was_n : 0, PREFSWB_STACK, "RAM", new_buf, sizeof new_buf);
    if (!n || !put_file(PREFSWB_FILE, new_buf, n)) {   /* MODE_NEWFILE may have emptied it: the old one back */
        if (had) put_file(PREFSWB_FILE, was_buf, was_n);
        else DeleteFile((CONST_STRPTR)PREFSWB_FILE);
        wbt_fail("P7", "ENV:Sys/Workbench.prefs could not be written");
        return 10;
    }
    gone = icon_wait(vol, 0, 10, "after RAM hidden");
    if (gone >= 0) gone = !gone;
    for (t = 0; t < 10 && (got = stack_now()) != PREFSWB_STACK; t++) Delay(50);
    printf("INFO P7 after the change: icon %s, default stack %lu\n", gone > 0 ? "gone" : gone == 0 ? "still there" : "not read", got);
    /* the old prefs back, whatever was seen */
    if (had) restored = put_file(PREFSWB_FILE, was_buf, was_n);
    else {
        n = wbgap_wbprefs_make(NULL, 0, old_stack ? old_stack : 4096, NULL, new_buf, sizeof new_buf);
        restored = n && put_file(PREFSWB_FILE, new_buf, n);
    }
    if (!restored) printf("INFO P7 the old prefs could NOT be put back: ENV:Sys/Workbench.prefs still hides RAM\n");
    back = icon_wait(vol, 1, 10, "after the old prefs");
    if (!had) DeleteFile((CONST_STRPTR)PREFSWB_FILE);
    printf("INFO P7 after the old prefs: icon %s, default stack %lu\n", back > 0 ? "back" : back == 0 ? "still gone" : "not read",
           stack_now());
    v = wbgap_wbprefs_judge(gone, got, PREFSWB_STACK, restored ? back : 0, why, sizeof why);
    wbt_say("P7", v, why);
    return v == WBGAP_FAIL ? 10 : 0;
}
