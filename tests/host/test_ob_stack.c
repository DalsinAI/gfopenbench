/* ob_stack's host test: the Information window's stack field (gap N6).
 *   test_ob_stack
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_stack.h"

static int failures, checks;

#define CHECK(c) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); } } while (0)

int main(void)
{
    char b[16];
    long v = -1;
    CHECK(ob_stack_format(b, sizeof b, 8192) && !strcmp(b, "8192"));
    CHECK(ob_stack_format(b, sizeof b, 0) && !strcmp(b, "0"));
    CHECK(!ob_stack_format(b, 2, 8192));
    CHECK(ob_stack_parse("8192", &v) && v == 8192);
    CHECK(ob_stack_parse("  4096  ", &v) && v == 4096);
    CHECK(ob_stack_parse("0", &v) && v == 0);
    CHECK(!ob_stack_parse("", &v));
    CHECK(!ob_stack_parse("   ", &v));
    CHECK(!ob_stack_parse("12x", &v));
    CHECK(!ob_stack_parse("-4", &v));
    CHECK(!ob_stack_parse("10000000", &v));
    CHECK(!ob_stack_parse(0, &v));
    printf("test_ob_stack: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
