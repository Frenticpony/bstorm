#pragma once

#include <bstorm/env.hpp>

#include <string>
#include <unordered_map>
#include <luajit/lua.hpp>

namespace bstorm
{
struct SourcePos;
void RegisterStandardAPI(lua_State* L, const std::wstring& type, const std::wstring& version, NameTable& table);
void RegisterRuntimeHelper(lua_State* L);

// helper
int GetCurrentLine(lua_State* L);
std::shared_ptr<SourcePos> GetSourcePos(lua_State* L);
int __UnsafeFunctionCommon(lua_State* L, lua_CFunction func);
template <lua_CFunction func>
int UnsafeFunction(lua_State* L)
{
    return __UnsafeFunctionCommon(L, func);
}

class Script;
Script* GetScript(lua_State* L);
void SetScript(lua_State* L, Script*);
}