/* wbgap_test: what a row's own file (tests/amiga/rows/<MODE>.c) takes from WBGapTest.c, and the markers
 * tests/amiga/gen-rows.sh lists. A mode is int <function>(int argc, char **argv), argv[1] its name.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_TEST_H
#define WBGAP_TEST_H

struct Window;

/* markers, each alone on a line at the file's left edge; they expand to nothing.
 * WBGAP_MODE(NAME, fn, NEWROWS): run by "WBGapTest NEWROWS" (the script's one line for every such mode), or SCRIPT:
 * the script has a line of its own for it. WBGAP_ROWMODE(ID, NAME): row ID's line comes from NAME (the table's mode). */
#define WBGAP_MODE(name, fn, when)
#define WBGAP_ROWMODE(id, name)

void wbt_pass(const char *row);
void wbt_fail(const char *row, const char *why);
void wbt_skip(const char *row, const char *why);
void wbt_say(const char *row, int state, const char *why);   /* WBGAP_PASS, _FAIL or _SKIP, as the judge gave */
int wbt_fails(void);                                          /* FAIL lines printed so far */
int wbt_run(const char *cmd);                                 /* SystemTags; its return code */
long wbt_read_all(const char *path, char *buf, long n);       /* the bytes read, -1 if none */
const char *wbt_dragtest(void);                               /* DragTest's full path */
int wbt_put_icon(const char *path, long type, char **tooltypes);
struct Window *wbt_find_window(const char *part);             /* a window whose title holds part */
struct Window *wbt_open_drawer(const char *path, const char *title, int secs, int *how);
int wbt_clear_drawer(const char *path, const char *title, const char *row);
int wbt_is_gfos(void);                                        /* GFOpenBench is the desktop (its port, or its menus seen) */
int wbt_desktop_has_disk(const char *row, const char *vol, const char *when); /* vol a disk icon: 1, 0, -1 unread */
/* APPDROP's drawer: RAM:WBGapD made afresh, its window open and its project d in it, found (I8, I9's setup); NULL with
 * *stage 1 (the window didn't open) or 2 (d never showed). wbt_drop_from: the screen point to press on d in it;
 * wbt_drop_drag: DragTest DRAG from one screen point to another; wbt_drop_clean: RAM:WBGapD closed and gone */
struct Window *wbt_drop_ready(int *stage);
void wbt_drop_from(struct Window *w, long xy[2]);
void wbt_drop_drag(const long from[2], const long to[2], const char *row);
void wbt_drop_clean(void);

#endif
