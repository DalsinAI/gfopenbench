/* M2, M6, M8, M9, M10, M12, M15, M16, M30, I14 (MENUDRIVE): the Workbench menus' items picked with the mouse.
 *
 * wbgap_menudrive_new_dir (M8): the first of after's n_after drawer names (RAM:WBGap's, after New Drawer) that is
 * not among before's n_before: its index, or -1 when none is new (names compared as they are).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_menudrive_new_dir(const char (*before)[32], int n_before, const char (*after)[32], int n_after);
