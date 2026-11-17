/* om_font's host test: the hand-drawn GFOS font GFOpenMeters 0.4.2 opens for a theme's DejaVu family (the CM4's
 * meters, 17 November 2026). The Open theme's "DejaVu Sans" 13 gives gfossans.font; Bold and Mono their own; the way
 * a family is written does not matter; a size that isn't drawn, a family with no twin, and nothing give none.
 *   test_om_font
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "om_font.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void is(const char *family, int size, const char *want)
{
    const char *got = om_font_drawn(family, size);
    CHECK(want ? got && !strcmp(got, want) : !got, "\"%s\" %d: %s, not %s", family ? family : "(null)", size,
          got ? got : "none", want ? want : "none");
}

int main(void)
{
    /* the Open, Sky and Glass themes' family, at each drawn size */
    is("DejaVu Sans", 13, "gfossans.font");
    is("DejaVu Sans", 11, "gfossans.font");
    is("DejaVu Sans", 16, "gfossans.font");
    /* written as the file is, or as fontindex names it */
    is("DejaVuSans", 13, "gfossans.font");
    is("DejaVuSans.font", 13, "gfossans.font");
    is("dejavu sans", 13, "gfossans.font");
    is("DejaVu Sans Book", 13, "gfossans.font");
    is("DejaVu-Sans", 13, "gfossans.font");
    /* Bold and Mono */
    is("DejaVu Sans Bold", 13, "gfossansbold.font");
    is("DejaVu Sans Mono", 11, "gfosmono.font");
    is("DejaVuSansMono.font", 16, "gfosmono.font");
    /* a theme that names the drawn one */
    is("GFOS Sans", 13, "gfossans.font");
    is("GFOS Mono", 13, "gfosmono.font");
    is("GFOSSansBold", 11, "gfossansbold.font");
    /* sizes that aren't drawn: the outline, as before */
    is("DejaVu Sans", 12, NULL);
    is("DejaVu Sans", 14, NULL);
    is("DejaVu Sans", 0, NULL);
    /* families with no drawn twin, and DejaVu Serif */
    is("DejaVu Serif", 13, NULL);
    is("Ubuntu", 12, NULL);
    is("Segoe UI", 12, NULL);
    is("system", 13, NULL);
    is("", 13, NULL);
    is(NULL, 13, NULL);
    is(".font", 13, NULL);
    /* too long to be a family */
    is("DejaVu Sans DejaVu Sans DejaVu Sans DejaVu Sans DejaVu Sans DejaVu Sans DejaVu Sans", 13, NULL);
    printf("om_font: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
