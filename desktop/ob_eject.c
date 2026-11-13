/* ob_eject.c: which way a disk is ejected (ob_eject.h). MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_eject.h"

static char low(char c) { return c >= 'A' && c <= 'Z' ? (char)(c + 32) : c; }

/* 1 when word (lower case) is in s, in any case */
static int has(const char *s, const char *word)
{
    const char *a, *b;
    for (; *s; s++) {
        for (a = s, b = word; *a && *b && low(*a) == *b; a++, b++)
            ;
        if (!*b) return 1;
    }
    return 0;
}

int ob_eject_kind(const char *dev, const char *exec_dev)
{
    if (!dev) return OB_EJECT_FIXED;
    if (low(dev[0]) == 'd' && low(dev[1]) == 'f' && dev[2] >= '0' && dev[2] <= '3' && !dev[3]) return OB_EJECT_FLOPPY;
    if (exec_dev && (has(exec_dev, "carddisk") || has(exec_dev, "compactflash") || has(exec_dev, "pcmcia")))
        return OB_EJECT_CARD;
    if (low(dev[0]) == 'c' && (low(dev[1]) == 'f' || low(dev[1]) == 'c') && dev[2] >= '0' && dev[2] <= '9' && !dev[3])
        return OB_EJECT_CARD;
    return OB_EJECT_FIXED;
}
