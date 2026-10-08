/* Observe a known passenger after restart/reload. No discovery or change requests. */
#include <stdio.h>
#include <string.h>
#include "journey_probe.h"
static tfx_api_v6 api = {.v5.v4.v3.v2.base.struct_size = sizeof(api)};
static void (*log_line)(const char *);
static int32_t target_person, target_destination;
static uint32_t failures, destination_worlds, observed_worlds, complete;
static tfx_person_journey previous[4];

static void sample(tfx_context context, void *unused) {
    (void)unused;
    if (complete) return;
    tfx_observation observation = {.struct_size = sizeof(observation)};
    tfx_person_journey journey = {.struct_size = sizeof(journey)};
    tfx_status status = api.v5.v4.v3.v2.base.read_observation(context, &observation);
    if (status == TFX_OK) status = api.read_person_journey(context, target_person, &journey);
    if (status == TFX_OK && (!journey.world_id || journey.world_id > 4 || journey.person != target_person))
        status = TFX_INVALID_STATE;
    char text[384];
    if (status != TFX_OK) {
        ++failures;
        snprintf(text, sizeof(text), "reload_query_error person=%d status=%u errors=%u\n",
            target_person, status, failures);
        log_line(text);
        if (failures >= 8) { complete = 1; log_line("reload_stopped reason=query_errors\n"); }
        return;
    }
    unsigned index = journey.world_id - 1;
    observed_worlds |= 1u << index;
    if (journey.flags == TFX_JOURNEY_AT_BUILDING && journey.destination == target_destination)
        destination_worlds |= 1u << index;
    if (memcmp(&journey, &previous[index], sizeof(journey))) {
        snprintf(text, sizeof(text), "reload_journey sequence=%llu world_id=%u person=%d flags=%u destination=%d line=%d stops=%d,%d vehicle=%d expected_destination=%d\n",
            (unsigned long long)observation.dispatch_sequence, journey.world_id, journey.person,
            journey.flags, journey.destination, journey.line, journey.boarding_stop,
            journey.alighting_stop, journey.vehicle, target_destination);
        log_line(text); previous[index] = journey;
    }
    if ((destination_worlds & 3u) == 3u || observation.dispatch_sequence == 3600) {
        snprintf(text, sizeof(text), "reload_summary observed_world_mask=%u destination_world_mask=%u query_errors=%u passenger_changes=0\n",
            observed_worlds, destination_worlds, failures);
        log_line(text); complete = 1;
    }
}
int tfx_journey_probe_complete(void) { return complete != 0; }
tfx_status tfx_register_journey_probe(void (*write_line)(const char *), int32_t person, int32_t destination) {
    if (!write_line || person < 0 || destination < 0) return TFX_INVALID_ARGUMENT;
    tfx_status status = tfx_get_api(TFX_CHANGE_ABI_VERSION, &api.v5.v4.v3.v2.base);
    if (status != TFX_OK) return status;
    log_line = write_line; target_person = person; target_destination = destination;
    failures = destination_worlds = observed_worlds = complete = 0;
    memset(previous, 0, sizeof(previous));
    tfx_client client = {.struct_size = sizeof(client), .id = "research.journey-after-reload",
        .before_terminal_selection = sample};
    status = api.v5.v4.v3.v2.base.register_client(&client);
    if (status == TFX_OK) {
        char text[160];
        snprintf(text, sizeof(text), "reload_target person=%d destination=%d passenger_changes=0\n", person, destination);
        log_line(text);
    }
    return status;
}
