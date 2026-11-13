/* ob_prot.h: the Information window's Hold and Pure bits (gap N7).
 * They sit in an Advanced section, not on the Script, Archived, Readable,
 * Writable, Executable, Deletable row. Plain C, no Amiga headers, tested
 * on the host (tests/host/test_ob_prot.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_PROT_H
#define OB_PROT_H

/* The eight letters h s p a r w e d. Hold, Pure, Script and Archived are
 * positive: the bit set means on. Read, Write, Execute and Delete are
 * inverted: the bit set means that permission is off. */
#define OB_PROT_HOLD    128L
#define OB_PROT_SCRIPT   64L
#define OB_PROT_PURE     32L
#define OB_PROT_ARCHIVE  16L
#define OB_PROT_READ      8L
#define OB_PROT_WRITE     4L
#define OB_PROT_EXECUTE   2L
#define OB_PROT_DELETE    1L

/* 1 when the name is Hold or Pure. */
int ob_prot_is_advanced(const char *name);

/* 1 when the name is one of the six on the main row. */
int ob_prot_is_main_row(const char *name);

int ob_prot_hold_on(long prot);
int ob_prot_pure_on(long prot);

/* The protection to write. Bits above the eight letters are kept.
 * readable, writable, executable and deletable are the checkbox
 * (1 means allowed). hold and pure are 1 when the bit is set. */
long ob_prot_apply(long prot, int script, int archived, int readable, int writable,
                   int executable, int deletable, int hold, int pure);

#endif
