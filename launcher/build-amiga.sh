#!/bin/sh
# gfopenlaunch.library for Cradle's stove_build (target launcher): the top build.sh, which builds it with the other
# libraries, into Cradle's OUT so the job lists it. MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
OUT=${OUT:-$HERE/../build/os3}
sh "$HERE/../build.sh" "$OUT"
