#ifndef TFX_PROBE_LIMITS_H
#define TFX_PROBE_LIMITS_H
/* Bound unconfirmed home loss; both-world home invalidation is a separate outcome. */
#define TFX_HOME_CLEAR_GRACE_CALLBACKS 32u
/* Research bounds are explicit build inputs, never runtime API overrides. */
#ifndef TFX_OBSERVATION_LIMIT
#define TFX_OBSERVATION_LIMIT 3600
#endif
#if TFX_OBSERVATION_LIMIT < 1 || TFX_OBSERVATION_LIMIT > 36000
#error "Observation limit must be between 1 and 36000 callbacks"
#endif
#ifndef TFX_PASSENGER_LIMIT
#define TFX_PASSENGER_LIMIT 1
#endif
#if TFX_PASSENGER_LIMIT < 1 || TFX_PASSENGER_LIMIT > 32
#error "Passenger limit must be between 1 and 32"
#endif
/* Full tracking remains the default for small regression trials. Larger runs
 * retain at most four diagnostic journeys plus one unconfirmed boarding. */
#ifndef TFX_JOURNEY_SAMPLE_LIMIT
#if TFX_PASSENGER_LIMIT <= 5
#define TFX_JOURNEY_SAMPLE_LIMIT TFX_PASSENGER_LIMIT
#else
#define TFX_JOURNEY_SAMPLE_LIMIT 4
#endif
#endif
#if TFX_JOURNEY_SAMPLE_LIMIT < 1 || TFX_JOURNEY_SAMPLE_LIMIT > 5 || TFX_JOURNEY_SAMPLE_LIMIT > TFX_PASSENGER_LIMIT
#error "Journey sample limit must be 1..5 and no greater than the attempt cap"
#endif
#if TFX_PASSENGER_LIMIT > 5 && TFX_JOURNEY_SAMPLE_LIMIT > 4
#error "Larger trials permit at most four concurrent journey samples"
#endif
#define TFX_TRACKER_SLOTS (TFX_JOURNEY_SAMPLE_LIMIT + 1)
#define TFX_FULL_JOURNEY_TRACKING (TFX_JOURNEY_SAMPLE_LIMIT == TFX_PASSENGER_LIMIT)
/* Dispatch callbacks, not game seconds. Missing either replica at this deadline
 * stops new changes; it does not refund the attempt or attempt a rollback. */
#define TFX_BOARDING_GRACE_CALLBACKS 600u
/* Optional test pacing: keep one attempt available for observed continuation. */
#ifndef TFX_RESERVE_LAST_FOR_LIFECYCLE
#define TFX_RESERVE_LAST_FOR_LIFECYCLE 0
#endif
#if TFX_RESERVE_LAST_FOR_LIFECYCLE != 0 && TFX_RESERVE_LAST_FOR_LIFECYCLE != 1
#error "Lifecycle reserve must be 0 or 1"
#endif
#if TFX_RESERVE_LAST_FOR_LIFECYCLE && (TFX_PASSENGER_LIMIT < 2 || !TFX_FULL_JOURNEY_TRACKING)
#error "Lifecycle reserve requires full tracking of at least two passenger attempts"
#endif
#endif
