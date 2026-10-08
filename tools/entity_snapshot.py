#!/usr/bin/env python3
"""Bounded Linux/Proton entity inventory using read-only /proc/PID/mem.

Supply fresh world addresses from a verified bridge capture in the SAME process.
Requires a paused disposable save. Matching reads are observational, not atomic.
No injection, engine calls, debugger attachment, or writes to game memory.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import struct

PIN = '5e720cd7fefd688a1675b61c6eecaadfd25c171da9e9e938af85e5efaebd8a35'
GAME = Path(__file__).resolve().parents[1] / 'game/installation/TransportFever3.exe'


class Snapshot:
    def __init__(self, pid):
        self.proc = Path('/proc') / str(pid)
        self.start = self.identity()
        self.fd = os.open(self.proc / 'mem', os.O_RDONLY)
        self.remaining = 4 * 1024 * 1024
        maps = (self.proc / 'maps').read_text().splitlines()
        starts = [int(line.split('-')[0], 16) for line in maps
                  if line.endswith(str(GAME.resolve())) and line.split()[2] == '00000000']
        if len(starts) != 1:
            raise ValueError('Game image mapping is missing or ambiguous')
        self.image = starts[0]
        if self.read(self.image, 2) != b'MZ':
            raise ValueError('Image header mismatch')
        nt = self.u32(self.image + 0x3c)
        if nt > 4096 or self.read(self.image + nt, 4) != b'PE\0\0':
            raise ValueError('PE header mismatch')
        self.image_size = self.u32(self.image + nt + 24 + 56)
        if not 0x3cf2ba8 < self.image_size < 0x10000000:
            raise ValueError('Unexpected game image size')

    def identity(self):
        return (self.proc / 'stat').read_text().split(') ', 1)[1].split()[19]

    def read(self, address, size):
        if not 0 < address < 2**64 - size or not 0 <= size <= self.remaining:
            raise ValueError('Read bounds/budget exceeded')
        self.remaining -= size
        data = os.pread(self.fd, size, address)
        if len(data) != size:
            raise ValueError('Short memory read')
        return data

    def u32(self, address):
        return struct.unpack('<I', self.read(address, 4))[0]

    def vector(self, address, stride, limit):
        begin, end, cap = struct.unpack('<QQQ', self.read(address, 24))
        if not (begin <= end <= cap and (end-begin) % stride == 0 and
                (cap-begin) % stride == 0 and (cap-begin)//stride <= limit):
            raise ValueError('Invalid vector')
        if not begin and (end or cap):
            raise ValueError('Null vector start')
        return begin, (end-begin)//stride

    def idle(self, world):
        value = self.read(world + 0x1f0, 9)
        return value[:8] == b'\0' * 8 and value[8] in (0, 1)

    def types(self, world):
        ctrl, slots, size, cap = struct.unpack('<QQQQ', self.read(world+0x48, 32))
        if not ctrl or not slots or not 0 < cap <= 4095 or cap & (cap+1) or not size <= 128:
            raise ValueError('Invalid type table')
        controls = self.read(ctrl, cap)
        result = {}
        for slot, flag in enumerate(controls):
            if flag & 128:
                continue
            rtti, index = struct.unpack('<QI', self.read(slots + slot*16, 12))
            if not 1 <= index <= 128 or index-1 in result:
                raise ValueError('Invalid/duplicate type index')
            if not self.image <= rtti < self.image + self.image_size - 256:
                raise ValueError('Type descriptor outside pinned image')
            name = self.read(rtti + 16, 240).split(b'\0', 1)[0].decode('ascii')
            if not name.startswith('.?') or not all(32 <= ord(c) < 127 for c in name):
                raise ValueError('Invalid type name')
            result[index-1] = (name, rtti-self.image)
        if len(result) != size:
            raise ValueError('Type table count mismatch')
        return result

    def component_data(self, world, type_index, handle, stride):
        stores, count = self.vector(world+0x78, 8, 128)
        if type_index >= count or not 0 <= handle < 0x80000000:
            raise ValueError('Invalid component handle')
        store = struct.unpack('<Q', self.read(stores+8*type_index, 8))[0]
        if handle < 0x40000000:
            base, count = self.vector(store+0x68, stride, 1 << 24)
            if handle >= count:
                raise ValueError('Component handle outside store')
            address = base + handle*stride
        else:
            index = handle-0x40000000
            pages, count = self.vector(store+0x80, 16, 1 << 19)
            if index//32 >= count:
                raise ValueError('Shared component handle outside pages')
            page = struct.unpack('<Q', self.read(pages+(index//32)*16, 8))[0]
            address = page+(index % 32)*stride
        return address, self.read(address, stride)

    def entity(self, world, entity, types):
        rows, count = self.vector(world+0x90, 24, 1 << 24)
        masks, mask_count = self.vector(world+0xc0, 16, 1 << 24)
        if not 0 <= entity < min(count, mask_count):
            raise ValueError('Entity ID outside current vectors')
        entries, count = self.vector(rows+entity*24, 8, 128)
        bits = int.from_bytes(self.read(masks+entity*16, 16), 'little')
        handles = struct.iter_unpack('<ii', self.read(entries, count*8)) if count else []
        result = {'entity': entity, 'mask': hex(bits), 'components': []}
        seen = set()
        for index, handle in handles:
            if index not in types or index in seen or not bits & (1 << index):
                raise ValueError('Row/mask/type inconsistency')
            seen.add(index)
            name, rva = types[index]
            result['components'].append({'type': name, 'type_index': index, 'rtti_rva': hex(rva)})
            if rva == 0x3cea768:
                addr, data = self.component_data(world, index, handle, 0x98)
                destinations, n = self.vector(addr+8, 4, 16)
                result['sim_person'] = {
                    'target_or_at_entity': struct.unpack_from('<i', data, 0x50)[0],
                    'destinations': list(struct.unpack('<'+'i'*n, self.read(destinations, n*4))) if n else [],
                    'raw_hex': data.hex(),
                }
        if sum(1 << i for i in seen) != bits:
            raise ValueError('Mask contains components absent from row')
        return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('pid', type=int)
    parser.add_argument('--worlds', nargs=2, type=lambda s: int(s, 16), required=True)
    parser.add_argument('--entities', nargs='+', type=int, required=True)
    args = parser.parse_args()
    if args.pid <= 0 or len(args.entities) > 16 or any(not 0 <= e < 1 << 24 for e in args.entities):
        parser.error('Provide a positive PID and at most 16 nonnegative entity IDs')
    with GAME.open('rb') as f:
        if hashlib.file_digest(f, 'sha256').hexdigest() != PIN:
            raise ValueError('Unsupported executable hash')
    snap = Snapshot(args.pid)
    try:
        output = {'pid': args.pid, 'process_start': snap.start, 'image_base': hex(snap.image),
                  'executable_sha256': PIN, 'read_only': True, 'atomic': False, 'worlds': []}
        for world in args.worlds:
            if not snap.idle(world):
                raise ValueError('Open modification window or unsupported boundary flag')
            boundary_before = snap.read(world + 0x1f0, 10).hex()
            types = snap.types(world)
            first = [snap.entity(world, e, types) for e in args.entities]
            second = [snap.entity(world, e, types) for e in args.entities]
            if first != second or not snap.idle(world):
                raise ValueError('Snapshot changed while reading')
            output['worlds'].append({'address': hex(world), 'repeated_reads_agree': True, 'boundary_before': boundary_before,
                                     'boundary_after': snap.read(world+0x1f0, 10).hex(), 'entities': second})
        if snap.identity() != snap.start:
            raise ValueError('Process identity changed')
        print(json.dumps(output, indent=2))
    finally:
        os.close(snap.fd)


if __name__ == '__main__':
    main()
