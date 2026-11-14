/* BAR's judges (BAR.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "BAR.h"

void wbgap_power_point(long left, long top, long width, long height, long xy[2])
{
    long cell = width < WBGAP_TRAY_CELL ? width : WBGAP_TRAY_CELL;
    xy[0] = left + width - (cell + 1) / 2;
    xy[1] = top + height / 2;
}

int wbgap_bar_menu_owner(long row_right, long menu_left, long menu_width)
{
    long right = menu_left + menu_width;
    if (right == row_right) return WBGAP_BAR_POWER;
    if (right == row_right - WBGAP_TRAY_CELL) return WBGAP_BAR_PREFS;
    if (menu_left == 0 && right < row_right) return WBGAP_BAR_UNSURE;   /* x clamped to 0: Power or Prefs */
    return WBGAP_BAR_OTHER;
}

int wbgap_point_in_box(long x, long y, long l, long t, long w, long h)
{
    return x >= l && x < l + w && y >= t && y < t + h;
}

long wbgap_power_height(int sticks, long row_height)
{
    if (sticks < 0) sticks = 0;
    return 3 + sticks * row_height + (sticks ? 6 : 0) + 3 * row_height + 3;
}

void wbgap_power_judge(long height, int sticks, long row_height, int *b1, int *b2)
{
    if (row_height > 0 && height == wbgap_power_height(sticks, row_height)) {
        *b1 = WBGAP_PASS;
        *b2 = sticks > 0 ? WBGAP_PASS : WBGAP_SKIP;
    } else if (row_height > 0 && sticks > 0 && height == wbgap_power_height(0, row_height)) {
        *b1 = WBGAP_PASS;   /* the power rows are there; the sticks' Eject rows aren't */
        *b2 = WBGAP_FAIL;
    } else {
        *b1 = WBGAP_FAIL;
        *b2 = sticks > 0 ? WBGAP_FAIL : WBGAP_SKIP;
    }
}
