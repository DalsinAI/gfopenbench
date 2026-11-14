#!/bin/sh
# The WBGap ARexx scripts (tests/amiga/*.rexx) leave no drawer window of theirs open: each RAM: drawer a script makes
# (MakeDir RAM:<name>) has a WINDOW 'RAM:<name>' CLOSE in the same script, so the rows after it (W2's Amiga+Tab)
# don't find it on the screen (gap count 14 November 2026: X3 left RAM:WBGapX3d open).
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/../.." && pwd)
fails=0
checked=0
for f in "$HERE"/tests/amiga/*.rexx; do
  for d in $(sed -n "s/.*MakeDir \(RAM:[A-Za-z0-9_/.-]*\).*/\1/p" "$f"); do
    checked=$((checked + 1))
    if grep -q "WINDOW '$d' CLOSE" "$f"; then
      echo "ok   $(basename "$f"): $d closed"
    else
      echo "FAIL $(basename "$f"): $d made but its window never closed"
      fails=$((fails + 1))
    fi
  done
done
# the check finds what it is meant to: a script that makes a drawer and never closes it fails
T=$(mktemp -d "${TMPDIR:-/tmp}/rexx-drawers.XXXXXX")
trap 'rm -rf "$T"' EXIT
printf "ADDRESS COMMAND 'MakeDir RAM:WBGapZ >NIL:'\nEXIT 0\n" > "$T/bad.rexx"
if sed -n "s/.*MakeDir \(RAM:[A-Za-z0-9_/.-]*\).*/\1/p" "$T/bad.rexx" | grep -q '^RAM:WBGapZ$' &&
   ! grep -q "WINDOW 'RAM:WBGapZ' CLOSE" "$T/bad.rexx"; then
  echo "ok   a drawer made and never closed is found"
else
  echo "FAIL a drawer made and never closed is not found"
  fails=$((fails + 1))
fi
[ "$checked" -gt 0 ] || { echo "FAIL no MakeDir RAM: found in tests/amiga/*.rexx"; fails=$((fails + 1)); }
echo "test_rexx_drawers: $checked drawers, $fails failures"
[ "$fails" -eq 0 ]
