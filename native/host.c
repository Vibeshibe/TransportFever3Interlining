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

#define CHANGE_HISTORY 64u
static struct {
    tfx_waiting_leg_change request;
    unsigned char raw[32];
    char owner[TFX_CLIENT_ID_SIZE];
    tfx_change_result result;
} changes[CHANGE_HISTORY];
static uint64_t ticket_counter, pending[TFX_MAX_CLIENTS];
static uint32_t pending_count, engine_attempts;
static int applying;
static char current_client[TFX_CLIENT_ID_SIZE];
static tfx_apply_change current_writer;
static void *writer_user;

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

static tfx_status read_terminal(tfx_context context, int32_t station, int32_t terminal, tfx_terminal *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_terminal(current_reader, station, terminal, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_entity_presence(tfx_context context, int32_t entity, tfx_entity_presence *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_presence(current_reader, entity, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status read_person_journey(tfx_context context, int32_t person, tfx_person_journey *out) {
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) status = tfx_query_journey(current_reader, person, out);
    ReleaseSRWLockShared(&lock);
    return status;
}

static tfx_status request_waiting_leg_change(tfx_context context, const tfx_waiting_leg_change *request, uint64_t *ticket) {
    if (!request || !ticket || request->struct_size != sizeof(*request) || request->reserved ||
        request->expected.struct_size != sizeof(request->expected) || request->expected.reserved ||
        request->expected.person < 0 || request->expected.line < 0 || request->expected.vehicle != -1 ||
        request->line < 0 || request->line == request->expected.line || request->boarding_stop < 0 ||
        request->alighting_stop < 0 || request->candidate_vehicle < 0 || !request->revalidate)
        return TFX_INVALID_ARGUMENT;
    AcquireSRWLockExclusive(&lock);
    tfx_status status = query_context(context);
    if (status != TFX_OK) goto done;
    if (applying) { status = TFX_BUSY; goto done; }
    if (!current_writer) { status = TFX_QUERY_UNAVAILABLE; goto done; }
    if (engine_attempts || pending_count == TFX_MAX_CLIENTS || ticket_counter == UINT64_MAX) {
        status = TFX_LIMIT_REACHED; goto done;
    }
    if (request->dispatch_sequence != sequence) { status = TFX_EXPECTED_STATE_CHANGED; goto done; }
    tfx_waiting_leg live = {.struct_size = sizeof(live)};
    status = tfx_query_waiting(current_reader, request->expected.person, &live);
    if (status != TFX_OK) goto done;
    if (memcmp(&live, &request->expected, sizeof(live))) { status = TFX_EXPECTED_STATE_CHANGED; goto done; }
    unsigned char raw[32];
    status = tfx_query_waiting_raw(current_reader, request->expected.person, raw);
    if (status != TFX_OK) goto done;
    uint64_t id = ++ticket_counter;
    unsigned slot = (unsigned)(id % CHANGE_HISTORY);
    changes[slot].request = *request;
    memcpy(changes[slot].raw, raw, sizeof(raw));
    memcpy(changes[slot].owner, current_client, sizeof(current_client));
    changes[slot].result = (tfx_change_result){sizeof(tfx_change_result), TFX_PENDING,
        id, sequence, request->expected.person, 0};
    for (unsigned i = 0; i < pending_count; ++i) {
        unsigned other = (unsigned)(pending[i] % CHANGE_HISTORY);
        if (changes[other].result.person == request->expected.person) {
            changes[other].result.status = TFX_REQUEST_CONFLICT;
            changes[slot].result.status = TFX_REQUEST_CONFLICT;
        }
    }
    pending[pending_count++] = id;
    *ticket = id;
done:
    ReleaseSRWLockExclusive(&lock);
    return status;
}

static tfx_status read_change_result(tfx_context context, uint64_t ticket, tfx_change_result *out) {
    if (!ticket || !out || out->struct_size != sizeof(*out)) return TFX_INVALID_ARGUMENT;
    AcquireSRWLockShared(&lock);
    tfx_status status = query_context(context);
    if (status == TFX_OK) {
        unsigned slot = (unsigned)(ticket % CHANGE_HISTORY);
        if (changes[slot].result.ticket != ticket || strcmp(changes[slot].owner, current_client)) status = TFX_NOT_FOUND;
        else *out = changes[slot].result;
    }
    ReleaseSRWLockShared(&lock);
    return status;
}

tfx_status tfx_get_api(uint32_t version, tfx_api *out) {
    if (version < TFX_ABI_VERSION || version > TFX_CHANGE_ABI_VERSION) return TFX_ABI_MISMATCH;
    size_t size = version == TFX_ABI_VERSION ? sizeof(*out) :
        version == TFX_QUERY_ABI_VERSION ? sizeof(tfx_api_v2) :
        version == TFX_VEHICLE_ABI_VERSION ? sizeof(tfx_api_v3) :
        version == TFX_PLATFORM_ABI_VERSION ? sizeof(tfx_api_v4) :
        version == TFX_TERMINAL_ABI_VERSION ? sizeof(tfx_api_v5) : sizeof(tfx_api_v6);
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
    if (version >= TFX_PLATFORM_ABI_VERSION) {
        out->capabilities |= TFX_CAP_PLATFORM_QUERIES;
        ((tfx_api_v4 *)out)->read_person_platforms = read_person_platforms;
    }
    if (version >= TFX_TERMINAL_ABI_VERSION) {
        out->capabilities |= TFX_CAP_TERMINAL_QUERIES;
        ((tfx_api_v5 *)out)->read_terminal = read_terminal;
        ((tfx_api_v5 *)out)->read_entity_presence = read_entity_presence;
    }
    if (version == TFX_CHANGE_ABI_VERSION) {
        out->capabilities |= TFX_CAP_PERSON_JOURNEY;
#ifndef TFX_READ_ONLY_BACKEND
        out->capabilities |= TFX_CAP_WAITING_LEG_CHANGE;
#endif
        tfx_api_v6 *v6 = (tfx_api_v6 *)out;
        v6->request_waiting_leg_change = request_waiting_leg_change;
        v6->read_change_result = read_change_result;
        v6->read_person_journey = read_person_journey;
    }
    return TFX_OK;
}

tfx_status tfx_dispatch(uint32_t delta_seconds_bits) {
    return tfx_dispatch_with_reader(delta_seconds_bits, NULL);
}

tfx_status tfx_dispatch_with_reader(uint32_t delta_seconds_bits, tfx_reader *reader) {
    return tfx_dispatch_with_writer(delta_seconds_bits, reader, NULL, NULL);
}

tfx_status tfx_dispatch_with_writer(uint32_t delta_seconds_bits, tfx_reader *reader,
                                    tfx_apply_change writer, void *user) {
    tfx_client selected[TFX_MAX_CLIENTS];
    AcquireSRWLockExclusive(&lock);
    if (dispatching) { ReleaseSRWLockExclusive(&lock); return TFX_BUSY; }
    /* Never recycle a token, even in the theoretical counter-overflow case. */
    if (sequence == UINT64_MAX || token_counter > UINT64_MAX - 2u * TFX_MAX_CLIENTS) {
        ReleaseSRWLockExclusive(&lock); return TFX_LIMIT_REACHED;
    }
    dispatching = 1;
    dispatch_thread = GetCurrentThreadId();
    current_reader = reader;
    current_writer = writer; writer_user = user; pending_count = 0; applying = 0;
    current_observation = (tfx_observation){sizeof(tfx_observation), delta_seconds_bits, ++sequence};
    uint32_t count = client_count;
    memcpy(selected, clients, count * sizeof(*clients));
    ReleaseSRWLockExclusive(&lock);
    for (uint32_t i = 0; i < count; ++i) {
        AcquireSRWLockExclusive(&lock);
        tfx_context token = current_token = ++token_counter;
        memcpy(current_client, selected[i].id, sizeof(current_client));
        ReleaseSRWLockExclusive(&lock);
        selected[i].before_terminal_selection(token, selected[i].user_data);
        AcquireSRWLockExclusive(&lock);
        current_token = 0;
        ReleaseSRWLockExclusive(&lock);
    }
    /* No engine operation runs while a consumer is gathering its snapshot. */
    applying = 1;
    for (unsigned i = 0; i < pending_count; ++i) {
        unsigned slot = (unsigned)(pending[i] % CHANGE_HISTORY);
        if (changes[slot].result.status != TFX_PENDING) continue;
        tfx_status status = engine_attempts ? TFX_LIMIT_REACHED : TFX_OK;
        if (status == TFX_OK) {
            AcquireSRWLockExclusive(&lock);
            current_token = ++token_counter;
            memcpy(current_client, changes[slot].owner, sizeof(current_client));
            tfx_context token = current_token;
            ReleaseSRWLockExclusive(&lock);
            status = changes[slot].request.revalidate(token, &changes[slot].request);
            AcquireSRWLockExclusive(&lock);
            current_token = 0;
            ReleaseSRWLockExclusive(&lock);
        }
        uint32_t attempted = 0;
        if (status == TFX_OK)
            status = current_writer(reader, &changes[slot].request, changes[slot].raw, &attempted, writer_user);
        /* Burn the process-wide allowance even if a postcondition failed. */
        if (attempted) ++engine_attempts;
        changes[slot].result.status = status;
        changes[slot].result.engine_attempted = attempted;
    }
    AcquireSRWLockExclusive(&lock);
    dispatch_thread = 0;
    current_reader = NULL;
    current_writer = NULL; writer_user = NULL; applying = 0; current_client[0] = 0;
    dispatching = 0;
    ReleaseSRWLockExclusive(&lock);
    return TFX_OK;
}
