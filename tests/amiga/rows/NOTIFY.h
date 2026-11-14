/* F1, F2 (NOTIFY): RAM:WBGap's window must be at least WBGAP_NOTIFY_W x WBGAP_NOTIFY_H to show its icons. 1, and
 * the size to make it in size[0], size[1] (each side as it is, or the least), when a side is short; else 0.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_NOTIFY_W 480
#define WBGAP_NOTIFY_H 240
int wbgap_notify_size(long w, long h, long size[2]);
