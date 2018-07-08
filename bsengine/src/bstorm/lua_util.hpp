#pragma once

#include <string>
#include <luajit/lua.hpp>

namespace bstorm
{
void* GetPointerFromLuaRegistry(lua_State* L, const char* key);
void SetPointerToLuaRegistry(lua_State* L, const char* key, void* p);

// serialize stack top chunk. this function doesn't remove stack top.
void SerializeChunk(lua_State* L, std::string& byteCode);
}