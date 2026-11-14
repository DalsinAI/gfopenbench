#!/bin/sh
# The WBGap ARexx scripts (tests/amiga/*.rexx) leave no drawer window of theirs open: every RAM: drawer named on a
# MakeDir line of a script (MakeDir RAM:<a> RAM:<b> ...) has a WINDOW 'RAM:<name>' CLOSE in the same script, so the
# rows after it (W2's Amiga+Tab) don't find it on the screen (gap count 14 November 2026: X3 left RAM:WBGapX3d open).
# Drawers made another way (NEWDRAWER, WBGapTest SETUP) are not looked at: this guards the MakeDir case only.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/../.." && pwd)
fails=0
checked=0

# the RAM: drawers named on file's MakeDir lines, one a line
made() {
  grep 'MakeDir' "$1" | sed 's/.*MakeDir//' | tr ' ' '\n' | sed -n "s/^'*\(RAM:[A-Za-z0-9_/.-]*\)'*$/\1/p"
}

# check_file FILE: one "ok"/"FAIL" line for each drawer FILE makes; sets checked and bad for the caller
check_file() {
  bad=0
  for d in $(made "$1"); do
    checked=$((checked + 1))
    if grep -q "WINDOW '$d' CLOSE" "$1"; then
      echo "ok   $(basename "$1"): $d closed"
    else
      echo "FAIL $(basename "$1"): $d made but its window never closed"
      bad=$((bad + 1))
    fi
  done
}

for f in "$HERE"/tests/amiga/*.rexx; do
  check_file "$f"
  fails=$((fails + bad))
done
real=$checked

# the check finds what it is meant to, through the same check_file: of two drawers made on one line, the one
# never closed fails and the closed one doesn't
T=$(mktemp -d "${TMPDIR:-/tmp}/rexx-drawers.XXXXXX")
trap 'rm -rf "$T"' EXIT
printf "ADDRESS COMMAND 'MakeDir RAM:WBGapY RAM:WBGapZ >NIL:'\nWINDOW 'RAM:WBGapY' CLOSE\nEXIT 0\n" > "$T/bad.rexx"
out=$(check_file "$T/bad.rexx")
check_file "$T/bad.rexx" > /dev/null
if [ "$bad" -eq 1 ] && echo "$out" | grep -q 'FAIL bad.rexx: RAM:WBGapZ' && echo "$out" | grep -q 'ok   bad.rexx: RAM:WBGapY'; then
  echo "ok   of two drawers on one MakeDir line, the one never closed is found"
else
  echo "FAIL the stand-in script's drawers were not judged right: $out"
  fails=$((fails + 1))
fi
[ "$real" -gt 0 ] || { echo "FAIL no MakeDir RAM: found in tests/amiga/*.rexx"; fails=$((fails + 1)); }
echo "test_rexx_drawers: $real drawers, $fails failures"
[ "$fails" -eq 0 ]
