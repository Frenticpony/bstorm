#pragma once

#include <luajit/lua.hpp>

namespace bstorm
{
void* GetPointerFromLuaRegistry(lua_State* L, const char* key);
void SetPointerToLuaRegistry(lua_State* L, const char* key, void* p);
}