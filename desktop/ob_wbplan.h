/* ob_wbplan: the plain-C parts of GFOpenBench as workbench.library's Workbench (0.1.56, gap item 4; ob_wb.c and
 * openbench.c use them), kept apart so the host tests reach them: a path into its drawer and name, a program's
 * ChangeWorkbenchSelectionA hook answer, and where programs' AppMenuItems go in the menus.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_WBPLAN_H
#define OB_WBPLAN_H

/* "Work:Pictures/x" into its drawer "Work:Pictures" (at most n - 1 characters) and its name "x", returned;
 * "Work:" into "Work:" and "". As dos.library's FilePart, from the last ':' or '/'. */
const char *ob_wbp_split(const char *path, char *dir, int n);

/* The hook's answer (ISMACTION_*, workbench/workbench.h: Unselect 0, Select 1, Ignore 2, Stop 3) as
 * wb_drawer_each's: 0 unselect, 1 select, 2 leave as it is, -1 stop. Anything else leaves it. */
#define OB_WBP_UNSELECT 0
#define OB_WBP_SELECT   1
#define OB_WBP_IGNORE   2
#define OB_WBP_STOP     3
int ob_wbp_hook_answer(long r);

/* UpdateWorkbench's and WBInfo's object: name in the drawer dir (as NameFromLock gives it), as one path in out (at
 * most n - 1 characters): "Work:" + "x" is "Work:x", "Work:a" + "x" is "Work:a/x", "Work:a/" + "x" is "Work:a/x",
 * any dir + "" is dir. 1, or 0 when it doesn't fit (out then "") or dir is empty with a name. */
int ob_wbp_join(const char *dir, const char *name, char *out, int n);

/* OpenWorkbenchObjectA's arguments (WBOPENA_ArgLock and WBOPENA_ArgName, row I11 through LAUNCH): a name as one
 * path in out (at most n - 1 characters). A name with a ':' is a full path already; with no drawer (no ArgLock
 * before it) the name is taken as it is; otherwise the name in dir, as ob_wbp_join puts them. 1, or 0 when it
 * doesn't fit or the name is empty (out then ""). */
int ob_wbp_arg_path(const char *dir, const char *name, char *out, int n);

/* What OpenWorkbenchObjectA does with the object's kind (OB_WBP_K_*, from its icon) and nargs arguments, as OS 3.2.3:
 * OB_WBP_RUN_OPEN, opened as it is opened with none (a drawer, a disk, or no arguments); OB_WBP_RUN_TOOL, the tool
 * started with the arguments; OB_WBP_RUN_PROJECT, the project's default tool started with the project, then the
 * arguments. */
#define OB_WBP_K_OTHER   0
#define OB_WBP_K_TOOL    1
#define OB_WBP_K_PROJECT 2
#define OB_WBP_K_DRAWER  3
#define OB_WBP_RUN_OPEN    0
#define OB_WBP_RUN_TOOL    1
#define OB_WBP_RUN_PROJECT 2
int ob_wbp_open_how(int kind, int nargs);

/* A program's AppMenuItem as the menus are laid out from it (ob_wb_menu's; role OWB_MENU_*: item 0, parent 1,
 * title 2) */
typedef struct ob_wbp_menu {
    const void *handle, *parent;            /* parent: its title or parent item, NULL: Tools */
    unsigned char role, visible;
} ob_wbp_menu;

/* A line of the menus made from them: m[index], as a title (level 0), an item (1) or a sub item (2) */
typedef struct ob_wbp_row {
    int index, level;
} ob_wbp_row;

/* The items under `under` (NULL: Tools), each parent item followed by its sub items; at most max rows. How many. */
int ob_wbp_items(const ob_wbp_menu *m, int n, const void *under, ob_wbp_row *out, int max);
/* The programs' own menus, after GFOpenBench's: each shown title followed by its items. How many rows. */
int ob_wbp_own(const ob_wbp_menu *m, int n, ob_wbp_row *out, int max);
/* Whether any Tools item is shown (a Tools menu is then made when GFOpenBench has none) */
int ob_wbp_any_tools(const ob_wbp_menu *m, int n);

/* Where the Tools items go among GFOpenBench's menu titles (in order): *own 1, at the end of its own "Tools" menu
 * (title k, returned); *own 0, in a Tools menu made before title k (k == nt: after the last), which is the one
 * after "Icons" or, with no Icons, the end. -1 when there is nothing to show (no Tools of its own, no items). */
int ob_wbp_tools_at(const char *const *titles, int nt, int any, int *own);

/* WhichWorkbenchObjectA (gap row A8c): what is at the place asked about, OB_WBP_AT_* (the desktop's own question
 * of the drawer window or the desktop there, wb_drawer_at or its icons), and the path found there, into the
 * answer: OB_WBP_WBO_* (workbench.h's WBO_ values) and the object's name in name (at most n - 1 characters).
 * An icon (a drawer's, a disk's, a tool's or a project's) is WBO_ICON named as its last part ("Work:a/b" is "b",
 * the disk "Work:" is "Work"); a drawer's own space off its icons is WBO_DRAWER, named as that drawer; anything else
 * (a frame, a heading, no window of ours) is WBO_NONE with name "". */
#define OB_WBP_AT_NONE  0
#define OB_WBP_AT_SPACE 1
#define OB_WBP_AT_ICON  2
#define OB_WBP_WBO_NONE   0
#define OB_WBP_WBO_DRAWER 1
#define OB_WBP_WBO_ICON   2
int ob_wbp_which(int at, const char *path, char *name, int n);

#endif
