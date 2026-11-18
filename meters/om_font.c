/* om_font: the hand-drawn GFOS font for a theme's outline family (om_font.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "om_font.h"

static const struct { const char *family, *drawn; } twins[] = {
    { "dejavusans", "gfossans.font" },
    { "dejavusansbook", "gfossans.font" },
    { "dejavusansbold", "gfossansbold.font" },
    { "dejavusansmono", "gfosmono.font" },
    { "gfossans", "gfossans.font" },
    { "gfossansbold", "gfossansbold.font" },
    { "gfosmono", "gfosmono.font" },
};

/* the family folded: lower case, no spaces, no ".font" at its end; 0 when it doesn't fit */
static int fold(const char *s, char *out, int max)
{
    int n = 0;
    for (; *s; s++) {
        char c = *s;
        if (c == ' ' || c == '-' || c == '_') continue;
        if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
        if (n >= max - 1) return 0;
        out[n++] = c;
    }
    out[n] = 0;
    if (n > 5 && !strcmp(out + n - 5, ".font")) out[n - 5] = 0;
    return 1;
}

const char *om_font_drawn(const char *family, int size)
{
    char f[64];
    unsigned i;
    if (!family || (size != 11 && size != 13 && size != 16) || !fold(family, f, sizeof f)) return 0;
    for (i = 0; i < sizeof twins / sizeof twins[0]; i++)
        if (!strcmp(f, twins[i].family)) return twins[i].drawn;
    return 0;
}
