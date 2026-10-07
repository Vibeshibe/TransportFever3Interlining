#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string.h>
#include "host.h"

static SRWLOCK lock = SRWLOCK_INIT;
static tfx_client clients[TFX_MAX_CLIENTS];
static uint32_t client_count;
static uint64_t sequence, token_counter;
static int dispatching;
static DWORD dispatch_thread;
static tfx_context current_token;
static tfx_observation current_observation;

static int valid_id(const char *id) {
    if (!id) return 0;
    for (uint32_t i = 0; i < TFX_CLIENT_ID_SIZE; ++i) {
        unsigned char c = (unsigned char)id[i];
        if (!c) return i != 0;
        if (!(c >= 'a' && c <= 'z') && !(c >= '0' && c <= '9') &&
            c != '.' && c != '_' && c != '-') return 0;
    }
    return 0;
}

static tfx_status register_client(const tfx_client *client) {
    if (!client || client->struct_size != sizeof(*client) || client->reserved ||
        !client->before_terminal_selection || !valid_id(client->id))
        return TFX_INVALID_ARGUMENT;
    tfx_status result = TFX_OK;
    AcquireSRWLockExclusive(&lock);
    if (dispatching) { result = TFX_BUSY; goto done; }
    uint32_t at = 0;
    while (at < client_count && strcmp(clients[at].id, client->id) < 0) ++at;
    if (at < client_count && !strcmp(clients[at].id, client->id)) {
        result = TFX_DUPLICATE_CLIENT; goto done;
    }
    if (client_count == TFX_MAX_CLIENTS) { result = TFX_LIMIT_REACHED; goto done; }
    memmove(&clients[at + 1], &clients[at], (client_count - at) * sizeof(*client));
    clients[at] = *client;
    ++client_count;
done:
    ReleaseSRWLockExclusive(&lock);
    return result;
}

static tfx_status unregister_client(const char *id) {
    if (!valid_id(id)) return TFX_INVALID_ARGUMENT;
    tfx_status result = TFX_NOT_FOUND;
    AcquireSRWLockExclusive(&lock);
    if (dispatching) { result = TFX_BUSY; goto done; }
    for (uint32_t i = 0; i < client_count; ++i) {
        if (strcmp(clients[i].id, id)) continue;
        --client_count;
        memmove(&clients[i], &clients[i + 1], (client_count - i) * sizeof(*clients));
        result = TFX_OK;
        break;
    }
done:
    ReleaseSRWLockExclusive(&lock);
    return result;
}

static tfx_status read_observation(tfx_context context, tfx_observation *out) {
    if (!out || out->struct_size != sizeof(*out)) return TFX_INVALID_ARGUMENT;
    tfx_status result;
    AcquireSRWLockShared(&lock);
    if (!context || context != current_token) result = TFX_STALE_CONTEXT;
    else if (GetCurrentThreadId() != dispatch_thread) result = TFX_WRONG_THREAD;
    else { *out = current_observation; result = TFX_OK; }
    ReleaseSRWLockShared(&lock);
    return result;
}

tfx_status tfx_get_api(uint32_t version, tfx_api *out) {
    if (version != TFX_ABI_VERSION) return TFX_ABI_MISMATCH;
    if (!out || out->struct_size != sizeof(*out)) return TFX_INVALID_ARGUMENT;
    *out = (tfx_api){sizeof(*out), TFX_ABI_VERSION, TFX_CAP_TERMINAL_OBSERVATION,
        register_client, unregister_client, read_observation};
    return TFX_OK;
}

tfx_status tfx_dispatch(uint32_t delta_seconds_bits) {
    tfx_client selected[TFX_MAX_CLIENTS];
    AcquireSRWLockExclusive(&lock);
    if (dispatching) { ReleaseSRWLockExclusive(&lock); return TFX_BUSY; }
    /* Never recycle a token, even in the theoretical counter-overflow case. */
    if (sequence == UINT64_MAX || token_counter > UINT64_MAX - TFX_MAX_CLIENTS) {
        ReleaseSRWLockExclusive(&lock); return TFX_LIMIT_REACHED;
    }
    dispatching = 1;
    dispatch_thread = GetCurrentThreadId();
    current_observation = (tfx_observation){sizeof(tfx_observation), delta_seconds_bits, ++sequence};
    uint32_t count = client_count;
    memcpy(selected, clients, count * sizeof(*clients));
    ReleaseSRWLockExclusive(&lock);
    for (uint32_t i = 0; i < count; ++i) {
        AcquireSRWLockExclusive(&lock);
        tfx_context token = current_token = ++token_counter;
        ReleaseSRWLockExclusive(&lock);
        selected[i].before_terminal_selection(token, selected[i].user_data);
        AcquireSRWLockExclusive(&lock);
        current_token = 0;
        ReleaseSRWLockExclusive(&lock);
    }
    AcquireSRWLockExclusive(&lock);
    dispatch_thread = 0;
    dispatching = 0;
    ReleaseSRWLockExclusive(&lock);
    return TFX_OK;
}
