/* Bounded example consumer. Uses only the public API; never an engine pointer. */
#include <stdio.h>
#include <string.h>
#include "query_probe.h"

static tfx_api_v3 api = {.v2.base.struct_size = sizeof(tfx_api_v3)};
static void (*log_line)(const char *);
static int32_t cursor;
static uint32_t samples, failures;

/* This consumer is deliberately specific to the disposable S1/S2/S3 save.
 * Entity IDs are inputs to the public API, never native pointers or SDK defaults. */
static void sample_vehicles(tfx_context context) {
    static const int32_t lines[] = {98278, 97742, 97743};
    static uint32_t tick, logged, errors, empty_logged;
    static tfx_vehicle seen[16];
    static uint32_t seen_count;
    if (logged >= 48 || errors >= 8) return;
    int32_t line_id = lines[tick++ % 3];
    tfx_line_stop stop = {.struct_size = sizeof(stop)};
    tfx_status status = api.v2.read_line_stop(context, line_id, 0, &stop);
    char text[512];
    if (status != TFX_OK) {
        snprintf(text, sizeof(text), "query_vehicle_line=%d status=%u\n", line_id, status);
        log_line(text); ++errors; return;
    }
    int32_t stop_index = (int32_t)((tick / 3) % (uint32_t)stop.stop_count);
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    status = api.read_stop_vehicles(context, line_id, stop_index, &list);
    if (status != TFX_OK || (list.count == 0 && empty_logged++ < 6)) {
        snprintf(text, sizeof(text), "query_stop_vehicles line=%d stop=%d status=%u count=%u\n",
            line_id, stop_index, status, list.count);
        log_line(text);
    }
    if (status != TFX_OK) { ++errors; return; }
    for (uint32_t i = 0; i < list.count && logged < 48; ++i) {
        tfx_vehicle v = {.struct_size = sizeof(v)};
        status = api.read_vehicle(context, list.vehicles[i], &v);
        if (status != TFX_OK) {
            snprintf(text, sizeof(text), "query_vehicle=%d status=%u\n", list.vehicles[i], status);
            log_line(text); if (++errors >= 8) return; continue;
        }
        uint32_t at = 0;
        while (at < seen_count && seen[at].vehicle != v.vehicle) ++at;
        if (at < seen_count && !memcmp(&seen[at], &v, sizeof(v))) continue;
        if (at < 16) { seen[at] = v; if (at == seen_count) ++seen_count; }
        snprintf(text, sizeof(text),
            "query_vehicle=%d index_line=%d index_stop=%d line=%d stop=%d state=%d load_state=%d arrival_station_index=%d terminal=%d locked=%u stopped=%u capacity=%u onboard=%u reserved=%u places_left=%u load_timer_bits=%08x\n",
            v.vehicle, line_id, stop_index, v.line, v.stop_index, v.state, v.load_state,
            v.arrival_station_index, v.arrival_terminal, v.arrival_locked, v.user_stopped,
            v.passenger_capacity, v.passengers_onboard, v.passengers_reserved, v.passenger_places_left,
            v.next_passenger_load_seconds_bits);
        log_line(text); ++logged;
    }
}

static void sample(tfx_context context, void *unused) {
    (void)unused;
    sample_vehicles(context);
    if (samples >= 24 || failures >= 8) return;
    tfx_waiting_page page = {.struct_size = sizeof(page)};
    tfx_status status = api.v2.scan_waiting_people(context, cursor, &page);
    char line[512];
    if (status != TFX_OK) {
        ++failures;
        snprintf(line, sizeof(line), "query_scan status=%u cursor=%d\n", status, cursor);
        log_line(line);
        return;
    }
    cursor = page.next_entity < 0 ? 0 : page.next_entity;
    for (uint32_t i = 0; i < page.count && i < 2 && samples < 24; ++i) {
        tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
        status = api.v2.read_waiting_leg(context, page.persons[i], &leg);
        if (status != TFX_OK) {
            if (failures++ < 8) {
                snprintf(line, sizeof(line), "query_waiting person=%d status=%u\n", page.persons[i], status);
                log_line(line);
            }
            continue;
        }
        tfx_line_stop board = {.struct_size = sizeof(board)}, alight = {.struct_size = sizeof(alight)};
        tfx_status board_status = api.v2.read_line_stop(context, leg.line, leg.boarding_stop, &board);
        tfx_status alight_status = api.v2.read_line_stop(context, leg.line, leg.alighting_stop, &alight);
        snprintf(line, sizeof(line),
            "query_person=%d line=%d stops=%d,%d vehicle=%d destination=%d arrival_bits=%016llx place=%u board_status=%u alight_status=%u\n",
            leg.person, leg.line, leg.boarding_stop, leg.alighting_stop, leg.vehicle, leg.destination,
            (unsigned long long)leg.arrival_time_bits, leg.flags, board_status, alight_status);
        log_line(line);
        tfx_line_stop *stops[2] = {&board, &alight};
        tfx_status statuses[2] = {board_status, alight_status};
        for (unsigned j = 0; j < 2; ++j) {
            if (statuses[j] != TFX_OK) continue;
            tfx_line_stop *s = stops[j];
            snprintf(line, sizeof(line),
                "query_stop line=%d index=%d count=%d group=%d station_index=%d station=%d terminal=%d load_mode=%d load=%u max_load_bits=%08x alternatives=%u\n",
                s->line, s->stop_index, s->stop_count, s->station_group, s->station_index, s->station_entity,
                s->terminal, s->load_mode, s->passenger_load_enabled, s->passenger_max_load_bits, s->has_alternative_terminals);
            log_line(line);
        }
        ++samples;
    }
}

tfx_status tfx_register_query_probe(void (*write_line)(const char *)) {
    log_line = write_line;
    tfx_status status = tfx_get_api(TFX_VEHICLE_ABI_VERSION, &api.v2.base);
    if (status != TFX_OK) return status;
    tfx_client client = {.struct_size = sizeof(client), .id = "research.passenger-route-observer",
        .before_terminal_selection = sample};
    return api.v2.base.register_client(&client);
}
