/* D2 (MOUNTICON): a drive not mounted at the start is mounted; its volume must get a disk icon. was_there: it was
 * mounted already (SKIP: nothing to watch); before, after: the desktop's icons read (-1 not read; after: 1 if the
 * volume is a disk among them); mount_rc: Mount's return code; vol_up: a volume came up on the drive.
 * Only a volume up and no icon is a FAIL; a test that can't be set up is a SKIP. The reason in why.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_mount_judge(int was_there, int before, int mount_rc, int vol_up, int after, char *why, unsigned cap);
