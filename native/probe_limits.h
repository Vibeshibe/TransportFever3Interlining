#ifndef TFX_PROBE_LIMITS_H
#define TFX_PROBE_LIMITS_H
/* Build-time capture duration; independent of the host's one-attempt allowance. */
#ifndef TFX_OBSERVATION_LIMIT
#define TFX_OBSERVATION_LIMIT 3600
#endif
#if TFX_OBSERVATION_LIMIT < 1 || TFX_OBSERVATION_LIMIT > 36000
#error "Observation limit must be between 1 and 36000 callbacks"
#endif
#endif
