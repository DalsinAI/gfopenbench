/* I8 and I9's judges (APPDROP.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "wbgap_rows.h"
#include "APPDROP.h"

int wbgap_drop_judge(int got, long type, long want_type, unsigned long id, unsigned long want_id, long nargs,
                     const char *name, const char *dir, const char *want_name, const char *want_dir, char *why,
                     unsigned cap)
{
    size_t dn = dir ? strlen(dir) : 0, wn = want_dir ? strlen(want_dir) : 0;
    if (!got) {
        snprintf(why, cap, "no AppMessage came");
        return WBGAP_FAIL;
    }
    if (type != want_type || id != want_id) {
        snprintf(why, cap, "an AppMessage of type %ld id %lu, not type %ld id %lu", type, id, want_type, want_id);
        return WBGAP_FAIL;
    }
    if (nargs < 1) {
        snprintf(why, cap, "the AppMessage has no arguments (am_NumArgs %ld)", nargs);
        return WBGAP_FAIL;
    }
    if (!name || !want_name || strcmp(name, want_name)) {
        snprintf(why, cap, "its first argument is \"%s\", not \"%s\"", name ? name : "", want_name ? want_name : "");
        return WBGAP_FAIL;
    }
    if (!wn || dn <= wn || strcmp(dir + dn - wn, want_dir) || (dir[dn - wn - 1] != ':' && dir[dn - wn - 1] != '/')) {
        snprintf(why, cap, "its first argument's drawer is \"%s\", not ...%s", dir ? dir : "", want_dir ? want_dir : "");
        return WBGAP_FAIL;
    }
    if (cap) why[0] = 0;
    return WBGAP_PASS;
}

int wbgap_parse_box(const char *text, long box[4])
{
    long b[4];
    char *e;
    const char *p = text;
    int k;
    if (!p) return 0;
    for (k = 0; k < 4; k++) {
        b[k] = strtol(p, &e, 10);
        if (e == p) return 0;
        p = e;
    }
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') p++;
    if (*p || b[2] < 1 || b[3] < 1) return 0;
    memcpy(box, b, sizeof b);
    return 1;
}
