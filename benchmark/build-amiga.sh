#!/bin/sh
# GFOpenBenchmark for Cradle's stove_build (target benchmark, stove os32-gcc16-open6): Cradle checks out
# this repository alone and sets STOVE and OUT, so this fetches the two checkouts benchmark/build.sh
# needs (amicradle-guest's common/sysprobe and accontrol.device's header; gfopenrtg's
# library/ogles3) at main into $OUT/deps, sparse and shallow, says which commit each was, builds, and
# leaves only the program in $OUT.
# MIT, Copyright (c) 2026 Dalsin Limited / the AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT=${OUT:-$HERE/../build/os3}
STOVE=${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16-open6}
DEPS=${DEPS:-$OUT/deps}
GIT_BASE=${AMIGACHROME_GIT_BASE:-https://github.com/}
mkdir -p "$DEPS"
fetch() {   # fetch NAME PATH...
    name=$1; shift
    if [ ! -d "$DEPS/$name/.git" ]; then
        git clone -q --depth 1 --filter=blob:none --sparse "${GIT_BASE}DalsinAI/$name" "$DEPS/$name" ||
            { echo "build-amiga.sh: couldn't fetch DalsinAI/$name" >&2; exit 1; }
        git -C "$DEPS/$name" sparse-checkout set "$@"
    fi
    echo "build-amiga.sh: $name at $(git -C "$DEPS/$name" rev-parse --short HEAD)"
}
fetch amicradle-guest common/sysprobe devices/accontrol.device/include
fetch gfopenrtg library/ogles3
OS32_GCC16=$STOVE GUEST="$DEPS/amicradle-guest" ORTG="$DEPS/gfopenrtg" sh "$HERE/build.sh" "$OUT"
rm -rf "$DEPS"
