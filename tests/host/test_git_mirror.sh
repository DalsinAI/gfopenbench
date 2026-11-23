#!/bin/sh
# tests/host/test_git_mirror.sh: tools/git-mirror.sh sends git fetches of https://github.com/DalsinAI/... to the
# mirror when the mirror answers, and changes nothing when it doesn't (GF-88); every build-amiga.sh that fetches
# DalsinAI's repositories sources it first. Host only: a bare repository in a temporary folder stands in for the
# mirror (file:// URL).
#   sh tests/host/test_git_mirror.sh
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
M="$HERE/tools/git-mirror.sh"
T=$(mktemp -d); trap 'rm -rf "$T"' EXIT
pass=0; fail=0
ok() { pass=$((pass+1)); echo "ok   $1"; }
no() { fail=$((fail+1)); echo "FAIL $1"; }
unset GIT_CONFIG_COUNT GIT_CONFIG_KEY_0 GIT_CONFIG_VALUE_0 2>/dev/null || true
# a mirror holding DalsinAI/amicradle-guest and DalsinAI/gfopengadtools, one commit each
mkdir -p "$T/src" "$T/mirror/DalsinAI"
git -C "$T/src" init -q
git -C "$T/src" -c user.name=t -c user.email=t@t commit -q --allow-empty -m one
want=$(git -C "$T/src" rev-parse HEAD)
git clone -q --bare "$T/src" "$T/mirror/DalsinAI/amicradle-guest"
git clone -q --bare "$T/src" "$T/mirror/DalsinAI/gfopengadtools"

# 1: the mirror answers: a clone of the GitHub URL, as desktop/build-amiga.sh makes it, comes from it
r=$(AMIGACHROME_GIT_MIRROR="file://$T/mirror/" sh -c '. "$1"; git clone -q --depth 1 https://github.com/DalsinAI/gfopengadtools "$2/a" && git -C "$2/a" rev-parse HEAD' x "$M" "$T" 2>"$T/err1" || true)
[ "$r" = "$want" ] && ok "mirror answers: github.com/DalsinAI cloned from it" || no "mirror answers: got '$r' ($(cat "$T/err1"))"
grep -q "from file://$T/mirror/" "$T/err1" && ok "the build says where DalsinAI's repositories come from" || no "no line naming the mirror"
# 2: the sparse, shallow clone benchmark/build-amiga.sh and guest-index.sh make goes there too
r=$(AMIGACHROME_GIT_MIRROR="file://$T/mirror/" sh -c '. "$1"; git clone -q --depth 1 --filter=blob:none --sparse https://github.com/DalsinAI/amicradle-guest "$2/b" && git -C "$2/b" rev-parse HEAD' x "$M" "$T" 2>/dev/null || true)
[ "$r" = "$want" ] && ok "a sparse clone of github.com/DalsinAI goes to the mirror" || no "sparse clone gave '$r'"
# 3: no mirror: nothing is set
r=$(AMIGACHROME_GIT_MIRROR="file://$T/none/" sh -c '. "$1"; echo "${GIT_CONFIG_COUNT:-unset}"' x "$M" 2>/dev/null)
[ "$r" = unset ] && ok "no mirror: git's settings are left alone" || no "no mirror, but GIT_CONFIG_COUNT=$r"
# 4: only DalsinAI's URLs are rewritten
r=$(AMIGACHROME_GIT_MIRROR="file://$T/mirror/" sh -c '. "$1"; git config --get-regexp "^url\\." ' x "$M" 2>/dev/null || true)
[ "$r" = "url.file://$T/mirror/DalsinAI/.insteadof https://github.com/DalsinAI/" ] && ok "only https://github.com/DalsinAI/ is rewritten" || no "insteadOf is '$r'"
# 5: a GIT_CONFIG_COUNT already set by the caller (the kitchen's own rewrite) is kept
r=$(GIT_CONFIG_COUNT=2 AMIGACHROME_GIT_MIRROR="file://$T/mirror/" sh -c '. "$1"; echo "$GIT_CONFIG_COUNT"' x "$M" 2>/dev/null)
[ "$r" = 2 ] && ok "the caller's own git settings are kept" || no "GIT_CONFIG_COUNT became $r"
# 6: each build-amiga.sh that fetches sources it before it fetches anything
for f in desktop/build-amiga.sh benchmark/build-amiga.sh launcher/build-amiga.sh tests/amiga/build-amiga.sh; do
    src=$(grep -n 'tools/git-mirror\.sh"$' "$HERE/$f" | head -n 1 | cut -d: -f1)
    first=$(grep -n 'git clone\|guest_index_fetch\|fetch [a-z]' "$HERE/$f" | grep -v '^[0-9]*:#' | grep -v '()' | head -n 1 | cut -d: -f1)
    [ -n "$src" ] && { [ -z "$first" ] || [ "$src" -lt "$first" ]; } && ok "$f sources tools/git-mirror.sh first" ||
        no "$f does not source tools/git-mirror.sh before it fetches"
done
# 7: no build script left that clones github.com without it
for f in $(grep -rl 'github\.com/' "$HERE"/*/build-amiga.sh "$HERE"/tests/amiga/*.sh 2>/dev/null); do
    case "$f" in */guest-index.sh) continue ;; esac   # sourced by tests/amiga and launcher's build-amiga.sh (6)
    grep -q 'tools/git-mirror\.sh"$' "$f" && ok "${f#"$HERE"/} has the mirror" || no "${f#"$HERE"/} clones github.com without the mirror"
done
echo "test_git_mirror: $pass passed, $fail failed"
[ $fail = 0 ]
