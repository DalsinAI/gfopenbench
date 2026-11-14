/* ob_wbprefs: see ob_wbprefs.h.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "ob_wbprefs.h"

static unsigned long be32(const unsigned char *b)
{
    return ((unsigned long)b[0] << 24) | ((unsigned long)b[1] << 16) | ((unsigned long)b[2] << 8) | b[3];
}

int ob_wbprefs_parse(const unsigned char *buf, long len, ob_wbprefs *p)
{
    long i;
    memset(p, 0, sizeof *p);
    if (!buf || len < 12 || be32(buf) != 0x464F524DUL || be32(buf + 8) != 0x50524546UL) return 0;   /* FORM, PREF */
    for (i = 12; i + 8 <= len;) {
        unsigned long id = be32(buf + i), sz = be32(buf + i + 4);
        const unsigned char *d = buf + i + 8;
        if (sz > (unsigned long)(len - i - 8)) break;
        if (id == 0x57424E43UL && sz >= 4)                                      /* WBNC: wbp_DefaultStackSize first */
            p->stack = be32(d);
        else if (id == 0x57424844UL && sz > 1 && p->nhidden < OB_WBP_MAX_HIDDEN) {   /* WBHD: a device's name */
            int k = 0;
            while (k < (int)sz && k < OB_WBP_NAME - 1 && d[k] && d[k] != ':') {
                p->hidden[p->nhidden][k] = (char)d[k];
                k++;
            }
            p->hidden[p->nhidden][k] = 0;
            if (k) p->nhidden++;
        }
        i += 8 + (long)sz + (long)(sz & 1);
    }
    return 1;
}

unsigned long ob_wbprefs_min_stack(const ob_wbprefs *p)
{
    return p && p->stack > 8192 && p->stack <= OB_WBP_STACK_MAX ? p->stack : 8192;
}

unsigned long ob_wbprefs_lib_stack(const ob_wbprefs *p)
{
    return p && p->stack >= 4096 && p->stack <= OB_WBP_STACK_MAX ? p->stack : 0;
}
