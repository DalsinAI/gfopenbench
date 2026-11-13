#!/usr/bin/env python3
"""A stand-in for GFNetBench until the real tool exists: prints its lines.

  fake_gfnetbench.py [SCALE] [ignored GFNetBench arguments...]

Every measured value is multiplied by SCALE (default 1), so a test can make a
slower or faster run. FAKE_NETBENCH_DROP=id,id leaves those metrics out, and
FAKE_NETBENCH_SKIP=id,id prints them as SKIP. For nettest.py's tests, and as
--runner "python3 tests/net/fake_gfnetbench.py 1 {cmd}" to try the group by hand.

MIT, Copyright (c) 2026 AmiCradle Team.
"""
import os
import sys

VALUES = [
    ("tcp_down", 40.0, "MB/s"), ("tcp_up", 30.0, "MB/s"),
    ("udp_rate", 20.0, "MB/s"), ("udp_loss", 0.2, "%"),
    ("req_rate", 500.0, "req/s"), ("req_p50", 1.5, "ms"), ("req_p95", 4.0, "ms"),
    ("dns_p50", 0.8, "ms"), ("dns_p95", 2.0, "ms"),
    ("cpu_idle", 35.0, "%"),
]


def main():
    try:
        scale = float(sys.argv[1]) if len(sys.argv) > 1 else 1.0
    except ValueError:
        scale = 1.0
    drop = set(filter(None, os.environ.get("FAKE_NETBENCH_DROP", "").split(",")))
    skip = set(filter(None, os.environ.get("FAKE_NETBENCH_SKIP", "").split(",")))
    print("INFO GFNetBench (fake) peer 127.0.0.1:5201")
    for mid, v, unit in VALUES:
        if mid in drop:
            continue
        if mid in skip:
            print("NET %s SKIP - %s (baseline -, limit -) peer has no service" % (mid, unit))
        else:
            print("NET %s PASS %s %s (baseline -, limit -)" % (mid, round(v * scale, 3), unit))
    return 0


if __name__ == "__main__":
    sys.exit(main())
