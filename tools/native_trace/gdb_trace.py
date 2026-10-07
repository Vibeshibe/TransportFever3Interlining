"""Source in GDB, then run interlining-trace EXE OUTPUT [MAX_HITS].

Observes function entries with hardware execution breakpoints. Does not write
passenger memory or call engine functions. Continuing resumes the attached game.
"""
import hashlib
import json
from pathlib import Path
import struct
import sys
import time

import gdb

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
from decode import boarding_arguments, onboard, waiting


def read(address, size):
    return bytes(gdb.selected_inferior().read_memory(address, size))


def reg(name):
    return int(gdb.selected_frame().read_register(name))


def gate(world):
    owner = struct.unpack("<Q", read(world + 0x1f0, 8))[0]
    return {"between_changes": hex(owner),
            "notification_in_process": read(owner + 0x50, 1)[0] if owner else None,
            "allow_nested_changes": read(world + 0x1f8, 1)[0]}


def module_base(exe):
    pid = gdb.selected_inferior().pid
    bases = set()
    for line in Path(f"/proc/{pid}/maps").read_text().splitlines():
        parts = line.split(None, 5)
        if len(parts) == 6 and parts[5] == str(exe) and int(parts[2], 16) == 0:
            bases.add(int(parts[0].split('-')[0], 16))
    if len(bases) != 1:
        raise ValueError(f"Expected one file-offset-zero mapping for {exe}; got {bases}")
    base = bases.pop()
    if read(base, 2) != b'MZ':
        raise ValueError("Mapped module does not start with MZ")
    return base


class Trace:
    def __init__(self, output, base, profile, max_hits):
        self.output = output
        self.base = base
        self.profile = profile
        self.max_hits = max_hits
        self.hits = 0
        self.records = 0
        self.started = time.monotonic()
        self.first_hit = None
        self.reason = "debugger_stop"

    def emit(self, event, **values):
        self.output.write(json.dumps({"event": event, "elapsed": time.monotonic() - self.started,
                                      **values}) + "\n")
        self.output.flush()

    def sample(self, name):
        self.hits += 1
        now = time.monotonic()
        if self.first_hit is None:
            self.first_hit = now
        if self.hits > self.max_hits or self.records >= 400 or now - self.first_hit > 120:
            self.reason = "observation_limit"
            return True
        if name == 'component_remove' and reg('r8') != self.base + int(self.profile['terminal_type_rva'], 16):
            return False
        if name == 'boarding_transition':
            values = boarding_arguments(read(reg('rsp') + 0x30, 40))
            world = reg('r8')
        else:
            world = reg('rcx')
            values = {"person": reg('edx')}
            if name != 'component_remove':
                data = read(reg('r8'), 32)
                values.update(waiting(data) if name == 'waiting_add' else onboard(data))
                values['notify'] = reg('r9') & 0xff
        values.update(world=hex(world), gate=gate(world),
                      thread=list(gdb.selected_thread().ptid),
                      caller=hex(struct.unpack('<Q', read(reg('rsp'), 8))[0]))
        self.records += 1
        self.emit(name, **values)
        return False


class Point(gdb.Breakpoint):
    def __init__(self, name, address, trace):
        super().__init__(f'*{address:#x}', type=gdb.BP_HARDWARE_BREAKPOINT, internal=True)
        self.name = name
        self.trace = trace

    def stop(self):
        try:
            return self.trace.sample(self.name)
        except Exception as error:
            self.trace.reason = 'read_error'
            self.trace.emit('error', point=self.name, detail=str(error))
            return True


class Command(gdb.Command):
    """Observe native component transitions: interlining-trace EXE OUTPUT [MAX_HITS]."""
    def __init__(self):
        super().__init__('interlining-trace', gdb.COMMAND_USER)

    def invoke(self, argument, from_tty):
        args = gdb.string_to_argv(argument)
        if len(args) not in (2, 3):
            raise gdb.GdbError('Usage: interlining-trace EXE OUTPUT [MAX_HITS]')
        exe = Path(args[0]).resolve(strict=True)
        limit = int(args[2]) if len(args) == 3 else 5000
        if not 1 <= limit <= 10000:
            raise gdb.GdbError('MAX_HITS must be between 1 and 10000')
        profile = json.loads((HERE / 'profile.json').read_text())
        if hashlib.sha256(exe.read_bytes()).hexdigest() != profile['sha256']:
            raise gdb.GdbError('Unsupported executable hash; refusing breakpoints')
        if gdb.parameter('non-stop'):
            raise gdb.GdbError('Requires GDB all-stop mode')
        if gdb.breakpoints():
            raise gdb.GdbError('Use a fresh debugger with no existing breakpoints')
        base = module_base(exe)
        for name, point in profile['points'].items():
            signature = bytes.fromhex(point['entry_bytes'])
            if read(base + int(point['rva'], 16), len(signature)) != signature:
                raise gdb.GdbError(f'Runtime entry bytes differ at {name}; refusing breakpoints')
        points = []
        with Path(args[1]).open('x') as output:
            trace = Trace(output, base, profile, limit)
            trace.emit('start', sha256=profile['sha256'], base=hex(base), pid=gdb.selected_inferior().pid,
                       max_hits=limit, max_records=400, max_seconds_from_first_hit=120)
            try:
                for name, point in profile['points'].items():
                    points.append(Point(name, base + int(point['rva'], 16), trace))
                gdb.write('Native observer ready; resumes the process, not the paused simulation.\n')
                gdb.execute('continue')
            finally:
                for point in points:
                    if point.is_valid():
                        point.delete()
                trace.emit('end', reason=trace.reason, hits=trace.hits, records=trace.records)
        gdb.write('Observer finished; breakpoints removed.\n')


Command()
