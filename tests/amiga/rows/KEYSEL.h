/* I16 (KEYSEL): keyboard selection in a drawer, as the design has it (designs/WORKBENCH-REDESIGN-20261113.md):
 * the cursor keys move the selection (Shift adds), Tab and Shift-Tab cycle. Three icons a, b and c in one row, left
 * to right; each step a click on one, or a key, and the selection it must leave. Plain C, for the host checks too.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef KEYSEL_H
#define KEYSEL_H

#define WBGAP_RAW_TAB 0x42
#define WBGAP_RAW_UP 0x4C
#define WBGAP_RAW_DOWN 0x4D
#define WBGAP_RAW_RIGHT 0x4E
#define WBGAP_RAW_LEFT 0x4F

struct wbgap_keysel_step {
    const char *what;   /* for the INFO and FAIL lines */
    char click;         /* 'a', 'b' or 'c': a click on that icon; 0: a key */
    int key;            /* the raw key, when click is 0 */
    int shift;          /* the key with Left Shift */
    const char *want;   /* the names selected after it, blank-separated */
};

int wbgap_keysel_count(void);
const struct wbgap_keysel_step *wbgap_keysel_step(int i);
#endif
