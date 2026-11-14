#!/bin/sh
# gen-rows.sh OUT: WBGapTest's per-row files (tests/amiga/rows, tests/host/rows) listed into the includes the shared
# files take, so a new row touches no shared line. Run by build.sh, tests/amiga/build-amiga.sh and tests/host/run.sh.
#   rows/<MODE>.c        the mode, on the Amiga:  WBGAP_MODE(<MODE>, <function>, NEWROWS|SCRIPT)
#                        and the rows it prints:  WBGAP_ROWMODE(<row id>, <MODE>)
#   rows/<MODE>_judge.c  its pure judge (host and Amiga), rows/<MODE>.h its prototype
#   ../host/rows/<MODE>_plan.c  its host checks:  WBGAP_PLAN(<function>)
# Writes OUT/wbgap_gen_modes.inc, OUT/wbgap_gen_rowmodes.inc and OUT/wbgap_gen_plans.inc, in file-name order.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
LC_ALL=C
export LC_ALL
HERE=$(cd "$(dirname "$0")" && pwd)
OUT=${1:?usage: gen-rows.sh OUT}
mkdir -p "$OUT"
modes="$OUT/wbgap_gen_modes.inc"
rowmodes="$OUT/wbgap_gen_rowmodes.inc"
plans="$OUT/wbgap_gen_plans.inc"
echo "/* made by tests/amiga/gen-rows.sh from tests/amiga/rows */" > "$modes"
echo "/* made by tests/amiga/gen-rows.sh from tests/amiga/rows */" > "$rowmodes"
echo "/* made by tests/host/../amiga/gen-rows.sh from tests/host/rows */" > "$plans"
for f in "$HERE"/rows/*.c; do
  [ -e "$f" ] || continue
  sed -n 's/^WBGAP_MODE(\([A-Z0-9]*\), *\([A-Za-z_0-9]*\), *NEWROWS)$/WBGAP_GEN_MODE("\1", \2, 1)/p
          s/^WBGAP_MODE(\([A-Z0-9]*\), *\([A-Za-z_0-9]*\), *SCRIPT)$/WBGAP_GEN_MODE("\1", \2, 0)/p' "$f" >> "$modes"
  sed -n 's/^WBGAP_ROWMODE(\([A-Za-z0-9]*\), *\([A-Z0-9]*\))$/WBGAP_GEN_ROWMODE("\1", "\2")/p' "$f" >> "$rowmodes"
done
for f in "$HERE"/../host/rows/*.c; do
  [ -e "$f" ] || continue
  sed -n 's/^WBGAP_PLAN(\([A-Za-z_0-9]*\))$/WBGAP_GEN_PLAN(\1)/p' "$f" >> "$plans"
done
