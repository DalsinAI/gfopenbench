/* D5r: pos holds the desktop's disk icons, "<left> <top> <width> <name>" a line (WBGapRoot.rexx's
 * RAM:wbgap-root-pos). The column is the disks whose left edge is less than one icon's width (16 at least) right of
 * the leftmost; its topmost (the first read wins a tie) must be vol, case not minded. top gets that icon's name.
 * A line with no name is passed over. 1: RAM Disk first; 0: another first; -1: no disk read (or cap 0).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
int wbgap_ram_first(const char *pos, const char *vol, char *top, unsigned cap);
