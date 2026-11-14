#!/bin/sh
# tests/amiga/build-amiga.sh with WBGapIndex (row X2): with AMICRADLE_GUEST unset it fetches amicradle-guest's
# tools/openindex itself, builds WBGapIndex and leaves no sources in OUT; with no amicradle-guest to fetch it builds
# the rest and says WBGapIndex wasn't built. A stand-in stove (its m68k-amigaos-gcc writes each -o file and logs
# its sources) and a local DalsinAI/amicradle-guest (AMIGACHROME_GIT_BASE) stand in for Cradle's.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/../.." && pwd)
T=$(mktemp -d "${TMPDIR:-/tmp}/guest-index.XXXXXX")
trap 'rm -rf "$T"' EXIT
fails=0
ok() { echo "ok   $1"; }
bad() { echo "FAIL $1"; fails=$((fails + 1)); }

# the stand-in stove
mkdir -p "$T/stove/prefix/bin"
cat > "$T/stove/prefix/bin/m68k-amigaos-gcc" << 'GCC'
#!/bin/sh
out=""
for a in "$@"; do
  case "$a" in -dumpversion) echo 16.2.0; exit 0 ;; esac
done
prev=""
for a in "$@"; do
  [ "$prev" = "-o" ] && out=$a
  case "$a" in *.c) echo "$a" >> "${GCC_LOG:?}" ;; esac
  prev=$a
done
[ -n "$out" ] && [ "$out" != /dev/null ] && echo built > "$out"
exit 0
GCC
chmod +x "$T/stove/prefix/bin/m68k-amigaos-gcc"

# a local amicradle-guest with the files WBGapIndex is built from, and one it isn't
G="$T/git/DalsinAI/amicradle-guest"
mkdir -p "$G/tools/openindex/amiga" "$G/tools/openindex/core" "$G/devices/acscsi.device/include" "$G/libraries/big"
echo '/* stand-in */' > "$G/tools/openindex/amiga/oix_acfs.c"
echo '/* stand-in */' > "$G/tools/openindex/core/oix_core.c"
echo '/* stand-in */' > "$G/devices/acscsi.device/include/acscsi.h"
echo 'not wanted' > "$G/libraries/big/big.c"
git -C "$G" init -q
git -C "$G" -c user.name=t -c user.email=t@t add -A
git -C "$G" -c user.name=t -c user.email=t@t commit -q -m guest

# 1. fetched: WBGapIndex built from the fetched openindex, nothing of the fetch left
OUT="$T/out1"
GCC_LOG="$T/log1" STOVE="$T/stove" OUT="$OUT" AMIGACHROME_GIT_BASE="file://$T/git/" \
  sh "$HERE/tests/amiga/build-amiga.sh" > "$T/run1" 2>&1 || { cat "$T/run1"; bad "build-amiga.sh with a guest to fetch: rc $?"; }
if [ -f "$OUT/WBGapIndex" ]; then ok "WBGapIndex built"; else bad "WBGapIndex not built"; cat "$T/run1"; fi
if grep -q "tools/openindex/amiga/oix_acfs.c" "$T/log1" && grep -q "tests/amiga/WBGapIndex.c" "$T/log1"; then
  ok "from the fetched tools/openindex"; else bad "WBGapIndex's sources not compiled"; fi
if grep -q "amicradle-guest at " "$T/run1"; then ok "the commit is said"; else bad "no commit said"; fi
if [ ! -e "$OUT/deps" ]; then ok "the fetched sources are gone from OUT"; else bad "OUT/deps left: $(ls -R "$OUT/deps" | head -5)"; fi
if [ -f "$OUT/WBGapTest" ] && [ -f "$OUT/WBGapTest.script" ]; then ok "WBGapTest and its script as before"; else bad "WBGapTest missing"; fi

# 2. sparse: only the two parts are checked out (guest-index.sh called on its own, so the clone can be looked at)
. "$HERE/tests/amiga/guest-index.sh"
( AMICRADLE_GUEST=""; unset AMICRADLE_GUEST; AMIGACHROME_GIT_BASE="file://$T/git/"
  guest_index_fetch "$T/deps2" > /dev/null
  if [ -f "$T/deps2/amicradle-guest/tools/openindex/core/oix_core.c" ] && [ ! -e "$T/deps2/amicradle-guest/libraries/big" ]; then
    echo "ok   sparse: tools/openindex there, the rest not checked out"
  else
    echo "FAIL sparse checkout: $(ls "$T/deps2/amicradle-guest")"
  fi
  guest_index_done
  [ ! -e "$T/deps2" ] && echo "ok   guest_index_done removes the fetch" || echo "FAIL guest_index_done left $T/deps2"
) > "$T/run2"
cat "$T/run2"
fails=$((fails + $(grep -c '^FAIL' "$T/run2" || true)))

# 3. a checkout given: nothing fetched, that one used
OUT="$T/out3"
GCC_LOG="$T/log3" STOVE="$T/stove" OUT="$OUT" AMICRADLE_GUEST="$G" AMIGACHROME_GIT_BASE="file://$T/nowhere/" \
  sh "$HERE/tests/amiga/build-amiga.sh" > "$T/run3" 2>&1 || bad "build-amiga.sh with AMICRADLE_GUEST: rc $?"
if [ -f "$OUT/WBGapIndex" ] && grep -q "$G/tools/openindex/amiga/oix_acfs.c" "$T/log3" && ! grep -q "amicradle-guest at" "$T/run3"; then
  ok "AMICRADLE_GUEST given: used, nothing fetched"; else bad "AMICRADLE_GUEST given"; cat "$T/run3"; fi
[ -e "$G/tools/openindex" ] && ok "the given checkout is left alone" || bad "the given checkout was removed"

# 4. nothing to fetch: the rest still built, WBGapIndex said not built, rc 0
OUT="$T/out4"
if GCC_LOG="$T/log4" STOVE="$T/stove" OUT="$OUT" AMIGACHROME_GIT_BASE="file://$T/nowhere/" \
  sh "$HERE/tests/amiga/build-amiga.sh" > "$T/run4" 2>&1; then ok "no guest to fetch: rc 0"; else bad "no guest to fetch: rc $?"; fi
if [ ! -e "$OUT/WBGapIndex" ] && [ -f "$OUT/WBGapTest" ] && grep -q "WBGapIndex not built" "$T/run4"; then
  ok "no guest: WBGapTest built, WBGapIndex said not built"; else bad "no guest"; cat "$T/run4"; fi

[ "$fails" -eq 0 ] && echo "test_guest_index: all passed" || echo "test_guest_index: $fails failed"
exit $(( fails != 0 ))
