/* ---- INFOSAVE: rows N2-N5, the Information window's Save, driven as a hand does ----
 * RAM:WBGapN/n's Information window (WBInfo) driven with DragTest: Deletable off, a comment, a default tool, tool
 * types a=1 edited to a=3, d=4 deleted, b=2 added, Save; then the file's protection and comment and its icon are
 * read back. GFOpenBench only (it finds the gadgets by desktop/ob_info_ids.h); SKIP on 3.2.3.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <workbench/workbench.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/icon.h>
#include <proto/intuition.h>
#include <proto/wb.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "INFOSAVE.h"
#include "../../../desktop/ob_info_ids.h"   /* the Information window's gadget ids */

WBGAP_MODE(INFOSAVE, wbt_infosave, SCRIPT)
WBGAP_ROWMODE(N2, INFOSAVE)
WBGAP_ROWMODE(N3, INFOSAVE)
WBGAP_ROWMODE(N4, INFOSAVE)
WBGAP_ROWMODE(N5, INFOSAVE)

static int note_file(const char *path, const char *text)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    if (!fh) return 0;
    Write(fh, (APTR)text, (LONG)strlen(text));
    Close(fh);
    return 1;
}

/* the box (window-relative) of every gadget of w with this id, joined: a GadTools listview is more than one */
static int gad_box(struct Window *w, UWORD id, LONG *x, LONG *y, LONG *gw, LONG *gh)
{
    struct Gadget *g;
    LONG x0 = 0x7fff, y0 = 0x7fff, x1 = -1, y1 = -1;
    ULONG lock = LockIBase(0);
    for (g = w->FirstGadget; g; g = g->NextGadget) {
        LONG l, t;
        if ((g->GadgetType & GTYP_SYSGADGET) || g->GadgetID != id || g->Width <= 0 || g->Height <= 0) continue;
        l = (g->Flags & GFLG_RELRIGHT) ? w->Width - 1 + g->LeftEdge : g->LeftEdge;
        t = (g->Flags & GFLG_RELBOTTOM) ? w->Height - 1 + g->TopEdge : g->TopEdge;
        if (l < x0) x0 = l;
        if (t < y0) y0 = t;
        if (l + g->Width > x1) x1 = l + g->Width;
        if (t + g->Height > y1) y1 = t + g->Height;
    }
    UnlockIBase(lock);
    if (x1 < 0) return 0;
    *x = x0, *y = y0, *gw = x1 - x0, *gh = y1 - y0;
    return 1;
}

/* a click on gadget id of w: its middle, or (row >= 0, a listview) the middle of that row, near its left */
static int click_gad(struct Window *w, UWORD id, int row)
{
    LONG x, y, gw, gh, fh = w->WScreen->Font ? w->WScreen->Font->ta_YSize : 8;
    char cmd[300];
    if (!gad_box(w, id, &x, &y, &gw, &gh)) return 0;
    if (row >= 0) x += 24, y += 2 + row * fh + fh / 2;
    else x += gw / 2, y += gh / 2;
    snprintf(cmd, sizeof cmd, "%s CLICK %ld %ld", wbt_dragtest(), (long)(w->LeftEdge + x), (long)(w->TopEdge + y));
    wbt_run(cmd);
    Delay(20);
    return 1;
}

static void key_q(int code, const char *qual)
{
    char cmd[300];
    snprintf(cmd, sizeof cmd, "%s KEY 0x%x QUAL %s", wbt_dragtest(), code, qual);
    wbt_run(cmd);
    Delay(15);
}

/* text typed on a US keyboard (wbgap_rawkey: a-z, A-Z with Shift, 0-9 - = . / space); 0 at a character it has no key for */
static int type_text(const char *t)
{
    for (; *t; t++) {
        int shift, code = wbgap_rawkey((unsigned char)*t, &shift);
        if (code < 0) return 0;
        if (shift) key_q(code, "LSHIFT"); else wbt_key(code);
    }
    return 1;
}

/* the string gadget id of w emptied (Right-Amiga X) and t typed into it; Return after it when ret */
static int set_string(struct Window *w, UWORD id, const char *t, int ret)
{
    if (!click_gad(w, id, -1)) return 0;
    key_q(0x32, "RAMIGA");
    if (!type_text(t)) return 0;
    if (ret) wbt_key(0x44);
    return 1;
}

static int infosave_row(void)
{
    static const char *const rows[] = { "N2", "N3", "N4", "N5" };
    static char *tt[] = { (char *)"a=1", (char *)"d=4", NULL };
    struct FileInfoBlock *fib;
    struct DiskObject *dob;
    struct Screen *wb;
    struct Window *iw = NULL;
    char why[120];
    BPTR dir, l;
    int t, i, done;
    if (!wbt_is_gfos()) {
        for (i = 0; i < 4; i++) wbt_skip(rows[i], "drives GFOpenBench's Information window by its gadget ids; 3.2.3's is checked by hand");
        return 0;
    }
    /* RAM:WBGapN/n: a project, every protection bit as a new file has it, no comment, no default tool,
     * tool types a=1 and d=4 */
    if ((l = CreateDir((CONST_STRPTR)"RAM:WBGapN"))) UnLock(l);
    /* a rerun: the last run's Save left it not deletable, so clear that before it is written again */
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);
    note_file("RAM:WBGapN/n", "WBGapTest INFOSAVE\n");
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);
    SetComment((CONST_STRPTR)"RAM:WBGapN/n", (CONST_STRPTR)"");
    if (!wbt_put_icon("RAM:WBGapN/n", WBPROJECT, tt)) {
        for (i = 0; i < 4; i++) wbt_fail(rows[i], "RAM:WBGapN/n.info could not be written");
        return 10;
    }
    if ((dob = GetDiskObject((CONST_STRPTR)"RAM:WBGapN/n"))) {   /* a default project icon may name a tool: none here */
        dob->do_DefaultTool = (STRPTR)"";
        PutDiskObject((CONST_STRPTR)"RAM:WBGapN/n", dob);
        FreeDiskObject(dob);
    }
    dir = Lock((CONST_STRPTR)"RAM:WBGapN", ACCESS_READ);
    wb = LockPubScreen((CONST_STRPTR)"Workbench");
    if (dir && wb && WBInfo(dir, (CONST_STRPTR)"n", wb))
        for (t = 0; t < 30 && !(iw = wbt_find_window("Information: n")); t++) Delay(10);
    if (wb) UnlockPubScreen(NULL, wb);
    if (dir) UnLock(dir);
    if (!iw) {
        for (i = 0; i < 4; i++) wbt_fail(rows[i], "WBInfo opened no Information window for RAM:WBGapN/n");
        return 10;
    }
    Delay(50);   /* drawn */
    printf("INFO INFOSAVE Information window %ldx%ld at %ld,%ld\n", (long)iw->Width, (long)iw->Height, (long)iw->LeftEdge, (long)iw->TopEdge);
    /* N2: Deletable off. N3: a comment. N5: a default tool. N4: a=1 made a=3, d=4 deleted, b=2 added. */
    done = click_gad(iw, G_PD, -1)
        && set_string(iw, G_COMMENT, "gapnote", 0)
        && set_string(iw, G_TOOL, "dir", 0)
        && click_gad(iw, G_TTLIST, 0) && (key_q(0x32, "RAMIGA"), type_text("a=3")) && (wbt_key(0x44), 1)
        && click_gad(iw, G_TTLIST, 1) && click_gad(iw, G_TTDEL, -1)
        && click_gad(iw, G_TTNEW, -1) && type_text("b=2") && (wbt_key(0x44), 1)
        && click_gad(iw, G_SAVE, -1);
    for (t = 0; t < 30 && wbt_find_window("Information: n"); t++) Delay(10);
    if ((iw = wbt_find_window("Information: n"))) {   /* not saved (a requester?): closed unsaved */
        printf("INFO INFOSAVE the window is still open after Save%s\n", done ? "" : " (a gadget wasn't found)");
        if (iw->UserPort && iw->UserPort->mp_SigTask) Signal((struct Task *)iw->UserPort->mp_SigTask, SIGBREAKF_CTRL_C);
        for (i = 0; i < 4; i++) wbt_fail(rows[i], "Save didn't close the Information window");
        return 10;
    }
    l = 0;
    if ((fib = AllocDosObject(DOS_FIB, NULL)) && (l = Lock((CONST_STRPTR)"RAM:WBGapN/n", ACCESS_READ)) && Examine(l, fib)) {
        wbt_say("N2", wbgap_infosave_prot(fib->fib_Protection, why, sizeof why), why);
        wbt_say("N3", wbgap_infosave_same("comment", (const char *)fib->fib_Comment, "gapnote", why, sizeof why), why);
    } else {
        wbt_fail("N2", "RAM:WBGapN/n can't be examined");
        wbt_fail("N3", "RAM:WBGapN/n can't be examined");
    }
    if (l) UnLock(l);
    if (fib) FreeDosObject(DOS_FIB, fib);
    if ((dob = GetDiskObject((CONST_STRPTR)"RAM:WBGapN/n"))) {
        char got[160];
        wbgap_join_tooltypes((const char *const *)dob->do_ToolTypes, got, sizeof got);
        wbt_say("N4", wbgap_infosave_same("tool types", got, "a=3,b=2", why, sizeof why), why);
        wbt_say("N5", wbgap_infosave_same("default tool", dob->do_DefaultTool ? (const char *)dob->do_DefaultTool : NULL, "dir",
                                         why, sizeof why), why);
        FreeDiskObject(dob);
    } else {
        wbt_fail("N4", "RAM:WBGapN/n.info can't be read");
        wbt_fail("N5", "RAM:WBGapN/n.info can't be read");
    }
    return wbt_fails() ? 10 : 0;
}

/* RAM:WBGapN is gone after every run, whichever way it ended (Lyta, #150) */
int wbt_infosave(int argc, char **argv)
{
    int rc;
    (void)argc;
    (void)argv;
    rc = infosave_row();
    SetProtection((CONST_STRPTR)"RAM:WBGapN/n", 0);   /* the Save left it not deletable */
    DeleteFile((CONST_STRPTR)"RAM:WBGapN/n.info");
    DeleteFile((CONST_STRPTR)"RAM:WBGapN/n");
    DeleteFile((CONST_STRPTR)"RAM:WBGapN");
    return rc;
}

