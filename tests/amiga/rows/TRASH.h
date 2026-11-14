/* I6 (TRASH): a file dragged onto a trashcan's icon on its own volume goes into that trashcan, its icon with it, as
 * on 3.2.3, and the trashcan stays where it was. What is where after the drag, each 1 if there: the file and its .info
 * where it came from (src, src_info) and in the trashcan (dst, dst_info), and the trashcan's own icon (can_info).
 * WBGAP_PASS, WBGAP_FAIL (why says which) or WBGAP_SKIP (can_info < 0: not looked at).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_trash_judge(int src, int src_info, int dst, int dst_info, int can_info, const char *name, char *why,
                      unsigned cap);
