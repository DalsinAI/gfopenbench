/* I6t, I6p (TRASHNOTE): GFOS's own trash. A file dragged onto its volume's Trashcan goes there through GFOpenFiles'
 * engine (of_delete), which writes a line "name TAB where-it-came-from" for it, and for its icon, into the Trashcan's
 * note (<volume>:Trashcan/.GFOpenFiles); GFOpenFiles puts it back from that note (of_restore, gfopenfiles.library's
 * OFRestore). 3.2.3's trashcan keeps no such note, so both rows are GFOS's only.
 *
 * wbgap_trashnote_line: 1 when note (the note's text, NULL if there is none) has a line for name whose path ends in
 * tail, after a ':' or a '/' ("WBGapR/f" matches "RAM:WBGapR/f" and "Ram Disk:WBGapR/f"), case not minded.
 * wbgap_trashnote_judge (I6t): the move (wbgap_move_judge: the file and its icon gone from src, in the Trashcan), then
 * the note's lines for the file and for its icon. WBGAP_PASS, or WBGAP_FAIL (why says which).
 * wbgap_putback_judge (I6p): rc, OFRestore's answer (0 is done; WBGAP_PUTBACK_NOLIB: gfopenfiles.library isn't
 * there, WBGAP_PUTBACK_NOTRIED: nothing in the Trashcan to put back, I6t failed; WBGAP_PUTBACK_NOCTX: OFNewContext
 * gave no context); back and back_info: the file and its
 * icon where they came from again; left: either still in the Trashcan; noted: the note still has a line for it.
 * WBGAP_PASS, WBGAP_FAIL or WBGAP_SKIP, the reason in why.
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#define WBGAP_PUTBACK_NOLIB (-1000)
#define WBGAP_PUTBACK_NOTRIED (-1001)
#define WBGAP_PUTBACK_NOCTX (-1002)
int wbgap_trashnote_line(const char *note, const char *name, const char *tail);
int wbgap_trashnote_judge(int src, int src_info, int dst, int dst_info, const char *note, const char *name,
                          const char *tail, char *why, unsigned cap);
int wbgap_putback_judge(long rc, int back, int back_info, int left, int noted, const char *name, char *why,
                        unsigned cap);
