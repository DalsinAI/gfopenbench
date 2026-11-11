#!/bin/sh
# Scratch: build meters in Cradle's cradle-builds checkout (siblings named owner__repo). Never merged.
HERE=$(cd "$(dirname "$0")" && pwd)
OGT=$HERE/../../DalsinAI__gfopengadtools OAP=$HERE/../../DalsinAI__gfopenprefs OS32_GCC16=${STOVE:-} exec sh "$HERE/build.sh" "${OUT:-$HERE/../build/os3}"
