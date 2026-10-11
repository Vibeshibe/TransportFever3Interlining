/* Demand-driven bounded discovery; candidate policy uses only the shared SDK. */
#include <stdio.h>
#include <string.h>
#include "candidate_probe.h"
#include "pilot_cache.h"
#include "station_cache.h"
#include "probe_limits.h"
#include "../src/interlining/native_candidate.h"
static tfx_api_v8 platform_api = {.v7.v6.v5.v4.v3.v2.base.struct_size = sizeof(platform_api)};
#define api platform_api.v7.v6.v5
static tfx_api_v7 lifecycle_api = {.v6.v5.v4.v3.v2.base.struct_size = sizeof(lifecycle_api)};
#define changes_api lifecycle_api.v6
/* Compact session ledger survives tracker release and entity reuse. */
typedef struct pilot_record {
    uint64_t ticket;
    int32_t person, destination;
    uint32_t result_logged, applied, arrived_worlds, onboard_worlds;
    uint32_t removed_worlds, invalidated_worlds, lifecycle_resolved, home_cleared_worlds;
    uint32_t sampled, tracking_closed;
} pilot_record;
typedef struct pilot_passenger {
    pilot_record *record;
    int32_t line, vehicle, boarding_stop, alighting_stop, home;
    uint32_t destination_slots;
    uint64_t home_pending_since, staged_sequence;
    tfx_person_journey last_journey[4];
} pilot_passenger;
static tfx_station_cache station_cache;
static pilot_record records[TFX_PASSENGER_LIMIT];
static pilot_passenger passengers[TFX_TRACKER_SLOTS];
static struct {
    uint32_t discovery_callbacks, scan_calls, waiting_reads, line_samples;
    uint32_t vehicle_reads, passenger_screens, stale_waiting, same_line;
    uint32_t assigned_vehicle, station_mismatch, selected_skips, validation_calls;
    uint32_t peak_trackers, indexed_picks, fallback_picks, hint_station_mismatch;
    uint32_t priority_picks, lookahead_reads, pages_drained, ready_logged;
    uint32_t preferred_picks, tracking_callbacks, boarding_wait_callbacks;
} metrics;
/* The consumer permits only one staged/unconfirmed boarding. Keep the route
 * copies until the host's deferred validator runs; no cache authorizes a write. */
static it_candidate pending_candidate;
static uint32_t passenger_count, pilot_done, summary_logged, tracking_blocked;
static void (*log_line)(const char *);
static int32_t scan_cursor, people[32], vehicles[TFX_PILOT_VEHICLES], lines[TFX_PILOT_LINES];
static tfx_waiting_page discovery_page;
static uint32_t discovery_remaining;
/* IDs only: a queued vehicle is re-read before any route check or mutation. */
static int32_t ready_hints[TFX_READY_HINTS];
static uint32_t ready_head, ready_count, evaluation_cursor;
static int32_t last_ready_stop[TFX_PILOT_VEHICLES];
static tfx_station_search station_search[TFX_PILOT_VEHICLES];
static int32_t search_line[TFX_PILOT_VEHICLES], search_stop[TFX_PILOT_VEHICLES];
static uint32_t line_count, line_cursor, line_replacement_cursor, line_stops[TFX_PILOT_LINES];
static uint32_t vehicle_replacement_cursor;
static uint32_t person_count, vehicle_count, vehicle_cursor, person_cursor, person_replacement_cursor;
static uint32_t failures, attempts, accepted, logged, ready, special_waiting_samples;
static uint32_t reasons[IT_REASON_COUNT];
static unsigned char last_reason[32][TFX_PILOT_VEHICLES]; /* Zero is unseen; reason+1 otherwise. */

static void error(const char *operation, tfx_status status) {
    if (failures++ >= 8) return;
    char text[160]; snprintf(text, sizeof(text), "candidate_query_error operation=%s status=%u\n", operation, status);
    log_line(text);
}
static void waiting_error(const char *operation, tfx_status status, int32_t person, uint64_t sequence) {
    if (failures++ >= 8) return;
    char text[192];
    snprintf(text, sizeof(text), "candidate_query_error operation=%s status=%u person=%d sequence=%llu\n",
        operation, status, person, (unsigned long long)sequence);
    log_line(text);
}
static void excluded_waiting(const tfx_waiting_leg *leg, uint64_t sequence) {
    if (leg->vehicle >= -1) return;
    if (special_waiting_samples++ >= 8) return;
    char text[192];
    snprintf(text, sizeof(text), "pilot_waiting_excluded person=%d sequence=%llu vehicle_sentinel=%d reason=unsupported_waiting_state\n",
        leg->person, (unsigned long long)sequence, leg->vehicle);
    log_line(text);
}
static void remember_person(int32_t person) {
    int32_t slot = tfx_pilot_remember(people, &person_count, &person_replacement_cursor, person);
    if (slot < 0) return;
    /* Evict old discovery IDs instead of freezing the first 32 forever. Each
     * eligibility check still re-reads current state; this is no route cache. */
    memset(last_reason[slot], 0, sizeof(last_reason[slot]));
}
static int selected_person(int32_t person);
static void ready_remember(int32_t vehicle) {
    for (uint32_t i = 0; i < ready_count; ++i)
        if (ready_hints[(ready_head + i) % TFX_READY_HINTS] == vehicle) return;
    if (ready_count == TFX_READY_HINTS) { ready_head = (ready_head + 1) % TFX_READY_HINTS; --ready_count; }
    ready_hints[(ready_head + ready_count++) % TFX_READY_HINTS] = vehicle;
}
static int32_t ready_take(void) {
    if (!ready_count) return -1;
    int32_t vehicle = ready_hints[ready_head];
    ready_head = (ready_head + 1) % TFX_READY_HINTS; --ready_count;
    return vehicle;
}
static int loading_ready(const tfx_vehicle *v) {
    uint32_t timer = v->next_passenger_load_seconds_bits;
    return v->state == 2 && v->load_state == 3 && !v->user_stopped &&
        v->line >= 0 && v->stop_index >= 0 && ((timer & 0x80000000u) || timer == 0);
}
static void discover(tfx_context context, uint64_t sequence) {
    ++metrics.discovery_callbacks;
    tfx_status status;
    if (!discovery_remaining) {
        ++metrics.scan_calls;
        discovery_page = (tfx_waiting_page){.struct_size = sizeof(discovery_page)};
        status = api.v4.v3.v2.scan_waiting_people(context, scan_cursor, &discovery_page);
        if (status != TFX_OK) { error("scan", status); return; }
        if (discovery_page.count > TFX_WAITING_PAGE_SIZE) { error("scan_count", TFX_INVALID_STATE); return; }
        scan_cursor = discovery_page.next_entity < 0 ? 0 : discovery_page.next_entity;
        discovery_remaining = discovery_page.count;
        for (uint32_t i = 0; i < discovery_page.count; ++i)
            if (!selected_person(discovery_page.persons[i])) remember_person(discovery_page.persons[i]);
            else ++metrics.selected_skips;
    }
    /* Learn one demand line per callback, including unrelated/unsupported legs.
     * New lines/vehicles replace old IDs; every stop and proposal is read fresh.
     * No full-network scan or save-specific entity IDs. */
    if (discovery_remaining) {
        /* Drain every ID before advancing to the next page. Reverse order
         * retains the existing bootstrap but never skips the rest of a page.
         * IDs may be stale: waiting/stop data is always queried afresh. */
        int32_t person = discovery_page.persons[--discovery_remaining];
        if (!discovery_remaining) ++metrics.pages_drained;
        if (!selected_person(person)) {
            tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
            ++metrics.waiting_reads;
            status = api.v4.v3.v2.read_waiting_leg(context, person, &leg);
            if (status != TFX_OK && status != TFX_NOT_FOUND) { waiting_error("discovery_waiting", status, person, sequence); return; }
            if (status == TFX_NOT_FOUND) {
                ++metrics.stale_waiting; tfx_station_forget(&station_cache, person);
            }
            if (status == TFX_OK) excluded_waiting(&leg, sequence);
            /* One extra copied stop query learns the waiting station. This is
             * only a search hint, never authorization to reuse a stale route. */
            if (status == TFX_OK && leg.vehicle == -1) {
                tfx_line_stop board = {.struct_size = sizeof(board)};
                tfx_status board_status = api.v4.v3.v2.read_line_stop(context, leg.line, leg.boarding_stop, &board);
                if (board_status == TFX_OK)
                    tfx_station_remember_leg(&station_cache, leg.person, board.station_group,
                        leg.line, leg.boarding_stop, leg.alighting_stop);
                else if (board_status == TFX_NOT_FOUND) tfx_station_forget(&station_cache, leg.person);
                else { error("discovery_boarding_stop", board_status); return; }
            } else if (status == TFX_OK) tfx_station_forget(&station_cache, leg.person);
            if (status == TFX_OK && leg.line >= 0) {
                int32_t slot = tfx_pilot_remember_bounded(lines, &line_count, &line_replacement_cursor, TFX_PILOT_LINES, leg.line);
                if (slot >= 0) line_stops[slot] = 0;
            }
        }
    }
    if (!line_count) return;
    uint32_t li = line_cursor++ % line_count;
    if (lines[li] < 0) return;
    ++metrics.line_samples;
    tfx_line_stop stop = {.struct_size = sizeof(stop)};
    status = api.v4.v3.v2.read_line_stop(context, lines[li], 0, &stop);
    if (status == TFX_NOT_FOUND) { lines[li] = -1; return; }
    if (status != TFX_OK) { error("line", status); return; }
    if (stop.stop_count <= 0) { error("line_count", TFX_INVALID_STATE); return; }
    tfx_stop_vehicles list = {.struct_size = sizeof(list)};
    int32_t index = (int32_t)(line_stops[li]++ % (uint32_t)stop.stop_count);
    status = api.v4.v3.read_stop_vehicles(context, lines[li], index, &list);
    if (status == TFX_NOT_FOUND) return;
    if (status != TFX_OK) { error("stop_vehicles", status); return; }
    if (list.count > TFX_STOP_VEHICLES_SIZE) { error("vehicle_count", TFX_INVALID_STATE); return; }
    for (uint32_t i = 0; i < list.count; ++i) {
        int32_t slot = tfx_pilot_remember_bounded(vehicles, &vehicle_count, &vehicle_replacement_cursor, TFX_PILOT_VEHICLES, list.vehicles[i]);
        if (slot >= 0) {
            last_ready_stop[slot] = -1;
            search_line[slot] = search_stop[slot] = -1;
            for (unsigned p = 0; p < 32; ++p) last_reason[p][slot] = 0;
        }
    }
}
static tfx_status revalidate(tfx_context context, const tfx_waiting_leg_change *request) {
    it_candidate fresh;
    tfx_status status = it_validate_candidate(&platform_api, context, request->expected.person, request->candidate_vehicle, &fresh);
    if (status != TFX_OK) return status;
    if (fresh.reason != IT_CANDIDATE) return TFX_POLICY_REJECTED;
    if (pending_candidate.expected.person != request->expected.person ||
        memcmp(&fresh.route, &pending_candidate.route, sizeof(fresh.route)) ||
        memcmp(&fresh.expected, &request->expected, sizeof(fresh.expected)) ||
        fresh.dispatch_sequence != request->dispatch_sequence || fresh.proposed_line != request->line ||
        fresh.boarding_stop != request->boarding_stop || fresh.alighting_stop != request->alighting_stop)
        return TFX_EXPECTED_STATE_CHANGED;
    return TFX_OK;
}

static int selected_person(int32_t person) {
    for (unsigned i = 0; i < passenger_count; ++i)
        if (records[i].person == person) return 1;
    return 0;
}
static int passenger_finished(const pilot_passenger *p) {
    return p->record->arrived_worlds == 3u || p->record->lifecycle_resolved;
}
static int lifecycle_pending(void) {
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i) {
        const pilot_passenger *p = &passengers[i];
        if (p->record && !p->record->lifecycle_resolved && (p->record->removed_worlds || p->record->invalidated_worlds || p->record->home_cleared_worlds)) return 1;
    }
    return 0;
}
static int attempt_reserved(void) {
    if (!TFX_RESERVE_LAST_FOR_LIFECYCLE || passenger_count + 1 != TFX_PASSENGER_LIMIT) return 0;
    for (unsigned i = 0; i < passenger_count; ++i)
        if (records[i].lifecycle_resolved) return 0;
    return 1;
}
static int boarding_pending(void) {
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i)
        if (passengers[i].record && (!passengers[i].record->result_logged || passengers[i].record->onboard_worlds != 3u)) return 1;
    return 0;
}
static unsigned active_trackers(void) {
    unsigned count = 0;
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i) count += passengers[i].record != NULL;
    return count;
}
static unsigned active_samples(void) {
    unsigned count = 0;
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i)
        if (passengers[i].record) count += passengers[i].record->sampled != 0;
    return count;
}
static void report_metrics(uint64_t sequence) {
    unsigned applied = 0, boarded = 0, sampled = 0, arrived = 0, lifecycle = 0, released = 0;
    for (unsigned i = 0; i < passenger_count; ++i) {
        const pilot_record *r = &records[i];
        applied += r->applied; boarded += r->onboard_worlds == 3u;
        sampled += r->sampled; arrived += r->arrived_worlds == 3u;
        lifecycle += r->lifecycle_resolved;
        released += r->tracking_closed && !r->sampled;
    }
    unsigned indexed_people = 0, indexed_stations = 0;
    for (unsigned i = 0; i < station_cache.count; ++i) {
        unsigned count = tfx_station_bucket_size(&station_cache.buckets[i]);
        indexed_people += count; indexed_stations += count != 0;
    }
    char text[1024];
    snprintf(text, sizeof(text), "pilot_totals sequence=%llu staged=%u applied=%u boarded=%u sampled=%u sampled_arrivals=%u sampled_lifecycle=%u unsampled_boardings=%u active_trackers=%u active_samples=%u peak_trackers=%u pending_boarding=%u unresolved_tracked=%u sampled_unresolved=%u\n",
        (unsigned long long)sequence, passenger_count, applied, boarded, sampled, arrived, lifecycle, released,
        active_trackers(), active_samples(), metrics.peak_trackers, (unsigned)boarding_pending(),
        passenger_count - arrived - lifecycle - released, sampled - arrived - lifecycle);
    log_line(text);
    snprintf(text, sizeof(text), "pilot_discovery sequence=%llu callbacks=%u scans=%u waiting_reads=%u line_samples=%u vehicle_reads=%u passenger_screens=%u stale_waiting=%u same_line=%u assigned_vehicle=%u station_mismatch=%u selected_skips=%u validations=%u indexed_picks=%u fallback_picks=%u hint_station_mismatch=%u indexed_people=%u indexed_stations=%u priority_picks=%u lookahead_reads=%u pages_drained=%u page_pending=%u preferred_picks=%u tracking_callbacks=%u boarding_wait_callbacks=%u\n",
        (unsigned long long)sequence, metrics.discovery_callbacks, metrics.scan_calls, metrics.waiting_reads,
        metrics.line_samples, metrics.vehicle_reads, metrics.passenger_screens, metrics.stale_waiting,
        metrics.same_line, metrics.assigned_vehicle, metrics.station_mismatch, metrics.selected_skips, metrics.validation_calls,
        metrics.indexed_picks, metrics.fallback_picks, metrics.hint_station_mismatch, indexed_people, indexed_stations,
        metrics.priority_picks, metrics.lookahead_reads, metrics.pages_drained, discovery_remaining,
        metrics.preferred_picks, metrics.tracking_callbacks, metrics.boarding_wait_callbacks);
    log_line(text);
}
static void summary(const char *reason) {
    if (summary_logged) return;
    summary_logged = 1;
    char text[768];
    snprintf(text, sizeof(text), "pilot_summary reason=%s staged=%u passenger_limit=%u people=%u vehicles=%u ready_samples=%u attempts=%u accepted=%u query_errors=%u switching_blocked=%u special_waiting_samples=%u\n",
        reason, passenger_count, (unsigned)TFX_PASSENGER_LIMIT, person_count, vehicle_count, ready, attempts, accepted, failures, (tracking_blocked || lifecycle_pending()), special_waiting_samples);
    log_line(text);
    for (unsigned i = 0; i < passenger_count; ++i) {
        const pilot_record *p = &records[i];
        snprintf(text, sizeof(text), "pilot_passenger_summary person=%d destination=%d ticket=%llu result_logged=%u onboard_world_mask=%u destination_world_mask=%u removed_world_mask=%u invalidated_world_mask=%u outcome=%s home_cleared_world_mask=%u sampled=%u tracking_closed=%u\n",
            p->person, p->destination, (unsigned long long)p->ticket, p->result_logged, p->onboard_worlds, p->arrived_worlds,
            p->removed_worlds, p->invalidated_worlds, p->arrived_worlds == 3u ? "original_destination_reached" :
            p->lifecycle_resolved && p->removed_worlds == 3u ? "passenger_removed" :
            p->lifecycle_resolved && p->invalidated_worlds == 3u ? "destination_invalidated" :
            p->lifecycle_resolved && p->home_cleared_worlds == 3u ? "home_invalidated" : p->tracking_closed && p->onboard_worlds == 3u ? "boarding_confirmed" : "unresolved",
            p->home_cleared_worlds, p->sampled, p->tracking_closed);
        log_line(text);
    }
    for (unsigned i = 0; i < IT_REASON_COUNT; ++i) if (reasons[i]) {
        snprintf(text, sizeof(text), "candidate_reason name=%s count=%u\n", it_reason_name((it_reason)i), reasons[i]);
        log_line(text);
    }
}
static void stop(const char *reason) { pilot_done = 1; summary(reason); }
static void resolve_lifecycle(pilot_passenger *p, uint64_t sequence, const char *outcome) {
    p->record->lifecycle_resolved = 1;
    char text[256];
    snprintf(text, sizeof(text), "pilot_lifecycle_resolved person=%d sequence=%llu outcome=%s arrival=0 attempt_retained=1\n",
        p->record->person, (unsigned long long)sequence, outcome);
    log_line(text);
}
/* One-world evidence pauses discovery. Matching both-world outcomes close only
 * this journey after confirmed boarding; its ID and consumed attempt are retained.
 * Unexplained query failures use a separate permanent block. */
static void lifecycle_event(pilot_passenger *p, unsigned bit, int removed, uint64_t sequence) {
    uint32_t *mask = removed ? &p->record->removed_worlds : &p->record->invalidated_worlds;
    const char *outcome = removed ? "passenger_removed" : "destination_invalidated";
    if (!(*mask & bit)) {
        *mask |= bit;
        char text[256];
        snprintf(text, sizeof(text), "pilot_lifecycle person=%d destination=%d sequence=%llu outcome=%s world_mask=%u confirmed=%u arrival=0\n",
            p->record->person, p->record->destination, (unsigned long long)sequence, outcome, *mask, *mask == 3u);
        log_line(text);
    }
    if (p->record->removed_worlds && p->record->invalidated_worlds) stop("lifecycle_world_disagreement");
    else if (p->record->arrived_worlds) stop("arrival_lifecycle_disagreement");
    else if (*mask == 3u) {
        if (p->record->onboard_worlds != 3u) { stop("lifecycle_before_confirmed_boarding"); return; }
        resolve_lifecycle(p, sequence, outcome);
    }
}
static void track_change(tfx_context context, pilot_passenger *p, uint64_t sequence) {
    char text[384];
    if (p->record->onboard_worlds != 3u && sequence < TFX_OBSERVATION_LIMIT &&
        sequence - p->staged_sequence >= TFX_BOARDING_GRACE_CALLBACKS) {
        snprintf(text, sizeof(text), "pilot_boarding_timeout person=%d sequence=%llu staged_sequence=%llu world_mask=%u arrival=unobserved\n",
            p->record->person, (unsigned long long)sequence, (unsigned long long)p->staged_sequence, p->record->onboard_worlds);
        log_line(text); stop("boarding_confirmation_timeout"); return;
    }
    if (!p->record->result_logged) {
        tfx_change_result result = {.struct_size = sizeof(result)};
        tfx_status status = changes_api.read_change_result(context, p->record->ticket, &result);
        if (status != TFX_OK) { error("change_result", status); return; }
        if (result.status == TFX_PENDING) return;
        snprintf(text, sizeof(text), "pilot_result ticket=%llu status=%u engine_attempted=%u person=%d\n",
            (unsigned long long)p->record->ticket, result.status, result.engine_attempted, result.person);
        log_line(text); p->record->result_logged = 1;
        if (result.status != TFX_OK || result.engine_attempted != 1 || result.person != p->record->person) {
            stop("change_rejected_or_failed"); return;
        }
        p->record->applied = 1;
    }
    if (passenger_finished(p)) return;
    if (p->record->home_cleared_worlds && sequence - p->home_pending_since >= TFX_HOME_CLEAR_GRACE_CALLBACKS) {
        snprintf(text, sizeof(text), "pilot_home_timeout person=%d sequence=%llu first_sequence=%llu world_mask=%u removed_world_mask=%u outcome=unresolved\n",
            p->record->person, (unsigned long long)sequence, (unsigned long long)p->home_pending_since,
            p->record->home_cleared_worlds, p->record->removed_worlds);
        log_line(text); stop("home_clear_timeout"); return;
    }
    tfx_person_lifecycle life = {.struct_size = sizeof(life)};
    tfx_status status = lifecycle_api.read_person_lifecycle(context, p->record->person, &life);
    if (status != TFX_OK) {
        tracking_blocked = 1;
        snprintf(text, sizeof(text), "pilot_lifecycle_error person=%d sequence=%llu status=%u outcome=unresolved\n",
            p->record->person, (unsigned long long)sequence, status);
        log_line(text); error("lifecycle", status); return;
    }
    if (!life.world_id || life.world_id > 2 || life.person != p->record->person ||
        (life.flags != 0 && life.flags != TFX_LIFECYCLE_ENTITY_PRESENT &&
         life.flags != (TFX_LIFECYCLE_ENTITY_PRESENT | TFX_LIFECYCLE_PERSON_PRESENT))) {
        stop("unexpected_lifecycle_identity"); return;
    }
    unsigned index = life.world_id - 1, bit = 1u << index;
    if (p->record->arrived_worlds & bit) return; /* A subsequent trip/retirement is separate. */
    if (!(life.flags & TFX_LIFECYCLE_PERSON_PRESENT)) {
        lifecycle_event(p, bit, 1, sequence); return;
    }
    /* Home loss can persist while onboard. Both-world clearing closes tracking
     * as home_invalidated, without claiming removal, arrival or native cause.
     * One-world evidence still freezes new changes under the original deadline. */
    if (!(p->record->removed_worlds & bit) && life.destinations[0] == -1 &&
        p->record->onboard_worlds == 3u && !p->record->arrived_worlds && !p->record->invalidated_worlds &&
        p->record->destination != p->home && life.target == p->record->destination) {
        unsigned retained_slots = 0;
        for (unsigned d = 0; d < 3; ++d)
            if (life.destinations[d] == p->record->destination) retained_slots |= 1u << d;
        if (retained_slots == p->destination_slots) {
            if (!p->record->home_cleared_worlds) p->home_pending_since = sequence;
            if (!(p->record->home_cleared_worlds & bit)) {
                p->record->home_cleared_worlds |= bit;
                snprintf(text, sizeof(text), "pilot_home_pending person=%d sequence=%llu world_id=%u expected_home=%d observed_home=-1 target=%d world_mask=%u grace_callbacks=%u arrival=0\n",
                    p->record->person, (unsigned long long)sequence, life.world_id, p->home,
                    life.target, p->record->home_cleared_worlds, TFX_HOME_CLEAR_GRACE_CALLBACKS);
                log_line(text);
            }
            if (p->record->home_cleared_worlds == 3u && !p->record->removed_worlds) {
                snprintf(text, sizeof(text), "pilot_lifecycle person=%d destination=%d sequence=%llu outcome=home_invalidated world_mask=3 confirmed=1 arrival=0\n",
                    p->record->person, p->record->destination, (unsigned long long)sequence);
                log_line(text);
                resolve_lifecycle(p, sequence, "home_invalidated");
            }
            return;
        }
    }
    if ((p->record->removed_worlds & bit) || (p->record->home_cleared_worlds & bit) || life.destinations[0] != p->home) {
        snprintf(text, sizeof(text), "pilot_identity_error person=%d sequence=%llu world_id=%u expected_home=%d observed_home=%d destinations=%d,%d,%d target=%d removed_world_mask=%u outcome=unresolved\n",
            p->record->person, (unsigned long long)sequence, life.world_id, p->home, life.destinations[0],
            life.destinations[0], life.destinations[1], life.destinations[2], life.target, p->record->removed_worlds);
        log_line(text);
        stop("passenger_identity_unresolved"); return;
    }
    unsigned still_assigned = 0, cleared = 0;
    for (unsigned i = 0; i < 3; ++i) {
        if (life.destinations[i] == p->record->destination) still_assigned = 1;
        if ((p->destination_slots & (1u << i)) && life.destinations[i] == -1) cleared |= 1u << i;
    }
    if (!still_assigned && cleared == p->destination_slots) {
        if (p->record->home_cleared_worlds) { stop("home_clear_lifecycle_disagreement"); return; }
        lifecycle_event(p, bit, 0, sequence); return;
    }
    if ((p->record->invalidated_worlds & bit) || !still_assigned) {
        stop("destination_assignment_unresolved"); return;
    }
    tfx_person_journey journey = {.struct_size = sizeof(journey)};
    status = changes_api.read_person_journey(context, p->record->person, &journey);
    if (status != TFX_OK) {
        snprintf(text, sizeof(text), "pilot_journey_error person=%d destination=%d sequence=%llu status=%u\n",
            p->record->person, p->record->destination, (unsigned long long)sequence, status);
        tracking_blocked = 1;
        log_line(text); error("journey", status); return;
    }
    /* This research trial expects exactly the two known replica worlds. */
    if (journey.world_id != life.world_id || journey.person != p->record->person || journey.destination != life.target) {
        stop("unexpected_world_or_person"); return;
    }
    if (journey.destination != p->record->destination) { stop("destination_changed_before_arrival"); return; }
    if (!(p->record->onboard_worlds & bit) && journey.flags == TFX_JOURNEY_ONBOARD) {
        if (journey.line != p->line || journey.vehicle != p->vehicle ||
            journey.boarding_stop != p->boarding_stop || journey.alighting_stop != p->alighting_stop) {
            stop("unexpected_first_boarding"); return;
        }
        p->record->onboard_worlds |= bit;
        if (p->record->onboard_worlds == 3u) {
            snprintf(text, sizeof(text), "pilot_boarded_both person=%d vehicle=%d sequence=%llu\n",
                p->record->person, p->vehicle, (unsigned long long)sequence);
            log_line(text);
        }
    }
    if (journey.flags == TFX_JOURNEY_AT_BUILDING && (p->record->onboard_worlds & bit)) {
        if (p->record->removed_worlds || p->record->invalidated_worlds || p->record->home_cleared_worlds) { stop("arrival_lifecycle_disagreement"); return; }
        p->record->arrived_worlds |= bit;
    }
    if ((p->last_journey[index].flags & TFX_JOURNEY_ONBOARD) && !(journey.flags & TFX_JOURNEY_ONBOARD)) {
        tfx_vehicle vehicle = {.struct_size = sizeof(vehicle)};
        tfx_status vehicle_status = api.v4.v3.read_vehicle(context, p->last_journey[index].vehicle, &vehicle);
        snprintf(text, sizeof(text), "pilot_left_vehicle person=%d world_id=%u vehicle=%d expected_alighting=%d vehicle_query_status=%u current_line=%d current_stop=%d\n",
            p->record->person, journey.world_id, p->last_journey[index].vehicle, p->last_journey[index].alighting_stop,
            vehicle_status, vehicle_status == TFX_OK ? vehicle.line : -1, vehicle_status == TFX_OK ? vehicle.stop_index : -1);
        log_line(text);
    }
    if (memcmp(&journey, &p->last_journey[index], sizeof(journey))) {
        snprintf(text, sizeof(text), "pilot_journey sequence=%llu world_id=%u person=%d flags=%u destination=%d line=%d stops=%d,%d vehicle=%d\n",
            (unsigned long long)sequence, journey.world_id, journey.person, journey.flags, journey.destination, journey.line,
            journey.boarding_stop, journey.alighting_stop, journey.vehicle);
        log_line(text); p->last_journey[index] = journey;
    }
}
static void settle_trackers(uint64_t sequence) {
    /* Free completed samples first, independent of physical slot order. */
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i) {
        pilot_passenger *p = &passengers[i];
        if (p->record && p->record->sampled && passenger_finished(p)) {
            p->record->tracking_closed = 1;
            p->record = NULL;
        }
    }
    for (unsigned i = 0; i < TFX_TRACKER_SLOTS; ++i) {
        pilot_passenger *p = &passengers[i];
        if (!p->record || p->record->onboard_worlds != 3u) continue;
        if (!p->record->sampled) {
            char text[256];
            if (active_samples() < TFX_JOURNEY_SAMPLE_LIMIT) {
                p->record->sampled = 1;
                snprintf(text, sizeof(text), "pilot_sample person=%d sequence=%llu selected=1 policy=first_free_slot sample_limit=%u\n",
                    p->record->person, (unsigned long long)sequence, (unsigned)TFX_JOURNEY_SAMPLE_LIMIT);
            } else {
                p->record->tracking_closed = 1;
                snprintf(text, sizeof(text), "pilot_sample person=%d sequence=%llu selected=0 outcome=boarding_confirmed arrival=unobserved\n",
                    p->record->person, (unsigned long long)sequence);
            }
            log_line(text);
        }
        if (p->record->tracking_closed || passenger_finished(p)) {
            p->record->tracking_closed = 1;
            p->record = NULL;
        }
    }
}
int tfx_pilot_complete(void) { return pilot_done != 0; }

static void evaluate(tfx_context context, uint64_t sequence) {
    uint32_t checks = vehicle_count < 4 ? vehicle_count : 4;
    for (uint32_t i = 0; i < checks && failures < 8; ++i) {
        uint32_t vi = vehicle_count;
        /* Favor observed loading windows on three of four first picks. The
         * fourth and remaining vehicle reads retain global exploration. */
        if (i == 0 && (evaluation_cursor++ & 3u) != 3u) {
            int32_t hinted = ready_take();
            for (uint32_t j = 0; j < vehicle_count; ++j)
                if (hinted >= 0 && vehicles[j] == hinted) { vi = j; ++metrics.priority_picks; break; }
        }
        if (vi == vehicle_count) vi = vehicle_cursor++ % vehicle_count;
        if (vehicles[vi] < 0) continue;
        ++metrics.vehicle_reads;
        tfx_vehicle v = {.struct_size = sizeof(v)};
        tfx_status status = api.v4.v3.read_vehicle(context, vehicles[vi], &v);
        if (status == TFX_NOT_FOUND) { vehicles[vi] = -1; continue; }
        if (status != TFX_OK) { error("vehicle", status); return; }
        if (!loading_ready(&v)) { last_ready_stop[vi] = -1; continue; }
        ready_remember(v.vehicle);
        tfx_station_search *search = &station_search[vi];
        if (search_line[vi] != v.line || search_stop[vi] != v.stop_index) {
            *search = (tfx_station_search){.line = -1, .boarding_stop = -1, .alighting_stop = -1};
            search_line[vi] = v.line; search_stop[vi] = v.stop_index;
        }
        ++ready;
        tfx_line_stop arriving = {.struct_size = sizeof(arriving)};
        status = api.v4.v3.v2.read_line_stop(context, v.line, v.stop_index, &arriving);
        if (status == TFX_NOT_FOUND) return; /* Keep at most one route-screening pass. */
        if (status != TFX_OK) { error("arriving_stop", status); return; }
        if (last_ready_stop[vi] != v.stop_index && metrics.ready_logged < 128) {
            last_ready_stop[vi] = v.stop_index; ++metrics.ready_logged;
            unsigned hints = 0;
            for (unsigned k = 0; k < station_cache.count; ++k)
                if (station_cache.buckets[k].station_group == arriving.station_group)
                    hints = tfx_station_bucket_size(&station_cache.buckets[k]);
            char text[256];
            snprintf(text, sizeof(text), "pilot_ready sequence=%llu vehicle=%d line=%d stop=%d station_group=%d station_hints=%u places_left=%u\n",
                (unsigned long long)sequence, v.vehicle, v.line, v.stop_index, arriving.station_group,
                hints, v.passenger_places_left);
            log_line(text);
        }
        uint32_t attempted = 0, checked_count = 0;
        int32_t checked[4];
        int staged = 0;
        /* A prior successful leg is a ranking hint, never a policy shortcut.
         * Every fourth opportunity explores other legs with its own cursor. */
        int prefer = (search->selections++ & 3u) != 3u && search->line >= 0;
        for (uint32_t j = 0; j < 4 && attempted < 1; ++j) {
            /* Prioritize this arriving station, with one global exploration
             * slot if screening reaches the fourth candidate. Never read the
             * same person twice in a callback or trust a cached eligibility. */
            int32_t person = -1;
            if (j < 3 && prefer) {
                person = tfx_station_pick(&station_cache, arriving.station_group, v.line,
                    &search->preferred_cursor, search, checked, checked_count);
                if (person >= 0) ++metrics.preferred_picks;
            }
            if (j < 3 && person < 0)
                person = tfx_station_pick(&station_cache, arriving.station_group, v.line,
                    &search->cursor, NULL, checked, checked_count);
            int focused = person >= 0;
            uint32_t pi = 32;
            if (focused) ++metrics.indexed_picks;
            else {
                for (uint32_t n = 0; n < person_count; ++n) {
                    uint32_t at = person_cursor++ % person_count;
                    unsigned duplicate = 0;
                    for (uint32_t k = 0; k < checked_count; ++k) duplicate |= checked[k] == people[at];
                    if (!duplicate) { person = people[at]; break; }
                }
                if (person < 0) break;
                ++metrics.fallback_picks;
            }
            checked[checked_count++] = person;
            for (uint32_t k = 0; k < person_count; ++k) if (people[k] == person) { pi = k; break; }
            ++metrics.passenger_screens;
            if (selected_person(person)) { ++metrics.selected_skips; tfx_station_forget(&station_cache, person); continue; }
            tfx_waiting_leg leg = {.struct_size = sizeof(leg)};
            ++metrics.waiting_reads;
            status = api.v4.v3.v2.read_waiting_leg(context, person, &leg);
            if (status == TFX_NOT_FOUND) { ++metrics.stale_waiting; tfx_station_forget(&station_cache, person); continue; }
            if (status != TFX_OK) { waiting_error("waiting", status, person, sequence); return; }
            excluded_waiting(&leg, sequence);
            if (leg.line == v.line) { ++metrics.same_line; continue; }
            if (leg.vehicle != -1) { ++metrics.assigned_vehicle; tfx_station_forget(&station_cache, person); continue; }
            tfx_line_stop board = {.struct_size = sizeof(board)};
            status = api.v4.v3.v2.read_line_stop(context, leg.line, leg.boarding_stop, &board);
            if (status == TFX_NOT_FOUND) { tfx_station_forget(&station_cache, person); continue; }
            if (status != TFX_OK) { error("waiting_stop", status); return; }
            tfx_station_remember_leg(&station_cache, person, board.station_group,
                leg.line, leg.boarding_stop, leg.alighting_stop);
            if (board.station_group != arriving.station_group) {
                ++metrics.station_mismatch; metrics.hint_station_mismatch += focused; continue;
            }
            ++attempted; ++metrics.validation_calls;
            it_candidate proposal;
            status = it_validate_candidate(&platform_api, context, leg.person, v.vehicle, &proposal);
            if (status == TFX_NOT_FOUND) continue;
            if (status != TFX_OK) { error("validate", status); return; }
            ++attempts; ++reasons[proposal.reason];
            if (proposal.reason == IT_CANDIDATE) ++accepted;
            if (passenger_count < TFX_PASSENGER_LIMIT && proposal.reason == IT_CANDIDATE) {
                tfx_waiting_leg_change request = {.struct_size = sizeof(request),
                    .dispatch_sequence = sequence, .expected = proposal.expected,
                    .line = proposal.proposed_line, .boarding_stop = proposal.boarding_stop,
                    .alighting_stop = proposal.alighting_stop, .candidate_vehicle = proposal.proposed_vehicle,
                    .revalidate = revalidate};
                pilot_passenger *p = NULL;
                for (unsigned slot = 0; slot < TFX_TRACKER_SLOTS; ++slot)
                    if (!passengers[slot].record) { p = &passengers[slot]; break; }
                if (!p) { stop("tracker_capacity_invariant"); return; }
                memset(p, 0, sizeof(*p));
                p->staged_sequence = sequence;
                tfx_person_lifecycle baseline = {.struct_size = sizeof(baseline)};
                status = lifecycle_api.read_person_lifecycle(context, leg.person, &baseline);
                if (status != TFX_OK) { error("lifecycle_baseline", status); stop("lifecycle_baseline_unavailable"); return; }
                if (baseline.flags != (TFX_LIFECYCLE_ENTITY_PRESENT | TFX_LIFECYCLE_PERSON_PRESENT) ||
                    !baseline.world_id || baseline.world_id > 2 || baseline.person != leg.person ||
                    baseline.target != leg.destination || baseline.destinations[0] < 0) {
                    stop("lifecycle_baseline_invalid"); return;
                }
                p->home = baseline.destinations[0];
                for (unsigned d = 0; d < 3; ++d)
                    if (baseline.destinations[d] == leg.destination) p->destination_slots |= 1u << d;
                if (!p->destination_slots) { stop("lifecycle_baseline_unassigned"); return; }
                p->record = &records[passenger_count];
                pending_candidate = proposal;
                status = changes_api.request_waiting_leg_change(context, &request, &p->record->ticket);
                if (status != TFX_OK) { p->record = NULL; error("request_change", status); stop("request_rejected"); return; }
                p->record->person = leg.person; p->record->destination = leg.destination;
                p->line = request.line; p->vehicle = request.candidate_vehicle;
                p->boarding_stop = request.boarding_stop; p->alighting_stop = request.alighting_stop;
                ++passenger_count; staged = 1;
                search->line = proposal.expected.line;
                search->boarding_stop = proposal.expected.boarding_stop;
                search->alighting_stop = proposal.expected.alighting_stop;
                tfx_station_forget(&station_cache, person);
                unsigned active = active_trackers();
                if (active > metrics.peak_trackers) metrics.peak_trackers = active;
                char message[192]; snprintf(message, sizeof(message), "pilot_staged ticket=%llu person=%d sequence=%llu ordinal=%u passenger_limit=%u\n",
                    (unsigned long long)p->record->ticket, p->record->person, (unsigned long long)sequence, passenger_count, (unsigned)TFX_PASSENGER_LIMIT);
                log_line(message);
            }
            if (!staged && (logged >= 80 || (pi < 32 && last_reason[pi][vi] == (unsigned char)(proposal.reason + 1)))) continue;
            if (pi < 32) last_reason[pi][vi] = (unsigned char)(proposal.reason + 1);
            char text[768];
            snprintf(text, sizeof(text),
                "candidate_result sequence=%llu person=%d old_line=%d old_stops=%d,%d destination=%d arrival_bits=%016llx vehicle=%d proposed_line=%d proposed_stops=%d,%d reason=%s board=%d,%d alight=%d,%d places_left=%u requires_engine_revalidation=%u cross_platform=%u alternative_platform=%u cross_platform_alighting=%u\n",
                (unsigned long long)sequence, proposal.expected.person, proposal.expected.line,
                proposal.expected.boarding_stop, proposal.expected.alighting_stop, proposal.expected.destination,
                (unsigned long long)proposal.expected.arrival_time_bits, v.vehicle, v.line,
                proposal.boarding_stop, proposal.alighting_stop, it_reason_name(proposal.reason),
                proposal.boarding_station, proposal.boarding_terminal, proposal.alighting_station,
                proposal.alighting_terminal, proposal.observed_places_left, proposal.requires_engine_revalidation, proposal.cross_platform, proposal.alternative_platform, proposal.cross_platform_alighting);
            log_line(text); ++logged;
            if (staged) return;
        }
        return; /* At most one ready vehicle / one validation per callback. */
    }
}
/* Only used after a callback performed no full candidate validation. This
 * adds at most 12 queries, never to a staging/revalidation callback. Its result
 * is merely a priority hint for a later fresh evaluation. */
static void lookahead(tfx_context context) {
    uint32_t count = vehicle_count < 12 ? vehicle_count : 12;
    for (uint32_t i = 0; i < count; ++i) {
        uint32_t vi = vehicle_cursor++ % vehicle_count;
        if (vehicles[vi] < 0) continue;
        tfx_vehicle v = {.struct_size = sizeof(v)};
        ++metrics.vehicle_reads; ++metrics.lookahead_reads;
        tfx_status status = api.v4.v3.read_vehicle(context, vehicles[vi], &v);
        if (status == TFX_NOT_FOUND) { vehicles[vi] = -1; continue; }
        if (status != TFX_OK) { error("lookahead_vehicle", status); return; }
        if (loading_ready(&v)) ready_remember(v.vehicle);
        else last_ready_stop[vi] = -1;
    }
}
static void sample(tfx_context context, void *unused) {
    (void)unused;
    if (pilot_done) return;
    tfx_observation observation = {.struct_size = sizeof(observation)};
    if (api.v4.v3.v2.base.read_observation(context, &observation) != TFX_OK) {
        stop("observation_unavailable"); return;
    }
    uint64_t sequence = observation.dispatch_sequence;
    /* Separate tracking and discovery budgets. Paired callback phases cover both
     * alternating worlds; a pending boarding is tracked on EVERY callback. */
    int pending_before = lifecycle_pending(), awaiting_boarding = boarding_pending();
    if (passenger_count && (tracking_blocked || pending_before || attempt_reserved() || awaiting_boarding || passenger_count == TFX_PASSENGER_LIMIT ||
            (active_trackers() && (sequence / 2) % 2 == 0) || sequence == TFX_OBSERVATION_LIMIT)) {
        ++metrics.tracking_callbacks;
        metrics.boarding_wait_callbacks += awaiting_boarding != 0;
        for (unsigned i = 0; i < TFX_TRACKER_SLOTS && !pilot_done; ++i)
            if (passengers[i].record) track_change(context, &passengers[i], sequence);
        if (!pilot_done) settle_trackers(sequence);
        unsigned arrived = 0, finished = 0;
        for (unsigned i = 0; i < passenger_count; ++i) {
            arrived += records[i].arrived_worlds == 3u;
            finished += records[i].tracking_closed;
        }
        if (!pilot_done && pending_before && !lifecycle_pending() && !tracking_blocked) {
            char text[160];
            snprintf(text, sizeof(text), "pilot_tracking_resumed sequence=%llu remaining_attempts=%u\n",
                (unsigned long long)sequence, (unsigned)TFX_PASSENGER_LIMIT - passenger_count);
            log_line(text);
        }
        if (!pilot_done && finished == TFX_PASSENGER_LIMIT && !tracking_blocked)
            stop(!TFX_FULL_JOURNEY_TRACKING ? "sampled_outcomes_observed" :
                arrived == TFX_PASSENGER_LIMIT ? "all_destinations_observed" : "all_outcomes_observed");
    } else if (!tracking_blocked && !lifecycle_pending() && !attempt_reserved() && failures < 8 && sequence < TFX_OBSERVATION_LIMIT) {
        discover(context, sequence);
        unsigned before = metrics.validation_calls, before_failures = failures;
        if (failures < 8) evaluate(context, sequence);
        if (failures == before_failures && failures < 8 && metrics.validation_calls == before)
            lookahead(context);
    }
    if (!pilot_done && failures >= 8) stop("query_errors");
    if (!pilot_done && sequence == TFX_OBSERVATION_LIMIT) stop("callback_limit");
    if (sequence % 600 == 0 || pilot_done) report_metrics(sequence);
}
tfx_status tfx_register_candidate_probe(void (*write_line)(const char *)) {
    if (!write_line) return TFX_INVALID_ARGUMENT;
    log_line = write_line;
    passenger_count = pilot_done = summary_logged = tracking_blocked = 0;
    scan_cursor = 0;
    line_count = line_cursor = line_replacement_cursor = vehicle_replacement_cursor = 0;
    memset(line_stops, 0, sizeof(line_stops));
    memset(&discovery_page, 0, sizeof(discovery_page)); discovery_remaining = 0;
    ready_head = ready_count = evaluation_cursor = 0;
    memset(station_search, 0, sizeof(station_search));
    for (unsigned i = 0; i < TFX_PILOT_VEHICLES; ++i)
        last_ready_stop[i] = search_line[i] = search_stop[i] = -1;
    person_count = vehicle_count = vehicle_cursor = person_cursor = person_replacement_cursor = 0;
    failures = attempts = accepted = logged = ready = special_waiting_samples = 0;
    memset(&pending_candidate, 0, sizeof(pending_candidate));
    memset(&station_cache, 0, sizeof(station_cache));
    memset(records, 0, sizeof(records)); memset(&metrics, 0, sizeof(metrics));
    memset(passengers, 0, sizeof(passengers)); memset(reasons, 0, sizeof(reasons));
    memset(last_reason, 0, sizeof(last_reason));
    char text[768];
    snprintf(text, sizeof(text), "pilot_limits passenger_limit=%u callback_limit=%u max_unconfirmed_boardings=1 lifecycle_policy=continue_after_confirmation reserve_last_for_lifecycle=%u home_clear_grace_callbacks=%u route_max_hops=%u discovery=drained_pages station_buckets=64 station_people=128 vehicle_slots=128 line_slots=64 vehicle_lookahead=12 selection=per_vehicle_leg_hints journey_sample_limit=%u boarding_grace_callbacks=%u tracking=%s platform_policy=native_same_station\n",
        (unsigned)TFX_PASSENGER_LIMIT, (unsigned)TFX_OBSERVATION_LIMIT, (unsigned)TFX_RESERVE_LAST_FOR_LIFECYCLE, TFX_HOME_CLEAR_GRACE_CALLBACKS, (unsigned)IT_MAX_LEG_HOPS, (unsigned)TFX_JOURNEY_SAMPLE_LIMIT, TFX_BOARDING_GRACE_CALLBACKS,
        TFX_FULL_JOURNEY_TRACKING ? "full" : "sampled");
    log_line(text);
    tfx_status status = tfx_get_api(TFX_STOP_TERMINALS_ABI_VERSION, &api.v4.v3.v2.base);
    if (status != TFX_OK) return status;
    status = tfx_get_api(TFX_LIFECYCLE_ABI_VERSION, &changes_api.v5.v4.v3.v2.base);
    if (status != TFX_OK) return status;
    tfx_client client = {.struct_size = sizeof(client), .id = "research.interlining-bounded",
        .before_terminal_selection = sample};
    return api.v4.v3.v2.base.register_client(&client);
}
