# tools/git-mirror.sh, sourced by the build-amiga.sh scripts (sh, set -eu) before they fetch anything.
# GitHub gives DalsinAI's private repositories only with a login, and a build outside the kitchen may have
# none. Where the git mirror on the kitchen's machine answers (LumaVAULT's, AMIGACHROME_GIT_MIRROR, default
# http://127.0.0.1:8418/), every git fetch of https://github.com/DalsinAI/... in the build goes there instead,
# through git's own insteadOf (GF-88, as gfamigaamule and gfamigardp do). Elsewhere nothing changes, and git
# settings the caller set already (GIT_CONFIG_COUNT: the kitchen's own rewrite) are kept.
# MIT, Copyright (c) 2026 AmiCradle Team.
GIT_MIRROR=${AMIGACHROME_GIT_MIRROR:-http://127.0.0.1:8418/}
GIT_MIRROR_PROBE=${GIT_MIRROR_PROBE:-amicradle-guest}
GIT_TERMINAL_PROMPT=0
export GIT_TERMINAL_PROMPT
if [ -z "${GIT_CONFIG_COUNT:-}" ] &&
   git ls-remote "${GIT_MIRROR}DalsinAI/${GIT_MIRROR_PROBE}" HEAD >/dev/null 2>&1; then
    GIT_CONFIG_COUNT=1
    GIT_CONFIG_KEY_0="url.${GIT_MIRROR}DalsinAI/.insteadOf"
    GIT_CONFIG_VALUE_0="https://github.com/DalsinAI/"
    export GIT_CONFIG_COUNT GIT_CONFIG_KEY_0 GIT_CONFIG_VALUE_0
    echo "build-amiga.sh: DalsinAI's repositories from ${GIT_MIRROR} (the git mirror)" >&2
fi
