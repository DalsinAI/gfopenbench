/* L1 (LAYOUT): the default layout's drawer, RAM:WBGapL, no icon snapshotted; WBGapLayout.rexx opens it and reads the
 * icons' places. MIT, Copyright (c) 2026 AmiCradle Team.
 * wbgap_layout_icons: what is made in it, in the order made (not name order, so a desktop that lays out by name
 * shows it): two of each kind, kind 't' a tool, 'p' a project, 'd' a drawer; WBGAP_LAYOUT_N of them. */
#define WBGAP_LAYOUT_N 6
struct wbgap_layout_icon {
    const char *name;
    char kind;
};
extern const struct wbgap_layout_icon wbgap_layout_icons[WBGAP_LAYOUT_N];
