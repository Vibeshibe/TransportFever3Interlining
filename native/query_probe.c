/* Bounded example consumer. Uses only the public API; never an engine pointer. */
#include <stdio.h>
#include <string.h>
#include "query_probe.h"

static tfx_api_v2 api = {.base.struct_size = sizeof(tfx_api_v2)};
static void (*log_line)(const char *);
static int32_t cursor;
static uint32_t samples, failures;

static void sample(tfx_context context, void *unused) {
    (void)unused;
    if (samples >= 24 || failures >= 8) return;
    tfx_waiting_page page = {.struct_size = sizeof(page)};
    tfx_status status = api.scan_waiting_people(context, cursor, &page);
    char line[512];
    if (status != TFX_OK) {
        ++failures;
        snprintf(line, sizeof(line), "query_scan status=%u cursor=%d\n", status, cursor);
        log_line(line);
        return;
    }
    cursor = page.next_entity < 0 ? 0 : page.next_entity;
    for (uint32_t i = 0; i < page.count && samples < 24; ++i) {
        tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
        status = api.read_waiting_leg(context, page.persons[i], &leg);
        if (status != TFX_OK) {
            if (failures++ < 8) {
                snprintf(line, sizeof(line), "query_waiting person=%d status=%u\n", page.persons[i], status);
                log_line(line);
            }
            continue;
        }
        tfx_line_stop board = {.struct_size = sizeof(board)}, alight = {.struct_size = sizeof(alight)};
        tfx_status board_status = api.read_line_stop(context, leg.line, leg.boarding_stop, &board);
        tfx_status alight_status = api.read_line_stop(context, leg.line, leg.alighting_stop, &alight);
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
    tfx_status status = tfx_get_api(TFX_QUERY_ABI_VERSION, &api.base);
    if (status != TFX_OK) return status;
    tfx_client client = {.struct_size = sizeof(client), .id = "research.passenger-route-observer",
        .before_terminal_selection = sample};
    return api.base.register_client(&client);
}
