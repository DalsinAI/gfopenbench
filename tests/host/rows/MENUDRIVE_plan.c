/* M8: the drawer New Drawer made (tests/amiga/rows/MENUDRIVE_judge.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#include "wbgap_rows.h"
#include "wbgap_check.h"
#include "MENUDRIVE.h"

void menudrive_plan(void);
WBGAP_PLAN(menudrive_plan)

void menudrive_plan(void)
{
    static const char before[3][32] = {"a", "Sub", "Unnamed1"};
    static const char same[3][32] = {"Sub", "Unnamed1", "a"};
    static const char one_new[4][32] = {"a", "Sub", "Unnamed2", "Unnamed1"};
    static const char cased[3][32] = {"a", "sub", "Unnamed1"};
    CHECK(wbgap_menudrive_new_dir(before, 3, same, 3) == -1, "the same names in another order: none new");
    CHECK(wbgap_menudrive_new_dir(before, 3, one_new, 4) == 2, "Unnamed2 among them: its index");
    CHECK(wbgap_menudrive_new_dir(before, 0, before, 3) == 0, "nothing before: the first is new");
    CHECK(wbgap_menudrive_new_dir(before, 3, before, 0) == -1, "nothing after: none new");
    CHECK(wbgap_menudrive_new_dir(before, 3, cased, 3) == 1, "names compared as they are: sub is not Sub");
    CHECK(wbgap_menudrive_new_dir(one_new, 4, before, 3) == -1, "one gone, none made: none new");
}
