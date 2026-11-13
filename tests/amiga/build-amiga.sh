#!/bin/sh
# The lab test programs alone (DragTest, WBGapTest and its scripts), for Cradle's stove_build of
# tests/amiga: STOVE and OUT as Cradle sets them. build.sh at the top builds them with everything else.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
OUT=${OUT:-$HERE/build-amiga}
mkdir -p "$OUT"
COMMON="-noixemul -m68040 -specs=$HERE/../../toolchain/m68k-soft-m68040.specs -std=gnu99 -Wall -Werror -O2 -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -fno-common"
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then COMMON="$COMMON -fno-late-combine-instructions"; fi
for t in DragTest WBGapTest; do
  # shellcheck disable=SC2086
  "$CC" $COMMON -o "$OUT/$t" "$HERE/$t.c"
done
cp "$HERE/WBGapTest.script" "$HERE/WBGapRexx.rexx" "$HERE/WBGapX4.rexx" "$HERE/WBGapX3.rexx" "$HERE/WBGapF3.rexx" "$OUT/"
ls -l "$OUT"
