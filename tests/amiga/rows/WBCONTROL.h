/* A5 (WBCONTROL): a WorkbenchControlA value is set (and put back) only if its Get worked, else it would be left
 * changed; the reading after the Set is then WBGAP_CTL_NOT_READ, which no wanted value is.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_CTL_NOT_READ (-0x7ffffffeL)
int wbgap_ctl_may_set(int get_ok);
long wbgap_ctl_reading(int get_ok, long read_back);
