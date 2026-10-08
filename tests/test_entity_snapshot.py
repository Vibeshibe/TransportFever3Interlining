"""Layout/identity regression fixtures for the read-only entity inventory."""
import importlib.util
from pathlib import Path
import struct
import unittest

spec = importlib.util.spec_from_file_location('entity_snapshot', Path(__file__).parents[1] / 'tools/entity_snapshot.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class InventoryTests(unittest.TestCase):
    def setUp(self):
        self.memory = bytearray(32768)
        self.snap = module.Snapshot.__new__(module.Snapshot)
        self.snap.read = self.read
        self.world = 1024
        self.pack(self.world + 0x90, '<QQQ', 4096, 4096 + 24*4, 4096 + 24*4)
        self.pack(self.world + 0xc0, '<QQQ', 8192, 8192 + 16*4, 8192 + 16*4)
        self.pack(4096+24, '<QQQ', 12288, 12296, 12296)
        self.pack(8192+16, '<QQ', 1 << 3, 0)
        self.pack(12288, '<ii', 3, 0)
        self.pack(self.world + 0x78, '<QQQ', 13000, 13032, 13032)
        self.pack(13024, '<Q', 14000)
        self.pack(14000+0x68, '<QQQ', 15000, 15000+0x98, 15000+0x98)
        self.pack(15008, '<QQQ', 16000, 16012, 16012)
        self.pack(16000, '<iii', 72934, 75501, -1)
        self.pack(15000+0x50, '<i', 72934)
        self.types = {3: ('.?AUSimPerson@component@ecs@@', 0x3cea768)}

    def read(self, address, size):
        if not 0 <= address <= address+size <= len(self.memory):
            raise ValueError('Fixture bounds')
        return bytes(self.memory[address:address+size])

    def pack(self, address, fmt, *values):
        struct.pack_into(fmt, self.memory, address, *values)

    def test_person_home_and_target_then_reused_asset_id(self):
        before = self.snap.entity(self.world, 1, self.types)
        self.assertEqual(before['sim_person']['destinations'], [72934, 75501, -1])
        self.assertEqual(before['sim_person']['target_or_at_entity'], 72934)
        # Same numeric ID with asset type must NOT be decoded as the old passenger.
        after = self.snap.entity(self.world, 1, {3: ('.?AUAssetGroup@component@ecs@@', 0x123)})
        self.assertNotIn('sim_person', after)
        self.assertEqual(before['entity'], after['entity'])

    def test_shared_person_storage(self):
        self.pack(12288, '<ii', 3, 0x4000001f)
        self.pack(14000+0x80, '<QQQ', 17000, 17016, 17016)
        self.pack(17000, '<Q', 18000)
        start = 18000+31*0x98
        self.memory[start:start+0x98] = self.memory[15000:15000+0x98]
        data = self.snap.entity(self.world, 1, self.types)
        self.assertEqual(data['sim_person']['target_or_at_entity'], 72934)

    def test_row_mask_disagreement_and_duplicate_handles_reject(self):
        self.pack(8192+16, '<QQ', 0, 0)
        with self.assertRaises(ValueError): self.snap.entity(self.world, 1, self.types)
        self.pack(8192+16, '<QQ', (1 << 3) | (1 << 4), 0)
        with self.assertRaises(ValueError): self.snap.entity(self.world, 1, self.types)
        self.pack(8192+16, '<QQ', 1 << 3, 0)
        self.pack(4096+24, '<QQQ', 12288, 12304, 12304)
        self.pack(12296, '<ii', 3, 0)
        with self.assertRaises(ValueError): self.snap.entity(self.world, 1, self.types)

    def test_oversized_destination_vector_and_bad_shared_index_reject(self):
        self.pack(15008, '<QQQ', 16000, 16068, 16068)
        with self.assertRaises(ValueError): self.snap.entity(self.world, 1, self.types)
        self.pack(12288, '<ii', 3, -1)
        with self.assertRaises(ValueError): self.snap.entity(self.world, 1, self.types)

    def test_closed_windows_only_without_claiming_write_boundary(self):
        for flag in [0, 1]:
            self.pack(self.world+0x1f0, '<QB', 0, flag)
            self.assertTrue(self.snap.idle(self.world))
        self.pack(self.world+0x1f0, '<QB', 1, 1)
        self.assertFalse(self.snap.idle(self.world))
        self.pack(self.world+0x1f0, '<QB', 0, 2)
        self.assertFalse(self.snap.idle(self.world))


if __name__ == '__main__':
    unittest.main()
