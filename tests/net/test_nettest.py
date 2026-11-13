#!/usr/bin/env python3
"""Host tests for nettest.py, WBGapTest's NET group, against fake_gfnetbench.py.

MIT, Copyright (c) 2026 AmiCradle Team.
"""
import json
import os
import shutil
import subprocess
import sys
import tempfile
import unittest

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import nettest  # noqa: E402

FAKE = os.path.join(HERE, "fake_gfnetbench.py")
LIMITS = {"limits": {"tcp_down": {"drop_pct": 10}, "tcp_up": {"drop_pct": 10},
                     "req_rate": {"drop_pct": 15}, "udp_loss": {"max": 1}},
          "machines": {"i32": {"bridge": {"baseline": {"tcp_down": 40.0, "tcp_up": 30.0, "req_rate": 500.0}}}}}


def fake(scale="1", env=None):
    e = dict(os.environ)
    e.update(env or {})
    return subprocess.run([sys.executable, FAKE, scale], capture_output=True, text=True, env=e).stdout


def states(lines):
    return {l.split()[1]: l.split()[2] for l in lines}


class Parse(unittest.TestCase):
    def test_values(self):
        got = nettest.parse_lines("INFO x\nNET tcp_down PASS 41.5 MB/s (baseline -, limit -)\njunk\n")
        self.assertEqual(got, {"tcp_down": ("PASS", 41.5, "MB/s", "41.5 MB/s (baseline -, limit -)")})

    def test_skip_and_fail_keep_reason(self):
        got = nettest.parse_lines("NET dns_p50 SKIP - ms no dns\nNET udp_rate FAIL - MB/s peer gone\n")
        self.assertEqual(got["dns_p50"][:3], ("SKIP", None, None))
        self.assertEqual(got["udp_rate"][0], "FAIL")
        self.assertIn("peer gone", got["udp_rate"][3])

    def test_pass_without_value_is_fail(self):
        self.assertEqual(nettest.parse_lines("NET tcp_up PASS fast\n")["tcp_up"][0], "FAIL")


class Judge(unittest.TestCase):
    def test_drop(self):
        self.assertEqual(nettest.judge("tcp_down", 36.0, 40.0, {"drop_pct": 10}), ("PASS", ">= 36"))
        self.assertEqual(nettest.judge("tcp_down", 35.9, 40.0, {"drop_pct": 10})[0], "FAIL")

    def test_drop_needs_baseline(self):
        self.assertEqual(nettest.judge("tcp_down", 1.0, None, {"drop_pct": 10}), ("SKIP", "no baseline"))

    def test_rise(self):
        self.assertEqual(nettest.judge("dns_p95", 2.5, 2.0, {"rise_pct": 25})[0], "PASS")
        self.assertEqual(nettest.judge("dns_p95", 2.6, 2.0, {"rise_pct": 25})[0], "FAIL")

    def test_max_without_baseline(self):
        self.assertEqual(nettest.judge("udp_loss", 1.0, None, {"max": 1}), ("PASS", "<= 1"))
        self.assertEqual(nettest.judge("udp_loss", 1.01, None, {"max": 1})[0], "FAIL")

    def test_min(self):
        self.assertEqual(nettest.judge("cpu_idle", 9.0, None, {"min": 10})[0], "FAIL")

    def test_no_rule_reports_only(self):
        self.assertEqual(nettest.judge("req_p50", 99.0, 1.0, None), ("PASS", "-"))


class Report(unittest.TestCase):
    def test_one_line_per_metric_in_order(self):
        lines = nettest.report(nettest.parse_lines(fake()), LIMITS, "i32", "bridge")
        self.assertEqual([l.split()[1] for l in lines], [m for m, _ in nettest.METRICS])
        self.assertTrue(all(l.split()[2] in ("PASS", "FAIL", "SKIP") for l in lines))
        self.assertTrue(all(l.startswith("NET ") for l in lines))

    def test_at_baseline_all_pass(self):
        s = states(nettest.report(nettest.parse_lines(fake()), LIMITS, "i32", "bridge"))
        self.assertEqual(set(s.values()), {"PASS"})

    def test_line_shape(self):
        lines = nettest.report(nettest.parse_lines(fake()), LIMITS, "i32", "bridge")
        self.assertIn("NET tcp_down PASS 40 MB/s (baseline 40, limit >= 36)", lines)
        self.assertIn("NET udp_loss PASS 0.2 % (baseline -, limit <= 1)", lines)
        self.assertIn("NET req_p50 PASS 1.5 ms (baseline -, limit -)", lines)

    def test_slower_fails_gated_only(self):
        s = states(nettest.report(nettest.parse_lines(fake("0.8")), LIMITS, "i32", "bridge"))
        self.assertEqual((s["tcp_down"], s["tcp_up"], s["req_rate"]), ("FAIL", "FAIL", "FAIL"))
        self.assertEqual(s["udp_rate"], "PASS")

    def test_small_drop_passes(self):
        s = states(nettest.report(nettest.parse_lines(fake("0.95")), LIMITS, "i32", "bridge"))
        self.assertEqual(s["tcp_down"], "PASS")

    def test_lossy_udp_fails(self):
        s = states(nettest.report(nettest.parse_lines(fake("6")), LIMITS, "i32", "bridge"))
        self.assertEqual(s["udp_loss"], "FAIL")   # 0.2 * 6 = 1.2 %

    def test_no_baseline_skips_gated(self):
        s = states(nettest.report(nettest.parse_lines(fake()), LIMITS, "cm4", "bridge"))
        self.assertEqual((s["tcp_down"], s["req_rate"], s["udp_loss"]), ("SKIP", "SKIP", "PASS"))

    def test_missing_and_skipped_metrics(self):
        text = fake(env={"FAKE_NETBENCH_DROP": "dns_p50", "FAKE_NETBENCH_SKIP": "dns_p95"})
        lines = nettest.report(nettest.parse_lines(text), LIMITS, "i32", "bridge")
        self.assertIn("NET dns_p50 SKIP - ms (baseline -, limit -) not measured", lines)
        self.assertTrue(any(l.startswith("NET dns_p95 SKIP") and "no service" in l for l in lines))
        self.assertEqual(len(lines), len(nettest.METRICS))

    def test_wrong_unit_fails(self):
        lines = nettest.report(nettest.parse_lines("NET tcp_down PASS 320 Mb/s\n"), LIMITS, "i32", "bridge")
        self.assertTrue(lines[0].startswith("NET tcp_down FAIL 320 Mb/s"))

    def test_efficiency_lines(self):
        lines = nettest.report(nettest.parse_lines(fake()), LIMITS, "i32", "bridge", {"tcp_down": 110.0})
        self.assertIn("NET tcp_down_eff PASS 36.4 % (baseline host link 110 MB/s, limit -)", lines)
        self.assertIn("NET tcp_up_eff SKIP - % (baseline host link - MB/s, limit -)", lines)


class Baseline(unittest.TestCase):
    def test_medians(self):
        runs = [nettest.parse_lines(fake(s)) for s in ("1", "0.5", "2")]
        m = nettest.medians(runs)
        self.assertEqual(m["tcp_down"], 40.0)
        self.assertEqual(set(m), {k for k, _ in nettest.METRICS})

    def test_medians_ignore_skips(self):
        runs = [nettest.parse_lines(fake(env={"FAKE_NETBENCH_SKIP": "dns_p50"})) for _ in range(3)]
        self.assertNotIn("dns_p50", nettest.medians(runs))


class Cli(unittest.TestCase):
    def setUp(self):
        self.d = tempfile.mkdtemp()
        self.lim = os.path.join(self.d, "limits.json")
        shutil.copy(os.path.join(HERE, "limits.json"), self.lim)

    def tearDown(self):
        shutil.rmtree(self.d)

    def cli(self, *args, scale="1"):
        runner = "%s %s %s {cmd}" % (sys.executable, FAKE, scale)
        return subprocess.run([sys.executable, "-B", os.path.join(HERE, "nettest.py")] + list(args) +
                              ["--peer", "127.0.0.1:5201", "--limits", self.lim, "--runner", runner],
                              capture_output=True, text=True)

    def test_shipped_limits_are_valid(self):
        with open(os.path.join(HERE, "limits.json")) as f:
            lim = json.load(f)
        self.assertTrue(set(lim["limits"]) <= {m for m, _ in nettest.METRICS})
        self.assertIn("host-bridge", lim["machines"]["instance-32"])

    def test_baseline_then_run(self):
        p = self.cli("run", "instance-32", "host-bridge")
        self.assertEqual(p.returncode, 0)
        self.assertIn("NET tcp_down SKIP 40 MB/s (baseline -, limit no baseline)", p.stdout)
        p = self.cli("baseline", "instance-32", "host-bridge", "--times", "3")
        self.assertEqual(p.returncode, 0, p.stdout + p.stderr)
        with open(self.lim) as f:
            self.assertEqual(json.load(f)["machines"]["instance-32"]["host-bridge"]["baseline"]["tcp_down"], 40.0)
        self.assertEqual(self.cli("run", "instance-32", "host-bridge").returncode, 0)
        p = self.cli("run", "instance-32", "host-bridge", scale="0.5")
        self.assertEqual(p.returncode, 5)
        self.assertIn("NET tcp_down FAIL 20 MB/s (baseline 40, limit >= 36)", p.stdout)

    def test_runner_that_fails(self):
        p = subprocess.run([sys.executable, "-B", os.path.join(HERE, "nettest.py"), "run", "i32", "bridge",
                            "--peer", "x:1", "--limits", self.lim, "--runner", "false {cmd}"],
                           capture_output=True, text=True)
        self.assertEqual(p.returncode, 20)
        self.assertIn("NET setup FAIL GFNetBench didn't run", p.stdout)

    def test_no_runner(self):
        e = dict(os.environ)
        e.pop("NETTEST_RUNNER", None)
        p = subprocess.run([sys.executable, "-B", os.path.join(HERE, "nettest.py"), "run", "i32", "bridge",
                            "--peer", "x:1", "--limits", self.lim], capture_output=True, text=True, env=e)
        self.assertEqual(p.returncode, 20)


if __name__ == "__main__":
    unittest.main(verbosity=1)
