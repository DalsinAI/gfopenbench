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
/* wbgap_trashnote_iconbox: RAM:Trashcan's icon by its name, from the line the Workbench's ARexx port gave
 * (GETATTR window.icons.all.N.left/top/width/height: "left top width height"). Its place is the drawer's own layout
 * place, in another frame from the window's inside (14 Nov lab: 130,78 by name where the window drew it at 146,58), so
 * only its size is checked: the image, without its label. box gets it. changed is the box the window's inside changed
 * in when the Trashcan was added, or NULL. 1: found, and its image fits in changed (2 pixels' slack); 0: no line, or
 * not four numbers with a size; -1: read, but bigger than what changed (not the icon the window drew). */
int wbgap_trashnote_iconbox(const char *line, const long *changed, long box[4]);
/* wbgap_trashnote_crowded: 1 when changed_w, the width the RAM Disk's window changed in when the Trashcan was added,
 * is wider than two icons, so more than the Trashcan moved and a drop on it could miss. The icon's width is the wider
 * of name_w (its image by its name, without its label; 0 when not read) and f_w (f's box, label and all), so a long
 * label under a narrow image doesn't count as a second icon. */
int wbgap_trashnote_crowded(long changed_w, long name_w, long f_w);
/* wbgap_trashnote_mapped: where the window drew the Trashcan, from its place by its name, when the RAM Disk's window
 * re-laid other icons as it was added (a full run's RAM: holds wbgap-results and more, all after "Trashcan" by name,
 * 14 Nov gap count): f's place by its name (name_f) and where its window drew it (changed_f, an uncrowded change in
 * the same kind of drawer window) give the offset from the layout frame to the window's inside; out is name_t moved
 * by it. 1 when out's middle lies in changed_t (what the RAM Disk's window changed in) and out fits the window's inside
 * (inner_w x inner_h, 0 not checked); 0 otherwise (no drop, as before). */
int wbgap_trashnote_mapped(const long *name_t, const long *name_f, const long *changed_f, const long *changed_t,
                           long inner_w, long inner_h, long out[4]);
#define WBGAP_PUTBACK_NOLIB (-1000)
#define WBGAP_PUTBACK_NOTRIED (-1001)
#define WBGAP_PUTBACK_NOCTX (-1002)
int wbgap_trashnote_line(const char *note, const char *name, const char *tail);
int wbgap_trashnote_judge(int src, int src_info, int dst, int dst_info, const char *note, const char *name,
                          const char *tail, char *why, unsigned cap);
int wbgap_putback_judge(long rc, int back, int back_info, int left, int noted, const char *name, char *why,
                        unsigned cap);
