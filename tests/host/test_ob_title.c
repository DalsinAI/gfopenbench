/* The screen title's words (desktop/ob_title.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>
#include "ob_title.h"

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

int main(void)
{
    char t[192], s[24];
    int k;
    CHECK(!ob_title_lines(47, 100) && ob_title_lines(47, 101) && ob_title_lines(48, 0) && !ob_title_lines(40, 200)
          && !ob_title_lines(47, 54), "the small line from intuition.library 47.101");
    /* the lines, with extended memory */
    k = ob_title_text(t, sizeof t, "    ", "0.1.70", "8 MB", "1.7 GB", "1.1 GB", 1, "1 (68k) + 4 (ARM)", 1);
    CHECK(!strcmp(t, "    OpenBench 0.1.70 \x1F Chip 8 MB \x1F Fast 1.7 GB \x1F Ext 1.1 GB \x1F GPU = 1, CPU = 1 (68k) + 4 (ARM)"),
          "lines, ext: \"%s\"", t);
    CHECK(k == (int)strlen(t), "the length answered");
    CHECK(!strchr(t, '-'), "no dash where the lines are");
    /* no extended memory: no Ext figure */
    ob_title_text(t, sizeof t, "", "0.1.70", "8 MB", "1.9 GB", "", 2, "1 (68k)", 1);
    CHECK(!strcmp(t, "OpenBench 0.1.70 \x1F Chip 8 MB \x1F Fast 1.9 GB \x1F GPU = 2, CPU = 1 (68k)"), "lines, no ext: \"%s\"", t);
    ob_title_text(t, sizeof t, "", "0.1.70", "8 MB", "1.9 GB", NULL, 2, "1 (68k)", 1);
    CHECK(!strstr(t, "Ext"), "NULL ext: \"%s\"", t);
    /* an Intuition without the line: the words parted as before */
    ob_title_text(t, sizeof t, "  ", "0.1.70", "8 MB", "1.7 GB", "1.1 GB", 1, "1 (68k)", 0);
    CHECK(!strcmp(t, "  OpenBench 0.1.70 - Chip 8 MB, Fast 1.7 GB, Ext 1.1 GB, GPU = 1, CPU = 1 (68k)"), "no lines: \"%s\"", t);
    CHECK(!strchr(t, 0x1F), "no 0x1F for an Intuition that would draw it as a letter");
    ob_title_text(t, sizeof t, "", "0.1.70", "8 MB", "1.9 GB", "", 0, "1 (68k)", 0);
    CHECK(!strcmp(t, "OpenBench 0.1.70 - Chip 8 MB, Fast 1.9 GB, GPU = 0, CPU = 1 (68k)"), "0.1.69's words: \"%s\"", t);
    /* a short buffer: cut, ended, never past it */
    memset(s, 'x', sizeof s);
    ob_title_text(s, 16, "", "0.1.70", "8 MB", "1.7 GB", "1.1 GB", 1, "1 (68k)", 1);
    CHECK(strlen(s) == 15 && s[16] == 'x', "a short buffer kept");
    printf(fails ? "ob_title: %d FAILED\n" : "ob_title: ok\n", fails);
    return fails != 0;
}
