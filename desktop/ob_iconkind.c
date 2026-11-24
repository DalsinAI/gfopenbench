/* ob_iconkind.c: see ob_iconkind.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>
#include <stdio.h>

#include "ob_iconkind.h"

static unsigned long be32(const unsigned char *p)
{
    return ((unsigned long)p[0] << 24) | ((unsigned long)p[1] << 16) | ((unsigned long)p[2] << 8) | p[3];
}

/* The chunks of the FORM ICON at buf[at] (its "FORM"): IMAG and ARGB. */
static int form_icon_kinds(const unsigned char *buf, long len, long at)
{
    unsigned long size = be32(buf + at + 4);
    long end, p;
    int k = 0;
    if (size < 4) return 0;
    end = (long)size > len - at - 8 ? len : at + 8 + (long)size;   /* a cut file: what there is of it */
    for (p = at + 12; p + 8 <= end;) {
        unsigned long cs = be32(buf + p + 4);
        if (!memcmp(buf + p, "IMAG", 4)) k |= OB_IK_GLOW;
        else if (!memcmp(buf + p, "ARGB", 4)) k |= OB_IK_ARGB;
        if (cs > (unsigned long)(end - p - 8)) break;
        p += 8 + (long)cs + (long)(cs & 1);                       /* chunks are padded to even */
    }
    return k;
}

int ob_icon_kind(const unsigned char *buf, long len)
{
    static const unsigned char png[8] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A };
    int k = 0;
    long i;
    if (!buf || len < 8) return 0;
    if (!memcmp(buf, png, 8)) return OB_IK_PNG;
    if (buf[0] != 0xE3 || buf[1] != 0x10) return 0;
    k = OB_IK_CLASSIC;
    for (i = 2; i + 4 <= len; i++) {
        if (buf[i] == 'F' && i + 12 <= len && !memcmp(buf + i, "FORM", 4) && !memcmp(buf + i + 8, "ICON", 4)) {
            k |= form_icon_kinds(buf, len, i);
            break;
        }
        /* a tool type of NewIcons' picture: "IM1=" at its start, after the string's 4-byte length */
        if (buf[i] == 'I' && i >= 6 && !memcmp(buf + i, "IM1=", 4)) {
            /* a whole tool type: its length counts its words and their 0, and nothing before the 0 is one */
            unsigned long n = be32(buf + i - 4);
            if (n >= 5 && n <= (unsigned long)(len - i) && !buf[i + (long)n - 1] && !memchr(buf + i, 0, n - 1))
                k |= OB_IK_NEWICON;
        }
    }
    return k;
}

char *ob_icon_kind_text(int kinds, int had_icon, char *out, int n)
{
    static const struct { int bit; const char *name; } order[] = {
        { OB_IK_PNG, "PNG (OS4/MorphOS/AROS)" },
        { OB_IK_ARGB, "ARGB (AROS)" },
        { OB_IK_GLOW, "GlowIcon (OS 3.5)" },
        { OB_IK_NEWICON, "NewIcon" },
        { OB_IK_CLASSIC, "Classic" },
    };
    int i, first = -1, k = 0, others = 0;
    if (!out || n <= 0) return out;
    out[0] = 0;
    if (!had_icon) { snprintf(out, (size_t)n, "Default icon"); return out; }
    for (i = 0; i < (int)(sizeof order / sizeof order[0]); i++)
        if (kinds & order[i].bit) { first = i; break; }
    if (first < 0) { snprintf(out, (size_t)n, "Unknown"); return out; }
    k = snprintf(out, (size_t)n, "%s", order[first].name);
    /* the others it also is; Classic is under every one but PNG, so it is named only on its own */
    for (i = first + 1; i < (int)(sizeof order / sizeof order[0]) - 1; i++)
        if ((kinds & order[i].bit) && k >= 0 && k < n) {
            k += snprintf(out + k, (size_t)(n - k), others++ ? ", %s" : " (also %s", order[i].name);
        }
    if (others && k >= 0 && k < n) snprintf(out + k, (size_t)(n - k), ")");
    return out;
}

int ob_icon_box_side(int picture, int min, int max)
{
    int s = picture + 12;
    if (s < min) s = min;
    if (s > max) s = max;
    return s;
}
