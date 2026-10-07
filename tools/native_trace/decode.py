"""Decode copied bytes for one pinned build; no process access or engine calls."""
import struct


def waiting(data):
    if len(data) != 32:
        raise ValueError("Expected 32-byte terminal component")
    line, board, alight = struct.unpack_from("<iii", data)
    return {"line": line, "boarding_stop": board, "alighting_stop": alight,
            "arrival_time_raw": data[16:24].hex(),
            "vehicle": struct.unpack_from("<i", data, 24)[0]}


def onboard(data):
    if len(data) != 32:
        raise ValueError("Expected 32-byte onboard component")
    line, board, alight, vehicle = struct.unpack_from("<iiii", data)
    return {"line": line, "boarding_stop": board, "alighting_stop": alight,
            "vehicle": vehicle}


def boarding_arguments(data):
    """Entry stack bytes from RSP+0x30: Windows x64 arguments 6 through 10."""
    if len(data) != 40:
        raise ValueError("Expected five eight-byte stack argument slots")
    return dict(zip(("person", "line", "boarding_stop", "alighting_stop", "vehicle"),
                    (struct.unpack_from("<i", data, n * 8)[0] for n in range(5))))
