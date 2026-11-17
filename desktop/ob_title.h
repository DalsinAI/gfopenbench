/* The screen title's words (0.1.70; Dale, 17 November 2026: Fast and the extended memory shown apart, and "rather
 * than dash between openbench version and chipmemor use the same line as bween teh screen depth and hte power, put
 * that sperator in between fast memor, ext, and the the gpu / cpiu").
 *   OpenBench 0.1.70 | Chip 8 MB | Fast 1.7 GB | Ext 1.1 GB | GPU = 1, CPU = 1 (68k) + 4 (ARM)
 * Each "|" is the title's small line: a 0x1F, which GFOpenIntuition 47.101 and up draws as the line GFOpenTitle has
 * beside Prefs and Power. On an Intuition that doesn't (lines 0) the words are parted as before, by " - " and ", ".
 * ext is NULL or "" when there is no extended memory: no Ext figure.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_TITLE_H
#define OB_TITLE_H

#include <stdio.h>

#define OB_TITLE_LINE "\x1F"

/* does this intuition.library draw a title's 0x1F as the small line? */
static inline int ob_title_lines(unsigned version, unsigned revision)
{
    return version > 47 || (version == 47 && revision >= 101);
}

static inline int ob_title_text(char *t, unsigned long size, const char *pad, const char *ver, const char *chip,
                                const char *fast, const char *ext, unsigned long gpus, const char *cpu, int lines)
{
    const char *first = lines ? " " OB_TITLE_LINE " " : " - ", *next = lines ? " " OB_TITLE_LINE " " : ", ";
    int k = snprintf(t, size, "%sOpenBench %s%sChip %s%sFast %s", pad, ver, first, chip, next, fast);
    if (k < 0 || (unsigned long)k >= size) return k;
    if (ext && ext[0]) {
        k += snprintf(t + k, size - k, "%sExt %s", next, ext);
        if ((unsigned long)k >= size) return k;
    }
    return k + snprintf(t + k, size - k, "%sGPU = %lu, CPU = %s", next, gpus, cpu);
}

#endif
