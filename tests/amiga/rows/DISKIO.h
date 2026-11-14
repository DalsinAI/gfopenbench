/* D1: a disk's icon goes while its file system is inhibited (as for Format) and comes back after. The three answers
 * of wbgap_root_has_disk: before, while inhibited, after; -1 where the icons weren't read. WBGAP_SKIP if it had no
 * icon before (nothing to watch) or a read failed (nothing seen), else WBGAP_PASS or WBGAP_FAIL, the reason in why.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_diskio_judge(int before, int inhibited, int after, char *why, unsigned cap);
