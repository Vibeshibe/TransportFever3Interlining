#!/usr/bin/env python3
"""Find candidate x64 RIP-relative LEA references to exact diagnostic strings.

This is a byte-pattern search, not a disassembler. Validate candidate instruction
boundaries in Ghidra. AVX loads and other reference forms are not covered.
Requires the locally installed pefile package; never modifies the executable.
"""

import argparse
from bisect import bisect_right
import hashlib
import json
from pathlib import Path
import re
import struct

import pefile


def inspect(path, needles):
    data = path.read_bytes()
    pe = pefile.PE(data=data, fast_load=True)
    try:
        base = pe.OPTIONAL_HEADER.ImageBase
        if pe.FILE_HEADER.Machine != 0x8664:
            raise ValueError("Expected an x64 PE executable")
        targets = {}
        for needle in needles:
            encoded = needle.encode("ascii") + b"\0"
            for match in re.finditer(re.escape(encoded), data):
                if match.start() == 0 or data[match.start() - 1] == 0:
                    targets[base + pe.get_rva_from_offset(match.start())] = needle
        exception = pe.OPTIONAL_HEADER.DATA_DIRECTORY[3]
        table = pe.get_data(exception.VirtualAddress, exception.Size)
        ranges = sorted((begin + base, end + base) for begin, end, _ in struct.iter_unpack("<III", table)
                        if begin and begin < end)
        starts = [begin for begin, _ in ranges]
        references = []
        pattern = re.compile(rb"[\x40-\x4f]\x8d[\x05\x0d\x15\x1d\x25\x2d\x35\x3d]")
        for section in pe.sections:
            if not section.Characteristics & 0x20000000:
                continue
            code = section.get_data()
            for match in pattern.finditer(code):
                offset = match.start()
                if offset + 7 > len(code):
                    continue
                address = base + section.VirtualAddress + offset
                target = address + 7 + struct.unpack_from("<i", code, offset + 3)[0]
                if target not in targets:
                    continue
                index = bisect_right(starts, address) - 1
                region = ranges[index] if index >= 0 and address < ranges[index][1] else None
                references.append({
                    "instruction": hex(address), "string": targets[target],
                    "target": hex(target),
                    "exception_range": [hex(n) for n in region] if region else None,
                })
        return {
            "sha256": hashlib.sha256(data).hexdigest(),
            "image_base": hex(base),
            "note": "LEA byte-pattern candidates only; validate disassembly and function boundaries",
            "strings_found": {hex(k): v for k, v in targets.items()},
            "references": references,
        }
    finally:
        pe.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("strings", nargs="+")
    parser.add_argument("--exe", type=Path, default=Path(__file__).resolve().parents[1]
                        / "game/installation/TransportFever3.exe")
    args = parser.parse_args()
    print(json.dumps(inspect(args.exe, args.strings), indent=2))


if __name__ == "__main__":
    main()
