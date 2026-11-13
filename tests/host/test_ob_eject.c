/* ob_eject's host test: which way Icons > Eject takes a selected disk (row M35: DF, USB, PCMCIA).
 *   test_ob_eject
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>

#include "ob_eject.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

int main(void)
{
    CHECK(ob_eject_kind("DF0", "trackdisk.device") == OB_EJECT_FLOPPY);
    CHECK(ob_eject_kind("df3", 0) == OB_EJECT_FLOPPY);
    CHECK(ob_eject_kind("DF4", "trackdisk.device") == OB_EJECT_FIXED);     /* there are four floppy drives */
    CHECK(ob_eject_kind("DF0X", 0) == OB_EJECT_FIXED);
    CHECK(ob_eject_kind("CF0", "compactflash.device") == OB_EJECT_CARD);
    CHECK(ob_eject_kind("CC0", "carddisk.device") == OB_EJECT_CARD);
    CHECK(ob_eject_kind("PC0", "CardDisk.device") == OB_EJECT_CARD);       /* by its exec device, any case */
    CHECK(ob_eject_kind("CF1", 0) == OB_EJECT_CARD);                       /* by its name when the device is unknown */
    CHECK(ob_eject_kind("MYCARD", "pcmcia_ata.device") == OB_EJECT_CARD);
    CHECK(ob_eject_kind("DH0", "acscsi.device") == OB_EJECT_FIXED);        /* the IDE disk: an acscsi unit */
    CHECK(ob_eject_kind("RAM", 0) == OB_EJECT_FIXED);
    CHECK(ob_eject_kind("CFX", "scsi.device") == OB_EJECT_FIXED);
    CHECK(ob_eject_kind(0, 0) == OB_EJECT_FIXED);
    CHECK(ob_eject_kind("", "") == OB_EJECT_FIXED);
    printf("test_ob_eject: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
