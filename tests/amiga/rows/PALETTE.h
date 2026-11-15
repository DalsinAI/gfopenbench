/* P3 (PALETTE): Palette's prefs (ENV:Sys/palette.prefs, an IFF FORM PREF with a 400-byte PALT: 16 reserved bytes,
 * the 4- and 8-colour DrawInfo pen lists, 32 words each, and 32 ColorSpecs of index and 16-bit r, g, b, the list
 * ending at index -1). Colour 0, the background pen's on a Workbench of the OS's pens, is set to one colour no theme
 * uses, and a drawer window's empty inside must take it: on 3.2.3 through IPrefs and the screen's colour 0, on GFOS
 * through GFOpenLayout's pens (designs/WORKBENCH-REDESIGN-20261113.md P3). Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef PALETTE_H
#define PALETTE_H

#define WBGAP_PALT_LEN 400
#define WBGAP_PAL_RGB 0xD03090UL   /* WBPATTERN's colour: no theme's drawer is this */
#define WBGAP_PAL_TOL 40

/* a palette.prefs made from orig (len bytes; NULL, not a FORM PREF, or with no whole PALT: one with PRHD and a PALT
 * of the OS's defaults, as Palette writes with no file) with colour index set to rgb (0xRRGGBB, each part made 16-bit
 * by repeating it), added to the list if it isn't there; every other chunk and colour as it was. Its length, 0 if cap
 * is too small or the list has no room */
long wbgap_palette_make(const unsigned char *orig, long len, int index, unsigned long rgb, unsigned char *out, long cap);
/* colour index in a palette.prefs as 0xRRGGBB (the top byte of each 16-bit part): 1, or 0 if it has none */
int wbgap_palette_colour(const unsigned char *buf, long len, int index, unsigned long *rgb);
/* the most common of n colours: how many there are of it, and it in *rgb (n 0: 0) */
int wbgap_palette_mode(const unsigned long *c, int n, unsigned long *rgb);
/* P3: n points of the drawer's empty inside read; same, how many were its most common colour before the change;
 * before, after, back: how many were the prefs' colour before, after and with the old file back; put_back whether it
 * went back */
int wbgap_palette_judge(int n, int same, int before, int after, int back, int put_back, char *why, unsigned cap);
#endif
