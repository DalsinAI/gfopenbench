#!/bin/sh
# gfopenlaunch.library for Cradle's stove_build (target launcher): the top build.sh, which builds it with the other
# libraries, into Cradle's OUT so the job lists it. MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT=${OUT:-$HERE/../build/os3}
# WBGapTest's WBGapIndex (row X2) needs amicradle-guest's tools/openindex: fetched when AMICRADLE_GUEST isn't set
. "$HERE/../tests/amiga/guest-index.sh"
guest_index_fetch "$OUT/deps"
sh "$HERE/../build.sh" "$OUT"
guest_index_done
