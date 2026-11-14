/* I8, I9 (APPDROP): an icon dragged onto an AppIcon and onto an AppWindow. MIT, Copyright (c) 2026 AmiCradle Team.
 * The drop corner and point (wbgap_drop_place, wbgap_drop_point) stay in wbgap_rows.h: APPWIN and DRAGFILE use them too. */
/* the AppMessage judged: its am_Type and am_ID against those wanted, one argument at least, the first named
 * want_name, in a drawer whose path (NameFromLock of its wa_Lock) ends in ":" or "/" and want_dir. got 0: no message came.
 * WBGAP_PASS or WBGAP_FAIL, the reason in why */
int wbgap_drop_judge(int got, long type, long want_type, unsigned long id, unsigned long want_id, long nargs,
                     const char *name, const char *dir, const char *want_name, const char *want_dir, char *why,
                     unsigned cap);
/* "x y w h" (WBGapAppPos.rexx, the AppIcon's box on the screen) into box: 1, or 0 if it isn't four numbers or
 * the width or height isn't above 0 */
int wbgap_parse_box(const char *text, long box[4]);
