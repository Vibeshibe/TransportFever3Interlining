#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static unsigned char world[0x200], controls[31], slots[31][16], stores[6][0x98];
static unsigned char data[6][32 * 0x1e8], stops[2][0x90];
static uint64_t store_ptrs[6], rows[6][3], pages[6][2], masks[6][2];
static int32_t handles[6][5][2], stations[2] = {90, 99};
static uint32_t load_word = 1, max_load = 0x3f400000; /* 0.75 */
static const uint32_t rvas[6] = {0x3cea8f0, 0x3cea768, 0x3cea8c0, 0x3cea828, 0x3cf2ba8, 0x3ceaaa0};
static const uint32_t strides[6] = {32, 0x98, 40, 24, 12, 0x1e8};
static const uint64_t image = 0x140000000;
static tfx_api_v2 api = {.base.struct_size = sizeof(api)};
static tfx_context expired;
static uint64_t unreadable;
static unsigned char terminal[0x200], vehicle_system[0x200], onboard_system[0x200];
static unsigned char terminal_dynamic[0xc0], vehicle_dynamic[0xc0], onboard_dynamic[0xc0];
static unsigned char index_controls[4][7], vehicle_slots[7][32], onboard_slots[7][32], assigned_slots[7][56];
static int32_t assigned_ids[7], vehicle_ids[17];
static uint32_t capacity, occupied, next_load;
static tfx_api_v3 v3 = {.v2.base.struct_size = sizeof(v3)};
static tfx_api_v4 v4 = {.v3.v2.base.struct_size = sizeof(v4)};
static unsigned char station_system[0x80], station_dynamic[0xb0], platform_slots[7][32], platform_controls[7];
static uint64_t system_ptrs[3];
static tfx_platform platform_values[17];

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
    r.terminal_system = address(terminal);
    return r;
}
static void fixture(void) {
    memset(world, 0, sizeof(world)); memset(controls, 0x80, sizeof(controls));
    memset(slots, 0, sizeof(slots)); memset(stores, 0, sizeof(stores));
    memset(data, 0, sizeof(data)); memset(stops, 0, sizeof(stops));
    memset(rows, 0, sizeof(rows)); memset(masks, 0, sizeof(masks)); unreadable = 0;
    uint64_t table[4] = {address(controls), address(slots), 6, 31};
    memcpy(world + 0x48, table, sizeof(table));
    vector_at(world + 0x78, store_ptrs, sizeof(store_ptrs));
    vector_at(world + 0x90, rows, sizeof(rows));
    vector_at(world + 0xc0, masks, sizeof(masks));
    for (unsigned i = 0; i < 6; ++i) {
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
    handles[4][0][0] = 5; handles[4][0][1] = 0;
    vector_at(rows[1], handles[1], 24);
    vector_at(rows[2], handles[2], 8);
    vector_at(rows[3], handles[3], 8);
    vector_at(rows[4], handles[4], 8);
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

static void table_at(void *at, unsigned char *ctrl, void *entries, uint64_t count, uint64_t cap) {
    uint64_t table[4] = {address(ctrl), address(entries), count, cap};
    memcpy(at, table, sizeof(table));
}

static void vehicle_fixture(void) {
    fixture();
    memset(terminal, 0, sizeof(terminal));
    memset(vehicle_system, 0, sizeof(vehicle_system)); memset(onboard_system, 0, sizeof(onboard_system));
    memset(terminal_dynamic, 0, sizeof(terminal_dynamic));
    memset(vehicle_dynamic, 0, sizeof(vehicle_dynamic)); memset(onboard_dynamic, 0, sizeof(onboard_dynamic));
    memset(index_controls, 0x80, sizeof(index_controls));
    memset(vehicle_slots, 0, sizeof(vehicle_slots)); memset(onboard_slots, 0, sizeof(onboard_slots));
    memset(assigned_slots, 0, sizeof(assigned_slots));
    put64(terminal + 0x28, address(vehicle_system)); put64(terminal + 0x30, address(onboard_system));
    put64(terminal + 0x78, address(terminal_dynamic));
    put64(vehicle_system + 0x1b0, address(vehicle_dynamic));
    put64(onboard_system + 0x78, address(onboard_dynamic));
    for (unsigned i = 0; i < 4; ++i) index_controls[i][0] = 0;
    table_at(vehicle_dynamic + 0x90, index_controls[0], vehicle_slots, 1, 7);
    table_at(onboard_dynamic + 0x60, index_controls[1], onboard_slots, 1, 7);
    table_at(terminal_dynamic + 0x90, index_controls[2], assigned_slots, 1, 7);
    table_at(assigned_slots[0] + 8, index_controls[3], assigned_ids, 1, 7);
    put32(vehicle_slots[0], 2); put32(vehicle_slots[0] + 4, 0);
    vehicle_ids[0] = 4; vector_at(vehicle_slots[0] + 8, vehicle_ids, 4);
    put32(onboard_slots[0], 4); occupied = 2; vector_at(onboard_slots[0] + 8, &occupied, 4);
    put32(assigned_slots[0], 4); assigned_ids[0] = 1;
    put32(data[0] + 24, 4);
    capacity = 10; next_load = 0xbf000000; /* -0.5 seconds */
    vector_at(data[5] + 0x70, &capacity, 4); vector_at(data[5] + 0x148, &next_load, 4);
    put32(data[5] + 0xa8, 2); put32(data[5] + 0x138, 3);
    put32(data[5] + 0xb8, 2); put32(data[5] + 0xbc, 0);
    put32(data[5] + 0xc0, 1); put32(data[5] + 0xc4, 7); data[5][0xc8] = 1;
}

static void vehicle_tests(void) {
    vehicle_fixture(); tfx_reader r = reader();
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    tfx_vehicle v = {.struct_size = sizeof(v)};
    CHECK(tfx_query_stop_vehicles(&r, 2, 0, &list) == TFX_OK && list.count == 1 && list.vehicles[0] == 4);
    CHECK(tfx_query_stop_vehicles(&r, 2, 1, &list) == TFX_OK && list.count == 0);
    CHECK(tfx_query_stop_vehicles(&r, 2, 2, &list) == TFX_NOT_FOUND);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_OK);
    CHECK(v.passenger_capacity == 10 && v.passengers_onboard == 2 && v.passengers_reserved == 1 && v.passenger_places_left == 7);
    CHECK(v.arrival_station_index == 1 && v.arrival_terminal == 7 && v.arrival_locked == 1);
    CHECK(v.next_passenger_load_seconds_bits == 0xbf000000 && v.line == 2 && v.stop_index == 0);
    tfx_vehicle unchanged = v;
    occupied = 10;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE && !memcmp(&v, &unchanged, sizeof(v)));
    vehicle_fixture(); r = reader(); put32(data[0] + 24, UINT32_MAX);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE); /* stale reservation index */
    vehicle_fixture(); r = reader();
    vector_at(rows[1], handles[1], 8); /* Freight entry: waiting but no SimPerson. */
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_OK && !v.passengers_reserved && v.passenger_places_left == 8);
    vehicle_fixture(); r = reader();
    memcpy(data[0] + strides[0], data[0], strides[0]);
    vector_at(stores[0] + 0x68, data[0], 2 * strides[0]);
    handles[5][0][0] = 0; handles[5][0][1] = 1; vector_at(rows[5], handles[5], 8);
    assigned_ids[1] = 5; index_controls[3][1] = 0; put64(assigned_slots[0] + 24, 2);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_OK && v.passengers_reserved == 1 && v.passenger_places_left == 7);
    vehicle_fixture(); r = reader(); next_load = 0x7f800000;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE);
    vehicle_fixture(); r = reader(); index_controls[1][0] = 0x80;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE); /* occupied count mismatch */
    vehicle_fixture(); r = reader();
    memset(onboard_dynamic + 0x60, 0, 32); memset(terminal_dynamic + 0x90, 0, 32);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_OK && v.passenger_places_left == 10); /* empty maps */
    vehicle_fixture(); r = reader();
    memcpy(onboard_slots[1], onboard_slots[0], 32); index_controls[1][1] = 0;
    put64(onboard_dynamic + 0x70, 2);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE); /* duplicate map key */
    vehicle_fixture(); r = reader(); assigned_ids[1] = 1; index_controls[3][1] = 0;
    put64(assigned_slots[0] + 24, 2);
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_INVALID_STATE); /* duplicate set entity */
    vehicle_fixture(); r = reader();
    memcpy(data[5] + 31 * strides[5], data[5], strides[5]); handles[4][0][1] = 0x4000001f;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_OK && v.passenger_places_left == 7);
    vehicle_fixture(); r = reader();
    vector_at(vehicle_slots[0] + 8, vehicle_ids, sizeof(vehicle_ids));
    tfx_stop_vehicles before = list;
    CHECK(tfx_query_stop_vehicles(&r, 2, 0, &list) == TFX_LIMIT_REACHED && !memcmp(&list, &before, sizeof(list)));
    vehicle_fixture(); r = reader(); vehicle_ids[1] = 4;
    vector_at(vehicle_slots[0] + 8, vehicle_ids, 8);
    CHECK(tfx_query_stop_vehicles(&r, 2, 0, &list) == TFX_INVALID_STATE);
    vehicle_fixture(); r = reader();
    put64(vehicle_dynamic + 0xa8, UINT64_MAX);
    CHECK(tfx_query_stop_vehicles(&r, 2, 0, &list) == TFX_INVALID_STATE);
    vehicle_fixture(); r = reader(); r.reads_left = 1;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_QUERY_BUDGET_EXHAUSTED);
    vehicle_fixture(); r = reader(); r.terminal_system = 0;
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_QUERY_UNAVAILABLE);
    vehicle_fixture(); r = reader(); controls[5] = 0x80; put64(world + 0x58, 5);
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK); /* ABI2 still works without vehicle type. */
    CHECK(tfx_query_vehicle(&r, 4, &v) == TFX_QUERY_UNAVAILABLE);
}

static void platform_fixture(void) {
    fixture();
    memset(station_system, 0, sizeof(station_system));
    memset(station_dynamic, 0, sizeof(station_dynamic));
    memset(platform_slots, 0, sizeof(platform_slots));
    memset(platform_controls, 0x80, sizeof(platform_controls));
    put64(station_system, image + 0x3706600);
    put64(station_system + 0x38, address(station_dynamic));
    system_ptrs[0] = address(station_system);
    vector_at(world + 0x30, system_ptrs, 8);
    platform_controls[0] = 0;
    table_at(station_dynamic + 0x70, platform_controls, platform_slots, 1, 7);
    put32(data[4], 90); put32(data[4] + 4, 5); put32(data[4] + 8, 2);
    put32(platform_slots[0], 90); put32(platform_slots[0] + 4, 5);
    platform_values[0] = (tfx_platform){99, 2}; platform_values[1] = (tfx_platform){99, 3};
    vector_at(platform_slots[0] + 8, platform_values, 16);
}

static void platform_tests(void) {
    platform_fixture(); tfx_reader r = reader();
    tfx_person_platforms p = {.struct_size = sizeof(p)};
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_OK && p.count == 2);
    CHECK(p.person == 1 && p.edge_entity == 90 && p.edge_index == 5 && p.place == 2);
    CHECK(p.platforms[0].station_entity == 99 && p.platforms[1].terminal == 3);
    tfx_person_platforms unchanged = p;
    platform_values[1] = platform_values[0];
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE && !memcmp(&p, &unchanged, sizeof(p)));
    platform_fixture(); r = reader();
    put32(data[4] + 4, 6);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_OK && !p.count); /* unmapped edge, not same entity's other edge */
    platform_fixture(); r = reader();
    vector_at(rows[1], handles[1], 16);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_NOT_FOUND); /* no waiting place */
    platform_fixture(); r = reader();
    put32(data[4] + 8, UINT32_MAX);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE);
    platform_fixture(); r = reader(); platform_values[0].terminal = -1;
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE);
    platform_fixture(); r = reader();
    vector_at(platform_slots[0] + 8, platform_values, sizeof(platform_values));
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_LIMIT_REACHED);
    platform_fixture(); r = reader();
    system_ptrs[1] = system_ptrs[0]; vector_at(world + 0x30, system_ptrs, 16);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE); /* ambiguous system ownership */
    platform_fixture(); r = reader(); put64(station_system, image + 0x3702758);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_QUERY_UNAVAILABLE);
    platform_fixture(); r = reader(); put64(world + 0x38, address(system_ptrs) + 1);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE);
    platform_fixture(); r = reader(); unreadable = address(station_dynamic + 0x70);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_INVALID_STATE);
    platform_fixture(); r = reader();
    memcpy(data[4] + 31 * strides[4], data[4], strides[4]); handles[1][2][1] = 0x4000001f;
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_OK && p.count == 2);
    platform_fixture(); r = reader();
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_OK && p.platforms[0].station_entity == 99);
    /* A second world's StationSystem owns a different map for the same person. */
    unsigned char world_b[sizeof(world)], system_b[sizeof(station_system)], dynamic_b[sizeof(station_dynamic)];
    unsigned char slots_b[sizeof(platform_slots)];
    tfx_platform values_b[1] = {{123, 7}};
    memcpy(world_b, world, sizeof(world)); memcpy(system_b, station_system, sizeof(system_b));
    memcpy(dynamic_b, station_dynamic, sizeof(dynamic_b)); memcpy(slots_b, platform_slots, sizeof(slots_b));
    uint64_t systems_b[1] = {address(system_b)};
    vector_at(world_b + 0x30, systems_b, sizeof(systems_b));
    put64(system_b + 0x38, address(dynamic_b));
    put64(dynamic_b + 0x78, address(slots_b)); vector_at(slots_b + 8, values_b, sizeof(values_b));
    tfx_reader second;
    tfx_reader_init(&second, address(world_b), image, copy_memory, NULL);
    CHECK(tfx_query_platforms(&second, 1, &p) == TFX_OK && p.count == 1 && p.platforms[0].station_entity == 123);
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_OK && p.count == 2 && p.platforms[0].station_entity == 99);
    r.reads_left = 0;
    CHECK(tfx_query_platforms(&r, 1, &p) == TFX_QUERY_BUDGET_EXHAUSTED);
}

static DWORD WINAPI wrong_thread(void *unused) {
    (void)unused;
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_WRONG_THREAD);
    tfx_vehicle v = {.struct_size = sizeof(v)};
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    CHECK(v3.read_vehicle(expired, 4, &v) == TFX_WRONG_THREAD);
    CHECK(v3.read_stop_vehicles(expired, 2, 0, &list) == TFX_WRONG_THREAD);
    tfx_person_platforms p = {.struct_size = sizeof(p)};
    CHECK(v4.read_person_platforms(expired, 1, &p) == TFX_WRONG_THREAD);
    return 0;
}

static void vehicle_client(tfx_context context, void *unused) {
    (void)unused;
    tfx_vehicle v = {.struct_size = sizeof(v)};
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    CHECK(v3.read_vehicle(context, 4, &v) == TFX_OK && v.passenger_places_left == 7);
    CHECK(v3.read_stop_vehicles(context, 2, 0, &list) == TFX_OK && list.count == 1);
    expired = context;
}

static void platform_client(tfx_context context, void *unused) {
    (void)unused;
    tfx_person_platforms p = {.struct_size = sizeof(p)};
    CHECK(v4.read_person_platforms(context, 1, &p) == TFX_OK && p.count == 2);
    expired = context;
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
    CHECK(tfx_get_api(3, &v3.v2.base) == TFX_OK && v3.v2.base.capabilities == 7);
    CHECK(tfx_get_api(4, &v4.v3.v2.base) == TFX_OK && v4.v3.v2.base.capabilities == 15);
    CHECK(api.base.abi_version == 2 && api.base.capabilities == 3);
    tfx_client c = {.struct_size = sizeof(c), .id = "tests.queries", .before_terminal_selection = client};
    CHECK(api.base.register_client(&c) == TFX_OK);
    unsigned char before[sizeof(world)]; memcpy(before, world, sizeof(world));
    r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    CHECK(!memcmp(before, world, sizeof(world)));
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_STALE_CONTEXT);
    CHECK(api.base.unregister_client(c.id) == TFX_OK);
    vehicle_tests(); vehicle_fixture(); r = reader();
    c.before_terminal_selection = vehicle_client;
    CHECK(v3.v2.base.register_client(&c) == TFX_OK);
    CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    tfx_vehicle v = {.struct_size = sizeof(v)};
    CHECK(v3.read_vehicle(expired, 4, &v) == TFX_STALE_CONTEXT);
    CHECK(v3.v2.base.unregister_client(c.id) == TFX_OK);
    platform_tests(); platform_fixture(); r = reader();
    c.before_terminal_selection = platform_client;
    CHECK(v4.v3.v2.base.register_client(&c) == TFX_OK);
    CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    tfx_person_platforms p = {.struct_size = sizeof(p)};
    CHECK(v4.read_person_platforms(expired, 1, &p) == TFX_STALE_CONTEXT);
    CHECK(v4.v3.v2.base.unregister_client(c.id) == TFX_OK);
    puts("PASS: platform edge membership, distinct worlds, missing/ambiguous systems, malformed/oversized results, ABI4 guards.");
    puts("PASS: vehicle capacity/reservations, stop index, selected terminal, shared storage, malformed tables, ABI3 guards.");
    puts("PASS: passenger/route copies, contiguous/shared storage, malformed/missing data, budgets, scan, ABI2 lifetime/thread guards.");
    return 0;
}
