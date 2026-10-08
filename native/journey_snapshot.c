/* Read-only, explicit research snapshot after an injected capture has ended.
 * World addresses are supplied from that capture, never persisted as bindings. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "queries.h"
static int copy_remote(void *user, uint64_t at, void *out, size_t bytes) {
    SIZE_T got;
    return ReadProcessMemory((HANDLE)user, (const void *)(uintptr_t)at, out, bytes, &got) && got == bytes;
}
static int idle(HANDLE process, uint64_t world) {
    uint64_t window; unsigned char nested;
    return copy_remote(process, world + 0x1f0, &window, 8) && !window &&
        copy_remote(process, world + 0x1f8, &nested, 1) && nested == 1;
}
/* Diagnostic only: compare known type bits with row handles after a failed query.
 * Layouts match queries.c's pinned entity vectors. Never infer arrival from absence. */
static void diagnose_components(HANDLE process, const tfx_reader *r, int32_t person) {
    uint64_t rows[3], masks[3], entries[3], bits[2];
    int32_t handles[128][2];
    if (!r->types_ready || !copy_remote(process, r->world + 0x90, rows, sizeof(rows)) ||
        !copy_remote(process, r->world + 0xc0, masks, sizeof(masks))) return;
    if (!rows[0] || rows[0] > rows[1] || rows[1] > rows[2] || (rows[2] - rows[0]) % 24 || (rows[1] - rows[0]) % 24 ||
        (rows[2] - rows[0]) / 24 > (1u << 24) || (uint32_t)person >= (rows[1] - rows[0]) / 24 ||
        !masks[0] || masks[0] > masks[1] || masks[1] > masks[2] || (masks[2] - masks[0]) % 16 || (masks[1] - masks[0]) % 16 ||
        (masks[2] - masks[0]) / 16 > (1u << 24) || (uint32_t)person >= (masks[1] - masks[0]) / 16) return;
    if (!copy_remote(process, rows[0] + (uint64_t)person * 24, entries, sizeof(entries)) ||
        !copy_remote(process, masks[0] + (uint64_t)person * 16, bits, sizeof(bits))) return;
    if (!entries[0] || entries[0] > entries[1] || entries[1] > entries[2] ||
        (entries[2] - entries[0]) % 8 || (entries[2] - entries[0]) / 8 > 128) return;
    size_t bytes = (size_t)(entries[1] - entries[0]);
    if (bytes % 8 || !copy_remote(process, entries[0], handles, bytes)) return;
    static const char *names[10] = {"waiting", "person", "line", "group", "place",
        "vehicle", "station", "onboard", "moving", "at_building"};
    for (unsigned t = 0; t < 10; ++t) {
        int32_t type = r->types[t];
        if (type < 0 || type >= 128) continue;
        unsigned found = 0;
        for (size_t h = 0; h < bytes / 8; ++h) found += handles[h][0] == type;
        printf("snapshot_component world_id=%u person=%d component=%s type_index=%d mask_present=%u row_matches=%u observational_only=1\n",
            r->world_id, person, names[t], type, (unsigned)((bits[type / 64] >> (type % 64)) & 1), found);
    }
    printf("snapshot_component_end world_id=%u person=%d idle_after=%d atomic=0\n",
        r->world_id, person, idle(process, r->world));
    fflush(stdout);
}
int main(int argc, char **argv) {
    if (argc != 7) { puts("Usage: journey_snapshot WINDOWS_PID WORLD1_HEX WORLD2_HEX PERSON DESTINATION SECONDS(1..120)"); return 2; }
    DWORD pid = (DWORD)strtoul(argv[1], NULL, 10);
    uint64_t worlds[2] = {strtoull(argv[2], NULL, 16), strtoull(argv[3], NULL, 16)};
    int32_t person = (int32_t)strtol(argv[4], NULL, 10), destination = (int32_t)strtol(argv[5], NULL, 10);
    unsigned seconds = (unsigned)strtoul(argv[6], NULL, 10);
    if (!pid || !worlds[0] || !worlds[1] || person < 0 || destination < 0 || !seconds || seconds > 120) return 2;
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process) { printf("OpenProcess failed: %lu\n", GetLastError()); return 1; }
    HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    MODULEENTRY32W module = {.dwSize = sizeof(module)}; uint64_t image = 0;
    if (modules != INVALID_HANDLE_VALUE && Module32FirstW(modules, &module)) do {
        if (!_wcsicmp(module.szModule, L"TransportFever3.exe")) image = (uint64_t)(uintptr_t)module.modBaseAddr;
    } while (Module32NextW(modules, &module));
    if (modules != INVALID_HANDLE_VALUE) CloseHandle(modules);
    if (!image) { CloseHandle(process); puts("Game module not found"); return 1; }
    tfx_person_journey previous[2] = {{0}};
    tfx_status previous_error[2] = {TFX_OK, TFX_OK};
    uint32_t arrived = 0; unsigned samples = 0;
    ULONGLONG started = GetTickCount64();
    while (GetTickCount64() - started < seconds * 1000u) {
        for (unsigned i = 0; i < 2; ++i) {
            if (!idle(process, worlds[i])) continue;
            tfx_reader reader; tfx_reader_init(&reader, worlds[i], image, copy_remote, process); reader.world_id = i + 1;
            tfx_person_journey first = {.struct_size = sizeof(first)}, second = {.struct_size = sizeof(second)};
            tfx_status status = tfx_query_journey(&reader, person, &first);
            if (status != TFX_OK) {
                if (status != previous_error[i]) {
                    tfx_entity_presence person_presence = {.struct_size = sizeof(person_presence)};
                    tfx_entity_presence destination_presence = {.struct_size = sizeof(destination_presence)};
                    tfx_status ps = tfx_query_presence(&reader, person, &person_presence);
                    tfx_status ds = tfx_query_presence(&reader, destination, &destination_presence);
                    printf("snapshot_query_error world_id=%u person=%d status=%u person_presence_status=%u person_has_components=%u destination=%d destination_presence_status=%u destination_has_components=%u idle_after=%d observational_only=1\n",
                        i + 1, person, status, ps, person_presence.has_components, destination,
                        ds, destination_presence.has_components, idle(process, worlds[i]));
                    fflush(stdout); previous_error[i] = status;
                    diagnose_components(process, &reader, person);
                }
                continue;
            }
            status = tfx_query_journey(&reader, person, &second);
            if (status != TFX_OK || memcmp(&first, &second, sizeof(first)) || !idle(process, worlds[i])) continue;
            ++samples;
            previous_error[i] = TFX_OK;
            if (memcmp(&second, &previous[i], sizeof(second))) {
                printf("snapshot world_id=%u person=%d flags=%u destination=%d line=%d stops=%d,%d vehicle=%d repeated_read_agrees=1\n",
                    second.world_id, second.person, second.flags, second.destination, second.line,
                    second.boarding_stop, second.alighting_stop, second.vehicle); fflush(stdout); previous[i] = second;
            }
            if ((second.flags & TFX_JOURNEY_AT_BUILDING) && second.destination == destination) arrived |= 1u << i;
        }
        if (arrived == 3) break;
        Sleep(250);
    }
    printf("snapshot_summary samples=%u destination_world_mask=%u observational_only=1\n", samples, arrived);
    CloseHandle(process);
    return arrived == 3 ? 0 : 3;
}
