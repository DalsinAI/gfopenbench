# Sourced by tests/amiga/build-amiga.sh and launcher/build-amiga.sh (WBGapTest row X2; Ivanova's full run, 14 November
# 2026: "X2 needs WBGapIndex, which the launcher job doesn't build"). Cradle checks out this repository alone, so when
# AMICRADLE_GUEST isn't set this fetches the two parts of amicradle-guest that WBGapIndex needs (tools/openindex and
# acscsi.device's header), at main, shallow and sparse, into $GI_DEPS, says which commit, and sets AMICRADLE_GUEST.
# A fetch that fails leaves it unset: WBGapIndex isn't built and the rest of the build goes on, as before.
# guest_index_done removes what was fetched. AMIGACHROME_GIT_BASE: where DalsinAI/<repo> is (default GitHub).
# MIT, Copyright (c) 2026 AmiCradle Team.
GI_FETCHED=""
guest_index_fetch() {   # guest_index_fetch DEPS_DIR
    [ -n "${AMICRADLE_GUEST:-}" ] && return 0
    GI_DEPS=$1
    gi_dir="$GI_DEPS/amicradle-guest"
    gi_base=${AMIGACHROME_GIT_BASE:-https://github.com/}
    mkdir -p "$GI_DEPS"
    if [ ! -d "$gi_dir/.git" ]; then
        if ! git clone -q --depth 1 --filter=blob:none --sparse "${gi_base}DalsinAI/amicradle-guest" "$gi_dir" 2>/dev/null ||
           ! git -C "$gi_dir" sparse-checkout set tools/openindex devices/acscsi.device/include; then
            rm -rf "$gi_dir"
            echo "build-amiga.sh: amicradle-guest not fetched; WBGapIndex not built"
            return 0
        fi
        GI_FETCHED=$gi_dir
    fi
    echo "build-amiga.sh: amicradle-guest at $(git -C "$gi_dir" rev-parse --short HEAD) (tools/openindex, for WBGapIndex)"
    AMICRADLE_GUEST=$gi_dir
    export AMICRADLE_GUEST
}
guest_index_done() {
    [ -n "$GI_FETCHED" ] && rm -rf "$GI_FETCHED"
    [ -n "${GI_DEPS:-}" ] && rmdir "$GI_DEPS" 2>/dev/null
    GI_FETCHED=""
    return 0
}
