#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static unsigned char world[0x200], controls[31], slots[31][16], stores[5][0x98];
static unsigned char data[5][32 * 0x98], stops[2][0x90];
static uint64_t store_ptrs[5], rows[6][3], pages[5][2], masks[6][2];
static int32_t handles[6][5][2], stations[2] = {90, 99};
static uint32_t load_word = 1, max_load = 0x3f400000; /* 0.75 */
static const uint32_t rvas[5] = {0x3cea8f0, 0x3cea768, 0x3cea8c0, 0x3cea828, 0x3cf2ba8};
static const uint32_t strides[5] = {32, 0x98, 40, 24, 12};
static const uint64_t image = 0x140000000;
static tfx_api_v2 api = {.base.struct_size = sizeof(api)};
static tfx_context expired;
static uint64_t unreadable;

static uint64_t address(const void *p) { return (uint64_t)(uintptr_t)p; }
static void put32(void *p, uint32_t v) { memcpy(p, &v, 4); }
static void put64(void *p, uint64_t v) { memcpy(p, &v, 8); }
static void vector_at(void *p, void *start, size_t size) {
    uint64_t v[3] = {address(start), address(start) + size, address(start) + size};
    memcpy(p, v, sizeof(v));
}
static int copy_memory(void *unused, uint64_t at, void *out, size_t size) {
    (void)unused;
    if (at == unreadable) return 0;
    SIZE_T got;
    return ReadProcessMemory(GetCurrentProcess(), (const void *)(uintptr_t)at, out, size, &got) && got == size;
}
static tfx_reader reader(void) {
    tfx_reader r;
    tfx_reader_init(&r, address(world), image, copy_memory, NULL);
    return r;
}
static void fixture(void) {
    memset(world, 0, sizeof(world)); memset(controls, 0x80, sizeof(controls));
    memset(slots, 0, sizeof(slots)); memset(stores, 0, sizeof(stores));
    memset(data, 0, sizeof(data)); memset(stops, 0, sizeof(stops));
    memset(rows, 0, sizeof(rows)); memset(masks, 0, sizeof(masks)); unreadable = 0;
    uint64_t table[4] = {address(controls), address(slots), 5, 31};
    memcpy(world + 0x48, table, sizeof(table));
    vector_at(world + 0x78, store_ptrs, sizeof(store_ptrs));
    vector_at(world + 0x90, rows, sizeof(rows));
    vector_at(world + 0xc0, masks, sizeof(masks));
    for (unsigned i = 0; i < 5; ++i) {
        controls[i] = (unsigned char)i;
        put64(slots[i], image + rvas[i]); put32(slots[i] + 8, i + 1);
        store_ptrs[i] = address(stores[i]);
        vector_at(stores[i] + 0x68, data[i], strides[i]);
        pages[i][0] = address(data[i]); pages[i][1] = 0;
        vector_at(stores[i] + 0x80, pages[i], sizeof(pages[i]));
    }
    handles[1][0][0] = 0; handles[1][0][1] = 0;
    handles[1][1][0] = 1; handles[1][1][1] = 0;
    handles[1][2][0] = 4; handles[1][2][1] = 0;
    handles[2][0][0] = 2; handles[2][0][1] = 0;
    handles[3][0][0] = 3; handles[3][0][1] = 0;
    vector_at(rows[1], handles[1], 24);
    vector_at(rows[2], handles[2], 8);
    vector_at(rows[3], handles[3], 8);
    masks[1][0] = (1 << 0) | (1 << 1) | (1 << 4);
    put32(data[0], 2); put32(data[0] + 4, 0); put32(data[0] + 8, 1);
    put64(data[0] + 16, UINT64_C(0x123456789abcdef0)); put32(data[0] + 24, UINT32_MAX);
    put32(data[1] + 0x50, 43210);
    memset(data[4], 0xa5, 12);
    vector_at(data[2], stops, sizeof(stops));
    vector_at(data[3], stations, sizeof(stations));
    load_word = 1; max_load = 0x3f400000;
    for (unsigned i = 0; i < 2; ++i) {
        put32(stops[i], 3); put32(stops[i] + 4, 1); put32(stops[i] + 8, 2 + i);
        vector_at(stops[i] + 0x50, &load_word, 4); put64(stops[i] + 0x68, 1);
        vector_at(stops[i] + 0x70, &max_load, 4);
    }
}

static DWORD WINAPI wrong_thread(void *unused) {
    (void)unused;
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_WRONG_THREAD);
    return 0;
}
static void client(tfx_context context, void *unused) {
    (void)unused;
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(api.read_waiting_leg(context, 1, &leg) == TFX_OK);
    CHECK(leg.destination == 43210);
    expired = context;
    HANDLE thread = CreateThread(NULL, 0, wrong_thread, NULL, 0, NULL);
    CHECK(thread && WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
    CloseHandle(thread);
    for (unsigned i = 1; i < 64; ++i) CHECK(api.read_waiting_leg(context, 1, &leg) == TFX_OK);
    CHECK(api.read_waiting_leg(context, 1, &leg) == TFX_QUERY_BUDGET_EXHAUSTED);
}

int main(void) {
    fixture();
    tfx_reader r = reader();
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK);
    CHECK(leg.line == 2 && leg.boarding_stop == 0 && leg.alighting_stop == 1 && leg.vehicle == -1);
    CHECK(leg.destination == 43210 && leg.arrival_time_bits == UINT64_C(0x123456789abcdef0));
    CHECK(leg.flags == TFX_HAS_TERMINAL_PLACE && leg.terminal_place_bytes[11] == 0xa5);
    tfx_waiting_leg unchanged = leg;
    CHECK(tfx_query_waiting(&r, 5, &leg) == TFX_NOT_FOUND && !memcmp(&leg, &unchanged, sizeof(leg)));
    CHECK(tfx_query_waiting(&r, INT32_MAX, &leg) == TFX_NOT_FOUND);
    CHECK(tfx_query_waiting(&r, -1, &leg) == TFX_INVALID_ARGUMENT);
    tfx_line_stop stop = {.struct_size = sizeof(stop)};
    CHECK(tfx_query_stop(&r, 2, 1, &stop) == TFX_OK);
    CHECK(stop.station_group == 3 && stop.station_index == 1 && stop.station_entity == 99 && stop.terminal == 3);
    CHECK(stop.stop_count == 2 && stop.passenger_load_enabled == 1 && stop.passenger_max_load_bits == 0x3f400000);
    CHECK(tfx_query_stop(&r, 2, 2, &stop) == TFX_NOT_FOUND);
    load_word = 0; CHECK(tfx_query_stop(&r, 2, 0, &stop) == TFX_OK && !stop.passenger_load_enabled);
    max_load = 0x7fc00000; CHECK(tfx_query_stop(&r, 2, 0, &stop) == TFX_INVALID_STATE);
    fixture(); r = reader();
    put32(stops[0] + 4, 99); CHECK(tfx_query_stop(&r, 2, 0, &stop) == TFX_INVALID_STATE);
    fixture(); r = reader();
    put64(data[2] + 8, address(stops) - 1); CHECK(tfx_query_stop(&r, 2, 0, &stop) == TFX_INVALID_STATE);
    fixture(); r = reader();
    /* High-tag shared storage: select entry 31 from the first 32-entry page. */
    for (unsigned t = 0; t < 5; ++t) memcpy(data[t] + 31 * strides[t], data[t], strides[t]);
    for (unsigned h = 0; h < 3; ++h) handles[1][h][1] = 0x4000001f;
    handles[2][0][1] = handles[3][0][1] = 0x4000001f;
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK && !memcmp(&leg, &unchanged, sizeof(leg)));
    CHECK(tfx_query_stop(&r, 2, 1, &stop) == TFX_OK && stop.station_entity == 99);
    handles[1][0][1] = 0x40000020; CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_INVALID_STATE);
    fixture(); r = reader();
    unreadable = address(data[0]); CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_INVALID_STATE);
    fixture(); r = reader(); controls[0] = 0x80;
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_INVALID_STATE); /* map size mismatch */
    fixture(); r = reader(); put32(slots[1] + 8, 1);
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_INVALID_STATE); /* duplicate type index */
    fixture(); r = reader();
    tfx_waiting_page page = {.struct_size = sizeof(page)};
    CHECK(tfx_query_scan(&r, 0, &page) == TFX_OK && page.count == 1 && page.persons[0] == 1 && page.next_entity == -1);
    CHECK(tfx_query_scan(&r, 6, &page) == TFX_OK && page.count == 0);
    uint64_t many_rows[1100][3] = {{0}}, many_masks[1100][2] = {{0}};
    vector_at(world + 0x90, many_rows, sizeof(many_rows));
    vector_at(world + 0xc0, many_masks, sizeof(many_masks));
    many_masks[1026][0] = 3;
    CHECK(tfx_query_scan(&r, 0, &page) == TFX_OK && !page.count && page.next_entity == 1024);
    CHECK(tfx_query_scan(&r, 1024, &page) == TFX_OK && page.count == 1 && page.persons[0] == 1026 && page.next_entity == -1);
    for (unsigned i = 0; i < 20; ++i) many_masks[i][0] = 3;
    CHECK(tfx_query_scan(&r, 0, &page) == TFX_OK && page.count == 16 && page.next_entity == 16);
    fixture(); r = reader();
    vector_at(rows[1], handles[1], 16);
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK && !leg.flags);
    fixture(); r = reader();
    r = reader(); r.bytes_left = 1;
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_QUERY_BUDGET_EXHAUSTED);
    CHECK(tfx_get_api(2, &api.base) == TFX_OK);
    CHECK(api.base.abi_version == 2 && api.base.capabilities == 3);
    tfx_client c = {.struct_size = sizeof(c), .id = "tests.queries", .before_terminal_selection = client};
    CHECK(api.base.register_client(&c) == TFX_OK);
    unsigned char before[sizeof(world)]; memcpy(before, world, sizeof(world));
    r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    CHECK(!memcmp(before, world, sizeof(world)));
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_STALE_CONTEXT);
    CHECK(api.base.unregister_client(c.id) == TFX_OK);
    puts("PASS: passenger/route copies, contiguous/shared storage, malformed/missing data, budgets, scan, ABI2 lifetime/thread guards.");
    return 0;
}
