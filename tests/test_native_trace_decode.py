"""Check component layouts and Windows x64 stack slots against known values."""
import struct
import sys
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools/native_trace'))
from decode import boarding_arguments, onboard, waiting


class DecodeTests(unittest.TestCase):
    def test_waiting_preserves_arrival_bits_and_negative_sentinel(self):
        data = bytearray(32)
        struct.pack_into('<iii', data, 0, 98278, 1, 2)
        struct.pack_into('<q', data, 16, 1147600)
        struct.pack_into('<i', data, 24, -1)
        self.assertEqual(waiting(data), {
            'line': 98278, 'boarding_stop': 1, 'alighting_stop': 2,
            'arrival_time_raw': data[16:24].hex(), 'vehicle': -1})

    def test_onboard_has_vehicle_at_different_offset(self):
        data = struct.pack('<iiii', 97743, 1, 2, 97959) + bytes(16)
        self.assertEqual(onboard(data), {
            'line': 97743, 'boarding_stop': 1, 'alighting_stop': 2, 'vehicle': 97959})

    def test_stack_slots_ignore_high_halves(self):
        data = bytearray(b'\xff' * 40)
        for n, value in enumerate((75625, 97743, 1, 2, 97959)):
            struct.pack_into('<i', data, n * 8, value)
        self.assertEqual(boarding_arguments(data), {
            'person': 75625, 'line': 97743, 'boarding_stop': 1,
            'alighting_stop': 2, 'vehicle': 97959})

    def test_wrong_sizes_rejected(self):
        for decode, size in ((waiting, 32), (onboard, 32), (boarding_arguments, 40)):
            for length in (0, size - 1, size + 1):
                with self.subTest(decoder=decode.__name__, length=length):
                    with self.assertRaises(ValueError):
                        decode(bytes(length))


if __name__ == '__main__':
    unittest.main()
