#!/usr/bin/env python3
"""WBGapTest's NET group: the Amiga's network speed against a host baseline.

  nettest.py run      MACHINE MODE --peer HOST:PORT [--limits FILE] [--runner CMD] [--iperf HOST]
  nettest.py baseline MACHINE MODE --peer HOST:PORT [--limits FILE] [--runner CMD] [--iperf HOST] [--times 3]

It runs GFNetBench on the Amiga through RUNNER (a command with {cmd} in it,
which runs one AmigaDOS line and prints its output: Cradle's agent, for
example `python3 cradle.py amiga_run lab:NAME {cmd}`; NETTEST_RUNNER in the
environment if --runner isn't given). It reads GFNetBench's lines, compares
each measured value with the machine's baseline and the group's limits in
tests/net/limits.json, and prints exactly one line per metric:

  NET <id> <PASS|FAIL|SKIP> <value> <unit> (baseline <b>, limit <l>)

With --iperf it also measures the host link with iperf3 (the ceiling) and
prints the bridge's efficiency as NET tcp_down_eff and tcp_up_eff. Those are
reported with a dash for the limit: the number to improve, not a gate.

baseline runs GFNetBench --times times, takes each metric's median and
writes it as MACHINE/MODE's baseline in the limits file.

Exit status: 0 if no FAIL, 5 if any FAIL, 20 if GFNetBench couldn't be run.

GFNetBench's lines are read as NET <id> <PASS|FAIL|SKIP> <value> <unit> ...;
only id, value and unit are used for a PASS line. Its own SKIP or FAIL
(the test couldn't run) is kept as it is, with its reason.

MIT, Copyright (c) 2026 AmiCradle Team.
"""
import argparse
import json
import os
import re
import shlex
import statistics
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
DEFAULT_LIMITS = os.path.join(HERE, "limits.json")

# every metric the group reports, in order, with its unit
METRICS = [
    ("tcp_down", "MB/s"), ("tcp_up", "MB/s"),
    ("udp_rate", "MB/s"), ("udp_loss", "%"),
    ("req_rate", "req/s"), ("req_p50", "ms"), ("req_p95", "ms"),
    ("dns_p50", "ms"), ("dns_p95", "ms"),
    ("cpu_idle", "%"),
]
UNITS = dict(METRICS)

LINE = re.compile(r"^NET\s+(\S+)\s+(PASS|FAIL|SKIP)\s*(.*)$")
VALUE = re.compile(r"^(-?\d+(?:\.\d+)?)\s+(\S+)")


def parse_lines(text):
    """GFNetBench's output: {id: (state, value or None, unit or None, rest)}; other lines are ignored."""
    got = {}
    for raw in text.splitlines():
        m = LINE.match(raw.strip())
        if not m:
            continue
        mid, state, rest = m.group(1), m.group(2), m.group(3).strip()
        v = VALUE.match(rest)
        if state == "PASS" and v:
            got[mid] = ("PASS", float(v.group(1)), v.group(2), rest)
        elif state == "PASS":
            got[mid] = ("FAIL", None, None, "no value: " + rest)
        else:
            got[mid] = (state, None, None, rest)
    return got


def fmt(x):
    if x is None:
        return "-"
    if abs(x - round(x)) < 1e-9:
        return str(int(round(x)))
    return ("%.3f" % x).rstrip("0").rstrip(".")


def judge(mid, value, baseline, rule):
    """(state, limit text) for one measured value. rule is the metric's limits entry, or None.

    drop_pct: FAIL when value < baseline * (1 - drop_pct/100); needs a baseline, else SKIP.
    rise_pct: FAIL when value > baseline * (1 + rise_pct/100); needs a baseline, else SKIP.
    max:      FAIL when value > max (no baseline needed).
    min:      FAIL when value < min (no baseline needed).
    No rule: PASS, reported only.
    """
    if not rule:
        return "PASS", "-"
    if "max" in rule:
        return ("FAIL" if value > rule["max"] else "PASS"), "<= " + fmt(rule["max"])
    if "min" in rule:
        return ("FAIL" if value < rule["min"] else "PASS"), ">= " + fmt(rule["min"])
    if "drop_pct" in rule:
        if baseline is None:
            return "SKIP", "no baseline"
        lim = baseline * (1 - rule["drop_pct"] / 100.0)
        return ("FAIL" if value < lim else "PASS"), ">= " + fmt(lim)
    if "rise_pct" in rule:
        if baseline is None:
            return "SKIP", "no baseline"
        lim = baseline * (1 + rule["rise_pct"] / 100.0)
        return ("FAIL" if value > lim else "PASS"), "<= " + fmt(lim)
    return "PASS", "-"


def report(got, limits, machine, mode, host_link=None):
    """The group's lines, one per metric in METRICS order (plus the efficiency lines when host_link is given)."""
    rules = limits.get("limits", {})
    base = limits.get("machines", {}).get(machine, {}).get(mode, {}).get("baseline", {})
    out = []
    for mid, unit in METRICS:
        if mid not in got:
            out.append("NET %s SKIP - %s (baseline %s, limit -) not measured" % (mid, unit, fmt(base.get(mid))))
            continue
        state, value, vunit, rest = got[mid]
        if value is None:
            out.append("NET %s %s - %s (baseline %s, limit -) %s" % (mid, state, unit, fmt(base.get(mid)), rest))
            continue
        if vunit != unit:
            out.append("NET %s FAIL %s %s (baseline %s, limit -) unit is %s, not %s"
                       % (mid, fmt(value), vunit, fmt(base.get(mid)), vunit, unit))
            continue
        st, lim = judge(mid, value, base.get(mid), rules.get(mid))
        out.append("NET %s %s %s %s (baseline %s, limit %s)" % (mid, st, fmt(value), unit, fmt(base.get(mid)), lim))
    if host_link is not None:
        for mid in ("tcp_down", "tcp_up"):
            link = host_link.get(mid)
            g = got.get(mid)
            if not link or not g or g[1] is None:
                out.append("NET %s_eff SKIP - %% (baseline host link %s MB/s, limit -)" % (mid, fmt(link)))
            else:
                out.append("NET %s_eff PASS %s %% (baseline host link %s MB/s, limit -)"
                           % (mid, fmt(round(100.0 * g[1] / link, 1)), fmt(link)))
    return out


def medians(runs):
    """Each metric's median over several parsed runs (PASS values only)."""
    out = {}
    for mid, _ in METRICS:
        vals = [r[mid][1] for r in runs if mid in r and r[mid][0] == "PASS" and r[mid][1] is not None]
        if vals:
            out[mid] = statistics.median(vals)
    return out


def set_baseline(limits, machine, mode, base):
    limits.setdefault("machines", {}).setdefault(machine, {}).setdefault(mode, {})["baseline"] = base
    return limits


def run_amiga(runner, line):
    """One AmigaDOS line through the runner; its output, or None if it couldn't run."""
    argv = [a.replace("{cmd}", line) for a in shlex.split(runner)]
    try:
        p = subprocess.run(argv, capture_output=True, text=True, timeout=900)
    except (OSError, subprocess.TimeoutExpired):
        return None
    return p.stdout if p.returncode == 0 or "NET " in p.stdout else None


def iperf_link(host):
    """The host link in MB/s by iperf3 (download = -R), or {} without iperf3."""
    out = {}
    for mid, extra in (("tcp_up", []), ("tcp_down", ["-R"])):
        try:
            p = subprocess.run(["iperf3", "-c", host, "-J", "-t", "5"] + extra,
                               capture_output=True, text=True, timeout=60)
            bps = json.loads(p.stdout)["end"]["sum_received"]["bits_per_second"]
            out[mid] = round(bps / 8 / 1e6, 1)
        except (OSError, subprocess.TimeoutExpired, ValueError, KeyError):
            pass
    return out


def bench_line(peer, label):
    return "GFNetBench PEER=%s TESTS=all RUNS=5 LABEL=%s" % (peer, label)


def main(argv=None):
    ap = argparse.ArgumentParser(description="WBGapTest's NET group")
    ap.add_argument("what", choices=["run", "baseline"])
    ap.add_argument("machine")
    ap.add_argument("mode")
    ap.add_argument("--peer", required=True)
    ap.add_argument("--limits", default=DEFAULT_LIMITS)
    ap.add_argument("--runner", default=os.environ.get("NETTEST_RUNNER"))
    ap.add_argument("--iperf")
    ap.add_argument("--times", type=int, default=3)
    a = ap.parse_args(argv)
    if not a.runner:
        print("NET setup SKIP no runner: give --runner or NETTEST_RUNNER")
        return 20
    with open(a.limits) as f:
        limits = json.load(f)
    line = bench_line(a.peer, "%s/%s" % (a.machine, a.mode))
    if a.what == "baseline":
        runs = []
        for _ in range(a.times):
            text = run_amiga(a.runner, line)
            if text is None:
                print("NET setup FAIL GFNetBench didn't run: %s" % line)
                return 20
            runs.append(parse_lines(text))
        base = medians(runs)
        set_baseline(limits, a.machine, a.mode, base)
        with open(a.limits, "w") as f:
            json.dump(limits, f, indent=2, sort_keys=True)
            f.write("\n")
        for mid, unit in METRICS:
            print("INFO NET baseline %s/%s %s %s %s" % (a.machine, a.mode, mid, fmt(base.get(mid)), unit))
        return 0
    text = run_amiga(a.runner, line)
    if text is None:
        print("NET setup FAIL GFNetBench didn't run: %s" % line)
        return 20
    lines = report(parse_lines(text), limits, a.machine, a.mode, iperf_link(a.iperf) if a.iperf else None)
    for l in lines:
        print(l)
    return 5 if any(l.split()[2] == "FAIL" for l in lines) else 0


if __name__ == "__main__":
    sys.exit(main())
