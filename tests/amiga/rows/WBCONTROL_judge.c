/* A5's judge (WBCONTROL.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "WBCONTROL.h"

int wbgap_ctl_may_set(int get_ok) { return get_ok != 0; }

long wbgap_ctl_reading(int get_ok, long read_back) { return get_ok ? read_back : WBGAP_CTL_NOT_READ; }
