/* WBGapTest's window probe (inside_sum): how far apart the pixels it reads are, so one look at a window costs about
 * the same at any size. Every second pixel as before up to WBG_PROBE_POINTS of them (a drawer's own window), a wider
 * grid past that (a drawer fitted to the screen: 1914 x 1056 read every second pixel was 505,000 ReadRGBPixel calls,
 * 3.9 s a look, Hudzen's probe 14 Nov 2026, so X1 measured its own polls). Plain C, host-tested
 * (tests/host/test_wbgap_probe.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_PROBE_H
#define WBGAP_PROBE_H

#define WBG_PROBE_POINTS 25000L                 /* about 0.2 s of ReadRGBPixel on the lab's RTG screen */

/* the step for an inside w x h: 2, or the least step whose grid has at most WBG_PROBE_POINTS points */
static int wbg_probe_step(long w, long h)
{
    int s = 2;
    if (w <= 0 || h <= 0) return s;
    while (((w + s - 1) / s) * ((h + s - 1) / s) > WBG_PROBE_POINTS) s++;
    return s;
}

#endif
