"""Boundary correlation must never reuse a world from a previous update."""
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/native_trace'))
from boundary import TerminalFrames


class BoundaryTests(unittest.TestCase):
    def setUp(self):
        self.frames = TerminalFrames()
        self.thread = (10, 11, 0)

    def enter(self, world=0x2000):
        return self.frames.enter(self.thread, 0x1000, world, 0x3000)

    def test_complete_scope_and_next_world(self):
        self.enter()
        self.frames.advance(self.thread, 'terminal_scope_open', world=0x2000)
        self.frames.advance(self.thread, 'terminal_scope_closed', world=0x2000)
        self.frames.advance(self.thread, 'terminal_exit', stack=0x1000)
        self.assertEqual(self.frames.active, {})
        next_frame = self.enter(world=0x4000)
        self.assertEqual(next_frame['world'], 0x4000)
        self.assertEqual(next_frame['update'], 2)

    def test_early_exit_and_partial_attachment(self):
        self.assertIsNone(self.frames.advance(self.thread, 'terminal_scope_open', world=2))
        self.enter()
        self.frames.advance(self.thread, 'terminal_exit', stack=0x1000)
        self.assertEqual(self.frames.active, {})

    def test_reject_mismatched_world_stack_and_phase(self):
        self.enter()
        for phase, args in (
            ('terminal_scope_open', {'world': 0x4000}),
            ('terminal_exit', {'stack': 0x9999}),
            ('terminal_scope_closed', {'world': 0x2000}),
        ):
            with self.subTest(phase=phase), self.assertRaises(ValueError):
                self.frames.advance(self.thread, phase, **args)
        with self.assertRaises(ValueError):
            self.enter()

    def test_threads_are_independent_and_snapshots_are_copied(self):
        frame = self.enter()
        frame['world'] = 0
        self.frames.enter((10, 12, 0), 0x1000, 0x5000, 0x3000)
        self.assertEqual(self.frames.advance(self.thread, 'terminal_exit', stack=0x1000)['world'], 0x2000)
        self.assertEqual(len(self.frames.active), 1)


if __name__ == '__main__':
    unittest.main()
