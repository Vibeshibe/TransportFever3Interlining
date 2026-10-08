#ifndef TFX_PROBE_LIMITS_H
#define TFX_PROBE_LIMITS_H
/* A cleared home is only pending evidence, never a resolved lifecycle outcome. */
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
#if TFX_PASSENGER_LIMIT < 1 || TFX_PASSENGER_LIMIT > 5
#error "Passenger limit must be between 1 and 5"
#endif
/* Optional test pacing: keep one attempt available for observed continuation. */
#ifndef TFX_RESERVE_LAST_FOR_LIFECYCLE
#define TFX_RESERVE_LAST_FOR_LIFECYCLE 0
#endif
#if TFX_RESERVE_LAST_FOR_LIFECYCLE != 0 && TFX_RESERVE_LAST_FOR_LIFECYCLE != 1
#error "Lifecycle reserve must be 0 or 1"
#endif
#if TFX_RESERVE_LAST_FOR_LIFECYCLE && TFX_PASSENGER_LIMIT < 2
#error "Lifecycle reserve requires at least two passenger attempts"
#endif
#endif
