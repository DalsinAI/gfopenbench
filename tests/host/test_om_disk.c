/* om_disk's host test: the host disk a drive shares, in GFOpenMeters' volume bars (0.4). The drives come from the
 * status' "volumes" list with their host disk's free space; one that has its disk to itself, one with a size of its
 * own, and a status cut short give none; the bar reads "81 GBytes (405 GBytes)".
 *   test_om_disk
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "om_disk.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

#define GB (1ULL << 30)
#define MB (1ULL << 20)

static void value(unsigned long long freeb, unsigned long long host, const char *want)
{
    char out[48];
    om_disk_value(out, sizeof out, freeb, host);
    CHECK(!strcmp(out, want), "%s, not %s", out, want);
}

int main(void)
{
    struct om_disk d[OM_DISK_MAX];
    const struct om_disk *f;
    char small[12];
    int n;

    /* the Pi 5: five drives on one disk, as Python's json.dumps writes it, with a drive of a set size between */
    static const char pi5[] =
        "{\"clock\": {\"zone\": \"Europe/London\"}, \"product\": \"AmiCradle\", \"instance\": {\"volumes\": \"no\"}, "
        "\"storageRoot\": \"/x\", \"volumes\": ["
        "{\"device\": \"DH0\", \"label\": \"System\", \"geometry\": {\"blockSize\": 4096, \"lbaSectors\": 22000000}, "
        "\"freeBytes\": 86973087744, \"provisioned\": \"auto\", \"os3Limit\": false, \"hostFreeBytes\": 434865438720, "
        "\"hostTotalBytes\": 491773755392, \"hostSharedBy\": 5, \"hidden\": false}, "
        "{\"device\": \"DH1\", \"label\": \"Work\", \"hostFreeBytes\": 434865438720, \"hostSharedBy\": 5}, "
        "{\"device\": \"DH5\", \"label\": \"Fixed\", \"provisioned\": 8589934592, \"freeBytes\": 100}, "
        "{\"device\": \"DH2\", \"label\": \"Games [1]\", \"tags\": [\"a\", {\"hostFreeBytes\": 1, \"hostSharedBy\": 9}], "
        "\"hostFreeBytes\": 434865438720, \"hostSharedBy\": 5}, "
        "{\"device\": \"DH6\", \"label\": \"Alone\", \"hostFreeBytes\": 1000000000, \"hostSharedBy\": 1}, "
        "{\"device\": \"DH7\", \"label\": \"Odd\", \"hostFreeBytes\": null, \"hostSharedBy\": 5}"
        "], \"usbDrives\": [{\"label\": \"Stick\", \"hostFreeBytes\": 5, \"hostSharedBy\": 3}]}";
    n = om_disk_read(pi5, d, OM_DISK_MAX);
    CHECK(n == 3, "%d drives, not 3", n);
    f = om_disk_find(d, n, "system");
    CHECK(f && f->host_free == 434865438720ULL && f->shared_by == 5, "System: not found, or not its disk");
    CHECK(om_disk_find(d, n, "WORK") != 0, "Work: not found");
    CHECK(om_disk_find(d, n, "Games [1]") != 0, "Games [1]: not found");
    CHECK(!om_disk_find(d, n, "Fixed"), "a drive of a set size has no host disk to show");
    CHECK(!om_disk_find(d, n, "Alone"), "a drive with its disk to itself has no brackets");
    CHECK(!om_disk_find(d, n, "Odd"), "a null is not a size");
    CHECK(!om_disk_find(d, n, "Stick"), "a list after \"volumes\" is not read");
    CHECK(!om_disk_find(d, n, "Sys"), "a name's start is not the name");
    CHECK(!om_disk_find(d, n, "System2"), "a longer name is not the name");
    CHECK(!om_disk_find(d, n, NULL) && !om_disk_find(NULL, 3, "System"), "nothing to find");

    /* compact, as the bridge may send it; no more than max are taken */
    n = om_disk_read("{\"volumes\":[{\"label\":\"A\",\"hostFreeBytes\":10,\"hostSharedBy\":2},"
                     "{\"label\":\"B\",\"hostFreeBytes\":10,\"hostSharedBy\":2}]}", d, 1);
    CHECK(n == 1 && !strcmp(d[0].label, "A"), "max 1: %d", n);

    /* cut short: in a number, in a name, before the list; and what is not a status */
    n = om_disk_read("{\"volumes\":[{\"label\":\"A\",\"hostSharedBy\":2,\"hostFreeBytes\":4348", d, OM_DISK_MAX);
    CHECK(n == 0, "a number cut short: %d", n);
    n = om_disk_read("{\"volumes\":[{\"hostFreeBytes\":10,\"hostSharedBy\":2,\"label\":\"Sys", d, OM_DISK_MAX);
    CHECK(n == 0, "a name cut short: %d", n);
    n = om_disk_read("{\"volumes\":[{\"label\":\"A\",\"hostFreeBytes\":10,\"hostSharedBy\":2},{\"label\":\"B\",\"hostFr", d, OM_DISK_MAX);
    CHECK(n == 1, "the drive before the cut: %d", n);
    CHECK(om_disk_read("{\"volumes\":", d, OM_DISK_MAX) == 0, "no list");
    CHECK(om_disk_read("{\"volumes\":{\"label\":\"A\",\"hostFreeBytes\":10,\"hostSharedBy\":2}}", d, OM_DISK_MAX) == 0, "not a list");
    CHECK(om_disk_read("{\"host\":{}}", d, OM_DISK_MAX) == 0, "an older host: no volumes' disks");
    CHECK(om_disk_read("", d, OM_DISK_MAX) == 0 && om_disk_read(NULL, d, OM_DISK_MAX) == 0, "nothing");
    /* an older host: volumes with none of the keys */
    CHECK(om_disk_read("{\"volumes\":[{\"label\":\"System\",\"freeBytes\":5}]}", d, OM_DISK_MAX) == 0, "no keys");

    /* the bar's text */
    value(81 * GB, 405 * GB, "81 GBytes (405 GBytes)");
    value(81 * GB, 0, "81 GBytes");                          /* no host disk told */
    value(405 * GB, 405 * GB, "405 GBytes");                 /* the share is all of it */
    value(405 * GB, 405 * GB + 5 * MB, "405 GBytes");        /* more, but written the same */
    value(405 * GB, 81 * GB, "405 GBytes");                  /* never less in the brackets */
    value(1371 * MB, 6855 * MB, "1,371 MBytes (6,855 MBytes)");
    value(9 * GB, 45 * GB, "9,216 MBytes (45 GBytes)");
    value(0, 0, "0 MBytes");
    value(2047 * GB, 99999 * GB, "2,047 GBytes (99,999 GBytes)");
    om_disk_bytes(small, sizeof small, 1371 * MB);
    CHECK(strlen(small) < sizeof small, "a small buffer is not overrun");

    printf("test_om_disk: %d checks, %d failures\n", checks, failures);
    return failures ? 1 : 0;
}
