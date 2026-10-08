#ifndef INTERLINING_NATIVE_CANDIDATE_H
#define INTERLINING_NATIVE_CANDIDATE_H
#include "../../native/include/tpf_bridge.h"

/* At most four forward stop intervals; bounds both search and callback cost. */
#define IT_MAX_LEG_HOPS 4

typedef enum it_reason {
    IT_CANDIDATE, IT_ALREADY_ASSIGNED, IT_SAME_LINE, IT_UNSUPPORTED_LEG,
    IT_VEHICLE_NOT_LOADING, IT_ALTERNATIVE_TERMINALS, IT_BOARDING_MISMATCH,
    IT_ALIGHTING_MISMATCH, IT_ACTUAL_TERMINAL_MISMATCH, IT_LOADING_DISABLED,
    IT_UNLOADING_DISABLED, IT_UNSUPPORTED_LOAD_LIMIT, IT_NO_CAPACITY,
    IT_NO_PLATFORM_MEMBERSHIP, IT_DESTINATION_MISSING, IT_NOT_IN_LOADING_INDEX,
    IT_LOAD_TIMER_PENDING, IT_ROUTE_DIVERGENCE, IT_REASON_COUNT
} it_reason;

typedef struct it_route_snapshot {
    uint32_t original_count, replacement_count;
    tfx_line_stop original[IT_MAX_LEG_HOPS + 1];
    tfx_line_stop replacement[IT_MAX_LEG_HOPS + 1];
} it_route_snapshot;

typedef struct it_candidate {
    it_reason reason;
    uint32_t requires_engine_revalidation; /* Always 1; no write authorization. */
    uint64_t dispatch_sequence;
    tfx_waiting_leg expected;
    int32_t proposed_vehicle, proposed_line, boarding_stop, alighting_stop;
    int32_t boarding_station, boarding_terminal, alighting_station, alighting_terminal;
    uint32_t observed_places_left;
    it_route_snapshot route; /* Copied leg data for exact deferred revalidation. */
} it_candidate;

/* Interlining policy consumer, NOT part of the generic bridge ABI. All reads
 * occur in one live callback. Query failure leaves output untouched; an ordinary
 * policy rejection returns TFX_OK with an explicit reason. */
tfx_status it_validate_candidate(const tfx_api_v5 *, tfx_context, int32_t person,
                                 int32_t vehicle, it_candidate *out);
const char *it_reason_name(it_reason reason);
#endif
