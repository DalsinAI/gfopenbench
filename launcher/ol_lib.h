/* ol_lib.h: gfopenlaunch.library's calls, for the programs that open it (GFOpenBench, and the Workbench tasks of
 * build plan rows 3 and 4). The library is launcher/ollib.c; its rules are launcher/ol_core.h.
 *
 *   ULONG OL_LaunchA(struct TagItem *tags)       -30  a0      a program started with a WBStartup; its id, 0 when not
 *   BOOL  OL_Ended(ULONG id)                     -36  d0      1 once that program has ended (or the id is unknown)
 *   LONG  OL_Running(STRPTR buf, LONG len)       -42  a0/d0   how many run; their names, one to a line, in buf
 *   LONG  OL_QuitAll(LONG secs)                  -48  d0      Ctrl-C to each, waited for up to secs; how many are left
 *
 * The library never waits for a program in OL_LaunchA: a process of its own takes each WBStartup back and unloads
 * the code. A caller that waits for one (SYS:WBStartup without DONOTWAIT) asks OL_Ended.
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OL_LIB_H
#define OL_LIB_H

#include <exec/types.h>
#include <utility/tagitem.h>

#define OL_LIBNAME "gfopenlaunch.library"
#define OL_LIBVER 1

#define OLA_Dummy (TAG_USER + 0x4F4C0000UL)    /* 'OL' */
#define OLA_Tool (OLA_Dummy + 1)               /* STRPTR: the program, a path (a resident of its name when no file) */
#define OLA_Arg (OLA_Dummy + 2)                /* STRPTR: one argument, a path; again for each, in order (32 at most) */
#define OLA_StackSize (OLA_Dummy + 3)          /* LONG: the icon's stack (do_StackSize) */
#define OLA_ToolTypes (OLA_Dummy + 4)          /* STRPTR *: the icon's tool types, for STACK and TOOLPRI */
#define OLA_MinStack (OLA_Dummy + 5)           /* LONG: Workbench's default stack, the least given (8192) */
#define OLA_NoResident (OLA_Dummy + 6)         /* BOOL: no resident when the file isn't there */

#define OL_ARGS_MAX 32

#if defined(__GNUC__) && defined(__mc68000__)
static inline ULONG OL_LaunchA(struct Library *base, struct TagItem *tags)
{
    register struct Library *a6_ __asm("a6") = base;
    register struct TagItem *a0_ __asm("a0") = tags;
    register ULONG d0_ __asm("d0");
    __asm volatile("jsr -30(%%a6)" : "=r"(d0_), "+r"(a0_) : "r"(a6_) : "d1", "a1", "fp0", "fp1", "cc", "memory");
    return d0_;
}
static inline BOOL OL_Ended(struct Library *base, ULONG id)
{
    register struct Library *a6_ __asm("a6") = base;
    register ULONG d0_ __asm("d0") = id;
    __asm volatile("jsr -36(%%a6)" : "+r"(d0_) : "r"(a6_) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
    return (BOOL)d0_;
}
static inline LONG OL_Running(struct Library *base, STRPTR buf, LONG len)
{
    register struct Library *a6_ __asm("a6") = base;
    register STRPTR a0_ __asm("a0") = buf;
    register LONG d0_ __asm("d0") = len;
    __asm volatile("jsr -42(%%a6)" : "+r"(d0_), "+r"(a0_) : "r"(a6_) : "d1", "a1", "fp0", "fp1", "cc", "memory");
    return d0_;
}
static inline LONG OL_QuitAll(struct Library *base, LONG secs)
{
    register struct Library *a6_ __asm("a6") = base;
    register LONG d0_ __asm("d0") = secs;
    __asm volatile("jsr -48(%%a6)" : "+r"(d0_) : "r"(a6_) : "d1", "a0", "a1", "fp0", "fp1", "cc", "memory");
    return d0_;
}
#endif

#endif
