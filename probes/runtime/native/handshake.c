/* Loading feasibility only: no Lua API calls, hooks, threads or engine access. */
#include <stdio.h>
struct lua_State;
__attribute__((visibility("default")))
int luaopen_tpf3_runtime_handshake(struct lua_State *state) {
    (void)state;
    fputs("[TPF3RuntimeProbe] native_handshake_executed platform=linux engine_changes=0\n", stdout);
    fflush(stdout);
    return 0;
}
