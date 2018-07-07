#include <bstorm/lua_util.hpp>

namespace bstorm
{
void* GetPointerFromLuaRegistry(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_rawget(L, LUA_REGISTRYINDEX);
    void* p = lua_touserdata(L, -1);
    return p;
}

void SetPointerToLuaRegistry(lua_State* L, const char* key, void* p)
{
    lua_pushstring(L, key);
    lua_pushlightuserdata(L, p);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

}