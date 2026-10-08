#ifndef TFX_PROBE_LIMITS_H
#define TFX_PROBE_LIMITS_H
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
#endif
