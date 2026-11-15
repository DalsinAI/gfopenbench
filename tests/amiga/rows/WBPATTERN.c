/* ---- P1: the root picture Prefs/WBPattern names (ENV:Sys/WBPattern.prefs) ----
 * A picture of one colour (WBGAP_PTRN_RGB, an ILBM of 32 x 24, RAM:WBGapP1.ilbm) is named for the root in a
 * WBPattern.prefs made from the file as it is (its drawers' and screen's PTRN chunks kept) and written to
 * ENV:Sys/WBPattern.prefs, which GFOpenBench watches (0.1.19) and IPrefs follows on 3.2.3. A grid of the Workbench
 * screen's points that no window covers (the desktop's own backdrop windows aside) is read before, after (up to 15 s,
 * for the picture's process to decode it) and with the old file put back (or deleted, if there was none): the colour
 * must cover at least half the points, and go again. Icons and labels over the desktop are why it is half, not all.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include <exec/types.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/intuition.h>

#include "wbgap_rows.h"
#include "wbgap_test.h"
#include "WBPATTERN.h"

WBGAP_MODE(WBPATTERN, wbt_wbpattern, SCRIPT)
WBGAP_ROWMODE(P1, WBPATTERN)

#define PTRN_FILE "ENV:Sys/WBPattern.prefs"
#define PTRN_PIC "RAM:WBGapP1.ilbm"
#define GRID_X 32
#define GRID_Y 20
#define MAXW 48

static unsigned char was_buf[4096], new_buf[4096 + 512], pic_buf[512];

static int put_file(const char *path, const unsigned char *buf, long n)
{
    BPTR fh = Open((CONST_STRPTR)path, MODE_NEWFILE);
    LONG w;
    if (!fh) return 0;
    w = Write(fh, (APTR)buf, n);
    Close(fh);
    return w == n;
}

/* the grid's points no window but a backdrop covers, below the screen's title bar */
static int points(struct Screen *s, short *px, short *py)
{
    struct Window *w[MAXW];
    int nw = wbt_windows_now(s, w, MAXW), n = 0, i, j, k, top = s->BarHeight + 2;
    for (j = 0; j < GRID_Y; j++)
        for (i = 0; i < GRID_X; i++) {
            int x = (int)((2L * i + 1) * s->Width / (2 * GRID_X)), y = top + (int)((2L * j + 1) * (s->Height - top) / (2 * GRID_Y));
            for (k = 0; k < nw; k++)
                if (!(w[k]->Flags & WFLG_BACKDROP) && x >= w[k]->LeftEdge && x < w[k]->LeftEdge + w[k]->Width &&
                    y >= w[k]->TopEdge && y < w[k]->TopEdge + w[k]->Height) break;
            if (k == nw) px[n] = (short)x, py[n] = (short)y, n++;
        }
    return n;
}

static int count(struct Screen *s, const short *px, const short *py, int n)
{
    int i, c = 0;
    for (i = 0; i < n; i++) c += wbgap_rgb_near(wbt_screen_rgb(s, px[i], py[i]), WBGAP_PTRN_RGB, WBGAP_PTRN_TOL);
    return c;
}

/* the most points of the colour seen within secs, stopping once want (1: at least half; 0: under a fifth) holds */
static int watch(struct Screen *s, const short *px, const short *py, int n, int want, int secs)
{
    int t, c = 0, best = want ? 0 : n;
    for (t = 0; t <= secs * 2; t++) {
        c = count(s, px, py, n);
        if (want ? c > best : c < best) best = c;
        if (want ? c * 2 >= n : c * 5 < n) break;
        Delay(25);
    }
    return best;
}

int wbt_wbpattern(int argc, char **argv)
{
    static short px[GRID_X * GRID_Y], py[GRID_X * GRID_Y];
    struct Screen *s;
    char why[160], name[300];
    long was_n, n, pn;
    int had, np, before, after = 0, back = 0, put_back, v;
    (void)argc;
    (void)argv;
    if (!(s = LockPubScreen((CONST_STRPTR)"Workbench"))) { wbt_skip("P1", "no Workbench screen"); return 0; }
    np = points(s, px, py);
    before = count(s, px, py, np);
    was_n = wbt_read_all(PTRN_FILE, (char *)was_buf, sizeof was_buf);
    had = was_n >= 0;
    if (had && was_n >= (long)sizeof was_buf - 1) {
        UnlockPubScreen(NULL, s);
        wbt_skip("P1", "ENV:Sys/WBPattern.prefs is bigger than this row reads; left as it is");
        return 0;
    }
    printf("INFO P1 %s %s (%ld bytes)%s%s; %d desktop points uncovered, %d the picture's colour\n", PTRN_FILE,
           had ? "there" : "not there", had ? was_n : 0L, had && wbgap_ptrn_root(was_buf, was_n, name, sizeof name) ? ", root " : "",
           had && wbgap_ptrn_root(was_buf, was_n, name, sizeof name) ? name : "", np, before);
    if (np < 20 || before * 5 >= np) {
        UnlockPubScreen(NULL, s);
        v = wbgap_ptrn_judge(np, before, 0, 0, 1, why, sizeof why);
        wbt_say("P1", v, why);
        return 0;
    }
    pn = wbgap_ilbm_solid(32, 24, WBGAP_PTRN_RGB >> 16, WBGAP_PTRN_RGB >> 8 & 0xff, WBGAP_PTRN_RGB & 0xff, pic_buf, sizeof pic_buf);
    n = wbgap_ptrn_make(had ? was_buf : NULL, had ? was_n : 0, PTRN_PIC, new_buf, sizeof new_buf);
    if (!pn || !n || !wbgap_ptrn_root(new_buf, n, name, sizeof name) || strcmp(name, PTRN_PIC)) {
        UnlockPubScreen(NULL, s);
        wbt_fail("P1", "the row's own picture or prefs file came out wrong (a test fault, not the desktop's); nothing written");
        return 10;
    }
    if (!put_file(PTRN_PIC, pic_buf, pn) || !put_file(PTRN_FILE, new_buf, n)) {
        if (had) put_file(PTRN_FILE, was_buf, was_n);
        else DeleteFile((CONST_STRPTR)PTRN_FILE);
        DeleteFile((CONST_STRPTR)PTRN_PIC);
        UnlockPubScreen(NULL, s);
        wbt_fail("P1", "RAM:WBGapP1.ilbm or ENV:Sys/WBPattern.prefs could not be written");
        return 10;
    }
    after = watch(s, px, py, np, 1, 15);
    printf("INFO P1 after the change: %d of %d points the picture's colour\n", after, np);
    /* the old prefs back, whatever was seen */
    put_back = had ? put_file(PTRN_FILE, was_buf, was_n) : DeleteFile((CONST_STRPTR)PTRN_FILE) != 0;
    back = watch(s, px, py, np, 0, 15);
    printf("INFO P1 after the old prefs%s: %d of %d points the picture's colour\n", put_back ? "" : " (NOT put back)", back, np);
    DeleteFile((CONST_STRPTR)PTRN_PIC);     /* GFOpenBench lets the old picture's file go (0.1.19); if not, it stays in RAM: */
    UnlockPubScreen(NULL, s);
    v = wbgap_ptrn_judge(np, before, after, back, put_back, why, sizeof why);
    wbt_say("P1", v, why);
    return v == WBGAP_FAIL ? 10 : 0;
}
