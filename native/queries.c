#include <string.h>
#include "queries.h"

/* Offsets/strides are pinned to the backend executable fingerprint. */
enum { WAITING, PERSON, LINE, GROUP, PLACE, VEHICLE, STATION, TYPE_COUNT };
static const uint32_t type_rvas[TYPE_COUNT] = {0x3cea8f0, 0x3cea768, 0x3cea8c0, 0x3cea828, 0x3cf2ba8, 0x3ceaaa0, 0x3cea860};
static const uint32_t strides[TYPE_COUNT] = {0x20, 0x98, 0x28, 0x18, 0x0c, 0x1e8, 0x58};
typedef struct { uint64_t begin, end, capacity; } vector;
typedef struct { uint64_t control, slots, size, capacity; } type_table;
#define TRY(expression) do { tfx_status status_ = (expression); if (status_ != TFX_OK) return status_; } while (0)
#define ENTITY_LIMIT (1u << 24)

static uint64_t u64(const void *data) { uint64_t v; memcpy(&v, data, 8); return v; }
static int32_t i32(const void *data) { int32_t v; memcpy(&v, data, 4); return v; }

void tfx_reader_init(tfx_reader *r, uint64_t world, uint64_t image, tfx_copy_memory copy, void *user) {
    *r = (tfx_reader){.world = world, .image = image, .copy = copy, .user = user,
        .bytes_left = 1024 * 1024, .reads_left = 8192, .queries_left = 64};
}

static tfx_status read_bytes(tfx_reader *r, uint64_t address, void *out, size_t bytes) {
    if (!r || !r->copy) return TFX_QUERY_UNAVAILABLE;
    if (!address || address > UINT64_MAX - bytes) return TFX_INVALID_STATE;
    if (!r->reads_left || bytes > r->bytes_left) return TFX_QUERY_BUDGET_EXHAUSTED;
    --r->reads_left;
    r->bytes_left -= (uint32_t)bytes;
    return r->copy(r->user, address, out, bytes) ? TFX_OK : TFX_INVALID_STATE;
}

static int valid_vector(vector v, uint64_t stride, uint64_t limit) {
    if (!v.begin) return !v.end && !v.capacity;
    return v.begin <= v.end && v.end <= v.capacity &&
        (v.end - v.begin) % stride == 0 && (v.capacity - v.begin) % stride == 0 &&
        (v.capacity - v.begin) / stride <= limit;
}

static tfx_status begin_query(tfx_reader *r) {
    if (!r || !r->copy) return TFX_QUERY_UNAVAILABLE;
    if (!r->queries_left) return TFX_QUERY_BUDGET_EXHAUSTED;
    --r->queries_left;
    if (!r->world || r->world > UINT64_MAX - 0xd8 || !r->image || r->image > UINT64_MAX - 0x3cf2ba8)
        return TFX_INVALID_STATE;
    if (r->types_ready) return TFX_OK;
    type_table table;
    TRY(read_bytes(r, r->world + 0x48, &table, sizeof(table)));
    if (!table.control || !table.slots || table.capacity > 4095 ||
        !table.capacity || (table.capacity & (table.capacity + 1)) ||
        table.size > 128 || table.size > table.capacity ||
        table.slots > UINT64_MAX - table.capacity * 16) return TFX_INVALID_STATE;
    unsigned char controls[4096];
    TRY(read_bytes(r, table.control, controls, (size_t)table.capacity));
    for (unsigned t = 0; t < TYPE_COUNT; ++t) r->types[t] = -1;
    uint64_t occupied = 0;
    for (uint64_t slot = 0; slot < table.capacity; ++slot) {
        if (controls[slot] & 0x80) continue; /* Empty, deleted, sentinel. */
        ++occupied;
        unsigned char entry[16];
        TRY(read_bytes(r, table.slots + slot * 16, entry, sizeof(entry)));
        for (unsigned t = 0; t < TYPE_COUNT; ++t) {
            if (u64(entry) != r->image + type_rvas[t]) continue;
            int32_t value = i32(entry + 8);
            if (r->types[t] != -1 || value < 1 || value > 128) return TFX_INVALID_STATE;
            r->types[t] = value - 1;
        }
    }
    if (occupied != table.size) return TFX_INVALID_STATE;
    for (unsigned t = 0; t < TYPE_COUNT; ++t) {
        if (r->types[t] == -1) {
            if (t >= VEHICLE) continue; /* Earlier queries do not require these types. */
            return TFX_QUERY_UNAVAILABLE;
        }
        for (unsigned other = 0; other < t; ++other)
            if (r->types[t] == r->types[other]) return TFX_INVALID_STATE;
    }
    r->types_ready = 1;
    return TFX_OK;
}

static tfx_status entity_vector(tfx_reader *r, vector *rows) {
    TRY(read_bytes(r, r->world + 0x90, rows, sizeof(*rows)));
    return valid_vector(*rows, 24, ENTITY_LIMIT) ? TFX_OK : TFX_INVALID_STATE;
}

static tfx_status component(tfx_reader *r, int32_t entity, unsigned type, void *out, size_t bytes) {
    if (entity < 0 || type >= TYPE_COUNT || bytes > strides[type]) return TFX_INVALID_ARGUMENT;
    if (r->types[type] < 0) return TFX_QUERY_UNAVAILABLE;
    vector rows, entries, storage;
    TRY(entity_vector(r, &rows));
    if ((uint32_t)entity >= (rows.end - rows.begin) / 24) return TFX_NOT_FOUND;
    TRY(read_bytes(r, rows.begin + (uint64_t)entity * 24, &entries, sizeof(entries)));
    if (!valid_vector(entries, 8, 128)) return TFX_INVALID_STATE;
    unsigned char handles[128 * 8];
    size_t length = (size_t)(entries.end - entries.begin);
    if (!length) return TFX_NOT_FOUND;
    TRY(read_bytes(r, entries.begin, handles, length));
    int found = 0;
    int32_t index = -1;
    for (size_t i = 0; i < length; i += 8) {
        if (i32(handles + i) != r->types[type]) continue;
        if (found++) return TFX_INVALID_STATE;
        index = i32(handles + i + 4);
    }
    if (!found) return TFX_NOT_FOUND;
    if (index < 0) return TFX_INVALID_STATE;
    TRY(read_bytes(r, r->world + 0x78, &storage, sizeof(storage)));
    if (!valid_vector(storage, 8, 128) || (uint32_t)r->types[type] >= (storage.end - storage.begin) / 8)
        return TFX_INVALID_STATE;
    uint64_t store;
    TRY(read_bytes(r, storage.begin + (uint64_t)r->types[type] * 8, &store, 8));
    if (!store || store > UINT64_MAX - 0x98) return TFX_INVALID_STATE;
    uint64_t address;
    if (index < 0x40000000) {
        vector data;
        TRY(read_bytes(r, store + 0x68, &data, sizeof(data)));
        if (!valid_vector(data, strides[type], ENTITY_LIMIT) ||
            (uint32_t)index >= (data.end - data.begin) / strides[type]) return TFX_INVALID_STATE;
        address = data.begin + (uint64_t)index * strides[type];
    } else {
        uint32_t shared = (uint32_t)index - 0x40000000;
        vector pages;
        TRY(read_bytes(r, store + 0x80, &pages, sizeof(pages)));
        if (!valid_vector(pages, 16, ENTITY_LIMIT / 32) ||
            shared / 32 >= (pages.end - pages.begin) / 16) return TFX_INVALID_STATE;
        TRY(read_bytes(r, pages.begin + (uint64_t)(shared / 32) * 16, &address, 8));
        if (!address || address > UINT64_MAX - 32u * strides[type]) return TFX_INVALID_STATE;
        address += (shared % 32u) * strides[type];
    }
    return read_bytes(r, address, out, bytes);
}

tfx_status tfx_query_waiting(tfx_reader *r, int32_t person, tfx_waiting_leg *out) {
    if (!out || out->struct_size != sizeof(*out) || person < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    unsigned char waiting[32], human[0x54], place[12];
    TRY(component(r, person, PERSON, human, sizeof(human)));
    if (i32(human) != 0) return TFX_INVALID_STATE;
    TRY(component(r, person, WAITING, waiting, sizeof(waiting)));
    tfx_waiting_leg result = {.struct_size = sizeof(result), .person = person,
        .line = i32(waiting), .boarding_stop = i32(waiting + 4), .alighting_stop = i32(waiting + 8),
        .vehicle = i32(waiting + 24), .destination = i32(human + 0x50),
        .arrival_time_bits = u64(waiting + 16)};
    if (result.line < 0 || result.boarding_stop < 0 || result.alighting_stop < 0 || result.vehicle < -1)
        return TFX_INVALID_STATE;
    tfx_status status = component(r, person, PLACE, place, sizeof(place));
    if (status == TFX_OK) {
        result.flags = TFX_HAS_TERMINAL_PLACE;
        memcpy(result.terminal_place_bytes, place, sizeof(place));
    } else if (status != TFX_NOT_FOUND) return status;
    *out = result;
    return TFX_OK;
}

tfx_status tfx_query_stop(tfx_reader *r, int32_t line, int32_t stop_index, tfx_line_stop *out) {
    if (!out || out->struct_size != sizeof(*out) || line < 0 || stop_index < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    vector stops;
    TRY(component(r, line, LINE, &stops, sizeof(stops)));
    if (!valid_vector(stops, 0x90, 4096)) return TFX_INVALID_STATE;
    uint64_t count = (stops.end - stops.begin) / 0x90;
    if ((uint32_t)stop_index >= count) return TFX_NOT_FOUND;
    unsigned char stop[0x90];
    TRY(read_bytes(r, stops.begin + (uint64_t)stop_index * 0x90, stop, sizeof(stop)));
    tfx_line_stop result = {.struct_size = sizeof(result), .line = line, .stop_index = stop_index,
        .stop_count = (int32_t)count, .station_group = i32(stop), .station_index = i32(stop + 4),
        .terminal = i32(stop + 8), .load_mode = i32(stop + 0x28)};
    if (result.station_group < 0 || result.station_index < 0 || result.terminal < 0 ||
        result.load_mode < 0 || result.load_mode > 3) return TFX_INVALID_STATE;
    vector stations;
    TRY(component(r, result.station_group, GROUP, &stations, sizeof(stations)));
    if (!valid_vector(stations, 4, 4096) ||
        (uint32_t)result.station_index >= (stations.end - stations.begin) / 4) return TFX_INVALID_STATE;
    TRY(read_bytes(r, stations.begin + (uint64_t)result.station_index * 4, &result.station_entity, 4));
    if (result.station_entity < 0) return TFX_INVALID_STATE;
    vector alternatives, load_words, max_load;
    memcpy(&alternatives, stop + 0x10, sizeof(alternatives));
    memcpy(&load_words, stop + 0x50, sizeof(load_words));
    memcpy(&max_load, stop + 0x70, sizeof(max_load));
    uint64_t load_count = u64(stop + 0x68);
    if (!valid_vector(alternatives, 1, 65536) || !valid_vector(load_words, 4, 4) ||
        !load_count || load_count > 128 || (load_count + 31) / 32 != (load_words.end - load_words.begin) / 4 ||
        !valid_vector(max_load, 4, 128) || (max_load.end - max_load.begin) / 4 != load_count)
        return TFX_INVALID_STATE;
    uint32_t bits;
    TRY(read_bytes(r, load_words.begin, &bits, 4));
    result.passenger_load_enabled = bits & 1;
    result.has_alternative_terminals = alternatives.begin != alternatives.end;
    TRY(read_bytes(r, max_load.begin, &result.passenger_max_load_bits, 4));
    /* Positive finite IEEE-754 [0,1], allowing either representation of zero. */
    uint32_t fraction = result.passenger_max_load_bits;
    if (fraction != 0x80000000u && fraction > 0x3f800000u) return TFX_INVALID_STATE;
    *out = result;
    return TFX_OK;
}

tfx_status tfx_query_scan(tfx_reader *r, int32_t first_entity, tfx_waiting_page *out) {
    if (!out || out->struct_size != sizeof(*out) || first_entity < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    vector rows, masks;
    TRY(entity_vector(r, &rows));
    uint64_t count = (rows.end - rows.begin) / 24;
    tfx_waiting_page result = {.struct_size = sizeof(result), .next_entity = -1};
    if ((uint32_t)first_entity >= count) { *out = result; return TFX_OK; }
    TRY(read_bytes(r, r->world + 0xc0, &masks, sizeof(masks)));
    if (!valid_vector(masks, 16, ENTITY_LIMIT) || (masks.end - masks.begin) / 16 < count)
        return TFX_INVALID_STATE;
    uint64_t limit = count - (uint32_t)first_entity;
    if (limit > 1024) limit = 1024;
    uint64_t copied[1024][2];
    TRY(read_bytes(r, masks.begin + (uint64_t)first_entity * 16, copied, (size_t)limit * 16));
    for (uint64_t i = 0; i < limit; ++i) {
        int32_t entity = first_entity + (int32_t)i;
        int waiting = r->types[WAITING], person = r->types[PERSON];
        result.next_entity = (uint64_t)entity + 1 < count ? entity + 1 : -1;
        if (!(copied[i][waiting / 64] & (UINT64_C(1) << (waiting % 64))) ||
            !(copied[i][person / 64] & (UINT64_C(1) << (person % 64)))) continue;
        result.persons[result.count++] = entity;
        if (result.count == TFX_WAITING_PAGE_SIZE) break;
    }
    *out = result;
    return TFX_OK;
}

/* Bounded Swiss-table copies. Scan occupied slots instead of calling engine
 * accessors: several nominal getters can detach copy-on-write storage. */
static tfx_status table_copy(tfx_reader *r, uint64_t address, uint32_t stride,
                             type_table *table, unsigned char controls[4096]) {
    TRY(read_bytes(r, address, table, sizeof(*table)));
    if (table->size > table->capacity || table->capacity > 4095 ||
        (table->capacity & (table->capacity + 1))) return TFX_INVALID_STATE;
    if (!table->capacity) return TFX_OK;
    if (!table->control || !table->slots ||
        table->slots > UINT64_MAX - table->capacity * stride) return TFX_INVALID_STATE;
    TRY(read_bytes(r, table->control, controls, (size_t)table->capacity));
    uint64_t occupied = 0;
    for (uint64_t i = 0; i < table->capacity; ++i) occupied += !(controls[i] & 0x80);
    return occupied == table->size ? TFX_OK : TFX_INVALID_STATE;
}

static tfx_status map_value(tfx_reader *r, uint64_t address, uint32_t stride,
                            const void *key, size_t key_size, uint64_t *value) {
    type_table table;
    unsigned char controls[4096];
    TRY(table_copy(r, address, stride, &table, controls));
    uint64_t found = 0;
    for (uint64_t i = 0; i < table.capacity; ++i) {
        if (controls[i] & 0x80) continue;
        unsigned char candidate[8];
        uint64_t slot = table.slots + i * stride;
        TRY(read_bytes(r, slot, candidate, key_size));
        if (memcmp(candidate, key, key_size)) continue;
        if (found) return TFX_INVALID_STATE;
        found = slot + 8;
    }
    if (!found) return TFX_NOT_FOUND;
    *value = found;
    return TFX_OK;
}

static tfx_status pointer_at(tfx_reader *r, uint64_t base, uint32_t offset, uint64_t *value) {
    if (!base || base > UINT64_MAX - offset - 8) return TFX_INVALID_STATE;
    TRY(read_bytes(r, base + offset, value, 8));
    return *value && *value <= UINT64_MAX - 0x200 ? TFX_OK : TFX_INVALID_STATE;
}

tfx_status tfx_query_stop_vehicles(tfx_reader *r, int32_t line, int32_t stop, tfx_stop_vehicles *out) {
    if (!out || out->struct_size != sizeof(*out) || line < 0 || stop < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    if (!r->terminal_system) return TFX_QUERY_UNAVAILABLE;
    vector stops;
    TRY(component(r, line, LINE, &stops, sizeof(stops)));
    if (!valid_vector(stops, 0x90, 4096)) return TFX_INVALID_STATE;
    if ((uint32_t)stop >= (stops.end - stops.begin) / 0x90) return TFX_NOT_FOUND;
    uint64_t system, dynamic, value;
    TRY(pointer_at(r, r->terminal_system, 0x28, &system));
    TRY(pointer_at(r, system, 0x1b0, &dynamic));
    int32_t key[2] = {line, stop};
    tfx_status status = map_value(r, dynamic + 0x90, 0x20, key, sizeof(key), &value);
    tfx_stop_vehicles result = {.struct_size = sizeof(result), .line = line, .stop_index = stop};
    if (status == TFX_NOT_FOUND) { *out = result; return TFX_OK; }
    if (status != TFX_OK) return status;
    vector vehicles;
    TRY(read_bytes(r, value, &vehicles, sizeof(vehicles)));
    if (!valid_vector(vehicles, 4, ENTITY_LIMIT)) return TFX_INVALID_STATE;
    uint64_t count = (vehicles.end - vehicles.begin) / 4;
    if (count > TFX_STOP_VEHICLES_SIZE) return TFX_LIMIT_REACHED;
    result.count = (uint32_t)count;
    if (count) TRY(read_bytes(r, vehicles.begin, result.vehicles, (size_t)count * 4));
    for (uint32_t i = 0; i < result.count; ++i) {
        if (result.vehicles[i] < 0) return TFX_INVALID_STATE;
        for (uint32_t j = 0; j < i; ++j)
            if (result.vehicles[i] == result.vehicles[j]) return TFX_INVALID_STATE;
    }
    *out = result;
    return TFX_OK;
}

static tfx_status reserved_passengers(tfx_reader *r, int32_t vehicle, uint32_t *count) {
    uint64_t dynamic, value;
    TRY(pointer_at(r, r->terminal_system, 0x78, &dynamic));
    tfx_status status = map_value(r, dynamic + 0x90, 0x38, &vehicle, 4, &value);
    *count = 0;
    if (status == TFX_NOT_FOUND) return TFX_OK;
    if (status != TFX_OK) return status;
    type_table set;
    unsigned char controls[4096];
    TRY(table_copy(r, value, 4, &set, controls));
    if (set.size > 1024) return TFX_LIMIT_REACHED;
    int32_t seen[1024];
    uint32_t seen_count = 0;
    for (uint64_t i = 0; i < set.capacity; ++i) {
        if (controls[i] & 0x80) continue;
        int32_t entity;
        TRY(read_bytes(r, set.slots + i * 4, &entity, 4));
        if (entity < 0) return TFX_INVALID_STATE;
        for (uint32_t j = 0; j < seen_count; ++j)
            if (seen[j] == entity) return TFX_INVALID_STATE;
        seen[seen_count++] = entity;
        unsigned char waiting[32];
        TRY(component(r, entity, WAITING, waiting, sizeof(waiting)));
        if (i32(waiting + 24) != vehicle) return TFX_INVALID_STATE;
        /* This index also contains freight. Only SimPerson contributes to
         * passenger occupancy; its cargo type must be zero. */
        int32_t cargo_type;
        status = component(r, entity, PERSON, &cargo_type, 4);
        if (status == TFX_NOT_FOUND) continue;
        if (status != TFX_OK) return status;
        if (cargo_type != 0) return TFX_INVALID_STATE;
        ++*count;
    }
    return TFX_OK;
}

tfx_status tfx_query_vehicle(tfx_reader *r, int32_t vehicle, tfx_vehicle *out) {
    if (!out || out->struct_size != sizeof(*out) || vehicle < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    if (!r->terminal_system) return TFX_QUERY_UNAVAILABLE;
    unsigned char data[0x160];
    TRY(component(r, vehicle, VEHICLE, data, sizeof(data)));
    tfx_vehicle result = {.struct_size = sizeof(result), .vehicle = vehicle,
        .line = i32(data + 0xb8), .stop_index = i32(data + 0xbc),
        .state = i32(data + 0xa8), .load_state = i32(data + 0x138),
        .arrival_station_index = i32(data + 0xc0), .arrival_terminal = i32(data + 0xc4),
        .arrival_locked = data[0xc8], .user_stopped = data[0xac]};
    if (result.line < -1 || result.stop_index < -1 || result.arrival_station_index < -1 ||
        result.arrival_terminal < -1 || result.arrival_locked > 1 || result.user_stopped > 1)
        return TFX_INVALID_STATE;
    vector capacities, timers;
    memcpy(&capacities, data + 0x70, sizeof(capacities));
    memcpy(&timers, data + 0x148, sizeof(timers));
    if (!valid_vector(capacities, 4, 128) || capacities.begin == capacities.end ||
        !valid_vector(timers, 4, 128) || timers.begin == timers.end) return TFX_INVALID_STATE;
    TRY(read_bytes(r, capacities.begin, &result.passenger_capacity, 4));
    TRY(read_bytes(r, timers.begin, &result.next_passenger_load_seconds_bits, 4));
    if (result.passenger_capacity > INT32_MAX ||
        (result.next_passenger_load_seconds_bits & 0x7f800000u) == 0x7f800000u) return TFX_INVALID_STATE;
    uint64_t system, dynamic, value;
    TRY(pointer_at(r, r->terminal_system, 0x30, &system));
    TRY(pointer_at(r, system, 0x78, &dynamic));
    tfx_status status = map_value(r, dynamic + 0x60, 0x20, &vehicle, 4, &value);
    if (status == TFX_OK) {
        vector counts;
        TRY(read_bytes(r, value, &counts, sizeof(counts)));
        if (!valid_vector(counts, 4, 128) || counts.begin == counts.end) return TFX_INVALID_STATE;
        TRY(read_bytes(r, counts.begin, &result.passengers_onboard, 4));
    } else if (status != TFX_NOT_FOUND) return status;
    TRY(reserved_passengers(r, vehicle, &result.passengers_reserved));
    uint64_t used = (uint64_t)result.passengers_onboard + result.passengers_reserved;
    if (used > result.passenger_capacity) return TFX_INVALID_STATE;
    result.passenger_places_left = result.passenger_capacity - (uint32_t)used;
    *out = result;
    return TFX_OK;
}

static tfx_status find_station_system(tfx_reader *r) {
    if (r->station_system) return TFX_OK; /* Cache only within this dispatch. */
    vector systems;
    TRY(read_bytes(r, r->world + 0x30, &systems, sizeof(systems)));
    if (!valid_vector(systems, 8, 256)) return TFX_INVALID_STATE;
    uint64_t pointers[256], found = 0;
    size_t bytes = (size_t)(systems.end - systems.begin);
    if (bytes) TRY(read_bytes(r, systems.begin, pointers, bytes));
    for (size_t i = 0; i < bytes / 8; ++i) {
        uint64_t vtable;
        TRY(read_bytes(r, pointers[i], &vtable, 8));
        /* RTTI .?AVStationSystem@ecs@@, validated against this executable. */
        if (vtable != r->image + 0x3706600) continue;
        if (found) return TFX_INVALID_STATE;
        found = pointers[i];
    }
    if (!found) return TFX_QUERY_UNAVAILABLE;
    r->station_system = found;
    return TFX_OK;
}

tfx_status tfx_query_platforms(tfx_reader *r, int32_t person, tfx_person_platforms *out) {
    if (!out || out->struct_size != sizeof(*out) || person < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    int32_t cargo_type;
    unsigned char waiting[32];
    int32_t place[3];
    TRY(component(r, person, PERSON, &cargo_type, 4));
    if (cargo_type != 0) return TFX_INVALID_STATE;
    TRY(component(r, person, WAITING, waiting, sizeof(waiting)));
    TRY(component(r, person, PLACE, place, sizeof(place)));
    if (place[0] < 0 || place[1] < 0 || place[2] < 0) return TFX_INVALID_STATE;
    TRY(find_station_system(r));
    uint64_t dynamic, value;
    TRY(pointer_at(r, r->station_system, 0x38, &dynamic));
    tfx_status status = map_value(r, dynamic + 0x70, 0x20, place, 8, &value);
    tfx_person_platforms result = {.struct_size = sizeof(result), .person = person,
        .edge_entity = place[0], .edge_index = place[1], .place = place[2]};
    if (status == TFX_NOT_FOUND) { *out = result; return TFX_OK; }
    if (status != TFX_OK) return status;
    vector platforms;
    TRY(read_bytes(r, value, &platforms, sizeof(platforms)));
    if (!valid_vector(platforms, 8, 4096)) return TFX_INVALID_STATE;
    uint64_t count = (platforms.end - platforms.begin) / 8;
    if (count > TFX_PERSON_PLATFORMS_SIZE) return TFX_LIMIT_REACHED;
    result.count = (uint32_t)count;
    if (count) TRY(read_bytes(r, platforms.begin, result.platforms, (size_t)count * 8));
    for (uint32_t i = 0; i < result.count; ++i) {
        if (result.platforms[i].station_entity < 0 || result.platforms[i].terminal < 0)
            return TFX_INVALID_STATE;
        for (uint32_t j = 0; j < i; ++j)
            if (!memcmp(&result.platforms[i], &result.platforms[j], sizeof(tfx_platform)))
                return TFX_INVALID_STATE;
    }
    *out = result;
    return TFX_OK;
}

tfx_status tfx_query_terminal(tfx_reader *r, int32_t station, int32_t terminal, tfx_terminal *out) {
    if (!out || out->struct_size != sizeof(*out) || station < 0 || terminal < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    vector terminals;
    TRY(component(r, station, STATION, &terminals, sizeof(terminals)));
    if (!valid_vector(terminals, 0x88, 4096)) return TFX_INVALID_STATE;
    uint64_t count = (terminals.end - terminals.begin) / 0x88;
    if ((uint32_t)terminal >= count) return TFX_NOT_FOUND;
    unsigned char flags[4];
    TRY(read_bytes(r, terminals.begin + (uint64_t)terminal * 0x88 + 0x60, flags, sizeof(flags)));
    if (flags[1] > 1 || flags[3] > 1) return TFX_INVALID_STATE;
    tfx_terminal result = {.struct_size = sizeof(result), .station_entity = station,
        .terminal = terminal, .terminal_count = (int32_t)count,
        .passengers_load = flags[1], .passengers_unload = flags[3]};
    *out = result;
    return TFX_OK;
}

tfx_status tfx_query_presence(tfx_reader *r, int32_t entity, tfx_entity_presence *out) {
    if (!out || out->struct_size != sizeof(*out) || entity < 0) return TFX_INVALID_ARGUMENT;
    TRY(begin_query(r));
    vector rows, entries, masks;
    TRY(entity_vector(r, &rows));
    tfx_entity_presence result = {.struct_size = sizeof(result), .entity = entity};
    if ((uint32_t)entity >= (rows.end - rows.begin) / 24) { *out = result; return TFX_OK; }
    TRY(read_bytes(r, rows.begin + (uint64_t)entity * 24, &entries, sizeof(entries)));
    if (!valid_vector(entries, 8, 128)) return TFX_INVALID_STATE;
    int32_t handles[128][2];
    size_t bytes = (size_t)(entries.end - entries.begin);
    if (bytes) TRY(read_bytes(r, entries.begin, handles, bytes));
    uint64_t expected[2] = {0}, actual[2];
    for (size_t i = 0; i < bytes / 8; ++i) {
        int32_t type = handles[i][0];
        if (type < 0 || type >= 128 || handles[i][1] < 0) return TFX_INVALID_STATE;
        uint64_t bit = UINT64_C(1) << (type % 64);
        if (expected[type / 64] & bit) return TFX_INVALID_STATE;
        expected[type / 64] |= bit;
    }
    TRY(read_bytes(r, r->world + 0xc0, &masks, sizeof(masks)));
    if (!valid_vector(masks, 16, ENTITY_LIMIT) ||
        (masks.end - masks.begin) / 16 < (rows.end - rows.begin) / 24) return TFX_INVALID_STATE;
    TRY(read_bytes(r, masks.begin + (uint64_t)entity * 16, actual, sizeof(actual)));
    if (memcmp(expected, actual, sizeof(expected))) return TFX_INVALID_STATE;
    result.has_components = bytes != 0;
    *out = result;
    return TFX_OK;
}
