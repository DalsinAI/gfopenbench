/* SCREENS (C1 C2a C2b C4 P4a P4b): screens and Intuition's pointer. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_NO_MODE 0xFFFFFFFFUL   /* graphics' INVALID_ID */

/* C1: the middle of a screen's depth gadget, in screen pixels; relright: GFLG_RELRIGHT (left is then from the right
 * edge, Width - 1 + left) */
void wbgap_depth_point(int relright, long left, long top, long w, long h, long screen_w, long xy[2]);

/* P4a: the pointer, moved to tx,ty, is within a pixel of it (mx,my: the screen's MouseX/Y) */
int wbgap_pointer_at(long mx, long my, long tx, long ty);

/* P4b: on an RTG mode the pointer is a sprite layer: the square under it holds what it holds with the pointer away.
 * mode: the Workbench screen's mode id; away1, over, away2: the square's sum before, under and after the pointer.
 * A chipset mode or a square that changed by itself is a SKIP. The reason in why. */
int wbgap_pointer_layer(unsigned long mode, unsigned long away1, unsigned long over, unsigned long away2, char *why,
                        unsigned cap);
