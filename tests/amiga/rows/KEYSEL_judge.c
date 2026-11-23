/* KEYSEL's steps (KEYSEL.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "KEYSEL.h"

static const struct wbgap_keysel_step steps[] = {
    {"a click on a", 'a', 0, 0, "a"},
    {"Cursor Right", 0, WBGAP_RAW_RIGHT, 0, "b"},
    {"Shift-Cursor Right", 0, WBGAP_RAW_RIGHT, 1, "b c"},
    {"a click on c", 'c', 0, 0, "c"},
    {"Cursor Left", 0, WBGAP_RAW_LEFT, 0, "b"},
    {"a click on a", 'a', 0, 0, "a"},
    {"Tab", 0, WBGAP_RAW_TAB, 0, "b"},
    {"Tab", 0, WBGAP_RAW_TAB, 0, "c"},
    {"Shift-Tab", 0, WBGAP_RAW_TAB, 1, "b"},
};

int wbgap_keysel_count(void) { return (int)(sizeof steps / sizeof steps[0]); }

const struct wbgap_keysel_step *wbgap_keysel_step(int i)
{
    return i >= 0 && i < wbgap_keysel_count() ? &steps[i] : 0;
}
