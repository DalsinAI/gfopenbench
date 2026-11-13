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
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/DragTest" "$HERE/DragTest.c"
# shellcheck disable=SC2086
"$CC" $COMMON -o "$OUT/WBGapTest" "$HERE/WBGapTest.c" "$HERE/wbgap_rows.c" "$HERE/wbgap_menu.c"
cp "$HERE/WBGapTest.script" "$HERE/WBGapCheck.script" "$HERE/WBGapRexx.rexx" "$HERE/WBGapLayout.rexx" "$OUT/"
ls -l "$OUT"
