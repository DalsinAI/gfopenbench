/* ob_iconkind's host test: the Information window's kind of icon (GFOpenBench 0.1.76).
 *   test_ob_iconkind [ICON_DIR]
 * With ICON_DIR (icon_fixtures.py's output), every icon there is also checked against what icon_core's own reader
 * found in it (the "formats" line of its .want): the two must agree on every kind.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ob_iconkind.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)
#define CHECK_STR(got, want) do { checks++; if (strcmp((got), (want))) { failures++; \
    printf("FAIL %s:%d: \"%s\", wanted \"%s\"\n", __FILE__, __LINE__, (got), (want)); } } while (0)

static unsigned char f[4096];
static long fl;

static void put(const void *p, long n) { memcpy(f + fl, p, (size_t)n); fl += n; }
static void put32(unsigned long v)
{
    unsigned char b[4] = { (unsigned char)(v >> 24), (unsigned char)(v >> 16), (unsigned char)(v >> 8), (unsigned char)v };
    put(b, 4);
}
static void classic(void)
{
    fl = 0;
    f[fl++] = 0xE3; f[fl++] = 0x10;
    memset(f + fl, 0, 76); fl += 76;            /* the rest of the DiskObject */
    memset(f + fl, 0x5A, 40); fl += 40;         /* an Image's planes */
}
static void tooltype(const char *s)
{
    put32((unsigned long)strlen(s) + 1);
    put(s, (long)strlen(s) + 1);
}
static void chunk(const char *id, unsigned long n)
{
    put(id, 4);
    put32(n);
    memset(f + fl, 0, n + (n & 1)); fl += (long)(n + (n & 1));
}
static long form_at;
static void form_start(void) { form_at = fl; put("FORM", 4); put32(0); put("ICON", 4); }
static void form_end(void)
{
    unsigned long n = (unsigned long)(fl - form_at - 8);
    f[form_at + 4] = (unsigned char)(n >> 24); f[form_at + 5] = (unsigned char)(n >> 16);
    f[form_at + 6] = (unsigned char)(n >> 8); f[form_at + 7] = (unsigned char)n;
}

/* icon_core's IC_FMT_ bits (icon/icon_core.h) as ours */
static int from_core(unsigned v)
{
    return (v & 0x01 ? OB_IK_CLASSIC : 0) | (v & 0x02 ? OB_IK_GLOW : 0) | (v & 0x04 ? OB_IK_ARGB : 0) |
           (v & 0x08 ? OB_IK_NEWICON : 0) | (v & 0x10 ? OB_IK_PNG : 0);
}

static void fixtures(const char *dir)
{
    char path[1024], line[256], name[200];
    FILE *m, *w, *i;
    int n = 0;
    snprintf(path, sizeof path, "%s/manifest.txt", dir);
    if (!(m = fopen(path, "r"))) { printf("FAIL no %s\n", path); failures++; return; }
    while (fgets(line, sizeof line, m) && sscanf(line, "%199s", name) == 1) {
        static unsigned char buf[1 << 20];
        long len;
        unsigned core = 0;
        int got;
        snprintf(path, sizeof path, "%s/%s.want", dir, name);
        if (!(w = fopen(path, "r"))) continue;
        while (fgets(line, sizeof line, w))
            if (sscanf(line, "formats 0x%x", &core) == 1) break;
        fclose(w);
        snprintf(path, sizeof path, "%s/%s.info", dir, name);
        if (!(i = fopen(path, "rb"))) continue;
        len = (long)fread(buf, 1, sizeof buf, i);
        fclose(i);
        got = ob_icon_kind(buf, len);
        checks++;
        if (got != from_core(core)) {
            failures++;
            printf("FAIL %s: kinds 0x%x, icon_core read 0x%x\n", name, (unsigned)got, (unsigned)from_core(core));
        }
        n++;
    }
    fclose(m);
    CHECK(n >= 10);
    printf("ob_iconkind: %d fixture icons agree with icon_core\n", n);
}

int main(int argc, char **argv)
{
    char t[96];
    static const unsigned char png[] = { 0x89, 'P', 'N', 'G', 0x0D, 0x0A, 0x1A, 0x0A, 0, 0, 0, 13, 'I', 'H', 'D', 'R' };

    /* no icon at all */
    CHECK(ob_icon_kind(NULL, 100) == 0);
    CHECK(ob_icon_kind((const unsigned char *)"\xE3\x10", 2) == 0);
    CHECK(ob_icon_kind((const unsigned char *)"hello, world", 12) == 0);
    CHECK(ob_icon_kind((const unsigned char *)"FORM\0\0\0\4ICON", 12) == 0);   /* a FORM ICON is no file on its own */

    classic();
    CHECK(ob_icon_kind(f, fl) == OB_IK_CLASSIC);

    /* a NewIcon: the IM1= tool types */
    classic();
    tooltype("CLI");
    tooltype("*** DON'T EDIT THE FOLLOWING LINES!! ***");
    tooltype("IM1=B}}!'??}}}}}}}}}");
    tooltype("IM2=B}}!'??}}}}}}}}}");
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_NEWICON));
    /* the last thing in the file */
    classic();
    tooltype("IM1=ab");
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_NEWICON));
    /* "IM1=" inside another tool type's words is not one */
    classic();
    tooltype("XIM1=no");
    CHECK(ob_icon_kind(f, fl) == OB_IK_CLASSIC);

    /* a GlowIcon: FORM ICON with FACE and IMAG, an odd-sized chunk padded */
    classic();
    tooltype("DONOTPROMPT");
    form_start(); chunk("FACE", 6); chunk("IMAG", 33); chunk("IMAG", 40); form_end();
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_GLOW));
    /* cut short in its last chunk: still the GlowIcon it began as */
    CHECK(ob_icon_kind(f, fl - 20) == (OB_IK_CLASSIC | OB_IK_GLOW));

    /* ARGB pictures, with and without IMAG ones */
    classic();
    form_start(); chunk("FACE", 6); chunk("ARGB", 100); form_end();
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_ARGB));
    classic();
    form_start(); chunk("FACE", 6); chunk("IMAG", 20); chunk("ARGB", 100); form_end();
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_GLOW | OB_IK_ARGB));

    /* a GlowIcon that is a NewIcon too */
    classic();
    tooltype("IM1=B}}!'");
    form_start(); chunk("FACE", 6); chunk("IMAG", 20); form_end();
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_NEWICON | OB_IK_GLOW));

    /* a chunk whose size runs past the file: no hang, no read past it */
    classic();
    form_start(); chunk("FACE", 6); put("IMAG", 4); put32(0x7FFFFFF0UL); form_end();
    CHECK(ob_icon_kind(f, fl) == (OB_IK_CLASSIC | OB_IK_GLOW));

    CHECK(ob_icon_kind(png, (long)sizeof png) == OB_IK_PNG);

    /* the words */
    CHECK_STR(ob_icon_kind_text(OB_IK_CLASSIC, 1, t, sizeof t), "Classic");
    CHECK_STR(ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_NEWICON, 1, t, sizeof t), "NewIcon");
    CHECK_STR(ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_GLOW, 1, t, sizeof t), "GlowIcon (OS 3.5)");
    CHECK_STR(ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_GLOW | OB_IK_NEWICON, 1, t, sizeof t), "GlowIcon (OS 3.5) (also NewIcon)");
    CHECK_STR(ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_GLOW | OB_IK_ARGB | OB_IK_NEWICON, 1, t, sizeof t),
              "ARGB (AROS) (also GlowIcon (OS 3.5), NewIcon)");
    CHECK_STR(ob_icon_kind_text(OB_IK_PNG, 1, t, sizeof t), "PNG (OS4/MorphOS/AROS)");
    CHECK_STR(ob_icon_kind_text(0, 1, t, sizeof t), "Unknown");
    CHECK_STR(ob_icon_kind_text(OB_IK_GLOW, 0, t, sizeof t), "Default icon");
    /* a short buffer is cut, and ended */
    ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_GLOW | OB_IK_NEWICON, 1, t, 12);
    CHECK(strlen(t) == 11);
    ob_icon_kind_text(OB_IK_CLASSIC | OB_IK_GLOW | OB_IK_ARGB | OB_IK_NEWICON, 1, t, 22);
    CHECK(strlen(t) == 21);

    /* the box */
    CHECK(ob_icon_box_side(32, 84, 200) == 84);
    CHECK(ob_icon_box_side(72, 84, 200) == 84);
    CHECK(ob_icon_box_side(73, 84, 200) == 85);
    CHECK(ob_icon_box_side(128, 84, 200) == 140);
    CHECK(ob_icon_box_side(256, 84, 200) == 200);

    if (argc > 1) fixtures(argv[1]);
    printf("ob_iconkind: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
