#include "lua_vm.h"

#include "utility/export/error.h"
using namespace UTILITY;

extern "C" {
#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"
}

#define STATE_PTR ((lua_State *)lua_state)

LUA_VM::LUA_VM(const char *file, LIB_REGISTER *lib_reg)
:lua_state(0)
{
    ENSURE(NULL != (lua_state=luaL_newstate()));

    luaL_openlibs(STATE_PTR);
    if(lib_reg) {
        lib_reg(STATE_PTR);
    }

    ENSURE(LUA_OK == luaL_loadfile(STATE_PTR, file));

    ENSURE(LUA_OK == lua_pcall(STATE_PTR, 0, 0, 0));
}

LUA_VM::~LUA_VM()
{
    if(STATE_PTR) {
        lua_close(STATE_PTR);
    }
}

int LUA_VM::run (const char *func, const char *args)
{
    int result;

    lua_getglobal(STATE_PTR, func);
    lua_pushstring(STATE_PTR, args);

    ENSURE(LUA_OK == lua_pcall(STATE_PTR, 1, 1, 0));
    //lua_call(lua_state, 1, 1);

    result = lua_tointeger(STATE_PTR, -1);
    lua_pop(STATE_PTR, 1);

    return result;
}

