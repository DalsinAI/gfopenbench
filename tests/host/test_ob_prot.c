/* ob_prot's host test: Hold and Pure stay in Advanced (gap N7).
 *   test_ob_prot
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "ob_prot.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

int main(void)
{
    static const char *const main_row[] = {
        "Script", "Archived", "Readable", "Writable", "Executable", "Deletable", 0
    };
    long p;
    int i;

    CHECK(ob_prot_is_advanced("Hold") && ob_prot_is_advanced("Pure"));
    CHECK(!ob_prot_is_advanced("Script") && !ob_prot_is_advanced("Archived"));
    CHECK(!ob_prot_is_advanced("Readable") && !ob_prot_is_advanced(0));
    CHECK(!ob_prot_is_main_row("Hold") && !ob_prot_is_main_row("Pure"));
    CHECK(!ob_prot_is_main_row(0) && !ob_prot_is_main_row("Advanced"));
    for (i = 0; main_row[i]; i++)
        CHECK(ob_prot_is_main_row(main_row[i]) && !ob_prot_is_advanced(main_row[i]));

    CHECK(ob_prot_hold_on(OB_PROT_HOLD) && !ob_prot_hold_on(0));
    CHECK(ob_prot_pure_on(OB_PROT_PURE) && !ob_prot_pure_on(OB_PROT_HOLD));
    CHECK(ob_prot_hold_on(0x180) && ob_prot_pure_on(0x20 | 0x100));

    /* A file that is readable and held, nothing else allowed, group bits kept. */
    p = ob_prot_apply(0x1100L, 0, 0, 1, 0, 0, 0, 1, 0);
    CHECK(p == (0x1100L | OB_PROT_HOLD | OB_PROT_WRITE | OB_PROT_EXECUTE | OB_PROT_DELETE));
    CHECK(ob_prot_hold_on(p) && !ob_prot_pure_on(p));

    /* Clearing Hold and setting Pure does not touch the six or the high bits. */
    p = ob_prot_apply(p, 0, 0, 1, 0, 0, 0, 0, 1);
    CHECK(!ob_prot_hold_on(p) && ob_prot_pure_on(p));
    CHECK((p & 0x1100L) == 0x1100L);
    CHECK((p & OB_PROT_READ) == 0);
    CHECK((p & (OB_PROT_WRITE | OB_PROT_EXECUTE | OB_PROT_DELETE)) ==
          (OB_PROT_WRITE | OB_PROT_EXECUTE | OB_PROT_DELETE));

    /* All six allowed, plus Script, Archived, Hold and Pure. */
    p = ob_prot_apply(0, 1, 1, 1, 1, 1, 1, 1, 1);
    CHECK(p == (OB_PROT_HOLD | OB_PROT_SCRIPT | OB_PROT_PURE | OB_PROT_ARCHIVE));

    /* None allowed and neither advanced bit: the four inverted bits, nothing else. */
    p = ob_prot_apply(0xFFL, 0, 0, 0, 0, 0, 0, 0, 0);
    CHECK(p == (OB_PROT_READ | OB_PROT_WRITE | OB_PROT_EXECUTE | OB_PROT_DELETE));

    printf("test_ob_prot: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
