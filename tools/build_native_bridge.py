#!/usr/bin/env python3
"""Build the experimental Windows x64 bridge; optional pinned local toolchain."""
import argparse
import hashlib
from pathlib import Path
import subprocess
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
NAME = "llvm-mingw-20250910-ucrt-ubuntu-22.04-x86_64"
URL = f"https://github.com/mstorsjo/llvm-mingw/releases/download/20250910/{NAME}.tar.xz"
SHA256 = "f83556c9ffa4d4291fadea1a0776c1383332dacdf4d7fbdf974c2928cb32c6f7"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--fetch-toolchain", action="store_true",
                        help="download/verify LLVM-MinGW into ignored tools directories")
    parser.add_argument("--toolchain", type=Path, default=ROOT / "tools/vendor" / NAME)
    parser.add_argument("--build-dir", type=Path, default=ROOT / "analysis/native-bridge/build")
    parser.add_argument("--observe-person", type=int, help="build an additional read-only journey observer for this ID")
    parser.add_argument("--observe-destination", type=int, help="original destination ID for the observer")
    parser.add_argument("--observation-limit", type=int, default=3600,
                        help="pilot callback cap, 1..36000 (default: 3600); observer remains at 3600")
    parser.add_argument("--passenger-limit", type=int, default=1,
                        help="distinct passenger attempt cap, 1..32 (default: 1)")
    parser.add_argument("--journey-sample-limit", type=int,
                        help="concurrent full journeys, 1..5; defaults to full tracking up to 5 attempts, otherwise 4")
    parser.add_argument("--reserve-last-attempt-for-lifecycle", action="store_true",
                        help="trial pacing: keep final attempt unused until a tracked lifecycle outcome is confirmed")
    args = parser.parse_args()
    if not 1 <= args.passenger_limit <= 32:
        parser.error("--passenger-limit must be between 1 and 32")
    if args.journey_sample_limit is None:
        args.journey_sample_limit = args.passenger_limit if args.passenger_limit <= 5 else 4
    if not 1 <= args.journey_sample_limit <= min(5, args.passenger_limit) or (
        args.passenger_limit > 5 and args.journey_sample_limit > 4
    ):
        parser.error("invalid --journey-sample-limit for the selected attempt cap")
    if args.reserve_last_attempt_for_lifecycle and (
        args.passenger_limit < 2 or args.journey_sample_limit != args.passenger_limit
    ):
        parser.error("--reserve-last-attempt-for-lifecycle requires full tracking of at least two attempts")
    if not 1 <= args.observation_limit <= 36000:
        parser.error("--observation-limit must be between 1 and 36000")
    observer = (args.observe_person, args.observe_destination)
    if any(value is not None for value in observer) and any(
        value is None or not 0 <= value <= 2147483647 for value in observer
    ):
        parser.error("--observe-person and --observe-destination must both be nonnegative int32 IDs")
    if args.fetch_toolchain:
        archive = ROOT / "tools/downloads" / f"{NAME}.tar.xz"
        archive.parent.mkdir(parents=True, exist_ok=True)
        if not archive.exists():
            temporary = archive.with_suffix(".partial")
            urllib.request.urlretrieve(URL, temporary)
            temporary.replace(archive)
        with archive.open("rb") as source:
            actual = hashlib.file_digest(source, "sha256").hexdigest()
        if actual != SHA256:
            raise SystemExit(f"Toolchain checksum mismatch: {archive}")
        destination = ROOT / "tools/vendor"
        destination.mkdir(parents=True, exist_ok=True)
        with tarfile.open(archive) as source:
            source.extractall(destination, filter="data")
    compiler = args.toolchain.resolve() / "bin/x86_64-w64-mingw32-clang"
    if not compiler.is_file():
        raise SystemExit("Toolchain missing; use --fetch-toolchain or --toolchain PATH")
    build = args.build_dir.resolve()
    subprocess.run([
        "cmake", "-S", str(ROOT / "native"), "-B", str(build), "-G", "Ninja",
        "-DCMAKE_SYSTEM_NAME=Windows", f"-DCMAKE_C_COMPILER={compiler}",
        f"-DCMAKE_ASM_COMPILER={compiler}", "-DCMAKE_BUILD_TYPE=RelWithDebInfo",
        f"-DTFX_PILOT_OBSERVATION_LIMIT={args.observation_limit}",
        f"-DTFX_PILOT_PASSENGER_LIMIT={args.passenger_limit}",
        f"-DTFX_PILOT_JOURNEY_SAMPLE_LIMIT={args.journey_sample_limit}",
        f"-DTFX_PILOT_RESERVE_LAST_FOR_LIFECYCLE={'ON' if args.reserve_last_attempt_for_lifecycle else 'OFF'}",
        f"-DTFX_OBSERVE_PERSON={args.observe_person if args.observe_person is not None else ''}",
        f"-DTFX_OBSERVE_DESTINATION={args.observe_destination if args.observe_destination is not None else ''}",
    ], check=True)
    subprocess.run(["cmake", "--build", str(build)], check=True)


if __name__ == "__main__":
    main()
