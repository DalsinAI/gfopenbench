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

#endif
