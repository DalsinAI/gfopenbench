/* INFOSAVE's judges (INFOSAVE.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_rows.h"
#include "INFOSAVE.h"

int wbgap_infosave_prot(long prot, char *why, unsigned cap)
{
    int ok = (prot & WBGAP_FIBF_DELETE) && !(prot & (WBGAP_FIBF_READ | WBGAP_FIBF_WRITE | WBGAP_FIBF_EXECUTE));
    if (why && cap) {
        if (ok) why[0] = 0;
        else snprintf(why, cap, "protection 0x%lx after Save, not Deletable off with R, W and E on", (unsigned long)prot);
    }
    return ok ? WBGAP_PASS : WBGAP_FAIL;
}

int wbgap_infosave_same(const char *what, const char *got, const char *want, char *why, unsigned cap)
{
    int ok = got && !strcmp(got, want);
    if (why && cap) {
        if (ok) why[0] = 0;
        else snprintf(why, cap, "the %s \"%s\" after Save, not \"%s\"", what, got ? got : "(none)", want);
    }
    return ok ? WBGAP_PASS : WBGAP_FAIL;
}

int wbgap_infosave_stack(long got, long want, char *why, unsigned cap)
{
    int ok = got == want;
    if (why && cap) {
        if (ok) why[0] = 0;
        else snprintf(why, cap, "the icon's stack %ld after Save, not %ld", got, want);
    }
    return ok ? WBGAP_PASS : WBGAP_FAIL;
}

int wbgap_infosave_hp(long before, long after, char *why, unsigned cap)
{
    const long hp = WBGAP_FIBF_HOLD | WBGAP_FIBF_PURE;
    int both = (after & hp) == hp, rest = (after & ~hp) == (before & ~hp);
    if (why && cap) {
        if (both && rest) why[0] = 0;
        else if (!both)
            snprintf(why, cap, "protection 0x%lx after Save: %s", (unsigned long)after,
                     !(after & WBGAP_FIBF_HOLD) && !(after & WBGAP_FIBF_PURE) ? "neither Hold nor Pure set"
                     : !(after & WBGAP_FIBF_HOLD) ? "Hold not set" : "Pure not set");
        else snprintf(why, cap, "protection 0x%lx after Save, was 0x%lx: other bits changed too", (unsigned long)after,
                      (unsigned long)before);
    }
    return both && rest ? WBGAP_PASS : WBGAP_FAIL;
}

void wbgap_join_tooltypes(const char *const *tt, char *out, unsigned cap)
{
    size_t n = 0;
    int i;
    if (!out || !cap) return;
    out[0] = 0;
    for (i = 0; tt && tt[i]; i++) {
        int k = snprintf(out + n, cap - n, "%s%s", i ? "," : "", tt[i]);
        if (k < 0 || (size_t)k >= cap - n) { out[cap - 1] = 0; return; }
        n += (size_t)k;
    }
}
