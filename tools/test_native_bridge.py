#!/usr/bin/env python3
"""Run the native suite in an isolated Wine prefix and record artifact fingerprints."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def sha256(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--wine", default="wine")
    parser.add_argument("--prefix", type=Path, default=ROOT / "tools/runtime/bridge-test")
    args = parser.parse_args()
    build = args.build_dir.resolve()
    cases = [[name] for name in (
        "pipeline_tests", "continuous_tests", "query_performance_tests", "continuous_host_tests",
        "discovery_tests", "candidate_tests", "pilot_tests", "pilot_reserve_tests",
        "pilot_sampled_tests", "bridge_tests", "query_tests", "host_batch_tests")]
    cases += [["host_batch_tests", "failure"], ["host_capacity_tests"],
              ["host_capacity_tests", "failure"], ["host_capacity_tests", "history"],
              ["journey_probe_tests", str(build / "tpf_shared_bridge.dll"), "--mutable"]]
    observer = build / "tpf_journey_observer.dll"
    if observer.exists():
        cases.append(["journey_probe_tests", str(observer)])
    env = dict(os.environ, WINEPREFIX=str(args.prefix.resolve()), WINEDEBUG="-all")
    results = []
    for case in cases:
        executable = build / (case[0] + ".exe")
        try:
            run = subprocess.run([args.wine, str(executable), *case[1:]],
                                 env=env, capture_output=True, text=True, timeout=60)
            row = dict(args=case, exit_code=run.returncode, stdout=run.stdout, stderr=run.stderr,
                       executable_sha256=sha256(executable))
        except (OSError, subprocess.TimeoutExpired) as error:
            row = dict(args=case, exit_code=-1, stdout="", stderr=str(error))
        results.append(row)
        print(case, row["exit_code"], row["stdout"].strip(), flush=True)
        if row["exit_code"]:
            print(row["stderr"], flush=True)
            break
    passed = len(results) == len(cases) and all(row["exit_code"] == 0 for row in results)
    dlls = {p.name: sha256(p) for p in build.glob("tpf_*.dll")}
    report = dict(recorded_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
                  results=results, all_passed=passed, dll_sha256=dlls)
    (build / "capacity-tests.json").write_text(json.dumps(report, indent=2) + "\n")
    raise SystemExit(0 if passed else 1)


if __name__ == "__main__":
    main()
