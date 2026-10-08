#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "host.h"
#include "slot.h"
#ifdef TFX_OBSERVE_PERSON
#include "journey_probe.h"
#else
#include "candidate_probe.h"
#include "mutation.h"
#include "mutation_entries.h"
#endif

/* These bindings are private and pinned to a single research executable. */
#define TERMINAL_UPDATE_RVA 0xb0db00u
#define TERMINAL_VTABLE_RVA 0x3702758u
#define TERMINAL_SLOT_RVA   0x37027b0u
#define OBSERVATION_LIMIT 3600
static const unsigned char expected_hash[32] = {
    0x5e,0x72,0x0c,0xd7,0xfe,0xfd,0x68,0x8a,0x16,0x75,0xb6,0x1c,0x6e,0xec,0xaa,0xdf,
    0xd2,0x5c,0x17,0x1d,0xa9,0xe9,0xe9,0x38,0xaf,0x85,0xe5,0xef,0xae,0xbd,0x8a,0x35};
static const unsigned char entry_bytes[16] = {
    0x48,0x8b,0xc4,0x53,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56,0x41,0x57,0x48,0x81};
/* Publish before installing the slot; never clear while the DLL is loaded. */
void *tfx_original;
extern void tfx_terminal_entry(void);
static unsigned char *image;
static volatile LONG start_state, enabled, simulation_thread, observed, rejected;
#ifndef TFX_OBSERVE_PERSON
static tfx_engine_changes change_operations;
#endif
static uint64_t world_addresses[4];
static uint32_t world_count;
static HANDLE ownership, log_file = INVALID_HANDLE_VALUE;

static int copy_memory(void *unused, uint64_t address, void *out, size_t size) {
    (void)unused;
    SIZE_T got = 0;
    return ReadProcessMemory(GetCurrentProcess(), (const void *)(uintptr_t)address, out, size, &got) && got == size;
}

static void log_line(const char *message) {
    DWORD written;
    if (log_file != INVALID_HANDLE_VALUE)
        WriteFile(log_file, message, (DWORD)strlen(message), &written, NULL);
    if (log_file != INVALID_HANDLE_VALUE && !strncmp(message, "change_", 7)) FlushFileBuffers(log_file);
}

static int open_log(HMODULE self) {
    wchar_t path[32768];
    DWORD length = GetModuleFileNameW(self, path, 32768);
    if (!length || length >= 32600) return 0;
    wchar_t *slash = wcsrchr(path, L'\\');
    if (!slash) return 0;
    swprintf(slash + 1, 160, L"tpf-bridge-%lu-%llu.log", GetCurrentProcessId(), GetTickCount64());
    log_file = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW,
                          FILE_ATTRIBUTE_NORMAL, NULL);
    return log_file != INVALID_HANDLE_VALUE;
}

static int supported_file(void) {
    wchar_t path[32768];
    DWORD n = GetModuleFileNameW(NULL, path, 32768);
    if (!n || n >= 32768) return 0;
    HANDLE file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (file == INVALID_HANDLE_VALUE) return 0;
    BCRYPT_ALG_HANDLE algorithm = NULL;
    BCRYPT_HASH_HANDLE hash = NULL;
    unsigned char digest[32], buffer[65536];
    int ok = 0;
    if (BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, NULL, 0) < 0 ||
        BCryptCreateHash(algorithm, &hash, NULL, 0, NULL, 0, 0) < 0) goto done;
    for (;;) {
        DWORD got;
        if (!ReadFile(file, buffer, sizeof(buffer), &got, NULL)) goto done;
        if (!got) break;
        if (BCryptHashData(hash, buffer, got, 0) < 0) goto done;
    }
    if (BCryptFinishHash(hash, digest, sizeof(digest), 0) >= 0)
        ok = !memcmp(digest, expected_hash, sizeof(digest));
done:
    if (hash) BCryptDestroyHash(hash);
    if (algorithm) BCryptCloseAlgorithmProvider(algorithm, 0);
    CloseHandle(file);
    return ok;
}

static int supported_memory(void) {
    IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)image;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE || dos->e_lfanew <= 0 || dos->e_lfanew > 4096) return 0;
    IMAGE_NT_HEADERS64 *nt = (IMAGE_NT_HEADERS64 *)(image + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE || nt->FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
        nt->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
        nt->OptionalHeader.SizeOfImage < TERMINAL_SLOT_RVA + sizeof(void *)) return 0;
    if (memcmp(image + TERMINAL_UPDATE_RVA, entry_bytes, sizeof(entry_bytes))) return 0;
#ifndef TFX_OBSERVE_PERSON
    for (unsigned i = 0; i < sizeof(change_entries) / sizeof(change_entries[0]); ++i)
        if (memcmp(image + change_entries[i].rva, change_entries[i].bytes, 16)) return 0;
#endif
    /* Adjacent lifecycle entries establish the expected table layout. */
    void **table = (void **)(image + TERMINAL_VTABLE_RVA);
    return table[0] == image + 0xb0c570 && table[4] == image + 0xb0c8b0 &&
           table[5] == image + 0xb0d030 && table[8] == image + 0xb0c730 &&
           table[9] == image + 0xb0c7f0 && table[11] == image + TERMINAL_UPDATE_RVA;
}

/* Called by assembly with the original first four ABI arguments. */
void tfx_observe(void *system, void *world, void *update_data, float delta) {
    if (!InterlockedCompareExchange(&enabled, 0, 0)) return;
    void *vtable = NULL, *window = NULL;
    unsigned char nested = 0;
    SIZE_T got;
    if (!system || !world || update_data ||
        !ReadProcessMemory(GetCurrentProcess(), system, &vtable, sizeof(vtable), &got) ||
        vtable != image + TERMINAL_VTABLE_RVA ||
        !ReadProcessMemory(GetCurrentProcess(), (unsigned char *)world + 0x1f0, &window, sizeof(window), &got) ||
        !ReadProcessMemory(GetCurrentProcess(), (unsigned char *)world + 0x1f8, &nested, sizeof(nested), &got) ||
        window || nested != 1) { InterlockedIncrement(&rejected); return; }
    LONG thread = (LONG)GetCurrentThreadId();
    LONG owner = InterlockedCompareExchange(&simulation_thread, thread, 0);
    if (owner && owner != thread) { InterlockedIncrement(&rejected); return; }
    LONG count = InterlockedIncrement(&observed);
    if (count > OBSERVATION_LIMIT) return;
    uint32_t bits;
    memcpy(&bits, &delta, sizeof(bits));
    tfx_reader reader;
    tfx_reader_init(&reader, (uint64_t)(uintptr_t)world, (uint64_t)(uintptr_t)image, copy_memory, NULL);
    reader.terminal_system = (uint64_t)(uintptr_t)system;
    uint32_t world_id = 0;
    for (unsigned i = 0; i < world_count; ++i) if (world_addresses[i] == reader.world) world_id = i + 1;
    if (!world_id) {
        if (world_count == 4) { InterlockedExchange(&enabled, 0); log_line("Refused: world limit exceeded.\n"); return; }
        world_addresses[world_count++] = reader.world; world_id = world_count;
        char line[128];
        snprintf(line, sizeof(line), "world_registered world_id=%u world=%p\n", world_id, world);
        log_line(line);
    }
    reader.world_id = world_id;
#ifdef TFX_OBSERVE_PERSON
    tfx_status status = tfx_dispatch_with_reader(bits, &reader);
    int complete = tfx_journey_probe_complete();
#else
    tfx_status status = tfx_dispatch_with_writer(bits, &reader, tfx_apply_waiting_change, &change_operations);
    int complete = tfx_pilot_complete();
#endif
    if (count <= 8 || count == OBSERVATION_LIMIT || status != TFX_OK) {
        char line[256];
        snprintf(line, sizeof(line), "observation=%ld thread=%lu world=%p delta_bits=%08x status=%u rejected=%ld\n",
                 count, GetCurrentThreadId(), world, bits, status, InterlockedCompareExchange(&rejected, 0, 0));
        log_line(line);
    }
    if (count == OBSERVATION_LIMIT || complete) {
        InterlockedExchange(&enabled, 0);
        log_line("Pilot observation complete; hook forwards only. No further changes permitted.\n");
        FlushFileBuffers(log_file);
    }
}

/* Explicit initialization OUTSIDE DllMain/loader lock. DLL remains pinned. */
__declspec(dllexport) DWORD WINAPI tfx_start(void *unused) {
    if (unused) return TFX_INVALID_ARGUMENT;
    if (InterlockedCompareExchange(&start_state, 1, 0)) return TFX_BUSY;
    HMODULE self;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)&tfx_start, &self) || !open_log(self))
        return TFX_SYSTEM_ERROR;
#ifdef TFX_OBSERVE_PERSON
    log_line("TPF shared bridge 0.6.3; read-only journey observer; limit=3600; ABI=1,2,3,4,5,6\n");
#else
    log_line("TPF shared bridge 0.6.3; one passenger change maximum; limit=3600; ABI=1,2,3,4,5,6\n");
#endif
    tfx_status status = TFX_UNSUPPORTED_BUILD;
    if (!supported_file()) { log_line("Refused: executable SHA-256 mismatch or unreadable.\n"); goto fail; }
    image = (unsigned char *)GetModuleHandleW(NULL);
    status = TFX_HOOK_CONFLICT;
    if (!supported_memory()) { log_line("Refused: live code/table does not match pinned build.\n"); goto fail; }
    wchar_t name[96];
    swprintf(name, 96, L"Local\\TPFSharedBridge-%lu", GetCurrentProcessId());
    ownership = CreateMutexW(NULL, FALSE, name);
    if (!ownership || GetLastError() == ERROR_ALREADY_EXISTS) {
        log_line("Refused: another bridge owns this process, or ownership unavailable.\n"); goto fail;
    }
    status = TFX_SYSTEM_ERROR;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
                          (LPCWSTR)&tfx_start, &self)) goto fail;
#ifdef TFX_OBSERVE_PERSON
    status = tfx_register_journey_probe(log_line, TFX_OBSERVE_PERSON, TFX_OBSERVE_DESTINATION);
#else
    change_operations = (tfx_engine_changes){
        (void (*)(void *))(image + 0x2bb4640),
        (void (*)(void *, int32_t, const void *, uint8_t))(image + 0x2bb7320),
        (void (*)(void *, int32_t, const void *, uint8_t, uint8_t))(image + 0xb21cd0),
        (void (*)(void *))(image + 0x2bb4d90), log_line};
    status = tfx_register_candidate_probe(log_line);
#endif
    if (status != TFX_OK) goto fail;
    tfx_original = image + TERMINAL_UPDATE_RVA;
    status = tfx_replace_slot((void **)(image + TERMINAL_SLOT_RVA), tfx_original, &tfx_terminal_entry);
    if (status != TFX_OK) {
        /* Keep ownership, module and original pointer alive: exchange may have
         * succeeded before protection restoration failed. Forwarding stays safe. */
        log_line("Refused: slot exchange/protection failure; observer disabled; DLL retained.\n");
        return status;
    }
    log_line("Installed verified virtual slot; no engine instructions relocated.\n");
    InterlockedExchange(&enabled, 1);
    return TFX_OK;
fail:
    if (ownership) { CloseHandle(ownership); ownership = NULL; }
    CloseHandle(log_file);
    log_file = INVALID_HANDLE_VALUE;
    return status;
}

/* Disables future observations; an already-running callback may finish. */
__declspec(dllexport) DWORD WINAPI tfx_stop(void *unused) {
    if (unused) return TFX_INVALID_ARGUMENT;
    InterlockedExchange(&enabled, 0);
    return TFX_OK;
}
