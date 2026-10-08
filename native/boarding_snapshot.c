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
    uint64_t window;
    unsigned char nested;
    return copy_remote(process, world + 0x1f0, &window, 8) && !window &&
           copy_remote(process, world + 0x1f8, &nested, 1) && nested == 1;
}

static uint64_t find_terminal(HANDLE process, uint64_t world, uint64_t image) {
    uint64_t v[3], ptrs[256], found = 0;
    if (!copy_remote(process, world + 0x30, v, sizeof(v)) || !v[0] || v[1] < v[0] || v[2] < v[1] ||
        (v[1] - v[0]) % 8 || (v[2] - v[0]) % 8 || (v[2] - v[0]) / 8 > 256)
        return 0;
    if (!copy_remote(process, v[0], ptrs, (size_t)(v[1] - v[0])))
        return 0;
    for (unsigned i = 0; i < (v[1] - v[0]) / 8; ++i) {
        uint64_t table;
        if (!copy_remote(process, ptrs[i], &table, 8))
            return 0;
        if (table == image + 0x3702758) {
            if (found)
                return 0;
            found = ptrs[i];
        }
    }
    return found;
}
int main(int argc, char **argv) {
    if (argc != 4) {
        puts("Usage: boarding_snapshot WINDOWS_PID WORLD1_HEX WORLD2_HEX");
        return 2;
    }
    DWORD pid = (DWORD)strtoul(argv[1], NULL, 10);
    uint64_t worlds[2] = {strtoull(argv[2], NULL, 16), strtoull(argv[3], NULL, 16)};
    if (!pid || !worlds[0] || !worlds[1])
        return 2;
    HANDLE process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!process)
        return 1;
    HANDLE modules = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    MODULEENTRY32W module = {.dwSize = sizeof(module)};
    uint64_t image = 0;
    if (modules != INVALID_HANDLE_VALUE && Module32FirstW(modules, &module))
        do {
            if (!_wcsicmp(module.szModule, L"TransportFever3.exe"))
                image = (uint64_t)(uintptr_t)module.modBaseAddr;
        } while (Module32NextW(modules, &module));
    if (modules != INVALID_HANDLE_VALUE)
        CloseHandle(modules);
    if (!image) {
        CloseHandle(process);
        return 1;
    }
    unsigned errors = 0;
    for (unsigned wi = 0; wi < 2; ++wi) {
        uint64_t world = worlds[wi], terminal = find_terminal(process, world, image);
        if (!idle(process, world) || !terminal) {
            printf("snapshot_unavailable world_id=%u\n", wi + 1);
            ++errors;
            continue;
        }
        printf("snapshot_begin world_id=%u world=%llx terminal=%llx observational_only=1\n", wi + 1,
               (unsigned long long)world, (unsigned long long)terminal);
        int32_t cursor = 0;
        unsigned pages = 0, people = 0;
        do {
            tfx_reader r;
            tfx_reader_init(&r, world, image, copy_remote, process);
            r.terminal_system = terminal;
            tfx_waiting_page page = {.struct_size = sizeof(page)};
            tfx_status status = tfx_query_scan(&r, cursor, &page);
            if (status != TFX_OK) {
                printf("scan_error status=%u\n", status);
                ++errors;
                break;
            }
            for (unsigned i = 0; i < page.count; ++i) {
                tfx_reader_init(&r, world, image, copy_remote, process);
                r.terminal_system = terminal;
                tfx_waiting_leg a = {.struct_size = sizeof(a)}, b = {.struct_size = sizeof(b)};
                status = tfx_query_waiting(&r, page.persons[i], &a);
                if (status != TFX_OK) {
                    ++errors;
                    continue;
                }
                if (a.line != 98278 && a.line != 97742 && a.line != 97743)
                    continue;
                status = tfx_query_waiting(&r, page.persons[i], &b);
                if (status != TFX_OK || memcmp(&a, &b, sizeof(a))) {
                    ++errors;
                    continue;
                }
                tfx_person_platforms platforms = {.struct_size = sizeof(platforms)};
                status = tfx_query_platforms(&r, a.person, &platforms);
                if (status != TFX_OK)
                    ++errors;
                printf("waiting world_id=%u person=%d line=%d stops=%d,%d destination=%d vehicle=%d "
                       "platform_status=%u platforms=",
                       wi + 1, a.person, a.line, a.boarding_stop, a.alighting_stop, a.destination, a.vehicle,
                       status);
                if (status == TFX_OK)
                    for (unsigned j = 0; j < platforms.count; ++j)
                        printf("%d:%d,", platforms.platforms[j].station_entity,
                               platforms.platforms[j].terminal);
                printf(" repeated_read_agrees=1\n");
                ++people;
            }
            cursor = page.next_entity;
            ++pages;
        } while (cursor >= 0 && pages < 512);
        const int32_t lines[] = {98278, 97742, 97743};
        for (unsigned li = 0; li < 3; ++li) {
            for (int stop = 0; stop < 64; ++stop) {
                tfx_reader r;
                tfx_reader_init(&r, world, image, copy_remote, process);
                r.terminal_system = terminal;
                tfx_line_stop s = {.struct_size = sizeof(s)};
                tfx_status status = tfx_query_stop(&r, lines[li], stop, &s);
                if (status != TFX_OK) {
                    printf("stop_error line=%d stop=%d status=%u\n", lines[li], stop, status);
                    ++errors;
                    break;
                }
                printf("stop world_id=%u line=%d index=%d station=%d terminal=%d group=%d\n", wi + 1, s.line,
                       s.stop_index, s.station_entity, s.terminal, s.station_group);
                tfx_stop_vehicles list = {.struct_size = sizeof(list)};
                status = tfx_query_stop_vehicles(&r, lines[li], stop, &list);
                if (status != TFX_OK) {
                    printf("vehicles_error status=%u\n", status);
                    ++errors;
                    break;
                }
                for (unsigned j = 0; j < list.count; ++j) {
                    tfx_reader_init(&r, world, image, copy_remote, process);
                    r.terminal_system = terminal;
                    tfx_vehicle a = {.struct_size = sizeof(a)}, b = {.struct_size = sizeof(b)};
                    status = tfx_query_vehicle(&r, list.vehicles[j], &a);
                    if (status != TFX_OK) {
                        ++errors;
                        continue;
                    }
                    status = tfx_query_vehicle(&r, list.vehicles[j], &b);
                    if (status != TFX_OK || memcmp(&a, &b, sizeof(a))) {
                        ++errors;
                        continue;
                    }
                    printf(
                        "vehicle world_id=%u id=%d line=%d stop=%d state=%d load_state=%d terminal=%d "
                        "locked=%u timer_bits=%08x places=%u onboard=%u reserved=%u repeated_read_agrees=1\n",
                        wi + 1, a.vehicle, a.line, a.stop_index, a.state, a.load_state, a.arrival_terminal,
                        a.arrival_locked, a.next_passenger_load_seconds_bits, a.passenger_places_left,
                        a.passengers_onboard, a.passengers_reserved);
                }
                if (stop + 1 >= s.stop_count)
                    break;
            }
        }
        int idle_after = idle(process, world);
        if (cursor >= 0 || !idle_after)
            ++errors;
        printf("snapshot_end world_id=%u pages=%u people=%u scan_complete=%u idle_after=%d\n", wi + 1, pages,
               people, cursor < 0, idle_after);
    }
    printf("snapshot_summary errors=%u observational_only=1\n", errors);
    CloseHandle(process);
    return errors ? 3 : 0;
}
