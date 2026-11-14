/* W1: a drawer window's size, zoom and depth gadgets (tests/amiga/rows/WINGAD_judge.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <string.h>

#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "WINGAD.h"

void wingad_plan(void);
WBGAP_PLAN(wingad_plan)

static struct wbgap_wingad wg_good(void)
{
    struct wbgap_wingad m;
    memset(&m, 0, sizeof m);
    m.has_size = m.has_zoom = m.has_depth = 1;
    m.size0[0] = 400; m.size0[1] = 300; m.size1[0] = 480; m.size1[1] = 360;
    m.zoom0[0] = 480; m.zoom0[1] = 360; m.zoom1[0] = 200; m.zoom1[1] = 100; m.zoom2[0] = 480; m.zoom2[1] = 361;
    m.front0 = 0; m.front1 = 1; m.front2 = 0;
    return m;
}

void wingad_plan(void)
{
    char why[240];
    long xy[2];
    int i = wbgap_find("W1");
    struct wbgap_wingad m = wg_good();
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_PASS && !why[0], "all three work: PASS");
    m.size1[0] = 470; m.size1[1] = 350;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_PASS, "grown by 70,50 (snapped): still PASS");
    m.size1[0] = 400; m.size1[1] = 300;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "size:") && !strstr(why, "zoom:"),
          "not grown: FAIL on size alone, \"%s\"", why);
    m = wg_good(); m.has_size = 0;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "no size gadget"), "no size gadget: FAIL");
    m = wg_good(); m.zoom1[0] = m.zoom0[0]; m.zoom1[1] = m.zoom0[1];
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't change"),
          "zoom did nothing: FAIL, \"%s\"", why);
    m = wg_good(); m.zoom2[0] = 200;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "give the size back"),
          "zoom didn't come back: FAIL, \"%s\"", why);
    m = wg_good(); m.zoom2[0] = m.zoom0[0] + 3;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL, "3 pixels off after zoom twice: FAIL");
    m = wg_good(); m.has_zoom = 0;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "no zoom gadget"), "no zoom gadget: FAIL");
    m = wg_good(); m.has_zoom = 0; m.zoom_off = 1;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_PASS && !why[0], "no zoom gadget, the look says zoom off: PASS");
    m = wg_good(); m.has_zoom = 0; m.zoom_off = 1; m.has_depth = 0;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && !strstr(why, "zoom:") && strstr(why, "depth:"),
          "zoom off excuses zoom only: \"%s\"", why);
    m = wg_good(); m.zoom_off = 1; m.zoom1[0] = m.zoom0[0]; m.zoom1[1] = m.zoom0[1];
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "didn't change"),
          "a zoom gadget there that does nothing: FAIL even with zoom off");
    CHECK(wbgap_wingad_zoom_off("Sky light\nzoom off\nscrollers auto") == 1, "GFOpenUp's look: zoom off");
    CHECK(wbgap_wingad_zoom_off("Sky light\r\n  ZOOM\tOff \r\n") == 1, "any case, tabs, spaces, CR: zoom off");
    CHECK(wbgap_wingad_zoom_off("zoom off") == 1, "the last line with no newline");
    CHECK(wbgap_wingad_zoom_off("Classic light\nscrollers auto\n") == 0, "Classic: no zoom line, 0");
    CHECK(wbgap_wingad_zoom_off("zoom on\nzoomoff\nzoom offline\n# zoom off\n") == 0, "zoom on, zoomoff, offline, a comment: 0");
    CHECK(wbgap_wingad_zoom_off("Open light\nscreen workbench classic\nzoom off\n") == 0,
          "zoom off but the Workbench screen Classic: its windows keep zoom, 0");
    CHECK(wbgap_wingad_zoom_off("Open light\nscreen workbench themed\nscreen custom classic\nzoom off\n") == 1,
          "only custom screens Classic: the Workbench's windows have no zoom, 1");
    CHECK(wbgap_wingad_zoom_off("zoom off\nScreen  Workbench\tCLASSIC \r\n") == 0, "screen workbench classic, any case and spacing");
    CHECK(wbgap_wingad_zoom_off("zoom off\nscreen workbench classical\n") == 1, "classical is not classic");
    CHECK(wbgap_wingad_zoom_off(NULL) == 0 && wbgap_wingad_zoom_off("") == 0, "no look: 0");
    m = wg_good(); m.front1 = 0;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "to the front"),
          "depth didn't bring it front: FAIL, \"%s\"", why);
    m = wg_good(); m.front2 = 1;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "behind"), "depth didn't send it back: FAIL");
    m = wg_good(); m.front0 = 1;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "cover"), "not covered to start: FAIL");
    m = wg_good(); m.front1 = -1;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "not read"), "order not read: FAIL");
    m = wg_good(); m.has_depth = 0; m.has_size = 0;
    CHECK(wbgap_wingad_judge(&m, why, sizeof why) == WBGAP_FAIL && strstr(why, "size:") && strstr(why, "depth:"),
          "two parts wrong: both named, \"%s\"", why);
    CHECK(wbgap_wingad_judge(NULL, why, sizeof why) == WBGAP_FAIL, "nothing measured: FAIL");
    m = wg_good();
    CHECK(wbgap_wingad_judge(&m, NULL, 0) == WBGAP_PASS, "no room for a reason: still judged");
    wbgap_gadget_middle(-17, -9, 18, 10, 1, 1, 0, 0, 400, 300, xy);
    CHECK(xy[0] == 391 && xy[1] == 295, "the size gadget at the bottom right: %ld,%ld", xy[0], xy[1]);
    wbgap_gadget_middle(-45, 0, 23, 11, 1, 0, 0, 0, 400, 300, xy);
    CHECK(xy[0] == 365 && xy[1] == 5, "a gadget relative to the right edge: %ld,%ld", xy[0], xy[1]);
    wbgap_gadget_middle(10, 4, -40, 6, 0, 0, 1, 0, 400, 300, xy);
    CHECK(xy[0] == 190 && xy[1] == 7, "a width relative to the window's: %ld,%ld", xy[0], xy[1]);
    CHECK(i >= 0 && wbgap_row(i)->mode && !strcmp(wbgap_row(i)->mode, "WINGAD"), "W1's line comes from WINGAD");
}
