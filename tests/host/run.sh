#!/bin/sh
# GFOpenBench host tests: wbspy's lines (tools/spyfmt.c), workbench.library's
# lists (workbench/wb_core.c), the desktop's No icons (desktop/ob_show.c), Snapshot's
# record (desktop/ob_snap.c), the ENV: files the desktop follows (desktop/ob_notify.c), the mounted volumes'
# fingerprint (desktop/ob_vols.c), a disk's .backdrop (desktop/ob_backdrop.c), the backdrop windows' places (desktop/ob_strip.h), and icon.library's core
# (icon/icon_core.c) against icons made by Python's own zlib.
# MIT, Copyright (c) 2026 AmiCradle Team.
set -eu
HERE=$(cd "$(dirname "$0")/../.." && pwd)
OUT=${TMPDIR:-/tmp}/openbench-host-tests
mkdir -p "$OUT"
cc -std=c99 -Wall -Wextra -Werror -O1 -I"$HERE/tools" "$HERE/tools/spyfmt.c" "$HERE/tests/host/test_spyfmt.c" -o "$OUT/test_spyfmt"
# wbgap_rows: WBGapTest's gap ids and its one line per id (tests/amiga/wbgap_rows.c; Workbench build row 7)
cc -std=c99 -Wall -Wextra -Werror -O1 -I"$HERE/tests/amiga" "$HERE/tests/amiga/wbgap_rows.c" \
  "$HERE/tests/host/test_wbgap_rows.c" -o "$OUT/test_wbgap_rows"
# wbgap_menu: the mouse's path to a menu item for WBGapTest MENUDRIVE (Workbench build row 7)
cc -std=c99 -Wall -Wextra -Werror -O1 -I"$HERE/tests/amiga" "$HERE/tests/amiga/wbgap_menu.c" \
  "$HERE/tests/host/test_wbgap_menu.c" -o "$OUT/test_wbgap_menu"
"$OUT/test_wbgap_menu"
# wbgap_index: the ACFS index's "::wb" lines as WBGapIndex reads them (tests/amiga/wbgap_index.c; build row 8's rows)
cc -std=c99 -Wall -Wextra -Werror -O1 -I"$HERE/tests/amiga" "$HERE/tests/amiga/wbgap_index.c" \
  "$HERE/tests/host/test_wbgap_index.c" -o "$OUT/test_wbgap_index"
"$OUT/test_wbgap_index"
"$OUT/test_wbgap_rows"
"$OUT/test_spyfmt"

# icon_core, with the address and undefined-behaviour checkers where the
# compiler has them
SAN="-fsanitize=address,undefined -fno-sanitize-recover=all"
echo 'int main(void){return 0;}' > "$OUT/san.c"
cc $SAN "$OUT/san.c" -o "$OUT/san" 2>/dev/null && "$OUT/san" || SAN=""
# shellcheck disable=SC2086
cc -std=gnu99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/icon" "$HERE/icon/icon_core.c" \
  "$HERE/tests/host/test_icon_core.c" -o "$OUT/test_icon_core"
# wb_core: workbench.library's lists (step L1)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/workbench" "$HERE/workbench/wb_core.c" \
  "$HERE/tests/host/test_wb_core.c" -o "$OUT/test_wb_core"
"$OUT/test_wb_core"
# ob_show: the desktop's No icons (GFOpenBench 0.1.37)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_show.c" \
  "$HERE/tests/host/test_ob_show.c" -o "$OUT/test_ob_show"
"$OUT/test_ob_show"
# ob_snap: Snapshot's record, the icon places GFOpenBench keeps (GFOpenBench 0.1.38)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_snap.c" \
  "$HERE/tests/host/test_ob_snap.c" -o "$OUT/test_ob_snap"
"$OUT/test_ob_snap"
# ob_notify: the ENV: files the desktop follows on one notify port, what each asks for (GFOpenBench 0.1.39)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_notify.c" "$HERE/desktop/ob_show.c" \
  "$HERE/tests/host/test_ob_notify.c" -o "$OUT/test_ob_notify"
"$OUT/test_ob_notify"
# ob_vols: the mounted volumes' fingerprint, a volume mounted or dismounted with no disk event (GFOpenBench 0.1.40)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_vols.c" \
  "$HERE/tests/host/test_ob_vols.c" -o "$OUT/test_ob_vols"
# ob_eject: which way Icons > Eject takes a disk, floppy, PCMCIA card or not removable (GFOpenBench 0.1.58, row M35)
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_eject.c" \
  "$HERE/tests/host/test_ob_eject.c" -o "$OUT/test_ob_eject"
"$OUT/test_ob_eject"
"$OUT/test_ob_vols"
# ob_stack: the Information window's stack field, a whole number of bytes (gap N6)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_stack.c" \
  "$HERE/tests/host/test_ob_stack.c" -o "$OUT/test_ob_stack"
"$OUT/test_ob_stack"
# ob_backdrop: a disk's .backdrop, Icons > Leave Out's line added and Put Away's taken out (GFOpenBench 0.1.41)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_backdrop.c" \
  "$HERE/tests/host/test_ob_backdrop.c" -o "$OUT/test_ob_backdrop"
"$OUT/test_ob_backdrop"
# ob_strip: the root and the strip under the screen's title bar (GFOpenBench 0.1.43)
cc -std=c99 -Wall -Wextra -Werror -O1 -I"$HERE/desktop" "$HERE/tests/host/test_ob_strip.c" -o "$OUT/test_ob_strip"
"$OUT/test_ob_strip"
# ob_host: the host's cores in the screen's title, from the host's status (GFOpenBench 0.1.45)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_host.c" \
  "$HERE/tests/host/test_ob_host.c" -o "$OUT/test_ob_host"
"$OUT/test_ob_host"
# ob_order: RAM Disk first, the boot volume second, on the desktop's column (GFOpenBench 0.1.46)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/tests/host/test_ob_order.c" -o "$OUT/test_ob_order"
"$OUT/test_ob_order"
# ob_startup: SYS:WBStartup by STARTPRI then name, one at a time unless DONOTWAIT (GFOpenBench 0.1.50)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/tests/host/test_ob_startup.c" -o "$OUT/test_ob_startup"
"$OUT/test_ob_startup"

# ol_core: the launcher's rules, gfopenlaunch.library's and GFOpenBench's (Workbench build plan row 6; S2, S2d, S3,
# I11, I12, A10)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/launcher" "$HERE/launcher/ol_core.c" \
  "$HERE/tests/host/test_ol_core.c" -o "$OUT/test_ol_core"
"$OUT/test_ol_core"

# GFOpenBenchmark's portable kernels: Dhrystone 2.1's own checks, LINPACK's residual, the FPU loops
# shellcheck disable=SC2086
cc -std=gnu99 -Wall -Wextra -Werror -O2 -g $SAN -I"$HERE/benchmark" "$HERE/benchmark/dhry_1.c" "$HERE/benchmark/dhry_2.c" \
  "$HERE/benchmark/whet.c" "$HERE/benchmark/linpack.c" "$HERE/benchmark/sgl.c" "$HERE/tests/host/test_benchmark.c" -lm \
  -o "$OUT/test_benchmark"
# ob_rexx: the ARexx ports' commands, finding them and GETATTR's names (GFOpenBench 0.1.53)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_rexx.c" \
  "$HERE/tests/host/test_ob_rexx.c" -o "$OUT/test_ob_rexx"
# ob_wbplan: GFOpenBench as workbench.library's Workbench, its paths, hook answers and AppMenuItems (GFOpenBench 0.1.56)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_wbplan.c" \
  "$HERE/tests/host/test_ob_wbplan.c" -o "$OUT/test_ob_wbplan"
"$OUT/test_ob_wbplan"
"$OUT/test_ob_rexx"
# ob_tasks: the Workbench task model: supervisor, job queue, low-memory reserve (build plan row 3)
# shellcheck disable=SC2086
cc -std=c99 -Wall -Wextra -Werror -O1 -g $SAN -I"$HERE/desktop" "$HERE/desktop/ob_tasks.c" \
  "$HERE/tests/host/test_ob_tasks.c" -o "$OUT/test_ob_tasks"
"$OUT/test_ob_tasks"
"$OUT/test_benchmark"

rm -rf "$OUT/icons"
python3 "$HERE/tests/host/icon_fixtures.py" "$OUT/icons"
"$OUT/test_icon_core" "$OUT/icons"
bad=0
while read -r name exact; do
  if ! cmp -s "$OUT/icons/$name.want" "$OUT/icons/$name.got"; then
    echo "FAIL $name: what was read differs from what was written"
    diff "$OUT/icons/$name.want" "$OUT/icons/$name.got" | head -20
    bad=1
  fi
done < "$OUT/icons/manifest.txt"
[ "$bad" = 0 ] && echo "icon dumps: all match"
exit $bad
