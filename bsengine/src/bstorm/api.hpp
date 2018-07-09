#pragma once

#include <bstorm/env.hpp>

#include <string>
#include <unordered_map>
#include <luajit/lua.hpp>

namespace bstorm
{
struct SourcePos;
void RegisterStandardAPI(lua_State* L, const std::wstring& type, const std::wstring& version, NameTable& table);

// helper
int GetCurrentLine(lua_State* L);
std::shared_ptr<SourcePos> GetSourcePos(lua_State* L);

class Script;
Script* GetScript(lua_State* L);
void SetScript(lua_State* L, Script*);
}