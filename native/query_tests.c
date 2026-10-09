#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"
#include "mutation.h"

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static unsigned char world[0x200], controls[31], slots[31][16], stores[10][0x98];
static unsigned char data[10][32 * 0x1e8], stops[2][0x90];
static uint64_t store_ptrs[10], rows[8][3], pages[10][2], masks[8][2];
static int32_t handles[8][5][2], stations[2] = {90, 99};
static uint32_t load_word = 1, max_load = 0x3f400000; /* 0.75 */
static const uint32_t rvas[7] = {0x3cea8f0, 0x3cea768, 0x3cea8c0, 0x3cea828, 0x3cf2ba8, 0x3ceaaa0, 0x3cea860};
static const uint32_t strides[7] = {32, 0x98, 40, 24, 12, 0x1e8, 0x58};
static const uint64_t image = 0x140000000;
static tfx_api_v2 api = {.base.struct_size = sizeof(api)};
static tfx_context expired;
static tfx_api_v8 v8 = {.v7.v6.v5.v4.v3.v2.base.struct_size = sizeof(v8)};
static tfx_api_v7 v7 = {.v6.v5.v4.v3.v2.base.struct_size = sizeof(v7)};
static int32_t person_destinations[3];
static uint64_t unreadable;
static unsigned char terminal[0x200], vehicle_system[0x200], onboard_system[0x200];
static unsigned char terminal_dynamic[0xc0], vehicle_dynamic[0xc0], onboard_dynamic[0xc0];
static unsigned char index_controls[4][7], vehicle_slots[7][32], onboard_slots[7][32], assigned_slots[7][56];
static int32_t assigned_ids[7], vehicle_ids[17];
static uint32_t capacity, occupied, next_load;
static tfx_api_v3 v3 = {.v2.base.struct_size = sizeof(v3)};
static tfx_api_v5 v5 = {.v4.v3.v2.base.struct_size = sizeof(v5)};
static unsigned char terminals[3][0x88];
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
    uint64_t table[4] = {address(controls), address(slots), 7, 31};
    memcpy(world + 0x48, table, sizeof(table));
    vector_at(world + 0x78, store_ptrs, sizeof(store_ptrs));
    vector_at(world + 0x90, rows, sizeof(rows));
    vector_at(world + 0xc0, masks, sizeof(masks));
    for (unsigned i = 0; i < 7; ++i) {
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
    masks[2][0] = 1 << 2; masks[3][0] = 1 << 3; masks[4][0] = 1 << 5;
    handles[5][0][0] = 6; handles[5][0][1] = 0;
    vector_at(rows[5], handles[5], 8); masks[5][0] = 1 << 6;
    memset(terminals, 0, sizeof(terminals));
    vector_at(data[6], terminals, sizeof(terminals));
    terminals[1][0x61] = 1; terminals[2][0x63] = 1;
    put32(data[0], 2); put32(data[0] + 4, 0); put32(data[0] + 8, 1);
    put64(data[0] + 16, UINT64_C(0x123456789abcdef0)); put32(data[0] + 24, UINT32_MAX);
    put32(data[1] + 0x50, 43210);
    person_destinations[0] = 555; person_destinations[1] = -1; person_destinations[2] = 43210;
    vector_at(data[1] + 8, person_destinations, sizeof(person_destinations));
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
    vehicle_fixture(); r = reader(); controls[5] = 0x80; put64(world + 0x58, 6);
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

static void terminal_tests(void) {
    fixture(); tfx_reader r = reader();
    tfx_terminal t = {.struct_size = sizeof(t)};
    CHECK(tfx_query_terminal(&r, 5, 1, &t) == TFX_OK && t.passengers_load && !t.passengers_unload);
    CHECK(t.terminal_count == 3 && t.station_entity == 5 && t.terminal == 1);
    CHECK(tfx_query_terminal(&r, 5, 2, &t) == TFX_OK && !t.passengers_load && t.passengers_unload);
    tfx_terminal unchanged = t;
    CHECK(tfx_query_terminal(&r, 5, 3, &t) == TFX_NOT_FOUND && !memcmp(&t, &unchanged, sizeof(t)));
    terminals[2][0x63] = 2;
    CHECK(tfx_query_terminal(&r, 5, 2, &t) == TFX_INVALID_STATE);
    fixture(); r = reader();
    memcpy(data[6] + 31 * strides[6], data[6], strides[6]); handles[5][0][1] = 0x4000001f;
    CHECK(tfx_query_terminal(&r, 5, 1, &t) == TFX_OK && t.passengers_load);
    fixture(); r = reader(); put64(data[6] + 8, address(terminals) + 1);
    CHECK(tfx_query_terminal(&r, 5, 1, &t) == TFX_INVALID_STATE);
    fixture(); r = reader();
    tfx_entity_presence p = {.struct_size = sizeof(p)};
    CHECK(tfx_query_presence(&r, 1, &p) == TFX_OK && p.has_components);
    CHECK(tfx_query_presence(&r, 0, &p) == TFX_OK && !p.has_components);
    CHECK(tfx_query_presence(&r, INT32_MAX, &p) == TFX_OK && !p.has_components);
    tfx_entity_presence before = p;
    masks[1][0] = 0;
    CHECK(tfx_query_presence(&r, 1, &p) == TFX_INVALID_STATE && !memcmp(&p, &before, sizeof(p)));
    fixture(); r = reader(); handles[1][1][0] = 0;
    CHECK(tfx_query_presence(&r, 1, &p) == TFX_INVALID_STATE);
    fixture(); r = reader(); r.reads_left = 0;
    CHECK(tfx_query_presence(&r, 1, &p) == TFX_QUERY_BUDGET_EXHAUSTED);
    fixture(); r = reader(); controls[6] = 0x80; put64(world + 0x58, 6);
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK);
    CHECK(tfx_query_terminal(&r, 5, 1, &t) == TFX_QUERY_UNAVAILABLE);
}

static void terminal_client(tfx_context context, void *unused) {
    (void)unused;
    tfx_terminal t = {.struct_size = sizeof(t)};
    tfx_entity_presence p = {.struct_size = sizeof(p)};
    CHECK(v5.read_terminal(context, 5, 1, &t) == TFX_OK && t.passengers_load);
    CHECK(v5.read_entity_presence(context, 1, &p) == TFX_OK && p.has_components);
    expired = context;
}

static DWORD WINAPI wrong_thread(void *unused) {
    (void)unused;
    tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_WRONG_THREAD);
    tfx_person_lifecycle life = {.struct_size = sizeof(life)};
    CHECK(v7.read_person_lifecycle(expired, 1, &life) == TFX_WRONG_THREAD);
    tfx_vehicle v = {.struct_size = sizeof(v)};
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    CHECK(v3.read_vehicle(expired, 4, &v) == TFX_WRONG_THREAD);
    CHECK(v3.read_stop_vehicles(expired, 2, 0, &list) == TFX_WRONG_THREAD);
    tfx_person_platforms p = {.struct_size = sizeof(p)};
    CHECK(v4.read_person_platforms(expired, 1, &p) == TFX_WRONG_THREAD);
    tfx_terminal t = {.struct_size = sizeof(t)};
    tfx_entity_presence e = {.struct_size = sizeof(e)};
    CHECK(v5.read_terminal(expired, 5, 1, &t) == TFX_WRONG_THREAD);
    CHECK(v5.read_entity_presence(expired, 1, &e) == TFX_WRONG_THREAD);
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

static void lifecycle_tests(void) {
    fixture(); tfx_reader r = reader(); r.world_id = 2;
    tfx_person_lifecycle life = {.struct_size = sizeof(life)};
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_OK && life.world_id == 2 && life.person == 1 &&
        life.flags == 3 && life.target == 43210 && life.destinations[0] == 555 && life.destinations[2] == 43210);
    person_destinations[2] = -1; put32(data[1] + 0x50, 555);
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_OK && life.destinations[2] == -1 && life.target == 555);
    /* A non-person entity is valid, but does not retain stale person output. */
    CHECK(tfx_query_lifecycle(&r, 5, &life) == TFX_OK && life.flags == 1 && life.target == -1 && life.destinations[0] == -1);
    CHECK(tfx_query_lifecycle(&r, 7, &life) == TFX_OK && !life.flags);
    CHECK(tfx_query_lifecycle(&r, 8, &life) == TFX_OK && !life.flags);
    tfx_person_lifecycle unchanged = life;
    masks[7][0] = 2; /* Missing row is not evidence of retirement when the mask disagrees. */
    CHECK(tfx_query_lifecycle(&r, 7, &life) == TFX_INVALID_STATE && !memcmp(&life, &unchanged, sizeof(life)));
    fixture(); r = reader(); masks[1][0] &= ~2u;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE && !memcmp(&life, &unchanged, sizeof(life)));
    fixture(); r = reader(); handles[1][2][0] = 1;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader(); handles[1][1][1] = -1;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader(); unreadable = address(person_destinations);
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE && !memcmp(&life, &unchanged, sizeof(life)));
    fixture(); r = reader(); vector_at(data[1] + 8, person_destinations, 8);
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader(); put64(data[1] + 24, address(person_destinations) + 68);
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader(); person_destinations[1] = -2;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader(); put32(data[1], 1);
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE);
    fixture(); r = reader();
    memcpy(data[1] + 31 * 0x98, data[1], 0x98); handles[1][1][1] = 0x4000001f;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_OK && life.destinations[2] == 43210);
    unchanged = life; handles[1][1][1] = 0x40000020;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_STATE && !memcmp(&life, &unchanged, sizeof(life)));
    fixture(); r = reader(); r.bytes_left = 1;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_QUERY_BUDGET_EXHAUSTED);
    r = reader(); r.queries_left = 0;
    CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_QUERY_BUDGET_EXHAUSTED);
    CHECK(tfx_query_lifecycle(&r, -1, &life) == TFX_INVALID_ARGUMENT);
    life.struct_size = 0; CHECK(tfx_query_lifecycle(&r, 1, &life) == TFX_INVALID_ARGUMENT);
}
static void lifecycle_client(tfx_context c, void *unused) {
    (void)unused;
    tfx_person_lifecycle life = {.struct_size = sizeof(life)};
    CHECK(v7.read_person_lifecycle(c, 1, &life) == TFX_OK && life.destinations[2] == 43210);
    expired = c;
    for (unsigned i = 1; i < 64; ++i) CHECK(v7.read_person_lifecycle(c, 1, &life) == TFX_OK);
    CHECK(v7.read_person_lifecycle(c, 1, &life) == TFX_QUERY_BUDGET_EXHAUSTED);
}

static tfx_api_v6 v6 = {.v5.v4.v3.v2.base.struct_size = sizeof(v6)};
static unsigned engine_step, validation_count, host_mode, host_readback;
static uint64_t tickets[2];
static tfx_context staged_context;
static tfx_waiting_leg_change staged_request;
static tfx_status wanted_result;
static int corrupt_after_end;
static void journey_fixture(void) {
    fixture();
    const uint32_t extra[3] = {0x3cea960, 0x3ceaa00, 0x3ceaa68};
    for (unsigned i = 7; i < 10; ++i) {
        controls[i] = (unsigned char)i;
        put64(slots[i], image + extra[i - 7]); put32(slots[i] + 8, i + 1);
        store_ptrs[i] = address(stores[i]);
        vector_at(stores[i] + 0x68, data[i], 32);
        pages[i][0] = address(data[i]); vector_at(stores[i] + 0x80, pages[i], sizeof(pages[i]));
    }
    put64(world + 0x58, 10); world[0x1f8] = 1;
    put32(data[0] + 12, 0xaabbccdd); put32(data[0] + 28, 0x1234abcd);
    engine_step = 0; corrupt_after_end = 0;
}
static void fake_begin(void *w) {
    CHECK(w == world && engine_step++ == 0); put64(world + 0x1f0, 1);
}
static void fake_remove(void *w, int32_t person, const void *type, uint8_t notify) {
    CHECK(w == world && person == 1 && address(type) == image + 0x3cea8f0 && notify == 1 && engine_step++ == 1);
    memset(data[0], 0, 32);
}
static void fake_add(void *w, int32_t person, const void *value, uint8_t notify, uint8_t local) {
    CHECK(w == world && person == 1 && notify == 1 && local == 1 && engine_step++ == 2);
    memcpy(data[0], value, 32);
}
static void fake_end(void *w) {
    CHECK(w == world && engine_step++ == 3); put64(world + 0x1f0, 0);
    if (corrupt_after_end) data[0][16] ^= 1;
}
static void fake_log(const char *text) { CHECK(text != NULL); }
static tfx_engine_changes fake_ops = {fake_begin, fake_remove, fake_add, fake_end, fake_log};
static tfx_status guard_change(tfx_context c, const tfx_waiting_leg_change *q) {
    ++validation_count;
    tfx_waiting_leg live = {.struct_size = sizeof(live)};
    CHECK(v6.v5.v4.v3.v2.read_waiting_leg(staged_context, 1, &live) == TFX_STALE_CONTEXT);
    CHECK(v6.v5.v4.v3.v2.read_waiting_leg(c, 1, &live) == TFX_OK);
    uint64_t unused = 0;
    CHECK(v6.request_waiting_leg_change(c, q, &unused) == TFX_BUSY);
    return host_mode == 2 ? TFX_POLICY_REJECTED : TFX_OK;
}
static tfx_waiting_leg_change make_request(tfx_reader *r) {
    tfx_waiting_leg_change q = {.struct_size = sizeof(q), .line = 6,
        .boarding_stop = 2, .alighting_stop = 3, .candidate_vehicle = 4, .revalidate = guard_change};
    q.expected.struct_size = sizeof(q.expected);
    CHECK(tfx_query_waiting(r, 1, &q.expected) == TFX_OK);
    return q;
}
static DWORD WINAPI change_wrong_thread(void *unused) {
    (void)unused; uint64_t ticket;
    CHECK(v6.request_waiting_leg_change(staged_context, &staged_request, &ticket) == TFX_WRONG_THREAD);
    return 0;
}
static void change_client(tfx_context c, void *user) {
    unsigned id = (unsigned)(uintptr_t)user;
    if (host_readback) {
        tfx_change_result result = {.struct_size = sizeof(result)};
        CHECK(v6.read_change_result(c, tickets[id], &result) == TFX_OK && result.status == wanted_result);
        CHECK(result.engine_attempted == (wanted_result == TFX_OK));
        if (host_mode == 1) CHECK(v6.read_change_result(c, tickets[1 - id], &result) == TFX_NOT_FOUND);
        return;
    }
    CHECK(engine_step == 0); /* A second observer sees no in-dispatch writes. */
    if (host_mode == 3 && id == 1) { data[0][12] ^= 1; return; }
    tfx_reader r = reader(); staged_request = make_request(&r);
    tfx_observation obs = {.struct_size = sizeof(obs)};
    CHECK(v6.v5.v4.v3.v2.base.read_observation(c, &obs) == TFX_OK);
    staged_request.dispatch_sequence = obs.dispatch_sequence;
    staged_context = c;
    if (host_mode == 5) {
        uint64_t sentinel = 999;
        CHECK(v6.request_waiting_leg_change(c, &staged_request, &sentinel) == TFX_QUERY_UNAVAILABLE && sentinel == 999);
        return;
    }
    if (host_mode == 4) {
        uint64_t sentinel = 999;
        CHECK(v6.request_waiting_leg_change(c, &staged_request, &sentinel) == TFX_LIMIT_REACHED && sentinel == 999);
        return;
    }
    HANDLE thread = CreateThread(NULL, 0, change_wrong_thread, NULL, 0, NULL);
    CHECK(thread && WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0); CloseHandle(thread);
    CHECK(v6.request_waiting_leg_change(c, &staged_request, &tickets[id]) == TFX_OK);
    tfx_change_result result = {.struct_size = sizeof(result)};
    CHECK(v6.read_change_result(c, tickets[id], &result) == TFX_OK);
    CHECK(result.status == (host_mode == 1 && id == 1 ? TFX_REQUEST_CONFLICT : TFX_PENDING));
}
static void mutation_tests(void) {
    CHECK(tfx_get_api(6, &v6.v5.v4.v3.v2.base) == TFX_OK && v6.v5.v4.v3.v2.base.capabilities == 127);
    journey_fixture(); tfx_reader r = reader(); r.world_id = 2;
    tfx_person_journey j = {.struct_size = sizeof(j)};
    CHECK(tfx_query_journey(&r, 1, &j) == TFX_OK && j.flags == TFX_JOURNEY_WAITING && j.world_id == 2 && j.vehicle == -1);
    for (int32_t sentinel = -2; sentinel >= -3; --sentinel) {
        journey_fixture(); put32(data[0] + 24, (uint32_t)sentinel); r = reader();
        tfx_waiting_leg_change q = make_request(&r); CHECK(q.expected.vehicle == sentinel);
        uint64_t ticket = 123;
        CHECK(v6.request_waiting_leg_change(0, &q, &ticket) == TFX_INVALID_ARGUMENT && ticket == 123);
        unsigned char raw[32]; CHECK(tfx_query_waiting_raw(&r, 1, raw) == TFX_OK);
        uint32_t attempted = 99;
        CHECK(tfx_apply_waiting_change(&r, &q, raw, &attempted, &fake_ops) == TFX_EXPECTED_STATE_CHANGED);
        CHECK(!attempted && !engine_step); /* Even a fabricated request cannot switch a sentinel. */
        tfx_person_journey unchanged_journey = j;
        CHECK(tfx_query_journey(&r, 1, &j) == TFX_INVALID_STATE && !memcmp(&j, &unchanged_journey, sizeof(j)));
    }
    journey_fixture(); r = reader();
    handles[1][0][0] = 7; masks[1][0] = (1u << 7) | (1u << 1) | (1u << 4);
    put32(data[7], 6); put32(data[7] + 4, 2); put32(data[7] + 8, 3); put32(data[7] + 12, 4);
    CHECK(tfx_query_journey(&r, 1, &j) == TFX_OK && j.flags == TFX_JOURNEY_ONBOARD && j.line == 6 && j.vehicle == 4);
    memcpy(data[7] + 31 * 32, data[7], 32); handles[1][0][1] = 0x4000001f;
    CHECK(tfx_query_journey(&r, 1, &j) == TFX_OK && j.vehicle == 4);
    masks[1][0] |= 1; CHECK(tfx_query_journey(&r, 1, &j) == TFX_INVALID_STATE);
    masks[1][0] = (1u << 1) | (1u << 8); CHECK(tfx_query_journey(&r, 1, &j) == TFX_OK && j.flags == TFX_JOURNEY_MOVING);
    masks[1][0] = (1u << 1) | (1u << 9); CHECK(tfx_query_journey(&r, 1, &j) == TFX_OK && j.flags == TFX_JOURNEY_AT_BUILDING && j.line == -1);
    for (unsigned scenario = 0; scenario < 6; ++scenario) {
        journey_fixture(); r = reader(); tfx_waiting_leg_change q = make_request(&r);
        unsigned char raw[32]; CHECK(tfx_query_waiting_raw(&r, 1, raw) == TFX_OK);
        uint32_t attempted = 99;
        if (scenario == 0) data[0][12] ^= 1;
        if (scenario == 1) put32(data[1] + 0x50, 77);
        if (scenario == 2) put64(world + 0x1f0, 1);
        if (scenario == 3) r.queries_left = 3;
        if (scenario == 4) corrupt_after_end = 1;
        tfx_status status = tfx_apply_waiting_change(&r, &q, raw, &attempted, &fake_ops);
        if (scenario < 2) CHECK(status == TFX_EXPECTED_STATE_CHANGED && !attempted && !engine_step);
        if (scenario == 2) CHECK(status == TFX_INVALID_STATE && !attempted && !engine_step);
        if (scenario == 3) CHECK(status == TFX_QUERY_BUDGET_EXHAUSTED && !attempted && !engine_step);
        if (scenario == 4) CHECK(status == TFX_INVALID_STATE && attempted == 1 && engine_step == 4);
        if (scenario == 5) {
            CHECK(status == TFX_OK && attempted == 1 && engine_step == 4);
            CHECK(!memcmp(data[0] + 12, raw + 12, 20));
        }
    }
    tfx_client clients[2] = {
        {.struct_size = sizeof(tfx_client), .id = "tests.change.a", .before_terminal_selection = change_client},
        {.struct_size = sizeof(tfx_client), .id = "tests.change.b", .before_terminal_selection = change_client, .user_data = (void *)1}
    };
    journey_fixture(); host_mode = 5; host_readback = 0; validation_count = 0;
    CHECK(v6.v5.v4.v3.v2.base.register_client(&clients[0]) == TFX_OK);
    r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    CHECK(!engine_step && !validation_count);
    CHECK(v6.v5.v4.v3.v2.base.unregister_client(clients[0].id) == TFX_OK);
    for (host_mode = 1; host_mode <= 4; ++host_mode) {
        journey_fixture(); validation_count = 0; host_readback = 0;
        CHECK(v6.v5.v4.v3.v2.base.register_client(&clients[0]) == TFX_OK);
        if (host_mode == 1 || host_mode == 3) CHECK(v6.v5.v4.v3.v2.base.register_client(&clients[1]) == TFX_OK);
        r = reader(); CHECK(tfx_dispatch_with_writer(0, &r, tfx_apply_waiting_change, &fake_ops) == TFX_OK);
        CHECK(!engine_step);
        if (host_mode == 1) CHECK(!validation_count);
        if (host_mode == 3) CHECK(v6.v5.v4.v3.v2.base.unregister_client(clients[1].id) == TFX_OK);
        wanted_result = host_mode == 1 ? TFX_REQUEST_CONFLICT : host_mode == 2 ? TFX_POLICY_REJECTED : TFX_EXPECTED_STATE_CHANGED;
        host_readback = 1; r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
        CHECK(v6.v5.v4.v3.v2.base.unregister_client(clients[0].id) == TFX_OK);
        if (host_mode == 1) CHECK(v6.v5.v4.v3.v2.base.unregister_client(clients[1].id) == TFX_OK);
        if (host_mode == 3) break;
    }
    journey_fixture(); host_mode = 0; host_readback = 0;
    CHECK(v6.v5.v4.v3.v2.base.register_client(&clients[0]) == TFX_OK);
    r = reader(); CHECK(tfx_dispatch_with_writer(0, &r, tfx_apply_waiting_change, &fake_ops) == TFX_OK && engine_step == 4);
    host_readback = 1; wanted_result = TFX_OK; r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    journey_fixture(); host_mode = 4; host_readback = 0; r = reader();
    CHECK(tfx_dispatch_with_writer(0, &r, tfx_apply_waiting_change, &fake_ops) == TFX_OK && !engine_step);
    CHECK(v6.v5.v4.v3.v2.base.unregister_client(clients[0].id) == TFX_OK);
    uint64_t ticket; CHECK(v6.request_waiting_leg_change(staged_context, &staged_request, &ticket) == TFX_STALE_CONTEXT);
    puts("PASS: ABI6 journeys; deferred conflict/stale/policy guards, ownership, context/thread expiry; engine call order/fifth argument, byte preservation, postconditions and process-wide one-attempt cap.");
}

static DWORD WINAPI options_wrong_thread(void *unused) {
    (void)unused;
    tfx_stop_terminals value = {.struct_size = sizeof(value)};
    CHECK(v8.read_stop_terminals(expired, 2, 0, &value) == TFX_WRONG_THREAD);
    return 0;
}
static void options_client(tfx_context c, void *unused) {
    (void)unused; expired = c;
    tfx_stop_terminals value = {.struct_size = sizeof(value)};
    CHECK(v8.read_stop_terminals(c, 2, 0, &value) == TFX_OK && value.count == 1);
    HANDLE thread = CreateThread(NULL, 0, options_wrong_thread, NULL, 0, NULL);
    CHECK(thread != NULL); CHECK(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0); CloseHandle(thread);
}
static void options_tests(void) {
    int32_t alternatives[16][2] = {{0, 1}, {1, 3}};
    fixture(); tfx_reader r = reader();
    tfx_stop_terminals value = {.struct_size = sizeof(value)};
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_OK && value.count == 1);
    CHECK(value.stop.station_group == 3 && value.terminals[0].station_index == 1 &&
        value.terminals[0].station_entity == 99 && value.terminals[0].terminal == 2);
    CHECK(r.queries_left == 63);
    vector_at(stops[0] + 0x10, alternatives, 16); r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_OK && value.count == 3 && value.stop.has_alternative_terminals);
    CHECK(value.terminals[1].station_entity == 90 && value.terminals[1].station_index == 0 && value.terminals[1].terminal == 1);
    CHECK(value.terminals[2].station_entity == 99 && value.terminals[2].terminal == 3);
    /* Both contiguous and shared line storage use the same copied contract. */
    handles[2][0][1] = 0x40000000; r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_OK && value.count == 3);
    tfx_stop_terminals unchanged = value;
    vector_at(stops[0] + 0x10, alternatives, 128); r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_LIMIT_REACHED && !memcmp(&value, &unchanged, sizeof(value)));
    vector_at(stops[0] + 0x10, alternatives, 9); r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_INVALID_STATE);
    vector_at(stops[0] + 0x10, alternatives, 16); alternatives[0][0] = -1; r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_INVALID_STATE);
    alternatives[0][0] = 2; r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_INVALID_STATE);
    alternatives[0][0] = 0; alternatives[0][1] = -1; r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_INVALID_STATE);
    alternatives[0][1] = 1; unreadable = address(alternatives); r = reader();
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_INVALID_STATE && !memcmp(&value, &unchanged, sizeof(value)));
    unreadable = 0; r = reader(); r.queries_left = 0;
    CHECK(tfx_query_stop_terminals(&r, 2, 0, &value) == TFX_QUERY_BUDGET_EXHAUSTED);
    CHECK(tfx_query_stop_terminals(&r, 2, 0, NULL) == TFX_INVALID_ARGUMENT);
    puts("PASS: ABI8 primary/alternative copies, station resolution, shared storage, complete-result limit, malformed data and failure preservation.");
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
    for (int32_t sentinel = -2; sentinel >= -3; --sentinel) {
        put32(data[0] + 24, (uint32_t)sentinel);
        CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_OK && leg.vehicle == sentinel);
        tfx_waiting_leg expected = unchanged; expected.vehicle = sentinel;
        CHECK(!memcmp(&leg, &expected, sizeof(leg))); /* Preserve opaque bytes and exact sentinel. */
    }
    put32(data[0] + 24, (uint32_t)-4); tfx_waiting_leg special = leg;
    CHECK(tfx_query_waiting(&r, 1, &leg) == TFX_INVALID_STATE && !memcmp(&leg, &special, sizeof(leg)));
    put32(data[0] + 24, UINT32_MAX); leg = unchanged;
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
    CHECK(tfx_get_api(7, &v7.v6.v5.v4.v3.v2.base) == TFX_OK && v7.v6.v5.v4.v3.v2.base.capabilities == 255);
    CHECK(tfx_get_api(3, &v3.v2.base) == TFX_OK && v3.v2.base.capabilities == 7);
    CHECK(tfx_get_api(4, &v4.v3.v2.base) == TFX_OK && v4.v3.v2.base.capabilities == 15);
    CHECK(tfx_get_api(5, &v5.v4.v3.v2.base) == TFX_OK && v5.v4.v3.v2.base.capabilities == 31);
    CHECK(api.base.abi_version == 2 && api.base.capabilities == 3);
    tfx_client c = {.struct_size = sizeof(c), .id = "tests.queries", .before_terminal_selection = client};
    CHECK(api.base.register_client(&c) == TFX_OK);
    unsigned char before[sizeof(world)]; memcpy(before, world, sizeof(world));
    r = reader(); CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    CHECK(!memcmp(before, world, sizeof(world)));
    CHECK(api.read_waiting_leg(expired, 1, &leg) == TFX_STALE_CONTEXT);
    CHECK(api.base.unregister_client(c.id) == TFX_OK);
    options_tests(); fixture(); r = reader();
    CHECK(tfx_get_api(8, &v8.v7.v6.v5.v4.v3.v2.base) == TFX_OK && v8.v7.v6.v5.v4.v3.v2.base.capabilities == 511);
    c.before_terminal_selection = options_client;
    CHECK(api.base.register_client(&c) == TFX_OK);
    CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    tfx_stop_terminals options = {.struct_size = sizeof(options)};
    CHECK(v8.read_stop_terminals(expired, 2, 0, &options) == TFX_STALE_CONTEXT);
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
    terminal_tests(); fixture(); r = reader();
    c.before_terminal_selection = terminal_client;
    CHECK(v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    tfx_terminal t = {.struct_size = sizeof(t)};
    CHECK(v5.read_terminal(expired, 5, 1, &t) == TFX_STALE_CONTEXT);
    CHECK(v5.v4.v3.v2.base.unregister_client(c.id) == TFX_OK);
    lifecycle_tests(); fixture(); r = reader();
    c.before_terminal_selection = lifecycle_client;
    CHECK(v7.v6.v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    CHECK(tfx_dispatch_with_reader(0, &r) == TFX_OK);
    tfx_person_lifecycle life = {.struct_size = sizeof(life)};
    CHECK(v7.read_person_lifecycle(expired, 1, &life) == TFX_STALE_CONTEXT);
    CHECK(v7.v6.v5.v4.v3.v2.base.unregister_client(c.id) == TFX_OK);
    puts("PASS: ABI7 copied lifecycle, row/mask consistency, absent/reused IDs, destination slots, shared storage, output preservation and context/thread/budget guards.");
    mutation_tests();
    puts("PASS: terminal load/unload flags, entity row/mask consistency, missing/shared data, ABI5 guards.");
    puts("PASS: platform edge membership, distinct worlds, missing/ambiguous systems, malformed/oversized results, ABI4 guards.");
    puts("PASS: vehicle capacity/reservations, stop index, selected terminal, shared storage, malformed tables, ABI3 guards.");
    puts("PASS: passenger/route copies, contiguous/shared storage, malformed/missing data, budgets, scan, ABI2 lifetime/thread guards.");
    return 0;
}
