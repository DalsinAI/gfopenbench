/* I13 (ICONFMT): the four icon formats a drawer shows: classic (planar), NewIcons, OS 3.5 GlowIcons and PNG. One
 * icon of each (ICONFMT_icons.h, made by iconfmt_gen.py), each a 40x30 picture of one colour no theme uses (the classic
 * one pen 1); the NewIcons and GlowIcons ones carry a classic picture of pen 1 too, so a desktop that can't read their
 * own picture shows pen 1. Plain C, for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef ICONFMT_H
#define ICONFMT_H

#define WBGAP_ICONFMT_N 4
#define WBGAP_ICONFMT_NEWICONS_RGB 0x30D090UL   /* keep in step with iconfmt_gen.py */
#define WBGAP_ICONFMT_GLOW_RGB 0x3090D0UL
#define WBGAP_ICONFMT_PNG_RGB 0xD0A030UL
#define WBGAP_ICONFMT_TOL 40

/* rgb (0xRRGGBB) within tol of want in each of its three parts: 1 */
int wbgap_iconfmt_near(unsigned long rgb, unsigned long want, unsigned tol);
/* the formats' names, classic, newicons, glowicons, png, as the icons' files are named */
const char *wbgap_iconfmt_name(int k);
/* I13: for each format k, n[k] points of its icon's picture read, near[k] of them its colour (the classic one's: pen
 * 1's), pen1[k] of them pen 1's colour (the classic picture the others carry; -1 for the classic one). Each must be
 * its colour in at least a quarter of its points. WBGAP_PASS, WBGAP_FAIL naming each that isn't (and whether pen 1, the
 * classic picture, showed instead), or WBGAP_SKIP if a format's points are too few to judge (under 10) */
int wbgap_iconfmt_judge(const int *n, const int *near, const int *pen1, char *why, unsigned cap);
#endif
