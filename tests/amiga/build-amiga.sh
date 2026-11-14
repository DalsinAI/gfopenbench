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
sh "$HERE/gen-rows.sh" "$OUT/gen"
# shellcheck disable=SC2086
"$CC" $COMMON -I"$HERE" -I"$HERE/rows" -I"$OUT/gen" -o "$OUT/WBGapTest" "$HERE/WBGapTest.c" "$HERE/wbgap_rows.c" "$HERE/wbgap_menu.c" "$HERE"/rows/*.c
# WBGapIndex: the ACFS index's rows, with amicradle-guest's tools/openindex (AMICRADLE_GUEST: its checkout, else
# fetched here: guest-index.sh, row X2)
. "$HERE/guest-index.sh"
guest_index_fetch "$OUT/deps"
if [ -n "${AMICRADLE_GUEST:-}" ]; then
  OIX="$AMICRADLE_GUEST/tools/openindex"
  # shellcheck disable=SC2086
  "$CC" $COMMON -I"$OIX/amiga" -I"$OIX/core" -I"$AMICRADLE_GUEST/devices/acscsi.device/include" -o "$OUT/WBGapIndex" \
    "$HERE/WBGapIndex.c" "$HERE/wbgap_index.c" "$OIX/amiga/oix_acfs.c" "$OIX/core/oix_core.c"
else
  echo "WBGapIndex not built: set AMICRADLE_GUEST to an amicradle-guest checkout"
fi
guest_index_done
cp "$HERE/WBGapTest.script" "$HERE/WBGapCheck.script" "$HERE/WBGapRexx.rexx" "$HERE/WBGapLayout.rexx" "$HERE/WBGapX4.rexx" "$HERE/WBGapX3.rexx" "$HERE/WBGapF3.rexx" "$HERE/WBGapDesk.rexx" "$HERE/WBGapRoot.rexx" "$OUT/"
cp "$HERE/WBGapAppPos.rexx" "$OUT/"
ls -l "$OUT"
