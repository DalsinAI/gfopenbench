/* I13 (ICONFMT): the four icon formats a drawer shows: classic (planar), NewIcons, OS 3.5 GlowIcons and PNG. One
 * icon of each (ICONFMT_icons.h, made by iconfmt_gen.py), each a 40x30 picture of one colour: the classic one pen 3,
 * the others a colour no theme uses. The NewIcons and GlowIcons ones carry a classic picture of pen 1 too, so a
 * desktop that can't read their own picture shows pen 1 there. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef ICONFMT_H
#define ICONFMT_H

#define WBGAP_ICONFMT_N 4
#define WBGAP_ICONFMT_NEWICONS_RGB 0x30D090UL   /* keep in step with iconfmt_gen.py */
#define WBGAP_ICONFMT_GLOW_RGB 0x3090D0UL
#define WBGAP_ICONFMT_PNG_RGB 0xD0A030UL
#define WBGAP_ICONFMT_TOL 40
/* a 40x30 picture read every 2 pixels is 300 points; half of it must be seen, whatever covers the rest */
#define WBGAP_ICONFMT_MIN 150

/* rgb (0xRRGGBB) within tol of want in each of its three parts: 1 */
int wbgap_iconfmt_near(unsigned long rgb, unsigned long want, unsigned tol);
/* the formats' names, b_classic, c_newicons, d_glowicons, e_png, as the icons' files are named */
const char *wbgap_iconfmt_name(int k);
/* I13: the drawer's whole inside read every 2 pixels (n points) with the four icons in it; near[k], how many were
 * format k's colour (the classic one's: pen 3's); pen1_added, how many more were pen 1's than before the icons went in
 * (labels, and any classic picture shown in place of NewIcons or GlowIcons). Each format must be its colour in
 * WBGAP_ICONFMT_MIN points. WBGAP_PASS; WBGAP_FAIL naming each that isn't (with pen 1's growth when it is two pictures'
 * worth, a classic picture shown instead); WBGAP_SKIP if fewer than 2000 points were read */
int wbgap_iconfmt_judge(int n, const int *near, int pen1_added, char *why, unsigned cap);
#endif
