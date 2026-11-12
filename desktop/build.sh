#!/bin/sh
# GFOpenBench, the desktop (desktop/openbench.c), with gfopengadtools' drawer window (workbench/wb_*.c) and
# its theme and drawing (lib/), from a gfopengadtools checkout: OGT, default ../../gfopengadtools; and GFOpenFiles'
# copy engine (engine/of_*.c), for drag and drop (0.1.26, ob_drop.c), from a gfopenfilemanager checkout:
# FILES, default ../../gfopenfilemanager.
#   desktop/build.sh [OUT_DIR]   (default build/os3)
# The compiler: OS32_GCC16 (as GFOpenFTP's build-os3.sh), else STOVE, else ~/AmiCradle/stoves/os32-gcc16.
# Not the os32 stove's GCC 6.5.0b: at -O2 its peephole2 clears a word and the field after it with one clr.l
# (it broke GFOpenCompress's LhA reader and zlib's deflate, both built into programs like this one), so this
# script stops when given a GCC older than 16. GCC 16.2.0b has a trap of its own: at -O2 it can turn a loop
# that shifts an array up into a forward copy, so everything here is built -fno-tree-loop-distribute-patterns,
# as GFOpenFTP and GFOpenFiles are (the stove leaves it on).
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")" && pwd)
# Soft float: the specs file links libnix's soft libm020 (maths through the
# mathieee libraries), not libm881, so one build runs on every 68040.
SPECS="-specs=$(cd "$HERE/.." && pwd)/toolchain/m68k-soft-m68040.specs"
OGT=${OGT:-$HERE/../../gfopengadtools}
STOVE=${OS32_GCC16:-${STOVE:-$HOME/AmiCradle/stoves/os32-gcc16}}
CC="$STOVE/prefix/bin/m68k-amigaos-gcc"
GCCV=$("$CC" -dumpversion 2>/dev/null || echo 0)
[ "${GCCV%%.*}" -ge 16 ] 2>/dev/null || { echo "$CC is GCC $GCCV: GFOpenBench needs GCC 16 (the os32-gcc16 stove; set OS32_GCC16)"; exit 2; }
OUT=${1:-$HERE/../build/os3}
mkdir -p "$OUT"
# gfopenprefs' Common headers (obmsg.h, wbclose.h, barlogo.h, and since 0.1.37 deskicons.h: gfopenprefs main
# with GFOpenPrefs 2.5 or later) and Setup: OAP, default ../../gfopenprefs. The
# dock is GFOpenDock, a program of its own (C:GFOpenDock), which GFOpenBench starts (0.1.23).
OAP=${OAP:-$HERE/../../gfopenprefs}
W="$OGT/workbench"
L="$OGT/lib"
C="$OAP/Common"
FLAGS="-noixemul -m68040 $SPECS -std=gnu99 -Wall -Werror -O2 -fno-tree-loop-distribute-patterns -fno-delete-null-pointer-checks -fno-common"
# GCC 16.2.0-open3's late-combine pass has a wrong-code bug (GCC PR 123853): at -O2 it can copy with (aN)+ and use aN
# again in the same move's destination, which the 68k works out after the step, so each store lands one place late.
# wbclose.h's strcpy(r->name, "OpenWBNotify") (libnix's inline strcpy) came out that way, so GFOpenBench's
# semaphore was named "" and FindSemaphore("OpenWBNotify") never found it. Off until GFOpenAmigaGCC open4
# (gfopengcc #29), on a compiler that knows the option.
if "$CC" -fno-late-combine-instructions -S -o /dev/null -x c /dev/null 2>/dev/null; then FLAGS="$FLAGS -fno-late-combine-instructions"; fi
TMP=$(mktemp -d)
# shellcheck disable=SC2086
# The programs built in (ob_builtin.h, ob_builtin_list.h): each block compiles one program's code with
# -DIN_OPENBENCH and its main() renamed into $TMP, adds its objects to BUILTIN_OBJS and its -DOB_HAVE_<NAME>
# to BUILTIN_DEFS. Keep the blocks in alphabetical order.
BUILTIN_OBJS=""
BUILTIN_DEFS=""
# --- built-ins: begin ---
# The editors GFOpenPrefs has as pages (Look, Menus, GFOpenDateTime, GFOpenDock's editor, GFOpenGamepads, GFOpenInput,
# GFOpenSerial, GFOpenSound, GFOpenTypes, TitleBar, Windows) were built in up to GFOpenBench 0.1.31; since 0.1.32 (the
# no-ROM-volume design of 24 October 2026) their old names open GFOpenPrefs at their page (openbench.c,
# old_prefs_open), and only these two are built in.
# GFOpenTata: Tata's window (gfopentata src/prefs/tataprefs.c), SYS:Prefs/GFOpenTata, with Tata's inspection
# (src/tata_inspect.c and the code it shares with C:Tata) linked in for openbench.c (OB_HAVE_TATA): before
# GFOpenBench runs a program or opens a project it asks Tata itself, with no patch (the Team, 11 October 2026:
# Tata a built-in feature of GFOpenBench). From a gfopentata checkout: TATA, default ../../gfopentata;
# without one GFOpenBench is built without Tata, and says so. Tata's list helpers are compiled once, here.
TATA=${TATA:-$HERE/../../gfopentata}
OB_TATA_DEFS=""
if [ -f "$TATA/src/tata_inspect.c" ]; then
    TS="$TATA/src"
    # Tata's sources with Tata's own allowance (its build.sh): GCC 16's truncation warnings stay warnings,
    # on a compiler that knows them (GCC 6.5 refuses -Wno-error= for one it doesn't)
    TATAF="$FLAGS"
    for w in format-truncation stringop-truncation; do
        if echo 'int x;' | "$CC" -Werror -Wno-error=$w -fsyntax-only -x c - 2>/dev/null; then TATAF="$TATAF -Wno-error=$w"; fi
    done
    for f in tata_db tata_files tata_inspect tata_ui tata_ads; do   # tata_ads: the Ads page (gfopentata 0.6.0)
        # shellcheck disable=SC2086
        "$CC" $TATAF -I"$TS" -DIN_OPENBENCH -c -o "$TMP/tata_$f.o" "$TS/$f.c"
        BUILTIN_OBJS="$BUILTIN_OBJS $TMP/tata_$f.o"
    done
    # the hashes at -O1, as Tata's own build.sh (GCC 6.5's -O2 has broken loops of their shape)
    # shellcheck disable=SC2086
    "$CC" $(echo $TATAF | sed 's/-O2/-O1/') -fno-lto -I"$TS" -DIN_OPENBENCH -c -o "$TMP/tata_hash.o" "$TS/tata_hash.c"
    # shellcheck disable=SC2086
    "$CC" $TATAF -Wno-pointer-sign -I"$TS" -I"$L" -DIN_OPENBENCH -Dmain=opentata_main -c -o "$TMP/opentata.o" "$TS/prefs/tataprefs.c"
    # its toolbar (the other built-ins draw none: compiled here, once)
    # shellcheck disable=SC2086
    "$CC" $FLAGS -I"$L" -c -o "$TMP/ogt_toolbar.o" "$L/ogt_toolbar.c"
    BUILTIN_OBJS="$BUILTIN_OBJS $TMP/tata_hash.o $TMP/opentata.o $TMP/ogt_toolbar.o"
    BUILTIN_DEFS="$BUILTIN_DEFS -DOB_HAVE_OPENTATA"
    OB_TATA_DEFS="-DOB_HAVE_TATA -I$TS"
    echo "GFOpenBench: Tata built in, from $TATA ($(cd "$TATA" && git rev-parse --short HEAD 2>/dev/null || echo 'not a git checkout'))"
else
    echo "GFOpenBench: Tata not built in (no gfopentata at $TATA: set TATA)"
fi
# GFOpenUp-Setup: the first-start wizard (gfopenprefs Setup/src/setup.c), SYS:Prefs/GFOpenUp-Setup, and
# SYS:WBStartup/GFOpenUp-Setup at the first start
# shellcheck disable=SC2086
"$CC" $FLAGS -DIN_OPENBENCH -Dmain=openup_setup_main -c -o "$TMP/openup_setup.o" "$OAP/Setup/src/setup.c"
BUILTIN_OBJS="$BUILTIN_OBJS $TMP/openup_setup.o"
BUILTIN_DEFS="$BUILTIN_DEFS -DOB_HAVE_OPENUP_SETUP"
# --- built-ins: end ---
# shellcheck disable=SC2086
"$CC" $FLAGS $BUILTIN_DEFS -I"$HERE" -c -o "$TMP/ob_builtin.o" "$HERE/ob_builtin.c"
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_malloc.o" "$HERE/ob_malloc.c"
# shellcheck disable=SC2086
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_info.o" "$HERE/ob_info.c"
# No icons on the desktop (0.1.37): what GFOpenBench does when ENV:Sys/DesktopIcons changes, plain C (tests/host)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_show.o" "$HERE/ob_show.c"
# Snapshot's record (0.1.38): the icon places GFOpenBench wrote, plain C (tests/host). Its drawers' half is gfopengadtools'
# wb_drawer.c with wb_look's snap and snapped (gfopengadtools main with Snapshot, 25 October 2026, or later)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_snap.o" "$HERE/ob_snap.c"
# The ENV: files GFOpenBench follows, on one notify port (0.1.39): what each asks for, plain C (tests/host)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_notify.o" "$HERE/ob_notify.c"
# The mounted volumes' fingerprint (0.1.40): a volume mounted or dismounted with no disk event, seen on the 2-second
# tick, plain C (tests/host)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_vols.o" "$HERE/ob_vols.c"
# A disk's .backdrop (0.1.41): Icons > Leave Out's line added and Put Away's taken out, plain C (tests/host). Its
# drawers' half is gfopengadtools' wb_drawer.c with wb_look's leave_out (gfopengadtools main with Leave Out, 25 October
# 2026, or later)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_backdrop.o" "$HERE/ob_backdrop.c"
# The host's cores in the screen's title (0.1.45): read from the host's status, plain C (tests/host)
# shellcheck disable=SC2086
"$CC" $FLAGS -c -o "$TMP/ob_host.o" "$HERE/ob_host.c"
# Drag and drop (0.1.26): ob_drop.c, with GFOpenFiles' engine built for the Amiga (-DOF_AMIGA: its memory from
# AllocVec, as it runs in a process of its own)
FILES=${FILES:-$HERE/../../gfopenfilemanager}
E="$FILES/engine"
[ -f "$E/of_ops.c" ] || { echo "desktop/build.sh: no gfopenfilemanager at $FILES (set FILES)" >&2; exit 1; }
echo "GFOpenBench: GFOpenFiles' engine from $FILES ($(cd "$FILES" && git rev-parse --short HEAD 2>/dev/null || echo 'not a git checkout'))"
DROP_OBJS=""
for f in of_ops of_path of_fs_amiga of_alloc_amiga; do
    # shellcheck disable=SC2086
    "$CC" $FLAGS -DOF_AMIGA -I"$E" -c -o "$TMP/$f.o" "$E/$f.c"
    DROP_OBJS="$DROP_OBJS $TMP/$f.o"
done
# shellcheck disable=SC2086
"$CC" $FLAGS -DOF_AMIGA -I"$E" -I"$W" -I"$L" -c -o "$TMP/ob_drop.o" "$HERE/ob_drop.c"
DROP_OBJS="$DROP_OBJS $TMP/ob_drop.o"
# wb_dir, as it is (0.1.23 to 0.1.31 renamed its wb_dir_read and wb_entry_icon for ROM:Prefs's editors; the ROM has
# no ROM: since the no-ROM-volume design, 24 October 2026)
# shellcheck disable=SC2086
"$CC" $FLAGS -I"$W" -I"$L" -c -o "$TMP/wb_dir.o" "$W/wb_dir.c"
# The drawers' search of every drive (0.1.49): gfopengadtools' wb_find.c with ACFind's code from an amicradle-guest
# checkout, GUEST, default ../../amicradle-guest (tools/openindex, and acscsi.device's header); without one,
# gfopengadtools' wb_find_none.c, where the search only beeps, and this says so
GUEST=${GUEST:-$HERE/../../amicradle-guest}
if [ -f "$GUEST/tools/openindex/amiga/acfind_live.c" ] && [ -f "$W/wb_find.c" ]; then
    OIX="$GUEST/tools/openindex/amiga"
    OIXC="$GUEST/tools/openindex/core"
    FIND_FLAGS="-I$OIX -I$OIXC -I$GUEST/devices/acscsi.device/include"
    FIND_SRC="$W/wb_find.c $OIX/oix_acfs.c $OIX/acfind_live.c $OIXC/oix_core.c $OIXC/acf_live.c"
    echo "GFOpenBench: the search of every drive from $GUEST ($(cd "$GUEST" && git rev-parse --short HEAD 2>/dev/null || echo 'not a git checkout'))"
else
    FIND_FLAGS=""
    FIND_SRC="$W/wb_find_none.c"
    echo "GFOpenBench: no amicradle-guest at $GUEST (set GUEST): drawers built without the search of every drive"
fi
# shellcheck disable=SC2086
"$CC" $FLAGS $FIND_FLAGS $OB_TATA_DEFS -I"$W" -I"$L" -I"$C" -o "$OUT/GFOpenBench" "$HERE/openbench.c" "$TMP/ob_builtin.o" "$TMP/ob_malloc.o" "$TMP/ob_info.o" "$TMP/ob_show.o" "$TMP/ob_snap.o" "$TMP/ob_notify.o" "$TMP/ob_vols.o" "$TMP/ob_backdrop.o" "$TMP/ob_host.o" \
    $BUILTIN_OBJS $DROP_OBJS \
    "$W/wb_drawer.c" "$TMP/wb_dir.o" "$W/wb_fmt.c" "$W/wb_pic.c" "$W/wb_ghost.c" "$W/wb_glyph.c" "$W/wb_thumb.c" "$W/wb_grid.c" $FIND_SRC \
    "$L/ogt_theme.c" "$L/ogt_draw.c" "$L/ogt_icons.c" "$L/ogt_font.c" -lamiga
rm -rf "$TMP"
echo "$OUT/GFOpenBench ($(wc -c < "$OUT/GFOpenBench") bytes)"
