/* Delayed boarding tests use the real consumer/policy, mocked copied queries. */
#define main serial_pilot_main
#include "pilot_tests.c"
#undef main
int main(void) {
    reset(); ticks(12);
    printf("Delayed first boarding: %u changes staged in 12 callbacks before any boarding confirmation.\n", staged);
#ifdef TFX_PIPELINE_BASELINE
    CHECK(staged == 1); return 0;
#else
    CHECK(TFX_CONTINUOUS && TFX_PENDING_BOARDING_LIMIT == 4 && staged == 4);
    ticks(40); CHECK(staged == 4 && !tfx_pilot_complete());
    boarded[0] = 1; ticks(12); CHECK(staged == 4); /* One replica does not free a seat/slot. */
    boarded[0] = 3; ticks(12); CHECK(staged == 5); /* Confirmation recycles a pending slot. */
    CHECK(strstr(output, "pilot_boarded_both") && !strstr(output, "candidate_query_error"));

    reset(); capacity_override = 1; reported_places = 2; ticks(80);
    if (staged != 2 || tfx_pilot_complete()) fprintf(stderr, "capacity staged=%u\n%s", staged, output);
    CHECK(staged == 2 && !tfx_pilot_complete()); /* Native free capacity excludes our unassigned commitments. */
    boarded[0] = 1; ticks(12); CHECK(staged == 2);
    reported_places = 1; ticks(12); CHECK(staged == 2); /* Falling capacity cannot admit another. */
    boarded[0] = boarded[1] = 3; reported_places = 2; ticks(12);
    CHECK(staged == 4); /* Holds release only on both-world confirmation. */

    reset(); tick(); CHECK(staged == 1); fail_result = 1; tick();
    CHECK(staged == 1 && tfx_pilot_complete() && strstr(output, "change_rejected_or_failed"));
    reset(); tick(); pending_result = 1; ticks(16); CHECK(staged == 1);
    pending_result = 0; ticks(12); CHECK(staged == 4);

    reset(); ticks(12); CHECK(staged == 4); boarded[0] = boarded[1] = boarded[2] = 3;
    no_capacity = 1; ticks(8); CHECK(!tfx_pilot_complete());
    uint64_t deadline = requests[3].dispatch_sequence + TFX_BOARDING_GRACE_CALLBACKS;
    sequence = deadline - 2; tick(); CHECK(!tfx_pilot_complete());
    tick(); CHECK(tfx_pilot_complete() && strstr(output, "boarding_confirmation_timeout"));

    reset(); tick(); wrong_vehicle = 1; boarded[0] = 3; ticks(4);
    CHECK(tfx_pilot_complete() && strstr(output, "unexpected_first_boarding"));
    reset(); tick(); wrong_destination = 1; ticks(4);
    CHECK(tfx_pilot_complete());

    reset(); shrink_on_revalidation = 1; tick(); CHECK(staged == 1);
    ticks(8); CHECK(!tfx_pilot_complete() && strstr(output, "pilot_canceled"));
    CHECK(staged == 1); reported_places = 20; ticks(16); CHECK(staged > 1);

    reset(); leg_hops = 4; screening_misses = busy_vehicles = 1; ticks(12);
    CHECK(staged == 4); for (unsigned i = 0; i < staged; ++i) boarded[i] = 3;
    ticks(4); CHECK(!tfx_pilot_complete());
    ticks(16); CHECK(staged == 8); /* Four full samples plus four pending boardings. */
    for (unsigned i = 0; i < 4; ++i) CHECK(lifecycle_calls[requests[i].expected.person - 100] > 1);
    CHECK(!strstr(output, "candidate_query_error"));
    printf("PASS: four pending boardings, replica confirmation, conservative seats, shrinking capacity, immediate result failures, exact oldest/unselected deadlines, wrong boarding/destination, canceled retry, four-hop budget and samples; peak=%u/64 queries.\n", max_queries);
    return 0;
#endif
}
