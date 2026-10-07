"""Pair terminal-update observations without retaining pointers across updates."""


class TerminalFrames:
    def __init__(self):
        self.active = {}
        self.sequence = 0

    def enter(self, thread, stack, world, caller):
        if thread in self.active:
            raise ValueError('Reentrant terminal update; unsupported observation')
        self.sequence += 1
        frame = {'update': self.sequence, 'world': world, 'entry_stack': stack,
                 'caller': caller, 'phase': 'terminal_enter'}
        self.active[thread] = frame
        return dict(frame)

    def advance(self, thread, phase, world=None, stack=None):
        frame = self.active.get(thread)
        if frame is None:
            return None  # Attachment can land in an update already in progress.
        if world is not None and world != frame['world']:
            raise ValueError('World changed inside one terminal update')
        allowed = {
            'terminal_enter': ('terminal_scope_open', 'terminal_exit'),
            'terminal_scope_open': ('terminal_scope_closed',),
            'terminal_scope_closed': ('terminal_exit',),
        }
        if phase not in allowed[frame['phase']]:
            raise ValueError(f"Unexpected phase {phase} after {frame['phase']}")
        if phase == 'terminal_exit':
            if stack != frame['entry_stack']:
                raise ValueError('Return stack does not match update entry')
            del self.active[thread]
        frame['phase'] = phase
        return dict(frame)
