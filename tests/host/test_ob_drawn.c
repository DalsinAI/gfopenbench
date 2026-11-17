/* ob_drawn's host test: the wait GFOpenMeters (and GFOpenTitle, through gfopenprefs Common/obdrawn.h) does while
 * GFOpenBench 0.1.71 says its desktop isn't drawn yet (Dale, the Pi 5, 17 November 2026: the meters and the tray
 * tore while GFOpenBench was starting). A GFOpenBench start, a Workbench reset, no GFOpenBench, an older one, a
 * picture that never comes, and the clock wrapping.
 *   test_ob_drawn
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#define OB_DRAWN_HOST
#include "ob_drawn.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

/* steps of 100 ms (GFOpenMeters' pace while it waits) until the step's answer isn't HOLD; how many it took */
static int wait_until(ob_hold *h, int bench, int drawn_at, unsigned int *now, int *answer)
{
    int n = 0, a;
    do {
        a = ob_hold_step(h, bench, drawn_at >= 0 && n >= drawn_at, *now);
        if (a != OBH_HOLD) break;
        *now += 100;
        n++;
    } while (n < 1000);
    *answer = a;
    return n;
}

int main(void)
{
    ob_hold h = { 0, 0, 0 };
    unsigned int now = 5000;
    int a, n, i, draws;

    /* no GFOpenBench (Workbench, or an older GFOpenBench with no port): no wait, ever, as before 0.4.1 */
    CHECK(ob_hold_step(&h, 0, 0, now) == OBH_GO, "no port: go at once");
    CHECK(ob_hold_step(&h, 0, 1, now) == OBH_GO, "no port (drawn read as 0 anyway): go");
    CHECK(!h.held, "no port: never held");

    /* GFOpenBench already drawn (the meters started by hand later): go at once */
    CHECK(ob_hold_step(&h, 1, 1, now) == OBH_GO, "drawn: go at once, no release to do");

    /* the start Dale saw: GFOpenBench up, its picture in 2.3 s; one hold, then one release, then go */
    n = wait_until(&h, 1, 23, &now, &a);
    CHECK(n == 23 && a == OBH_RELEASE, "start: held 23 looks then released (%d, %d)", n, a);
    CHECK(ob_hold_step(&h, 1, 1, now) == OBH_GO, "after the release: go, not a second release");

    /* exactly one draw over the whole start: the panel opens on RELEASE or GO only once while held before */
    h = (ob_hold){ 0, 0, 0 };
    draws = 0;
    for (i = 0; i < 60; i++) {
        int s = ob_hold_step(&h, 1, i >= 30, now + 100U * i);
        if (s == OBH_RELEASE) draws++;
    }
    CHECK(draws == 1, "one release over a 3 s start (%d)", draws);

    /* a Workbench reset: GFOpenBench says "not drawn" while it is down and coming up; held again, released again */
    CHECK(ob_hold_step(&h, 1, 0, now) == OBH_HOLD, "reset: held while the desktop is down");
    CHECK(ob_hold_step(&h, 1, 0, now + 500) == OBH_HOLD, "reset: still held");
    CHECK(ob_hold_step(&h, 1, 1, now + 900) == OBH_RELEASE, "reset: released once drawn again");

    /* GFOpenBench ends while something waits: released (no port, nothing to wait for) */
    h = (ob_hold){ 0, 0, 0 };
    CHECK(ob_hold_step(&h, 1, 0, now) == OBH_HOLD, "held");
    CHECK(ob_hold_step(&h, 0, 0, now + 100) == OBH_RELEASE, "GFOpenBench gone: released");

    /* a picture that never comes: never more than OB_DRAWN_CAP_MS, then go; no second wait until it is drawn */
    h = (ob_hold){ 0, 0, 0 };
    n = wait_until(&h, 1, -1, &now, &a);
    CHECK(a == OBH_RELEASE && n == (int)(OB_DRAWN_CAP_MS / 100), "never drawn: released at the cap (%d looks)", n);
    CHECK(ob_hold_step(&h, 1, 0, now + 100) == OBH_GO, "after the cap: go, not held again");
    CHECK(ob_hold_step(&h, 1, 0, now + 60000) == OBH_GO, "after the cap, a minute on: still go");
    CHECK(ob_hold_step(&h, 1, 1, now + 60100) == OBH_GO, "drawn at last: go (nothing held)");
    CHECK(ob_hold_step(&h, 1, 0, now + 60200) == OBH_HOLD, "a later reset waits again (the cap is per wait)");

    /* the clock wraps (GFOpenTitle's milliseconds from CurrentTime): the wait still ends at the cap, not at once */
    h = (ob_hold){ 0, 0, 0 };
    now = 0xFFFFFFFFU - 300;
    CHECK(ob_hold_step(&h, 1, 0, now) == OBH_HOLD, "wrap: held");
    CHECK(ob_hold_step(&h, 1, 0, now + 1000) == OBH_HOLD, "wrap: still held 1 s on, across the wrap");
    CHECK(ob_hold_step(&h, 1, 0, now + OB_DRAWN_CAP_MS) == OBH_RELEASE, "wrap: released at the cap");

    printf("ob_drawn: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
