/* Explicit development loader. No automatic startup, proxy DLL or persistence. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

static uintptr_t remote_module(DWORD pid, const wchar_t *name, int full_path) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    MODULEENTRY32W entry = {.dwSize = sizeof(entry)};
    uintptr_t base = 0;
    if (Module32FirstW(snapshot, &entry)) do {
        if (!_wcsicmp(full_path ? entry.szExePath : entry.szModule, name)) {
            base = (uintptr_t)entry.modBaseAddr; break;
        }
    } while (Module32NextW(snapshot, &entry));
    CloseHandle(snapshot);
    return base;
}

static int run_remote(HANDLE process, uintptr_t address, void *argument, DWORD *result) {
    HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)address, argument, 0, NULL);
    if (!thread) return 0;
    DWORD wait = WaitForSingleObject(thread, 30000);
    int ok = wait == WAIT_OBJECT_0 && GetExitCodeThread(thread, result);
    CloseHandle(thread);
    if (!ok) fwprintf(stderr, L"Remote call did not complete; do not unload or terminate its thread.\n");
    return ok;
}

int wmain(int argc, wchar_t **argv) {
    if (argc == 2 && !wcscmp(argv[1], L"--list")) {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot == INVALID_HANDLE_VALUE) return 3;
        PROCESSENTRY32W entry = {.dwSize = sizeof(entry)};
        if (Process32FirstW(snapshot, &entry)) do {
            wprintf(L"%lu %ls\n", entry.th32ProcessID, entry.szExeFile);
        } while (Process32NextW(snapshot, &entry));
        CloseHandle(snapshot);
        return 0;
    }
    if (argc != 3 && argc != 4) {
        fwprintf(stderr, L"Usage: bridge_loader.exe --list | WINDOWS_PID ABSOLUTE_DLL_PATH [--stop]\n"); return 2;
    }
    int stop = argc == 4 && !wcscmp(argv[3], L"--stop");
    wchar_t *end;
    unsigned long parsed = wcstoul(argv[1], &end, 10);
    if (!parsed || *end || (argc == 4 && !stop)) return 2;
    DWORD pid = (DWORD)parsed;
    wchar_t path[32768];
    DWORD length = GetFullPathNameW(argv[2], 32768, path, NULL);
    if (!length || length >= MAX_PATH || GetFileAttributesW(path) == INVALID_FILE_ATTRIBUTES) {
        fwprintf(stderr, L"DLL must exist; pilot requires a Windows path shorter than MAX_PATH.\n"); return 2;
    }
    HMODULE local = LoadLibraryExW(path, NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (!local) return 3;
    FARPROC entry = GetProcAddress(local, stop ? "tfx_stop" : "tfx_start");
    if (!entry) { FreeLibrary(local); return 3; }
    uintptr_t entry_rva = (uintptr_t)entry - (uintptr_t)local;
    FreeLibrary(local);
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, pid);
    if (!process) { fwprintf(stderr, L"OpenProcess failed: %lu\n", GetLastError()); return 3; }
    int exit_code = 3;
    uintptr_t remote = remote_module(pid, path, 1);
    if (!remote && stop) { fwprintf(stderr, L"Bridge is not loaded.\n"); goto done; }
    if (!remote) {
        FARPROC load = GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        HMODULE owner;
        wchar_t owner_path[MAX_PATH];
        if (!load || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCWSTR)load, &owner) ||
            !GetModuleFileNameW(owner, owner_path, MAX_PATH)) goto done;
        wchar_t *name = wcsrchr(owner_path, L'\\');
        uintptr_t remote_owner = remote_module(pid, name ? name + 1 : owner_path, 0);
        if (!remote_owner) goto done;
        SIZE_T size = (wcslen(path) + 1) * sizeof(wchar_t), written;
        void *argument = VirtualAllocEx(process, NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (!argument) goto done;
        if (!WriteProcessMemory(process, argument, path, size, &written) || written != size) {
            VirtualFreeEx(process, argument, 0, MEM_RELEASE); goto done;
        }
        DWORD result;
        if (!run_remote(process, remote_owner + (uintptr_t)load - (uintptr_t)owner, argument, &result)) {
            /* A timed-out remote loader could still be reading this argument. */
            goto done;
        }
        VirtualFreeEx(process, argument, 0, MEM_RELEASE);
        remote = remote_module(pid, path, 1); /* Do not truncate a 64-bit HMODULE to exit code. */
        if (!remote) { fwprintf(stderr, L"LoadLibrary did not load the requested DLL.\n"); goto done; }
    }
    DWORD status;
    if (!run_remote(process, remote + entry_rva, NULL, &status)) goto done;
    wprintf(L"%ls returned %lu. Log is beside the DLL.\n", stop ? L"tfx_stop" : L"tfx_start", status);
    exit_code = status == 0 ? 0 : 1;
done:
    CloseHandle(process);
    return exit_code;
}
