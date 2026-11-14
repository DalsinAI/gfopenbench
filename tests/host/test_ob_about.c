/* GFOpenBench's About (desktop/ob_about.h): the lines the About window and its fallback show. MIT, Copyright (c)
 * 2026 AmiCradle Team. */
#include "ob_about.h"

#include <stdio.h>
#include <string.h>

static int fails, checks;
#define CHECK(c) do { ++checks; if (!(c)) { ++fails; printf("FAIL %s:%d %s\n", __FILE__, __LINE__, #c); } } while (0)

int main(void)
{
    const char *t = OB_ABOUT_TEXT, *r = OB_ABOUT_REQ("0.1.67"), *p;
    int lines = 1, longest = 0, run = 0;
    for (p = t; *p; p++) {
        if (*p == '\n') { lines++; run = 0; } else if (++run > longest) longest = run;
    }
    CHECK(!strcmp(OB_ABOUT_NAME, "GFOpenBench") && lines == 2);
    CHECK(strstr(t, "MIT, Copyright (c) 2026 AmiCradle Team.") != NULL);
    CHECK(longest * 8 <= 640 - 120);      /* a line in topaz 8 beside the mark, inside a 640-pixel screen */
    CHECK(!strncmp(r, "GFOpenBench 0.1.67\n\n", 20) && strstr(r, "Designed, tested and delivered by The AmiCradle Team."));
    CHECK(strchr(t, '%') == NULL);        /* the EasyRequest takes it as a format: no conversions in it */
    printf("test_ob_about: %d checks, %d failed\n", checks, fails);
    return fails != 0;
}
