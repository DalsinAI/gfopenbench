#!/usr/bin/env python3
# Copyright (c) 2026 AmiCradle Team. MIT licence.
"""Layout guard: wave-1 drawing goes through GFOpenLayout (Dale, 13 Nov 06:50).

A pull request fails if a line it adds calls a graphics.library drawing
function outside GFOpenLayout's own sources. Calls already in the tree only
warn, and are listed in one comment on the PR. An exception needs a line
"LAYOUT-EXCEPTION: <reason>" in the PR body and a PR comment reading
"LAYOUT-EXCEPTION: APPROVE <sha8>" for the PR's current head with a
"Reviewer: <name> (Team 3)" line. Team 3 doesn't post on GitHub: it writes the
approval in amicradle-teams (teams/team5/inbox/), and Team 5 copies it onto the
PR with the note's path, as for REVIEW notes. The comment's team is self-declared
(every team posts as SacredTrees), so the merger checks the note exists.

usage: layout_guard.py BASE_SHA HEAD_SHA [--body FILE] [--comments FILE]
                       [--report FILE]
       layout_guard.py --self-test
Exit 0: pass (perhaps with warnings). Exit 1: new drawing calls, no approved
exception.
"""
import argparse
import fnmatch
import os
import re
import subprocess
import sys

CALLS = ("RectFill", "BltBitMap", "BltBitMapRastPort", "BltMaskBitMapRastPort",
         "BltTemplate", "ClipBlit", "Text", "Move", "Draw", "SetAPen", "SetBPen",
         "SetDrMd", "ScrollRaster", "EraseRect")
# drawing calls that only warn, even when new (Team 3's review, 07:06): the fail list is Dale's
WARN_CALLS = ("WritePixel", "PolyDraw", "SetRast", "SetABPenDrMd", "SetRPAttrsA", "AreaMove",
              "AreaDraw", "AreaEnd", "DrawEllipse", "AreaEllipse", "BltClear", "DrawImage",
              "DrawBorder", "PrintIText")
NAMES = "|".join(CALLS + WARN_CALLS)
# a C call: the name not preceded by an identifier char, '.', '->' or '::'
C_CALL = re.compile(r"(?<![A-Za-z0-9_.>:])(%s)\s*\(" % NAMES)
# 68k assembly: jsr _LVOSetAPen(a6), or a library offset macro
ASM_CALL = re.compile(r"\b_LVO(%s)\b" % NAMES)
# a prototype or definition (a host stub), not a call
DEFN = re.compile(r"^\s*(?:extern\s+|static\s+|inline\s+|__\w+\s+)*"
                  r"(?:void|VOID|LONG|ULONG|WORD|UWORD|BYTE|UBYTE|BOOL|int|long|short|"
                  r"unsigned\s+\w+|struct\s+\w+)\s*\**\s*(?:%s)\s*\(" % NAMES)
SOURCE = ("*.c", "*.h", "*.cc", "*.cpp", "*.cxx", "*.hpp", "*.m",
          "*.s", "*.S", "*.asm", "*.i")
# GFOpenLayout's own sources, wherever they sit
LAYOUT_PATHS = re.compile(r"(^|/)(gfopenlayout|openlayout|olui)[^/]*(/|$)", re.I)
ALLOW_FILE = ".layout-guard-allow"
EXC_LINE = re.compile(r"^\s*LAYOUT-EXCEPTION:\s*(\S.*)$", re.M)


def strip_noise(line):
    """Drop string and char literals and comments, so only code is matched."""
    line = re.sub(r'"(?:\\.|[^"\\])*"', '""', line)
    line = re.sub(r"'(?:\\.|[^'\\])*'", "''", line)
    line = re.sub(r"/\*.*?\*/", " ", line)
    line = re.sub(r"/\*.*$", " ", line)
    line = re.sub(r"//.*$", " ", line)
    if line.lstrip().startswith(("*", ";")):   # comment block body, asm comment
        return ""
    return line


def calls_in(path, line):
    if not any(fnmatch.fnmatch(os.path.basename(path), p) for p in SOURCE):
        return []
    code = strip_noise(line)
    if not code.strip():
        return []
    if path.endswith((".s", ".S", ".asm", ".i")):
        code = re.sub(r";.*$", " ", code)
        return ASM_CALL.findall(code)
    if DEFN.match(code):
        return []
    return C_CALL.findall(code) + ASM_CALL.findall(code)


def allowed(path, allow):
    if LAYOUT_PATHS.search(path):
        return True
    return any(fnmatch.fnmatch(path, g) for g in allow)


def read_allow(root="."):
    try:
        with open(os.path.join(root, ALLOW_FILE)) as f:
            return [l.strip() for l in f if l.strip() and not l.startswith("#")]
    except OSError:
        return []


def git(*args):
    return subprocess.run(("git",) + args, check=True, capture_output=True,
                          text=True, errors="replace").stdout


def added_lines(base, head):
    """(path, line number, text) for every line the PR adds."""
    out, path, num = [], None, 0
    diff = git("diff", "--unified=0", "--no-color", "--no-renames",
               "%s...%s" % (base, head))
    for l in diff.splitlines():
        if l.startswith("+++ "):
            path = l[6:] if l.startswith("+++ b/") else None
        elif l.startswith("@@"):
            m = re.match(r"@@ -\S+ \+(\d+)", l)
            num = int(m.group(1)) if m else 0
        elif l.startswith("+") and path:
            out.append((path, num, l[1:]))
            num += 1
    return out


def tree_lines(head, only=None):
    out = []
    for path in git("ls-tree", "-r", "--name-only", head).splitlines():
        if only is not None and path not in only:
            continue
        if not any(fnmatch.fnmatch(os.path.basename(path), p) for p in SOURCE):
            continue
        try:
            text = git("show", "%s:%s" % (head, path))
        except subprocess.CalledProcessError:
            continue
        for i, l in enumerate(text.splitlines(), 1):
            out.append((path, i, l))
    return out


def scan(lines, allow):
    hits = []
    for path, num, text in lines:
        if allowed(path, allow):
            continue
        for name in calls_in(path, text):
            hits.append((path, num, name))
    return hits


def exception(body, comments, head):
    """(reason, approved) from the PR body and the PR's comments."""
    m = EXC_LINE.search(body or "")
    if not m:
        return None, False
    ok = re.compile(r"LAYOUT-EXCEPTION:\s*APPROVE\s+%s" % re.escape(head[:8]))
    approved = any(ok.search(c) and re.search(r"\(Team 3\)", c)
                   for c in comments)
    return m.group(1).strip(), approved


def split_comments(text):
    return [c for c in (text or "").split("\n\x1e\n") if c.strip()]


def report(new, warn_new, old, reason, approved, head):
    r = ["<!-- layout-guard -->", "**Layout guard** (head `%s`): wave-1 drawing goes "
         "through GFOpenLayout, not straight to graphics.library." % head[:8], ""]
    if new:
        r.append("New drawing calls in this PR (%d):" % len(new))
        r += ["- `%s:%d` %s()" % h for h in new]
        if reason and approved:
            r.append("\nException approved by Team 3: %s" % reason)
        elif reason:
            r.append("\nException asked for (%s), waiting for Team 3's "
                     "`LAYOUT-EXCEPTION: APPROVE %s`." % (reason, head[:8]))
        else:
            r.append("\nMove these into GFOpenLayout calls (ask Team 3 for what is "
                     "missing), or put `LAYOUT-EXCEPTION: <reason>` in the PR "
                     "description for Team 3 to approve.")
    else:
        r.append("No new drawing calls that fail the check.")
    if warn_new:
        r += ["", "New calls on the warning list (they don't fail the check; GFOpenLayout should "
              "still do them) (%d):" % len(warn_new)]
        r += ["- `%s:%d` %s()" % h for h in warn_new]
    r += ["", "_A call counts as new when its file has more calls of that name than the base; "
          "a call moved from one file to another counts as new in the second file._"]
    if old:
        r += ["", "<details><summary>Calls already in the tree, warning only (%d)"
              "</summary>\n" % len(old)]
        r += ["- `%s:%d` %s()" % h for h in old]
        r.append("\n</details>")
    return "\n".join(r) + "\n"


def self_test():
    t = lambda p, l: calls_in(p, l)
    assert t("a.c", "    SetAPen(rp, 1);") == ["SetAPen"]
    assert t("a.c", "RectFill(rp,0,0,9,9); Move(rp,1,1); Draw(rp,2,2);") == ["RectFill", "Move", "Draw"]
    assert t("a.c", "  ob_Text(rp, s);") == []
    assert t("a.c", "  obj->Draw(x);") == []
    assert t("a.c", "  w.Move(x);") == []
    assert t("a.c", "  /* SetAPen(rp,1) */ x = 1;") == []
    assert t("a.c", "  // RectFill(rp)") == []
    assert t("a.c", '  puts("Text(here)");') == []
    assert t("a.c", "void SetAPen(struct RastPort *rp, ULONG pen)") == []
    assert t("a.h", "extern void RectFill(struct RastPort *, LONG, LONG, LONG, LONG);") == []
    assert t("a.c", "    Text (rp, s, n);") == ["Text"]
    assert t("a.c", "  if (ok) EraseRect(rp, 0, 0, 1, 1);") == ["EraseRect"]
    assert t("a.s", "    jsr _LVOBltBitMap(a6)  ; blit") == ["BltBitMap"]
    assert t("a.s", "; jsr _LVOSetAPen(a6)") == []
    assert t("a.c", "  BltBitMapRastPort(bm,0,0,rp,0,0,8,8,0xc0);") == ["BltBitMapRastPort"]
    assert t("README.md", "SetAPen(rp, 1)") == []
    assert allowed("gfopenlayout/src/draw.c", [])
    assert allowed("lib/olui_draw.c", [])
    assert allowed("tests/host/stub_gfx.c", ["tests/host/stub_*.c"])
    assert not allowed("desktop/openbench.c", ["tests/*"])
    body = "Fixes it.\nLAYOUT-EXCEPTION: the pointer sprite has no layout call yet\n"
    h = "0123456789abcdef"
    assert exception(body, [], h) == ("the pointer sprite has no layout call yet", False)
    assert exception(body, ["LAYOUT-EXCEPTION: APPROVE 01234567\nReviewer: X (Team 2)"], h)[1] is False
    assert exception(body, ["LAYOUT-EXCEPTION: APPROVE 01234567\nReviewer: X (Team 3)"], h)[1] is True
    assert exception(body, ["LAYOUT-EXCEPTION: APPROVE 89abcdef\nReviewer: X (Team 3)"], h)[1] is False
    assert exception("no line", ["LAYOUT-EXCEPTION: APPROVE 01234567 (Team 3)"], h) == (None, False)
    assert t("a.c", "  WritePixel(rp, 1, 1);") == ["WritePixel"]
    assert t("a.c", "  DrawImage(rp, im, 0, 0);") == ["DrawImage"]
    assert split_comments("a\n\x1e\nb\nc\n\x1e\n") == ["a", "b\nc"]
    print("layout_guard self-test: 28 checks passed")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("base", nargs="?")
    ap.add_argument("head", nargs="?")
    ap.add_argument("--body", help="file holding the PR description")
    ap.add_argument("--comments", help="file of PR comments, split by a line holding \\x1e")
    ap.add_argument("--report", help="write the PR comment (markdown) here")
    ap.add_argument("--self-test", action="store_true")
    a = ap.parse_args()
    if a.self_test:
        self_test()
        return 0
    if not (a.base and a.head):
        ap.error("BASE_SHA and HEAD_SHA are needed")
    head = git("rev-parse", a.head).strip()
    allow = read_allow()
    base = git("merge-base", a.base, head).strip()
    added = scan(added_lines(base, head), allow)
    # a call only moved or re-indented is not new: per file and name, only the
    # calls beyond what the base already had count
    from collections import Counter
    before = Counter((p, name) for p, _, name in scan(tree_lines(base, {p for p, _, _ in added}), allow))
    after = Counter((p, name) for p, _, name in scan(tree_lines(head, {p for p, _, _ in added}), allow))
    extra = {k: after[k] - before[k] for k in after if after[k] > before[k]}
    new, warn_new = [], []
    for p, n, name in added:
        if name in WARN_CALLS:
            if extra.get((p, name), 0) > 0:
                warn_new.append((p, n, name))
                extra[(p, name)] -= 1
            continue
        if extra.get((p, name), 0) > 0:
            new.append((p, n, name))
            extra[(p, name)] -= 1
    newset = {(p, n) for p, n, _ in new}
    newset |= {(p, n) for p, n, _ in warn_new}
    old = [h for h in scan(tree_lines(head), allow) if (h[0], h[1]) not in newset]
    body = open(a.body).read() if a.body and os.path.exists(a.body) else ""
    comments = split_comments(open(a.comments).read()) if a.comments and os.path.exists(a.comments) else []
    reason, approved = exception(body, comments, head)
    # GitHub shows only the first few annotations; the full list goes in the PR comment
    for p, n, name in old[:5]:
        print("::warning file=%s,line=%d::%s() draws straight to graphics.library; "
              "move it to GFOpenLayout" % (p, n, name))
    if old:
        print("::warning::%d drawing call(s) already in the tree outside GFOpenLayout "
              "(warning only; listed in the PR comment)" % len(old))
    for p, n, name in warn_new:
        print("::warning file=%s,line=%d::new %s() call outside GFOpenLayout (warning list)" % (p, n, name))
    for p, n, name in new:
        kind = "warning" if (reason and approved) else "error"
        print("::%s file=%s,line=%d::new %s() call outside GFOpenLayout" % (kind, p, n, name))
    if a.report:
        with open(a.report, "w") as f:
            f.write(report(new, warn_new, old, reason, approved, head))
    print("layout guard: %d new call(s), %d already in the tree, exception: %s"
          % (len(new), len(old), "approved" if approved else ("asked" if reason else "none")))
    return 1 if new and not (reason and approved) else 0


if __name__ == "__main__":
    sys.exit(main())
