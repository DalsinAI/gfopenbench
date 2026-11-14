/* I4 and I7's judge (DRAGFILE.h), plain C for the host checks too. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "DRAGFILE.h"

long wbgap_dragfile_width(long screen_w)
{
    long ww = screen_w > 0 ? screen_w / 2 : 0;
    if (ww > WBGAP_DRAGFILE_WW) ww = WBGAP_DRAGFILE_WW;
    return ww < WBGAP_DRAGFILE_MINW ? 0 : ww;
}
