/* ob_host.h: the host's cores in the screen's title (GFOpenBench 0.1.45). Plain C with no Amiga headers, tested on
 * the host (tests/host/test_ob_host.c).
 *
 * The title's CPU part shows the whole machine: the emulated 68k and the cores of the computer it runs on, read from
 * the host, "CPU = 1 (68k) + 4 (ARM)" on the Raspberry Pi card, "CPU = 1 (68k) + 12 (x64)" on a 12-thread PC (Dale,
 * 11 November 2026). It is a readout of the host, whether or not the CPU cores board is fitted: the board gives the
 * Amiga cores of its own, and when it is fitted they show too, between the two ("+ 10 (m68k/x64)", barlogo_isas).
 *
 * Where it comes from: accontrol.device's ACCMD_BOARD, query ACC_QUERY_STATUS (7), the host's /api/system/status as
 * JSON (amicradle native_bridge.py, the bridge's commands 7 and 8), whose "host" block has "machine" (Python's
 * platform.machine(): "aarch64", "x86_64") and "logicalCores" (os.cpu_count()). GFOpenBenchmark reads the same
 * status (benchmark/gfbench.c).
 *
 * MIT, Copyright (c) 2026 AmiCradle Team. */
#ifndef OB_HOST_H
#define OB_HOST_H

/* The title's name for a host machine: "ARM" for aarch64 and arm*, "x64" for x86_64 and amd64, "x86" for i386 to
 * i686, "PPC" for ppc* and powerpc*, else the machine's own name (at most 15 characters). out has 16 bytes. */
void ob_host_isa(const char *machine, char out[16]);

/* From the host's status (JSON, NUL-ended): its logical cores, and its machine's name for the title in isa (16
 * bytes). 0 when the status has no "host" block or no cores in it. */
unsigned ob_host_cores(const char *status, char isa[16]);

#endif
