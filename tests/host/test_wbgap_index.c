/* wbgap_index's host test: the ACFS index's "::wb" lines as WBGapIndex reads them (fields, settings, compat names,
 * ORDER hits).
 *   test_wbgap_index
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "wbgap_index.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static const char *const line =
    "added=1763018400000000000 trash=live trashed=0 final=0 reclaim=604800 compat=My Long Name~0123456789abcdef "
    "sort=on:NAME,REVERSE show=on:ALL arrange=off:";

static void fields(void)
{
    char v[200];
    CHECK(wbgap_wb_field(line, "added", v, sizeof v) && !strcmp(v, "1763018400000000000"), "added: \"%s\"", v);
    CHECK(wbgap_wb_field(line, "trash", v, sizeof v) && !strcmp(v, "live"), "trash: \"%s\"", v);
    CHECK(wbgap_wb_field(line, "trashed", v, sizeof v) && !strcmp(v, "0"), "trashed is its own key, not trash's: \"%s\"", v);
    CHECK(wbgap_wb_field(line, "compat", v, sizeof v) && !strcmp(v, "My Long Name~0123456789abcdef"), "compat keeps its spaces: \"%s\"", v);
    CHECK(wbgap_wb_field(line, "sort", v, sizeof v) && !strcmp(v, "on:NAME,REVERSE"), "sort: \"%s\"", v);
    CHECK(wbgap_wb_field(line, "arrange", v, sizeof v) && !strcmp(v, "off:"), "arrange, last: \"%s\"", v);
    CHECK(!wbgap_wb_field(line, "colour", v, sizeof v), "a key that isn't there");
    CHECK(!wbgap_wb_field("", "added", v, sizeof v), "an empty line");
    CHECK(wbgap_wb_field(line, "sort", v, 5) && !strcmp(v, "on:N"), "cut to the room given: \"%s\"", v);
    CHECK(wbgap_wb_field("reclaim=3600\n", "reclaim", v, sizeof v) && !strcmp(v, "3600"), "a line's end: \"%s\"", v);
    CHECK(wbgap_wb_field("xsort=on:A sort=off:B", "sort", v, sizeof v) && !strcmp(v, "off:B"), "a key inside another word is no key: \"%s\"", v);
}

static void settings(void)
{
    char v[64];
    int on = -1;
    CHECK(wbgap_wb_setting("on:NAME,REVERSE", &on, v, sizeof v) && on == 1 && !strcmp(v, "NAME,REVERSE"), "on: %d \"%s\"", on, v);
    CHECK(wbgap_wb_setting("off:DATE", &on, v, sizeof v) && on == 0 && !strcmp(v, "DATE"), "off with a value: %d \"%s\"", on, v);
    CHECK(wbgap_wb_setting("off", &on, v, sizeof v) && on == 0 && !v[0], "off alone");
    CHECK(wbgap_wb_setting("off:", &on, v, sizeof v) && on == 0 && !v[0], "off and a colon");
    CHECK(!wbgap_wb_setting("maybe:X", &on, v, sizeof v), "neither");
    CHECK(!wbgap_wb_setting("", &on, v, sizeof v), "nothing");
    CHECK(wbgap_same("NAME,REVERSE", "name,reverse") && !wbgap_same("NAME", "NAME,REVERSE"), "values in any case");
}

static void compat(void)
{
    char n[200];
    memset(n, 'a', 90);
    strcpy(n + 90, "~0123456789abcdef");   /* 107 */
    CHECK(strlen(n) == 107 && wbgap_compat_ok(n), "107 characters ending in ~ and 16 hex digits");
    strcpy(n + 90, "~0123456789ABCDEF");
    CHECK(wbgap_compat_ok(n), "upper-case hex too");
    strcpy(n + 90, "~0123456789abcdeg");
    CHECK(!wbgap_compat_ok(n), "a g isn't hex");
    strcpy(n + 90, "x0123456789abcdef");
    CHECK(!wbgap_compat_ok(n), "no ~");
    memset(n, 'a', 91);
    strcpy(n + 91, "~0123456789abcdef");   /* 108 */
    CHECK(!wbgap_compat_ok(n), "108 is too long for 3.2.3");
    CHECK(wbgap_compat_ok("ab~0123456789abcdef"), "a short one");
    CHECK(!wbgap_compat_ok("~0123456789abcde"), "15 digits");
    CHECK(!wbgap_compat_ok(""), "empty");
}

static void order(void)
{
    const char *n = wbgap_order_name("1763018400000000000 B");
    CHECK(n && !strcmp(n, "B"), "the name: \"%s\"", n ? n : "(null)");
    n = wbgap_order_name("12 a name with spaces");
    CHECK(n && !strcmp(n, "a name with spaces"), "a name with spaces: \"%s\"", n ? n : "(null)");
    CHECK(wbgap_order_name("B") == NULL && wbgap_order_name("12") == NULL && wbgap_order_name("") == NULL, "no number, no name");
    CHECK(wbgap_order_name("x12 B") == NULL, "the number first");
}

int main(void)
{
    fields();
    settings();
    compat();
    order();
    printf("test_wbgap_index: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}
