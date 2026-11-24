/* ob_iconkind.h: what kind of icon a .info file is, for the Information window (GFOpenBench 0.1.76; Dale,
 * 24 November 2026: "information should show the icon regardless for icon format, and show what kind of icon it
 * is"). Read from the file's own bytes, so the answer is the same whichever icon.library is in use (ours, or
 * OS 3.2.3's, which reads a NewIcon as a classic icon and refuses a PNG one). Plain C, no Amiga headers, tested on
 * the host (tests/host/test_ob_iconkind.c).
 *
 * The kinds:
 *   Classic   a DiskObject (0xE310), the picture in its Image planes (OS 1.x to 3.2)
 *   NewIcon   a classic icon whose tool types carry IM1= (and IM2=) lines
 *   GlowIcon  a classic icon followed by FORM ICON with IMAG chunks (OS 3.5, ColorIcon)
 *   ARGB      FORM ICON with ARGB chunks: 32-bit pictures (AROS)
 *   PNG       the file is a PNG with an icOn chunk (OS4, MorphOS and AROS PNG icons)
 * A file can be more than one (a GlowIcon that is a NewIcon too); the window names the one drawn first and the
 * rest after it.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_ICONKIND_H
#define OB_ICONKIND_H

#define OB_IK_CLASSIC  0x01
#define OB_IK_NEWICON  0x02
#define OB_IK_GLOW     0x04
#define OB_IK_ARGB     0x08
#define OB_IK_PNG      0x10

/* The kinds (OB_IK_ bits) of an icon file's first len bytes; 0 when it is no icon file at all. */
int ob_icon_kind(const unsigned char *buf, long len);

/* The words for kinds, into out (n bytes, always ended): the kind drawn first, then the others in brackets,
 * "GlowIcon (OS 3.5) (also NewIcon)". had_icon 0: there is no .info and the default icon for its type shows,
 * "Default icon". kinds 0 with an icon: "Unknown". Returns out. */
char *ob_icon_kind_text(int kinds, int had_icon, char *out, int n);

/* The icon box's side for a picture of w x h: at least min (the window's 84), the picture and a margin of 6 on
 * each side when it is bigger, at most max (a bigger picture is clipped to the box). */
int ob_icon_box_side(int picture, int min, int max);

#endif
