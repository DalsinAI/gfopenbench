#!/bin/sh
# gen-rows.sh OUT: WBGapTest's per-row files (tests/amiga/rows, tests/host/rows) listed into the includes the shared
# files take, so a new row touches no shared line. Run by build.sh, tests/amiga/build-amiga.sh and tests/host/run.sh.
#   rows/<MODE>.c        the mode, on the Amiga:  WBGAP_MODE(<MODE>, <function>, NEWROWS|SCRIPT)
#                        and the rows it prints:  WBGAP_ROWMODE(<row id>, <MODE>)
#   rows/<MODE>_judge.c  its pure judge (host and Amiga), rows/<MODE>.h its prototype; no markers in these
#   ../host/rows/<MODE>_plan.c  its host checks:  WBGAP_PLAN(<function>)
# Writes OUT/wbgap_gen_modes.inc, _rowmodes.inc, _modenames.inc and _plans.inc, in file-name order. A WBGAP_ line it
# can't read, or a mode file without exactly one WBGAP_MODE named as the file, stops it with rc 1: no row goes quiet.
# A marker is exact: one blank after each comma, none at the end.
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
names="$OUT/wbgap_gen_modenames.inc"
echo "/* made by tests/amiga/gen-rows.sh from tests/amiga/rows */" > "$names"
bad=0
# every line starting WBGAP_ must be one of the markers, exactly; a mode file has exactly one WBGAP_MODE, named as
# the file: a marker that isn't read would drop its row without a word
check() {   # file, the sed script that turns its good marker lines into nothing
  left=$(grep '^WBGAP_' "$1" | sed "$2" | grep -v '^$' || true)
  if [ -n "$left" ]; then
    echo "gen-rows.sh: $1: a WBGAP_ line not read: $left" >&2
    bad=1
  fi
}
MODE_RE='s/^WBGAP_MODE([A-Z0-9][A-Z0-9]*, [A-Za-z_][A-Za-z_0-9]*, NEWROWS)$//;s/^WBGAP_MODE([A-Z0-9][A-Z0-9]*, [A-Za-z_][A-Za-z_0-9]*, SCRIPT)$//'
ROW_RE='s/^WBGAP_ROWMODE([A-Za-z0-9][A-Za-z0-9]*, [A-Z0-9][A-Z0-9]*)$//'
PLAN_RE='s/^WBGAP_PLAN([A-Za-z_][A-Za-z_0-9]*)$//'
for f in "$HERE"/rows/*.c; do
  [ -e "$f" ] || continue
  case "$f" in
    *_judge.c)   # markers belong in the mode file only: one here would be listed nowhere
      if grep -q '^WBGAP_' "$f"; then
        echo "gen-rows.sh: $f: a WBGAP_ line in a judge file (put it in the mode file): $(grep '^WBGAP_' "$f")" >&2
        bad=1
      fi
      continue ;;
  esac
  check "$f" "$MODE_RE;$ROW_RE"
  base=$(basename "$f" .c)
  n=$(grep -c "^WBGAP_MODE($base, " "$f" || true)
  all=$(grep -c '^WBGAP_MODE(' "$f" || true)
  if [ "$n" != 1 ] || [ "$all" != 1 ]; then
    echo "gen-rows.sh: $f: needs exactly one WBGAP_MODE($base, ...) line (has $all, $n named $base)" >&2
    bad=1
  fi
  sed -n 's/^WBGAP_MODE(\([A-Z0-9]*\), \([A-Za-z_0-9]*\), NEWROWS)$/WBGAP_GEN_MODE("\1", \2, 1)/p
          s/^WBGAP_MODE(\([A-Z0-9]*\), \([A-Za-z_0-9]*\), SCRIPT)$/WBGAP_GEN_MODE("\1", \2, 0)/p' "$f" >> "$modes"
  sed -n 's/^WBGAP_MODE(\([A-Z0-9]*\), .*$/WBGAP_GEN_MODENAME("\1")/p' "$f" >> "$names"
  sed -n 's/^WBGAP_ROWMODE(\([A-Za-z0-9]*\), \([A-Z0-9]*\))$/WBGAP_GEN_ROWMODE("\1", "\2")/p' "$f" >> "$rowmodes"
done
for f in "$HERE"/../host/rows/*.c; do
  [ -e "$f" ] || continue
  check "$f" "$PLAN_RE"
  sed -n 's/^WBGAP_PLAN(\([A-Za-z_0-9]*\))$/WBGAP_GEN_PLAN(\1)/p' "$f" >> "$plans"
done
if [ "$bad" != 0 ]; then
  rm -f "$modes" "$rowmodes" "$plans" "$names"
  exit 1
fi
