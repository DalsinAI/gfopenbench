/* F1 and F2's window size (NOTIFY.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "NOTIFY.h"

int wbgap_notify_size(long w, long h, long size[2])
{
    size[0] = w < WBGAP_NOTIFY_W ? WBGAP_NOTIFY_W : w;
    size[1] = h < WBGAP_NOTIFY_H ? WBGAP_NOTIFY_H : h;
    return w < WBGAP_NOTIFY_W || h < WBGAP_NOTIFY_H;
}
