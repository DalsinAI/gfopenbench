/* wbgap_index: reading the ACFS index's "::wb" answers (gfosmotherboards #140, Workbench build row 8) for
 * WBGapIndex. Plain C99, no Amiga headers: tests/host/test_wbgap_index.c runs it on the host.
 *
 * A "WB" hit's line: added=NS trash=live|trash|final trashed=NS final=NS reclaim=NS compat=NAME
 *                    sort=on|off:VALUE show=on|off:VALUE arrange=on|off:VALUE
 * A value runs to the next " key=" of those keys, so a compat name may hold spaces.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef WBGAP_INDEX_H
#define WBGAP_INDEX_H

/* the value of key in line, into out (cut to n - 1): 1 if the key is there */
int wbgap_wb_field(const char *line, const char *key, char *out, int n);

/* a setting's value, "on:NAME,REVERSE", "off:NAME" or "off": *on 1 or 0, the value after the colon into val;
 * 0 if it is neither on nor off */
int wbgap_wb_setting(const char *value, int *on, char *val, int n);

/* the compat name for a long name, as the decision asks (X2): 107 characters at most, ending in "~" and 16 hex
 * digits; 1 if name is that */
int wbgap_compat_ok(const char *name);

/* "NS NAME" (an ORDER hit): the name part, or NULL */
const char *wbgap_order_name(const char *line);

/* case-insensitive equality, for the settings' values */
int wbgap_same(const char *a, const char *b);

#endif
