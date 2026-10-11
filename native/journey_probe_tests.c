#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "journey_probe.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL %d: %s\n", __LINE__, #x); exit(1); } } while (0)
static tfx_client client;
static tfx_person_journey next;
static tfx_status query_status;
static uint64_t sequence;
static char output[16384];
static void write_line(const char *text) {
    CHECK(strlen(output) + strlen(text) < sizeof(output)); strcat(output, text);
}
static tfx_status register_client(const tfx_client *value) { client = *value; return TFX_OK; }
static tfx_status read_observation(tfx_context context, tfx_observation *out) {
    CHECK(context == 42); out->dispatch_sequence = ++sequence; return TFX_OK;
}
static tfx_status read_journey(tfx_context context, int32_t person, tfx_person_journey *out) {
    CHECK(context == 42 && person == 123);
    if (query_status != TFX_OK) return query_status;
    *out = next; return TFX_OK;
}
tfx_status tfx_get_api(uint32_t version, tfx_api *base) {
    CHECK(version == 6 && base->struct_size == sizeof(tfx_api_v6));
    tfx_api_v6 *api = (tfx_api_v6 *)base;
    /* Any accidental write API use would dereference NULL and fail the test. */
    api->request_waiting_leg_change = NULL; api->read_change_result = NULL;
    base->register_client = register_client; base->read_observation = read_observation;
    api->read_person_journey = read_journey; return TFX_OK;
}
static void reset(void) {
    sequence = 0; query_status = TFX_OK; output[0] = 0;
    next = (tfx_person_journey){.struct_size = sizeof(next), .world_id = 1, .person = 123,
        .destination = 456, .line = 7, .boarding_stop = 1, .alighting_stop = 2, .vehicle = 8};
    CHECK(tfx_register_journey_probe(write_line, 123, 456) == TFX_OK);
    CHECK(!tfx_journey_probe_complete());
}
static void sample(unsigned world, unsigned flags, int destination) {
    next.world_id = world; next.flags = flags; next.destination = destination;
    client.before_terminal_selection(42, NULL);
}
int main(int argc, char **argv) {
    if (argc == 2 || argc == 3) {
        int mutable = argc == 3;
        if (mutable) CHECK(!strcmp(argv[2], "--mutable"));
        HMODULE module = LoadLibraryA(argv[1]); CHECK(module);
        tfx_status (*get_api)(uint32_t, tfx_api *) = (void *)GetProcAddress(module, "tfx_get_api");
        DWORD (WINAPI *start)(void *) = (void *)GetProcAddress(module, "tfx_start");
        CHECK(get_api && start);
        tfx_api_v6 observer = {.v5.v4.v3.v2.base.struct_size = sizeof(observer)};
        CHECK(get_api(6, &observer.v5.v4.v3.v2.base) == TFX_OK);
        CHECK(observer.v5.v4.v3.v2.base.capabilities == (mutable ? 127u : 95u) && observer.read_person_journey);
        tfx_api_v7 life = {.v6.v5.v4.v3.v2.base.struct_size = sizeof(life)};
        CHECK(get_api(7, &life.v6.v5.v4.v3.v2.base) == TFX_OK);
        CHECK(life.v6.v5.v4.v3.v2.base.capabilities == (mutable ? 255u : 223u) && life.read_person_lifecycle);
        tfx_person_lifecycle snapshot = {.struct_size = sizeof(snapshot)};
        CHECK(life.read_person_lifecycle(0, 123, &snapshot) == TFX_STALE_CONTEXT);
        tfx_api_v8 platforms = {.v7.v6.v5.v4.v3.v2.base.struct_size = sizeof(platforms)};
        CHECK(get_api(8, &platforms.v7.v6.v5.v4.v3.v2.base) == TFX_OK);
        CHECK(platforms.v7.v6.v5.v4.v3.v2.base.capabilities == (mutable ? 511u : 479u) && platforms.read_stop_terminals);
        tfx_stop_terminals options = {.struct_size = sizeof(options)};
        CHECK(platforms.read_stop_terminals(0, 1, 0, &options) == TFX_STALE_CONTEXT);
        tfx_api_v9 network = {.v8.v7.v6.v5.v4.v3.v2.base.struct_size = sizeof(network)};
        CHECK(get_api(9, &network.v8.v7.v6.v5.v4.v3.v2.base) == TFX_OK);
        CHECK(network.v8.v7.v6.v5.v4.v3.v2.base.capabilities == (mutable ? 1023u : 991u) && network.scan_lines);
        tfx_line_page page = {.struct_size = sizeof(page)};
        CHECK(network.scan_lines(0, 0, &page) == TFX_STALE_CONTEXT);
        CHECK(start(NULL) == TFX_UNSUPPORTED_BUILD);
        FreeLibrary(module);
        puts("PASS: DLL preserves ABI6/7, provides ABI8 copied terminals and refuses the unsupported fixture executable.");
    } else CHECK(argc == 1);
    CHECK(tfx_register_journey_probe(NULL, 123, 456) == TFX_INVALID_ARGUMENT);
    CHECK(tfx_register_journey_probe(write_line, -1, 456) == TFX_INVALID_ARGUMENT);
    reset(); sample(1, TFX_JOURNEY_ONBOARD, 456);
    sample(2, TFX_JOURNEY_ONBOARD, 456);
    sample(1, TFX_JOURNEY_AT_BUILDING, 999);
    sample(2, TFX_JOURNEY_MOVING, 456); CHECK(!tfx_journey_probe_complete());
    sample(1, TFX_JOURNEY_AT_BUILDING, 456); CHECK(!tfx_journey_probe_complete());
    sample(2, TFX_JOURNEY_AT_BUILDING, 456); CHECK(tfx_journey_probe_complete());
    CHECK(strstr(output, "destination_world_mask=3 query_errors=0 passenger_changes=0"));
    reset(); sample(1, TFX_JOURNEY_AT_BUILDING, 456);
    sample(1, TFX_JOURNEY_AT_BUILDING, 456); CHECK(!tfx_journey_probe_complete());
    sample(2, TFX_JOURNEY_AT_BUILDING | TFX_JOURNEY_MOVING, 456); CHECK(!tfx_journey_probe_complete());
    sample(2, TFX_JOURNEY_AT_BUILDING, 456); CHECK(tfx_journey_probe_complete());
    reset(); query_status = TFX_NOT_FOUND;
    for (unsigned i = 0; i < 7; ++i) { sample(1, 0, 456); CHECK(!tfx_journey_probe_complete()); }
    sample(1, 0, 456); CHECK(tfx_journey_probe_complete());
    CHECK(strstr(output, "reload_stopped reason=query_errors") && !strstr(output, "destination_world_mask=3"));
    reset(); sequence = 3599; sample(1, TFX_JOURNEY_ONBOARD, 456);
    CHECK(tfx_journey_probe_complete() && strstr(output, "destination_world_mask=0"));
    reset(); sample(0, 0, 456); sample(5, 0, 456);
    CHECK(strstr(output, "errors=2") && !strstr(output, "reload_journey"));
    puts("PASS: read-only reload observer distinguishes progress/arrival, requires both worlds and expected destination, and stops on errors or cap.");
    return 0;
}
