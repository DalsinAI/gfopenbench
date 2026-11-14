/* BAR (B1 B2): GFOpenTitle's Power cell and its list, judged. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
/* The system bar (gfopenprefs TitleBar, GFOpenTitle 0.9): its row window on the title bar holds, left to right, the
 * memory and time, the systray, then Prefs and Power, one cell each (Common/traygrid.h's TRAY_CELL). Power's is the
 * row's last cell: the point (x, y) to click it, from the row's box */
#define WBGAP_TRAY_CELL 30
void wbgap_power_point(long left, long top, long width, long height, long xy[2]);
/* whose list opened, from where it hangs: Power's ends at the row's right edge, Prefs' one cell in from it (a list
 * pushed to x 0 by a narrow screen can't be told: UNSURE) */
enum { WBGAP_BAR_POWER, WBGAP_BAR_PREFS, WBGAP_BAR_OTHER, WBGAP_BAR_UNSURE };
int wbgap_bar_menu_owner(long row_right, long menu_left, long menu_width);
/* B1: 1 if (x, y) is inside the box l, t, w x h: the Power click's point under the list it opened */
int wbgap_point_in_box(long x, long y, long l, long t, long w, long h);
/* Power's list (GFOpenTitle's menu_open, MK_POWER): 3 pixels, an "Eject <volume>" row for each USB stick mounted
 * (a DOS device on gfopenusbacfs.device or usbdisk.device with a handler running) and a 6-pixel line after them,
 * then Reboot, Restart the machine... and Power off..., then 3 pixels; a row is the bar font's height + 4 */
long wbgap_power_height(int sticks, long row_height);
/* B1 (the three power rows) and B2 (an Eject row for each stick) from the list's height: WBGAP_PASS, WBGAP_FAIL or
 * WBGAP_SKIP (B2 with no stick mounted) */
void wbgap_power_judge(long height, int sticks, long row_height, int *b1, int *b2);
