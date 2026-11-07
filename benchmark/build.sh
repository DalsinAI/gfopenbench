#!/bin/sh
# GFOpenBenchmark with the GCC 16 stove (os32-gcc16-open6: m68k-amigaos-gcc 16.2, NDK 3.2):
#   GUEST=<amicradle-guest> ORTG=<gfopenrtg> sh benchmark/build.sh [OUT_DIR]   (default build/os3)
# It needs two checkouts beside it: amicradle-guest (GUEST, default ../../amicradle-guest) for
# common/sysprobe (ACInfo SPEED's loop, the CPU's name, accontrol.device's host status) and
# gfopenrtg (ORTG, default ../../gfopenrtg) for ogles3.library's dispatch slots, which its
# library/ogles3/gen_ogles3.py makes from ogles3_api.txt. build-amiga.sh fetches both for Cradle's
# stove_build. The compiler: OS32_GCC16, else STOVE, else ~/AmiCradle/stoves/os32-gcc16-open6; it
# stops when given a GCC older than 16.
# Built for the 68040 with its FPU (-m68881, libm881): the FPU tests are of the FPU, not of soft
# float, so this is the one program here not built with toolchain/m68k-soft-m68040.specs.
# MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
GUEST=${GUEST:-$HERE/../../amicradle-guest}
ORTG=${ORTG:-$HERE/../../gfopenrtg}
STOVE=${OS32_GCC16:-${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16-open6}}
if [ -x "$STOVE/prefix/bin/m68k-amigaos-gcc" ]; then PREFIX=$STOVE/prefix; else PREFIX=$STOVE; fi
CC="$PREFIX/bin/m68k-amigaos-gcc"
GCCV=$("$CC" -dumpversion 2>/dev/null || echo 0)
[ "${GCCV%%.*}" -ge 16 ] 2>/dev/null || { echo "$CC is GCC $GCCV: GFOpenBenchmark needs GCC 16 (the os32-gcc16-open6 stove; set OS32_GCC16)"; exit 2; }
OUT=${1:-$HERE/../build/os3}
[ -f "$GUEST/common/sysprobe/sysprobe.c" ] || { echo "no amicradle-guest checkout at $GUEST (set GUEST)"; exit 2; }
[ -f "$ORTG/library/ogles3/gen_ogles3.py" ] || { echo "no gfopenrtg checkout at $ORTG (set ORTG)"; exit 2; }
mkdir -p "$OUT"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
python3 "$ORTG/library/ogles3/gen_ogles3.py" "$TMP" "$ORTG/library/ogles3/ogles3_api.txt"
FLAGS="-noixemul -m68040 -m68881 -O2 -std=gnu99 -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -fno-common"
# GCC 16.2.0-open3's late-combine wrong-code bug (GCC PR 123853), as build.sh at the top says: off where the compiler knows the option
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then FLAGS="$FLAGS -fno-late-combine-instructions"; fi
GCCID=$("$CC" --version | head -1 | sed 's/^m68k-amigaos-gcc //')
INC="-I$HERE -I$GUEST/common/sysprobe -I$GUEST/devices/accontrol.device/include -I$TMP"
# shellcheck disable=SC2086
"$CC" $FLAGS -Os -Wall -Wno-pointer-sign $INC -c -o "$TMP/sysprobe.o" "$GUEST/common/sysprobe/sysprobe.c"
OBJS="$TMP/sysprobe.o"
for f in gfbench cpu dhry_1 dhry_2 fputests whet linpack sgl mem gfx2d gl; do
  # shellcheck disable=SC2086
  "$CC" $FLAGS -Wall -Wextra -Werror -Wno-unused-parameter $INC "-DGFB_CFLAGS=\"$GCCID $FLAGS\"" -c -o "$TMP/$f.o" "$HERE/$f.c"
  OBJS="$OBJS $TMP/$f.o"
done
# shellcheck disable=SC2086
"$CC" $FLAGS -o "$OUT/GFOpenBenchmark" $OBJS -lm
und=$("$PREFIX/bin/m68k-amigaos-nm" -u "$OUT/GFOpenBenchmark" 2>/dev/null || true)
[ -z "$und" ] || { echo "undefined in GFOpenBenchmark: $und" >&2; exit 1; }
# the FSGL loops are there, and the kernels use the FPU's own instructions, not soft-float calls
"$PREFIX/bin/m68k-amigaos-objdump" -d "$TMP/sgl.o" > "$TMP/dis.txt"
grep -q fsglmul "$TMP/dis.txt" && grep -q fsgldiv "$TMP/dis.txt" || { echo "GFOpenBenchmark: no FSGLMUL/FSGLDIV in sgl.o" >&2; exit 1; }
for f in sgl whet linpack fputests; do
  if "$PREFIX/bin/m68k-amigaos-nm" -u "$TMP/$f.o" | grep -q "__mul[sd]f3\|__div[sd]f3\|__add[sd]f3"; then
    echo "GFOpenBenchmark: soft-float calls in $f.o" >&2; exit 1
  fi
done
cp "$HERE/README.md" "$OUT/GFOpenBenchmark.readme" 2>/dev/null || true
echo "$OUT/GFOpenBenchmark ($(wc -c < "$OUT/GFOpenBenchmark" | tr -d ' ') bytes, ogles3 slots from gfopenrtg $(git -C "$ORTG" rev-parse --short HEAD 2>/dev/null || echo '?'), sysprobe from amicradle-guest $(git -C "$GUEST" rev-parse --short HEAD 2>/dev/null || echo '?'))"
sha256sum "$OUT/GFOpenBenchmark" 2>/dev/null || true
