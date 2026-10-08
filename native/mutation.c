#include <stdio.h>
#include <string.h>
#include "mutation.h"

static int boundary(tfx_reader *r) {
    uint64_t window;
    unsigned char flags[2];
    return r->world <= UINT64_MAX - 0x1fa &&
        r->copy(r->user, r->world + 0x1f0, &window, sizeof(window)) && !window &&
        r->copy(r->user, r->world + 0x1f8, flags, sizeof(flags)) && flags[0] == 1 && flags[1] <= 1;
}

tfx_status tfx_apply_waiting_change(tfx_reader *r, const tfx_waiting_leg_change *request,
    const unsigned char expected_raw[32], uint32_t *attempted, void *operations) {
    tfx_engine_changes *ops = operations;
    if (!r || !r->copy || !request || !expected_raw || !attempted || !ops ||
        !ops->begin || !ops->remove || !ops->add_waiting || !ops->end || !ops->log)
        return TFX_INVALID_ARGUMENT;
    *attempted = 0;
    if (!boundary(r)) return TFX_INVALID_STATE;
    tfx_waiting_leg live = {.struct_size = sizeof(live)};
    tfx_status status = tfx_query_waiting(r, request->expected.person, &live);
    if (status != TFX_OK) return status;
    if (memcmp(&live, &request->expected, sizeof(live)) || live.vehicle != -1)
        return TFX_EXPECTED_STATE_CHANGED;
    tfx_person_journey journey = {.struct_size = sizeof(journey)};
    status = tfx_query_journey(r, live.person, &journey);
    if (status != TFX_OK) return status;
    if (journey.flags != TFX_JOURNEY_WAITING) return TFX_INVALID_STATE;
    unsigned char raw[32], replacement[32];
    status = tfx_query_waiting_raw(r, live.person, raw);
    if (status != TFX_OK) return status;
    if (memcmp(raw, expected_raw, sizeof(raw))) return TFX_EXPECTED_STATE_CHANGED;
    memcpy(replacement, raw, sizeof(raw));
    memcpy(replacement, &request->line, 4);
    memcpy(replacement + 4, &request->boarding_stop, 4);
    memcpy(replacement + 8, &request->alighting_stop, 4);
    /* The query budget must cover postconditions too, before beginning changes. */
    if (r->queries_left < 2 || r->reads_left < 128 || r->bytes_left < 8192)
        return TFX_QUERY_BUDGET_EXHAUSTED;
    void *world = (void *)(uintptr_t)r->world;
    char text[256];
    snprintf(text, sizeof(text), "change_begin world=%p person=%d old_line=%d new_line=%d stops=%d,%d vehicle=%d\n",
        world, live.person, live.line, request->line, request->boarding_stop, request->alighting_stop, request->candidate_vehicle);
    ops->log(text);
    /* No normal rejection after first remove. Engine asserts/exceptions are not
     * recoverable transactions. The complete value is stack-owned, never borrowed. */
    *attempted = 1;
    ops->begin(world);
    ops->remove(world, live.person, (const void *)(uintptr_t)(r->image + 0x3cea8f0), 1);
    ops->add_waiting(world, live.person, replacement, 1, 1);
    ops->end(world);
    if (!boundary(r)) { ops->log("change_postcondition_failed boundary\n"); return TFX_INVALID_STATE; }
    tfx_waiting_leg expected = request->expected;
    expected.line = request->line; expected.boarding_stop = request->boarding_stop;
    expected.alighting_stop = request->alighting_stop;
    status = tfx_query_waiting(r, live.person, &live);
    if (status != TFX_OK || memcmp(&live, &expected, sizeof(live))) {
        ops->log("change_postcondition_failed waiting_destination_or_place\n"); return TFX_INVALID_STATE;
    }
    status = tfx_query_waiting_raw(r, live.person, raw);
    if (status != TFX_OK || memcmp(raw, replacement, sizeof(raw))) {
        ops->log("change_postcondition_failed raw_waiting\n"); return TFX_INVALID_STATE;
    }
    ops->log("change_applied arrival_destination_place_preserved=1 engine_scope_closed=1\n");
    return TFX_OK;
}
