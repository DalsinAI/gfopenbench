/* SCREENS' judges (SCREENS.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "wbgap_rows.h"
#include "SCREENS.h"

void wbgap_depth_point(int relright, long left, long top, long w, long h, long screen_w, long xy[2])
{
    xy[0] = (relright ? screen_w - 1 + left : left) + w / 2;
    xy[1] = top + h / 2;
}

int wbgap_pointer_at(long mx, long my, long tx, long ty)
{
    return mx >= tx - 1 && mx <= tx + 1 && my >= ty - 1 && my <= ty + 1;
}

int wbgap_pointer_layer(unsigned long mode, unsigned long away1, unsigned long over, unsigned long away2, char *why,
                        unsigned cap)
{
    char r[80] = "";
    int v = WBGAP_SKIP;
    if (mode == WBGAP_NO_MODE || (mode & 0xFFFF1000UL) < 0x00100000UL)
        snprintf(r, sizeof r, "Workbench is not on an RTG mode (the chipset's sprites)");
    else if (away1 != away2) snprintf(r, sizeof r, "the screen changed under the square by itself; run again");
    else if (over != away1) { snprintf(r, sizeof r, "the pointer is drawn into the screen's pixels, not a sprite layer"); v = WBGAP_FAIL; }
    else v = WBGAP_PASS;
    if (why && cap) snprintf(why, cap, "%s", r);
    return v;
}
