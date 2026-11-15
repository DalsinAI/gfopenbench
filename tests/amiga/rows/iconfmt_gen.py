#!/usr/bin/env python3
"""I13's four icons, one per format, each a 40x30 picture of one colour:
classic (planar, pen 1), NewIcons, OS 3.5 GlowIcons (FORM ICON) and a PNG icon,
made with tests/host/icon_fixtures.py's writers. The NewIcons and GlowIcons ones
carry a classic picture of pen 1 too, so a desktop that doesn't read their own
picture shows pen 1, not their colour.

  iconfmt_gen.py > tests/amiga/rows/ICONFMT_icons.h

The header is kept in the tree; tests/host/run.sh checks it is what this makes.
MIT, Copyright (c) 2026 AmiCradle Team.
"""
import os
import struct
import sys

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "host"))
import icon_fixtures as F  # noqa: E402

W, H = 40, 30
# keep these in step with ICONFMT.h
NEWICONS_RGB = 0x30D090
GLOW_RGB = 0x3090D0
PNG_RGB = 0xD0A030


def rgb3(v):
    return bytes([v >> 16 & 255, v >> 8 & 255, v & 255])


def base(x, tooltypes=None):
    return {"type": 4, "tool": None, "tooltypes": tooltypes, "x": x, "y": 70, "stack": 4096,
            "planar": [(W, H, 2, bytes([1]) * (W * H))]}


def icons():
    out = []
    out.append(("classic", F.classic(base(10))))
    tts = ["*** DON'T EDIT THE FOLLOWING LINES!! ***"] + \
        F.newicons_lines(1, W, H, rgb3(NEWICONS_RGB) + rgb3(NEWICONS_RGB), bytes(W * H), False)
    out.append(("newicons", F.classic(base(70, tts))))
    pix = bytes(W * H)
    pal = rgb3(GLOW_RGB) * 2
    i0, pr = F.rle(pix, 1), F.rle(pal, 8)
    body = struct.pack(">BBBBBBHH", 0, 1, 2, 1, 1, 1, len(i0) - 1, len(pr) - 1) + i0 + pr
    form = b"ICON" + F.chunk(b"FACE", struct.pack(">BBBBH", W - 1, H - 1, 0, 0, len(pal) - 1)) + F.chunk(b"IMAG", body)
    out.append(("glowicons", F.classic(base(130)) + b"FORM" + struct.pack(">I", len(form)) + form))
    o = dict(base(190), planar=[])
    row = rgb3(PNG_RGB) * W

    def rows(x0, y0, dx, dy, pw, ph):
        for _ in range(ph):
            yield row
    out.append(("png", F.png_encode(W, H, 2, 8, rows, None, None, False, [(b"icOn", F.icon_chunk(o))])))
    return out


def main():
    print("/* made by tests/amiga/rows/iconfmt_gen.py: don't edit. MIT, Copyright (c) 2026 AmiCradle Team. */")
    names = []
    for name, data in icons():
        names.append(name)
        print("static const unsigned char iconfmt_%s[%d] = {" % (name, len(data)))
        for i in range(0, len(data), 20):
            print("    " + ", ".join("0x%02x" % b for b in data[i:i + 20]) + ",")
        print("};")
    print("#define ICONFMT_N %d" % len(names))


if __name__ == "__main__":
    main()
