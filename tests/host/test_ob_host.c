/* ob_host's host test: the host's cores in the screen's title (GFOpenBench 0.1.45). The machine names map to ARM, x64,
 * x86 and PPC; the cores and machine come from the status' top-level "host" block (as the bridge sends it, the clock
 * first), not from a "host" or "machine" elsewhere; no host block or no cores is 0.
 *   test_ob_host
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#include <stdio.h>
#include <string.h>

#include "ob_host.h"

static int failures, checks;

#define CHECK(c, ...) do { checks++; if (!(c)) { failures++; printf("FAIL %s:%d: ", __FILE__, __LINE__); \
    printf(__VA_ARGS__); printf("\n"); } } while (0)

static void isa(const char *machine, const char *want)
{
    char out[16];
    ob_host_isa(machine, out);
    CHECK(!strcmp(out, want), "%s: %s, not %s", machine ? machine : "(null)", out, want);
}

static void cores(const char *status, unsigned want, const char *want_isa)
{
    char out[16];
    unsigned n = ob_host_cores(status, out);
    CHECK(n == want, "%u cores, not %u: %s", n, want, status ? status : "(null)");
    if (want) CHECK(!strcmp(out, want_isa), "%s, not %s", out, want_isa);
}

int main(void)
{
    isa("aarch64", "ARM");
    isa("armv7l", "ARM");
    isa("arm64", "ARM");
    isa("x86_64", "x64");
    isa("AMD64", "x64");
    isa("i686", "x86");
    isa("ppc64le", "PPC");
    isa("powerpc", "PPC");
    isa("riscv64", "riscv64");
    isa("a-very-long-machine-name", "a-very-long-mac");
    isa("", "");
    isa(NULL, "");

    /* the card: the bridge's status, the clock first, compact */
    cores("{\"clock\":{\"zone\":\"Europe/London\",\"offset\":0},\"product\":\"AmiCradle\",\"instance\":{\"root\":\"/home/x/host\","
          "\"name\":\"host\"},\"host\":{\"platform\":\"Linux-6.12-aarch64\",\"machine\":\"aarch64\",\"architecture\":\"arm64\","
          "\"python\":\"3.13\",\"logicalCores\":4,\"hugePages\":{\"machine\":\"x\"}},\"cpu\":{\"engine\":{\"name\":\"68040\"}}}",
          4, "ARM");
    /* a PC, with spaces as Python's json.dumps writes them, and the nested block before the keys */
    cores("{\"host\": {\"hugePages\": {\"machine\": \"no\", \"logicalCores\": 99}, \"machine\": \"x86_64\", \"logicalCores\": 12}}",
          12, "x64");
    /* no machine: the architecture */
    cores("{\"host\":{\"logicalCores\":8,\"architecture\":\"ppc64\"}}", 8, "PPC");
    /* no host block, no cores, nonsense */
    cores("{\"cpu\":{\"logicalCores\":4}}", 0, "");
    cores("{\"host\":\"daletop\",\"cpu\":{}}", 0, "");
    cores("{\"host\":{\"machine\":\"aarch64\"}}", 0, "");
    cores("{\"host\":{\"logicalCores\":0}}", 0, "");
    cores("{\"host\":{\"logicalCores\":\"four\"}}", 0, "");
    cores("{\"host\":{\"logicalCores\":4", 4, "");
    cores("{\"host\":{\"machine\":\"aarch64", 0, "");
    cores("", 0, "");
    cores(NULL, 0, "");

    printf("ob_host: %d checks, %d failed\n", checks, failures);
    return failures != 0;
}
