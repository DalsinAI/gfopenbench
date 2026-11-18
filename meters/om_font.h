/* om_font.h: the hand-drawn GFOS font for a theme's outline family, for GFOpenMeters 0.4.2 (Dale, the CM4, 17 November
 * 2026: "cm4s fonts look wrong").
 *
 * GFOpenMeters draws its words one bit deep (a plane, then its colours), in the theme's font when Font prefs sets no
 * Workbench icon font. The Open theme names "DejaVu Sans" 13, and DejaVuSans.font is an outline: scaled to 13 pixels
 * one bit deep, with no hinting, its stems came out one pixel on some letters and two on others (the L, R, A, M and
 * B of "Locked RAM" and "GBytes" heavy beside the o, c, k, e), and wider. A board with no Look prefs had no theme,
 * so the meters fell back to the screen's font, GFOS Sans 13, and read evenly; so the CM4, which has Look prefs,
 * and the Pi 5, which has none, showed the same panel in two different faces.
 *
 * GFOS Sans, Bold and Mono are DejaVu's own shapes drawn again by hand at 11, 13 and 16 pixels (DalsinAI/gfosfonts,
 * Dale 13 November 2026: the default fonts are hand-drawn, "correcting these"). So where a theme names a DejaVu
 * family at a size that is drawn, the meters open the drawn one first, and the outline only when it isn't there.
 *
 * Plain C, tested on the host (tests/host/test_om_font.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OM_FONT_H
#define OM_FONT_H

/* The drawn font file ("gfossans.font", "gfossansbold.font", "gfosmono.font") for a theme's family ("DejaVu Sans",
 * "DejaVuSans", "dejavu sans mono", "DejaVuSans.font", ...: case, spaces and a ".font" do not matter) at size
 * pixels, or 0 when the family has no drawn twin or that size isn't drawn (11, 13 and 16 are). */
const char *om_font_drawn(const char *family, int size);

#endif
