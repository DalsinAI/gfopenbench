#!/usr/bin/env python3
"""GFOpenMeters' icon (meters/make_icon.py, meters/build.sh): amicradle's icon writer is found where amicradle
main has it (scripts/icons/amiga_icon.py, since amicradle #1179) and where an older checkout has it
(scripts/amiga_icon.py), the newer place first, and a folder with neither is refused by name. The tooltypes are
still read from openmeters.c and still carry DONOTWAIT. No Pillow and no amicradle checkout needed.

    python3 tests/host/test_meters_icon.py

MIT, Copyright (c) 2026 AmiCradle Team."""
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "meters"))
import make_icon  # noqa: E402

fails = 0


def check(what: str, ok: bool, more: str = "") -> None:
    global fails
    print(f"{'ok  ' if ok else 'FAIL'} {what}{' (' + more + ')' if more and not ok else ''}")
    fails += 0 if ok else 1


def main() -> int:
    with tempfile.TemporaryDirectory() as t:
        tmp = Path(t)
        new, old, both, none = (tmp / n for n in ("new", "old", "both", "none"))
        for d in (new / "scripts/icons", old / "scripts", both / "scripts/icons", both / "scripts", none / "scripts"):
            d.mkdir(parents=True, exist_ok=True)
        for f in (new / "scripts/icons", old / "scripts", both / "scripts/icons", both / "scripts"):
            (f / "amiga_icon.py").write_text("")
        check("amicradle main: scripts/icons", make_icon.icon_scripts(new) == new / "scripts/icons")
        check("an older checkout: scripts", make_icon.icon_scripts(old) == old / "scripts")
        check("both there: scripts/icons first", make_icon.icon_scripts(both) == both / "scripts/icons")
        check("neither: None", make_icon.icon_scripts(none) is None)
        r = subprocess.run([sys.executable, str(ROOT / "meters/make_icon.py"), str(none), str(tmp / "x.info")],
                           capture_output=True, text=True)
        check("neither: make_icon.py stops and names the folder, and writes no icon",
              r.returncode != 0 and str(none) in r.stderr and not (tmp / "x.info").exists(), r.stderr[-200:])
    tt = make_icon.tooltypes()
    check("the tooltypes from openmeters.c carry DONOTWAIT", "DONOTWAIT" in tt, str(tt))
    sh = (ROOT / "meters/build.sh").read_text()
    check("build.sh looks in both places", "scripts/icons/amiga_icon.py" in sh and '"$1/scripts/amiga_icon.py"' in sh)
    print("all passed" if not fails else f"{fails} FAILED")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
