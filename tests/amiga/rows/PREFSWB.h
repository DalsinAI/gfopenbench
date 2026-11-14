/* P7 (PREFSWB): Workbench prefs as Prefs/Workbench's Use leaves them (ENV:Sys/Workbench.prefs, an IFF FORM PREF), a
 * device hidden and the default stack set; the desktop and workbench.library must follow. Plain C, for the host checks
 * too. MIT, Copyright (c) 2026 AmiCradle Team. */
/* a Workbench.prefs made from orig (len bytes; NULL, or not a FORM PREF: one made afresh, as mkprefs.py's) with WBNC's
 * default stack (its first long word) set to stack, and a WBHD chunk for hide added (hide NULL, or already hidden:
 * none added); its length, 0 if cap is too small */
long wbgap_wbprefs_make(const unsigned char *orig, long len, unsigned long stack, const char *hide, unsigned char *out,
                        long cap);
/* WBNC's default stack in a Workbench.prefs, 0 if there is none */
unsigned long wbgap_wbprefs_stack(const unsigned char *buf, long len);
/* whether a WBHD chunk names dev, case and a ':' at its end aside */
int wbgap_wbprefs_hides(const unsigned char *buf, long len, const char *dev);
/* P7: gone, the hidden device's disk icon after the change (1 gone, 0 still there, -1 not read); got, the default stack
 * workbench.library gives after it (want: the prefs'); back, the icon after the old prefs (1, 0, -1 not read) */
int wbgap_wbprefs_judge(int gone, unsigned long got, unsigned long want, int back, char *why, unsigned cap);
