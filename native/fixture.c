/* Disposable target for loader tests. Deliberately NOT a supported game build. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
int wmain(void) {
    printf("fixture Windows PID=%lu\n", GetCurrentProcessId());
    fflush(stdout);
    Sleep(60000);
    return 0;
}
