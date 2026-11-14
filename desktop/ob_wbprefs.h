/* ob_wbprefs.h: ENV:Sys/Workbench.prefs as GFOpenBench follows it (WBGapTest P7). An IFF FORM PREF: a WBNC chunk,
 * whose first long is wbp_DefaultStackSize, and a WBHD chunk for each hidden device (its name). Plain C, no Amiga
 * headers, tested on the host (tests/host/test_ob_wbprefs.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_WBPREFS_H
#define OB_WBPREFS_H

#define OB_WBP_MAX_HIDDEN 16
#define OB_WBP_NAME 32
#define OB_WBP_STACK_MAX (1024UL * 1024UL)

typedef struct {
    unsigned long stack;                        /* WBNC's default stack as the file has it; 0 with no WBNC */
    int nhidden;
    char hidden[OB_WBP_MAX_HIDDEN][OB_WBP_NAME]; /* the hidden devices' names, no ':' */
} ob_wbprefs;

/* Reads buf (len bytes of the file) into p. 1 if it is a FORM PREF, else 0 (p then empty). */
int ob_wbprefs_parse(const unsigned char *buf, long len, ob_wbprefs *p);

/* The tools' least stack GFOpenBench gives: the prefs' when more than 8192 (and no more than 1 MiB), else 8192. */
unsigned long ob_wbprefs_min_stack(const ob_wbprefs *p);

/* What workbench.library's default stack (WBCTRLA_SetDefaultStackSize) is to be set to, as Prefs/Workbench's Use
 * sets it on 3.2.3: the prefs' own, from 4096 to 1 MiB; 0 to leave it as it is (no WBNC, or out of range). */
unsigned long ob_wbprefs_lib_stack(const ob_wbprefs *p);

#endif
