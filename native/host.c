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
static tfx_reader *current_reader;

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

static tfx_status query_context(tfx_context context) {
    if (!context || context != current_token) return TFX_STALE_CONTEXT;
    if (GetCurrentThreadId() != dispatch_thread) return TFX_WRONG_THREAD;
    return current_reader ? TFX_OK : TFX_QUERY_UNAVAILABLE;
}

static tfx_status read_waiting_leg(tfx_context context, int32_t person, tfx_waiting_leg *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_waiting(current_reader, person, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_line_stop(tfx_context context, int32_t line, int32_t stop, tfx_line_stop *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_stop(current_reader, line, stop, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status scan_waiting_people(tfx_context context, int32_t first_entity, tfx_waiting_page *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_scan(current_reader, first_entity, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_stop_vehicles(tfx_context context, int32_t line, int32_t stop, tfx_stop_vehicles *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_stop_vehicles(current_reader, line, stop, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_vehicle(tfx_context context, int32_t vehicle, tfx_vehicle *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_vehicle(current_reader, vehicle, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_person_platforms(tfx_context context, int32_t person, tfx_person_platforms *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_platforms(current_reader, person, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

tfx_status tfx_get_api(uint32_t version, tfx_api *out) {
    if (version < TFX_ABI_VERSION || version > TFX_PLATFORM_ABI_VERSION) return TFX_ABI_MISMATCH;
    size_t size = version == TFX_ABI_VERSION ? sizeof(*out) :
        version == TFX_QUERY_ABI_VERSION ? sizeof(tfx_api_v2) :
        version == TFX_VEHICLE_ABI_VERSION ? sizeof(tfx_api_v3) : sizeof(tfx_api_v4);
    if (!out || out->struct_size != size) return TFX_INVALID_ARGUMENT;
    *out = (tfx_api){sizeof(*out), TFX_ABI_VERSION, TFX_CAP_TERMINAL_OBSERVATION,
        register_client, unregister_client, read_observation};
    if (version >= TFX_QUERY_ABI_VERSION) {
        out->struct_size = (uint32_t)size;
        out->abi_version = version;
        out->capabilities |= TFX_CAP_PASSENGER_ROUTE_QUERIES;
        tfx_api_v2 *v2 = (tfx_api_v2 *)out;
        v2->read_waiting_leg = read_waiting_leg;
        v2->read_line_stop = read_line_stop;
        v2->scan_waiting_people = scan_waiting_people;
    }
    if (version >= TFX_VEHICLE_ABI_VERSION) {
        out->capabilities |= TFX_CAP_VEHICLE_QUERIES;
        tfx_api_v3 *v3 = (tfx_api_v3 *)out;
        v3->read_stop_vehicles = read_stop_vehicles;
        v3->read_vehicle = read_vehicle;
    }
    if (version == TFX_PLATFORM_ABI_VERSION) {
        out->capabilities |= TFX_CAP_PLATFORM_QUERIES;
        ((tfx_api_v4 *)out)->read_person_platforms = read_person_platforms;
    }
    return TFX_OK;
}

tfx_status tfx_dispatch(uint32_t delta_seconds_bits) {
    return tfx_dispatch_with_reader(delta_seconds_bits, NULL);
}

tfx_status tfx_dispatch_with_reader(uint32_t delta_seconds_bits, tfx_reader *reader) {
    tfx_client selected[TFX_MAX_CLIENTS];
    AcquireSRWLockExclusive(&lock);
    if (dispatching) { ReleaseSRWLockExclusive(&lock); return TFX_BUSY; }
    /* Never recycle a token, even in the theoretical counter-overflow case. */
    if (sequence == UINT64_MAX || token_counter > UINT64_MAX - TFX_MAX_CLIENTS) {
        ReleaseSRWLockExclusive(&lock); return TFX_LIMIT_REACHED;
    }
    dispatching = 1;
    dispatch_thread = GetCurrentThreadId();
    current_reader = reader;
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
    current_reader = NULL;
    dispatching = 0;
    ReleaseSRWLockExclusive(&lock);
    return TFX_OK;
}
