/* W1 (WINGAD): a drawer window's own gadgets, as on 3.2.3. The size gadget dragged 80,60 grows the window (by 60,40
 * or more: a desktop may snap it); the zoom gadget clicked changes its size and clicked again gives the size back
 * (within 2 pixels); the depth gadget clicked brings it in front of a window that covered it, and clicked again sends
 * it behind that window. Each measure -1 where it couldn't be taken (that gadget missing), which is a FAIL: the row is
 * about the gadgets being there. The one exception is a missing zoom gadget on a desktop whose look says "zoom off"
 * (ENV:GFOpenGadTools/Look, gfopengadtools DESIGN.md "How windows behave": GFOS has no zoom anywhere by design, and
 * a double-click on a title bar brings the window to the front): an INFO, not a FAIL.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
struct wbgap_wingad {
    long size0[2], size1[2];               /* width, height before and after the size gadget's drag */
    long zoom0[2], zoom1[2], zoom2[2];     /* before the zoom gadget, after one click, after the second */
    int front0, front1, front2;            /* the drawer window in front of the cover: before, after one depth click,
                                            * after the second (1, 0; -1 not read) */
    int has_size, has_zoom, has_depth;     /* the gadget found on the window */
    int zoom_off;                          /* the desktop's look says "zoom off" (wbgap_wingad_zoom_off) */
};
/* WBGAP_PASS, or WBGAP_FAIL with every part that failed in why ("size: ...; zoom: ...") */
int wbgap_wingad_judge(const struct wbgap_wingad *m, char *why, unsigned cap);
/* 1 if the text of ENV:GFOpenGadTools/Look (NULL: none) has a line "zoom off", no line "screen workbench classic",
 * and a theme other than Classic (line 1's first word, as GFOpenLook reads it; a blank or ';' line 1 keeps Classic):
 * Classic screens and the Classic theme keep their zoom gadgets (gfopengadtools DESIGN.md, "How windows behave").
 * Any case, words split by spaces or tabs, spaces round them and a CR at the end allowed; else 0 */
int wbgap_wingad_zoom_off(const char *look);
/* the middle of a window's gadget on the window (its own coordinates), from its Left/TopEdge, Width/Height and its
 * GFLG_REL* flags as four 0/1 (relright, relbottom, relwidth, relheight), the window being win_w x win_h */
void wbgap_gadget_middle(long left, long top, long w, long h, int relright, int relbottom, int relwidth,
                         int relheight, long win_w, long win_h, long xy[2]);
