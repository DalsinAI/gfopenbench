/* om_disk.h: the host disk a drive shares, for GFOpenMeters 0.4 (Dale, 17 November 2026: "perhaps a (fulldisk) next
 * to its allocation"). On AmiCradle each drive that is a folder is told what it holds plus an equal share of the host
 * disk's free space, so five drives on a 458 GB disk with 405 GB free each show 81 GBytes, and that read as a limit
 * when any one of them can grow into all 405. The bar now reads "81 GBytes (405 GBytes)": the drive's share, then
 * what is free on the whole disk.
 *
 * Where it comes from: accontrol.device's ACCMD_BOARD, query ACC_QUERY_STATUS (7), the host's /api/system/status as
 * JSON, whose "volumes" list gives each such drive "label" (the volume's name), "hostFreeBytes" and "hostSharedBy"
 * (amicradle #1248). A drive with a size of its own, a stick, a share, and any drive on a host that says none of
 * this has no brackets, and neither has a drive that has its host disk to itself (its share is the whole of it).
 *
 * Plain C, tested on the host (tests/host/test_om_disk.c). MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OM_DISK_H
#define OM_DISK_H

#define OM_DISK_MAX 24

struct om_disk {
    char label[32];                /* the volume's name */
    unsigned long long host_free;  /* free on the host disk it shares, in bytes */
    unsigned shared_by;            /* how many drives share that disk, this one included */
};

/* From the host's status (JSON, NUL-ended; it may be cut short): the drives that share a host disk with another,
 * into out (at most max). The number found. */
int om_disk_read(const char *status, struct om_disk *out, int max);

/* The drive of that volume name (as AmigaDOS compares names: case does not matter), or 0. */
const struct om_disk *om_disk_find(const struct om_disk *d, int n, const char *volume);

/* A size as the bars write it: "405 GBytes" from 10 GB, "1,371 MBytes" below. */
void om_disk_bytes(char *out, int size, unsigned long long bytes);

/* A volume's bar: "81 GBytes", and with host_free (not 0, and more than freeb as written) "81 GBytes (405 GBytes)". */
void om_disk_value(char *out, int size, unsigned long long freeb, unsigned long long host_free);

#endif
