/* Host test for tests/amiga/wbgap_probe.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include "wbgap_probe.h"

static int fails, checks;
#define CHECK(c, m) do { checks++; if (!(c)) { fails++; printf("FAIL %s\n", m); } } while (0)

static long points(long w, long h, int s) { return ((w + s - 1) / s) * ((h + s - 1) / s); }

int main(void)
{
    int s;
    CHECK(wbg_probe_step(390, 95) == 2, "a drawer's own window: every second pixel, as before");
    CHECK(wbg_probe_step(0, 100) == 2 && wbg_probe_step(100, -1) == 2, "no inside: 2");
    s = wbg_probe_step(1914, 1056);
    CHECK(s > 2 && points(1914, 1056, s) <= WBG_PROBE_POINTS, "the fitted 1914 x 1056 window: within the budget");
    CHECK(points(1914, 1056, s - 1) > WBG_PROBE_POINTS, "and the least step that is");
    CHECK(s <= 12, "a 1914-wide grid still lands in every icon (12 px or less apart)");
    s = wbg_probe_step(4000, 3000);
    CHECK(points(4000, 3000, s) <= WBG_PROBE_POINTS, "a 4K screen: within the budget");
    CHECK(points(312, 160, wbg_probe_step(312, 160)) == points(312, 160, 2), "a small window: the same pixels as before");
    printf("%s   %d checks (WBGapTest's window probe)\n", fails ? "FAIL" : "ok", checks);
    return fails != 0;
}
