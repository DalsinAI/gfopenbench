/* ob_stack.h: the Information window's stack field (gap N6). A tool or a project's
 * stack size, as a whole number of bytes, the icon's do_StackSize. Plain C,
 * no Amiga headers, tested on the host (tests/host/test_ob_stack.c).
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_STACK_H
#define OB_STACK_H

#include <stddef.h>

/* Writes the number into dst. 1 if it fitted. */
int ob_stack_format(char *dst, size_t n, long stack);

/* 1 and *out set when s is a whole number, 0 to 9999999, spaces allowed
 * at either end. Anything else, including empty, is refused. */
int ob_stack_parse(const char *s, long *out);

#endif
