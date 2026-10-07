#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "slot.h"

tfx_status tfx_replace_slot(void **slot, void *expected, void *replacement) {
    if (!slot || ((uintptr_t)slot & 7) || !expected || !replacement)
        return TFX_INVALID_ARGUMENT;
    DWORD previous, ignored;
    if (!VirtualProtect(slot, sizeof(*slot), PAGE_READWRITE, &previous))
        return TFX_SYSTEM_ERROR;
    void *found = InterlockedCompareExchangePointer(slot, replacement, expected);
    if (!VirtualProtect(slot, sizeof(*slot), previous, &ignored))
        return TFX_PROTECTION_ERROR; /* Pointer may be installed: NEVER unload. */
    return found == expected ? TFX_OK : TFX_HOOK_CONFLICT;
}
