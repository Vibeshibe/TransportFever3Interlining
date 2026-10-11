#define main bounded_host_main
#include "host_batch_tests.c"
#undef main
static void confirm(unsigned world_id) { journey_mode = 1; dispatch(world_id, 0); journey_mode = 0; }
int main(void) {
    CHECK(TFX_CONTINUOUS);
    CHECK(tfx_get_api(6, &api.v5.v4.v3.v2.base) == TFX_OK);
    tfx_client c = {.struct_size = sizeof(c), .id = "tests.continuous", .before_terminal_selection = client};
    CHECK(api.v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    count = 1; people[0] = 1; arrival_bits = 10; expected_request = TFX_OK;
    expected_results[0] = TFX_OK; expected_attempts[0] = 1;
    dispatch(1, 0); dispatch(2, 1); CHECK(writes == 1);
    expected_request = TFX_LIMIT_REACHED; arrival_bits = 20;
    dispatch(2, 0); CHECK(writes == 1); /* No new episode before both-world boarding. */
    confirm(1); dispatch(2, 0); CHECK(writes == 1);
    journey_wrong = 1; confirm(2); dispatch(2, 0); CHECK(writes == 1);
    journey_wrong = 0; confirm(2);
    arrival_bits = 10; dispatch(2, 0); CHECK(writes == 1); /* Same wait still blocked. */
    arrival_bits = 20; expected_request = TFX_OK; dispatch(1, 0); dispatch(2, 1); CHECK(writes == 2);
    confirm(1); confirm(2);
    expected_request = TFX_LIMIT_REACHED; arrival_bits = 10; dispatch(1, 0); CHECK(writes == 2); /* Old replica cannot replay. */
    CHECK(api.v5.v4.v3.v2.base.unregister_client(c.id) == TFX_OK);
    strcpy(c.id, "tests.other"); CHECK(api.v5.v4.v3.v2.base.register_client(&c) == TFX_OK);
    arrival_bits = 20; dispatch(1, 0); CHECK(writes == 2); /* Guard is shared across clients. */
    for (unsigned episode = 0; episode < 4096; ++episode) {
        people[0] = (int32_t)(episode % 1024) + 1000;
        arrival_bits = 100 + episode / 1024;
        expected_request = TFX_OK; dispatch(1, 0); dispatch(2, 1); confirm(1); confirm(2);
        expected_request = TFX_LIMIT_REACHED; dispatch(2, 0);
    }
    CHECK(writes == 4098);
    /* Recoverable stale-policy rejection does not consume an episode. */
    people[0] = 5000; expected_request = TFX_OK; reject_policy = 1;
    expected_results[0] = TFX_POLICY_REJECTED; expected_attempts[0] = 0;
    dispatch(1, 0); dispatch(2, 1); CHECK(writes == 4098);
    reject_policy = 0; fail_write = 1; expected_results[0] = TFX_INVALID_STATE; expected_attempts[0] = 1;
    dispatch(1, 0); dispatch(2, 1); CHECK(writes == 4099);
    people[0] = 5001; expected_request = TFX_LIMIT_REACHED; dispatch(1, 0); CHECK(writes == 4099);
    puts("PASS: 4096 continuous episodes beyond lifetime caps; later journeys, cross-world/client duplicate guards, wrong boarding, stale replay, recoverable rejection and uncertain-write latch.");
    return 0;
}
