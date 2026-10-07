#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "host.h"
#include "slot.h"

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); exit(1); } } while (0)
static tfx_api api = {.struct_size = sizeof(tfx_api)};
static tfx_context expired;
static int callbacks, original_calls, observer_calls;
static void *expected_system, *expected_world;
void *tfx_original;
extern void tfx_terminal_entry(void);

static DWORD WINAPI wrong_thread(void *unused) {
    (void)unused;
    tfx_observation observation = {.struct_size = sizeof(observation)};
    CHECK(api.read_observation(expired, &observation) == TFX_WRONG_THREAD);
    CHECK(tfx_dispatch(0) == TFX_BUSY);
    return 0;
}

static void first_client(tfx_context context, void *user_data) {
    CHECK(user_data == &callbacks);
    CHECK(callbacks++ % 2 == 0);
    tfx_observation observation = {.struct_size = sizeof(observation)};
    CHECK(api.read_observation(context, &observation) == TFX_OK);
    CHECK(observation.delta_seconds_bits == 0x3e800000); /* 0.25f */
    CHECK(observation.dispatch_sequence != 0);
    CHECK(api.unregister_client("a.first") == TFX_BUSY);
    CHECK(tfx_dispatch(0) == TFX_BUSY);
    expired = context;
    HANDLE thread = CreateThread(NULL, 0, wrong_thread, NULL, 0, NULL);
    CHECK(thread != NULL);
    CHECK(WaitForSingleObject(thread, 5000) == WAIT_OBJECT_0);
    CloseHandle(thread);
}

static void second_client(tfx_context context, void *unused) {
    (void)unused;
    CHECK(callbacks++ % 2 == 1);
    tfx_observation observation = {.struct_size = sizeof(observation)};
    CHECK(api.read_observation(expired, &observation) == TFX_STALE_CONTEXT);
    CHECK(api.read_observation(context, &observation) == TFX_OK);
    tfx_client client = { .struct_size = sizeof(client), .id = "during.dispatch",
                         .before_terminal_selection = second_client };
    CHECK(api.register_client(&client) == TFX_BUSY);
}

/* The assembly uses this test observer, never a special mode in the real DLL. */
void tfx_observe(void *system, void *world, void *data, float delta) {
    CHECK(system == expected_system && world == expected_world && data == (void *)0x1357);
    CHECK(delta == 0.25f);
    ++observer_calls;
    uint32_t bits;
    memcpy(&bits, &delta, sizeof(bits));
    CHECK(tfx_dispatch(bits) == TFX_OK);
}

static uint64_t original(void *system, void *world, void *data, float delta, uint64_t stack_arg) {
    CHECK(system == expected_system && world == expected_world && data == (void *)0x1357);
    CHECK(delta == 0.25f && stack_arg == UINT64_C(0xabcdef0123456789));
    ++original_calls;
    return UINT64_C(0xfedcba9876543210);
}

int main(void) {
    CHECK(tfx_get_api(999, &api) == TFX_ABI_MISMATCH);
    tfx_api bad = {0};
    CHECK(tfx_get_api(1, &bad) == TFX_INVALID_ARGUMENT);
    CHECK(tfx_get_api(1, &api) == TFX_OK);
    CHECK(api.capabilities == TFX_CAP_TERMINAL_OBSERVATION);
    tfx_client first = {.struct_size = sizeof(first), .id = "a.first",
                       .before_terminal_selection = first_client, .user_data = &callbacks};
    tfx_client second = {.struct_size = sizeof(second), .id = "z.second",
                        .before_terminal_selection = second_client};
    CHECK(api.register_client(&second) == TFX_OK);
    CHECK(api.register_client(&first) == TFX_OK);
    CHECK(api.register_client(&first) == TFX_DUPLICATE_CLIENT);
    tfx_client invalid = first;
    memset(invalid.id, 'a', sizeof(invalid.id));
    CHECK(api.register_client(&invalid) == TFX_INVALID_ARGUMENT);
    invalid = first; invalid.reserved = 1;
    CHECK(api.register_client(&invalid) == TFX_INVALID_ARGUMENT);

    void **slot = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    CHECK(slot != NULL);
    *slot = tfx_original = (void *)&original;
    expected_system = &first;
    expected_world = &second;
    DWORD old;
    CHECK(VirtualProtect(slot, 4096, PAGE_READONLY, &old));
    CHECK(tfx_replace_slot(slot, (void *)&main, &tfx_terminal_entry) == TFX_HOOK_CONFLICT);
    CHECK(*slot == (void *)&original);
    CHECK(tfx_replace_slot(slot, (void *)&original, &tfx_terminal_entry) == TFX_OK);
    CHECK(tfx_replace_slot(slot, (void *)&original, &tfx_terminal_entry) == TFX_HOOK_CONFLICT);
    MEMORY_BASIC_INFORMATION memory;
    CHECK(VirtualQuery(slot, &memory, sizeof(memory)) == sizeof(memory));
    CHECK(memory.Protect == PAGE_READONLY);
    typedef uint64_t (*update_fn)(void *, void *, void *, float, uint64_t);
    for (int i = 0; i < 3; ++i)
        CHECK(((update_fn)*slot)(expected_system, expected_world, (void *)0x1357, 0.25f,
              UINT64_C(0xabcdef0123456789)) == UINT64_C(0xfedcba9876543210));
    CHECK(callbacks == 6 && observer_calls == 3 && original_calls == 3);
    tfx_observation observation = {.struct_size = sizeof(observation)};
    CHECK(api.read_observation(expired, &observation) == TFX_STALE_CONTEXT);
    CHECK(api.unregister_client("a.first") == TFX_OK);
    CHECK(api.unregister_client("z.second") == TFX_OK);
    CHECK(api.unregister_client("z.second") == TFX_NOT_FOUND);
    CHECK(tfx_dispatch(0) == TFX_OK);
    CHECK(tfx_replace_slot(slot, &tfx_terminal_entry, &original) == TFX_OK);
    CHECK(VirtualFree(slot, 0, MEM_RELEASE));
    for (unsigned i = 0; i < TFX_MAX_CLIENTS; ++i) {
        snprintf(second.id, sizeof(second.id), "capacity.%02u", i);
        CHECK(api.register_client(&second) == TFX_OK);
    }
    strcpy(second.id, "overflow");
    CHECK(api.register_client(&second) == TFX_LIMIT_REACHED);
    puts("PASS: registry order/lifetime/thread/limits; guarded slot ownership/protection; x64 arguments, stack and original return/call count.");
    return 0;
}
