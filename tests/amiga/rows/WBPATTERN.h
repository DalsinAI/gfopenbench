/* P1 (WBPATTERN): the root picture from Prefs/WBPattern's file (ENV:Sys/WBPattern.prefs, an IFF FORM PREF with a
 * PTRN chunk for WBP_ROOT naming a picture). A picture of one colour is made and named, and that colour must cover the
 * desktop; with the old file back, it must go. Plain C, for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBPATTERN_H
#define WBPATTERN_H

/* a WBPattern.prefs made from orig (len bytes; NULL, or not a FORM PREF: one made afresh, with a PRHD) with its root's
 * PTRN chunk (wbp_Which 0) taken out and a new one for the picture file name put straight after PRHD (GFOpenBench
 * reads only the first 2 KB); the other chunks (PRHD, the drawers' and the screen's PTRN) as they were, in order. Its length, 0 if cap is too small or name too long */
long wbgap_ptrn_make(const unsigned char *orig, long len, const char *name, unsigned char *out, long cap);
/* the picture a WBPattern.prefs names for the root, as GFOpenBench reads it (desktop/openbench.c root_picture): a PTRN
 * with wbp_Which 0, no WBPF_PATTERN and a name; 1 and the name in out, 0 if there is none */
int wbgap_ptrn_root(const unsigned char *buf, long len, char *out, int cap);
/* an ILBM of w x h (each 1 to 256) in one colour r g b: one bitplane, both CMAP colours that colour, BODY not packed.
 * Its length, 0 if cap is too small */
long wbgap_ilbm_solid(int w, int h, unsigned r, unsigned g, unsigned b, unsigned char *out, long cap);
/* rgb (0xRRGGBB) is within tol of want in each of its three parts: 1 */
int wbgap_rgb_near(unsigned long rgb, unsigned long want, unsigned tol);
/* P1: n desktop points read (those no window covers); before, after and back, how many of them were the picture's
 * colour before the change, after it, and after the old file was put back; put_back, whether the old file went back */
int wbgap_ptrn_judge(int n, int before, int after, int back, int put_back, char *why, unsigned cap);

#define WBGAP_PTRN_RGB 0xD03090UL   /* the picture's colour: no theme's desktop is this */
#define WBGAP_PTRN_TOL 40           /* a remap onto a screen of few pens comes this near */
#endif
