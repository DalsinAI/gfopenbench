/* ob_stack.c: see ob_stack.h. MIT, Copyright (c) 2026 AmiCradle Team. */
#include "ob_stack.h"

#include <stdio.h>
#include <stdlib.h>

int ob_stack_format(char *dst, size_t n, long stack)
{
    int w;
    if (!dst || n == 0) return 0;
    if (stack < 0) stack = 0;
    w = snprintf(dst, n, "%ld", stack);
    return w > 0 && (size_t)w < n;
}

int ob_stack_parse(const char *s, long *out)
{
    char *end;
    long v;
    if (!s || !out) return 0;
    while (*s == ' ') s++;
    if (*s < '0' || *s > '9') return 0;
    v = strtol(s, &end, 10);
    if (end == s || v < 0 || v > 9999999L) return 0;
    while (*end == ' ') end++;
    if (*end) return 0;
    *out = v;
    return 1;
}
