#!/bin/sh
# tools/stove-dir.sh: the kitchen stove first, the old path only when it alone is there,
# and every build script's default goes through it.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/../.." && pwd)
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
fail=0
want() { got=$(HOME="$T/$1" sh "$HERE/tools/stove-dir.sh"); [ "$got" = "$T/$1/$2" ] || { echo "stove-dir $1: got $got, want $T/$1/$2"; fail=1; }; }
mkdir -p "$T/k/AmiCradle/kitchen/stoves/os32-gcc16" "$T/o/AmiCradle/stoves/os32-gcc16" \
  "$T/b/AmiCradle/kitchen/stoves/os32-gcc16" "$T/b/AmiCradle/stoves/os32-gcc16" "$T/n"
want k AmiCradle/kitchen/stoves/os32-gcc16
want o AmiCradle/stoves/os32-gcc16
want b AmiCradle/kitchen/stoves/os32-gcc16
want n AmiCradle/kitchen/stoves/os32-gcc16
for f in build.sh desktop/build.sh desktop/build-amiga.sh benchmark/build.sh benchmark/build-amiga.sh \
  meters/build.sh tests/amiga/build-amiga.sh .github/workflows/stove.yml; do
  grep -q 'stove-dir.sh' "$HERE/$f" || { echo "$f: default stove not from tools/stove-dir.sh"; fail=1; }
  if grep -q '\$HOME/AmiCradle/stoves/os32-gcc16' "$HERE/$f"; then echo "$f: still names the old stove path"; fail=1; fi
done
[ $fail = 0 ] && echo "stove-dir: kitchen first, 8 scripts use it"
exit $fail
