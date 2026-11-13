#!/bin/sh
# GFOpenBench's libraries and tools with the os32-gcc16 stove (m68k-amigaos-gcc 16, NDK 3.2; the NDK is never
# in this repository). Step R1: our workbench.library and icon.library,
# which forward every call to Hyperion's originals, and wbspy.
#   build.sh [OUT_DIR]   (default build/os3)
# The compiler: OS32_GCC16, else STOVE, else ~/AmiCradle/stoves/os32-gcc16 (as desktop/build.sh); it stops
# when given an older GCC (the os32 stove's GCC 6.5.0b merges a word clear and the long clear after it into one
# clr.l at -O2), and builds -fno-tree-loop-distribute-patterns (GCC 16.2.0b's loop-shift trap).
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
# Soft float: the specs file links libnix's soft libm020 (maths through the
# mathieee libraries), not libm881, so one build runs on every 68040.
SPECS="-specs=$HERE/toolchain/m68k-soft-m68040.specs"
STOVE=${OS32_GCC16:-${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16}}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
GCCV=$("$CC" -dumpversion 2>/dev/null || echo 0)
[ "${GCCV%%.*}" -ge 16 ] 2>/dev/null || { echo "$CC is GCC $GCCV: this needs GCC 16 (the os32-gcc16 stove; set OS32_GCC16)"; exit 2; }
OUT=${1:-$HERE/build/os3}
mkdir -p "$OUT"
COMMON="-noixemul -m68040 $SPECS -std=gnu99 -Wall -Werror -O2 -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -fno-common"
# GCC 16.2.0-open3's late-combine pass has a wrong-code bug (GCC PR 123853): at -O2 it can copy with (aN)+ and use aN
# again in the same move's destination, which the 68k works out after the step, so each store lands one place late
# (libnix's inline strcpy of a constant does it: gfopenprefs' wbclose.h in desktop/ and meters/). Off until
# GFOpenAmigaGCC open4 (gfopengcc #29), on a compiler that knows the option.
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then COMMON="$COMMON -fno-late-combine-instructions"; fi
# The libraries: no C library and no start code (and no loop turned into a
# call to the memset they define themselves). Each file's first bytes
# are its own (PROXY_START), so the order of the file's top level is kept.
LIB="$COMMON -ffreestanding -fno-builtin -fno-tree-loop-distribute-patterns -fno-toplevel-reorder -nostartfiles -nostdlib"
# shellcheck disable=SC2086
"$CC" $LIB -o "$OUT/workbench.library" "$HERE/workbench/wblib.c" "$HERE/proxy/proxy.c" -lgcc
# shellcheck disable=SC2086
"$CC" $LIB -o "$OUT/icon.library" "$HERE/icon/iconlib.c" "$HERE/proxy/proxy.c" -lgcc
# Step R2: icon.library's own calls, not switched on: gfopenicon.library
# (icon.library's table under a test name) and iconprobe, which calls it
# and Hyperion's side by side.
# shellcheck disable=SC2086
"$CC" $LIB -Wextra -c -o /dev/null "$HERE/icon/icon_core.c"
# shellcheck disable=SC2086
"$CC" $LIB -o "$OUT/gfopenicon.library" "$HERE/icon/openicon.c" "$HERE/icon/icon_core.c" -lgcc
und=$("$STOVE/prefix/bin/m68k-amigaos-nm" -u "$OUT/gfopenicon.library" || true)
[ -z "$und" ] || { echo "undefined in gfopenicon.library: $und" >&2; exit 1; }
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/iconprobe" "$HERE/tools/iconprobe.c"
# the host tests' icons, for iconprobe in a lab (SYS:Storage/GFOpenBench-Icons)
if command -v python3 >/dev/null; then
  rm -rf "$OUT/GFOpenBench-Icons"
  python3 "$HERE/tests/host/icon_fixtures.py" "$OUT/GFOpenBench-Icons" >/dev/null
  rm -f "$OUT/GFOpenBench-Icons/"*.want "$OUT/GFOpenBench-Icons/manifest.txt" "$OUT/GFOpenBench-Icons/deflate.bin"
fi
# The launcher module (Workbench build plan row 6): gfopenlaunch.library, which GFOpenBench 0.1.56 opens to start
# programs. It ships as its own file in gfosmodules/ (Libs/), loaded when first opened.
# shellcheck disable=SC2086
"$CC" $LIB -Wextra -I"$HERE/launcher" -o "$OUT/gfopenlaunch.library" "$HERE/launcher/ollib.c" "$HERE/launcher/ol_core.c" -lgcc
und=$("$STOVE/prefix/bin/m68k-amigaos-nm" -u "$OUT/gfopenlaunch.library" || true)
[ -z "$und" ] || { echo "undefined in gfopenlaunch.library: $und" >&2; exit 1; }
# Step L1 (docs/design/Design-Open-Workbench-Library.md): workbench.library's
# own lists, not switched on: gfopenbench.library (workbench.library's table
# under a test name) and WorkbenchABITest, which calls it and the system's
# side by side.
# shellcheck disable=SC2086
"$CC" $LIB -Wextra -c -o /dev/null "$HERE/workbench/wb_core.c"
# shellcheck disable=SC2086
"$CC" $LIB -o "$OUT/gfopenbench.library" "$HERE/workbench/openbench.c" "$HERE/workbench/wb_core.c" -lgcc
und=$("$STOVE/prefix/bin/m68k-amigaos-nm" -u "$OUT/gfopenbench.library" || true)
[ -z "$und" ] || { echo "undefined in gfopenbench.library: $und" >&2; exit 1; }
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/WorkbenchABITest" "$HERE/tests/amiga/WorkbenchABITest.c"
# DragTest: a lab's mouse and keys from inside it, at exact places, for the drag and drop tests (GFOpenBench 0.1.35)
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/DragTest" "$HERE/tests/amiga/DragTest.c"
# WBGapTest: the Workbench gap rows (amicradle-teams designs/OPENBENCH-GAPS-20261112.md), with DragTest
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/WBGapTest" "$HERE/tests/amiga/WBGapTest.c"
cp "$HERE/tests/amiga/WBGapTest.script" "$HERE/tests/amiga/WBGapRexx.rexx" "$HERE/tests/amiga/WBGapX4.rexx" "$HERE/tests/amiga/WBGapX3.rexx" "$OUT/"
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/wbspy" "$HERE/tools/wbspy.c" "$HERE/tools/spyfmt.c"
# r1swap: lets OS 3.2's own Workbench start under R1, for step L2 (lab only)
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/r1swap" "$HERE/tools/r1swap.c"
# alohaprobe: what Intuition tells a Workbench, and when (the native screen mode's step 4; lab only)
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/alohaprobe" "$HERE/tools/alohaprobe.c"
cp "$HERE/lab/Install-R1" "$HERE/lab/Uninstall-R1" "$OUT/"
size() { wc -c < "$1" | tr -d ' '; }
echo "$OUT: workbench.library ($(size "$OUT/workbench.library") bytes), icon.library ($(size "$OUT/icon.library") bytes), wbspy ($(size "$OUT/wbspy") bytes), alohaprobe ($(size "$OUT/alohaprobe") bytes), gfopenicon.library ($(size "$OUT/gfopenicon.library") bytes), iconprobe ($(size "$OUT/iconprobe") bytes), gfopenbench.library ($(size "$OUT/gfopenbench.library") bytes), WorkbenchABITest ($(size "$OUT/WorkbenchABITest") bytes)"
sha256sum "$OUT/gfopenicon.library" "$OUT/iconprobe" "$OUT/gfopenbench.library" "$OUT/WorkbenchABITest" 2>/dev/null || true
# GFOpenBenchmark (benchmark/README.md): with amicradle-guest and gfopenrtg checkouts when GUEST and ORTG
# name them, else benchmark/build-amiga.sh fetches the parts it needs of both at main
if [ -n "${GUEST:-}" ] && [ -n "${ORTG:-}" ]; then
  OS32_GCC16=$STOVE sh "$HERE/benchmark/build.sh" "$OUT"
else
  OUT="$OUT" STOVE="$STOVE" DEPS="$OUT/benchmark-deps" sh "$HERE/benchmark/build-amiga.sh"
fi
