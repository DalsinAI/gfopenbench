/* X3 (LOWMEM): low memory. MIT, Copyright (c) 2026 AmiCradle Team.
 * After taking every piece of memory it can, the mode gives WBGAP_LOWMEM_GIVE bytes back for the system in pieces
 * of WBGAP_LOWMEM_PIECE bytes at most (no program's code fits in one), and only every other such piece, so no two
 * freed pieces lie side by side and join into a bigger one. wbgap_lowmem_give_back: 1 if a piece of size bytes,
 * the nth (from 0) of those no bigger than WBGAP_LOWMEM_PIECE, is given back; 0 if it is kept taken. */
#define WBGAP_LOWMEM_GIVE 49152UL
#define WBGAP_LOWMEM_PIECE 4096UL
int wbgap_lowmem_give_back(unsigned long size, int nth);
