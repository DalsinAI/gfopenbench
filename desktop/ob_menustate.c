/* ob_menustate: which of the desktop's menu items can act now (ob_menustate.h).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_menustate.h"

int ob_ms_enabled(const ob_ms_facts *f, int need)
{
    if (!f)
        return 1;
    switch (need) {
    case OB_MS_SEL:       return f->sel > 0;
    case OB_MS_SEL_FILE:  return f->sel_file > 0;
    case OB_MS_DISK:      return f->disks > 0;
    case OB_MS_LOUT:      return f->louts > 0;
    case OB_MS_LOUT_TEMP: return f->louts_temp > 0;
    case OB_MS_RECOVER:   return f->recover > 0;
    case OB_MS_PUTBACK:   return f->recover_open != 0;
    case OB_MS_SHOWN:     return f->shown > 0;
    case OB_MS_FILES:     return f->files > 0;
    case OB_MS_ANY_DISK:  return f->all_disks > 0;
    default:              return 1;
    }
}
