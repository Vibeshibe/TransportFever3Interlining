#!/usr/bin/env python3
"""Read the local executable and report reproducible reverse-engineering anchors."""

import argparse
import hashlib
import json
from pathlib import Path
import struct


PROJECT = Path(__file__).resolve().parents[1]
ANCHORS = (
    "PathFactory::Compute",
    "ecs::SimEntityAtTerminalSystem::Update",
    "ecs::SimPersonSystem::NoteSimEntityNeedsPathChanged",
    "ecs::SimPersonSystem::NoteSimPersonDoneWaiting",
    "ecs::TransportVehicleSystem::HandleVehicleArrived",
    "town2lineCostFactor",
    "OnModifyPathfinding",
)


def inspect(executable):
    data = executable.read_bytes()
    if data[:2] != b"MZ":
        raise ValueError("Expected a Windows PE executable")
    pe = struct.unpack_from("<I", data, 0x3C)[0]
    if data[pe:pe + 4] != b"PE\0\0":
        raise ValueError("Invalid PE signature")
    section_count = struct.unpack_from("<H", data, pe + 6)[0]
    optional_size = struct.unpack_from("<H", data, pe + 20)[0]
    if struct.unpack_from("<H", data, pe + 24)[0] != 0x20B:
        raise ValueError("Expected a PE32+ executable")
    image_base = struct.unpack_from("<Q", data, pe + 48)[0]
    sections = []
    for index in range(section_count):
        start = pe + 24 + optional_size + index * 40
        name = data[start:start + 8].rstrip(b"\0").decode("ascii")
        _, rva, raw_size, raw_offset = struct.unpack_from("<IIII", data, start + 8)
        if raw_offset + raw_size > len(data):
            raise ValueError(f"Truncated section: {name}")
        sections.append((name, rva, raw_size, raw_offset))

    anchors = []
    for text in ANCHORS:
        needle = text.encode("ascii") + b"\0"
        offset = data.find(needle)
        while offset != -1:
            if offset == 0 or data[offset - 1] == 0:
                section = next((s for s in sections if s[3] <= offset < s[3] + s[2]), None)
                rva = section[1] + offset - section[3] if section else None
                anchors.append({
                    "text": text,
                    "file_offset": hex(offset),
                    "section": section[0] if section else None,
                    "rva": hex(rva) if rva is not None else None,
                    "preferred_va": hex(image_base + rva) if rva is not None else None,
                })
            offset = data.find(needle, offset + 1)

    return {
        "file": executable.name,
        "size": len(data),
        "sha256": hashlib.sha256(data).hexdigest(),
        "preferred_image_base": hex(image_base),
        "note": "Addresses refer to strings, not functions or verified patch locations.",
        "anchors": anchors,
        "missing_anchors": [a for a in ANCHORS if not any(h["text"] == a for h in anchors)],
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=PROJECT / "game/installation/TransportFever3.exe")
    args = parser.parse_args()
    try:
        report = inspect(args.exe)
    except (OSError, ValueError, struct.error) as error:
        parser.exit(1, f"Inspection failed: {error}\n")
    print(json.dumps(report, indent=2))


if __name__ == "__main__":
    main()
