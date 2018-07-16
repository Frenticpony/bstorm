#pragma once

#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/script_info.hpp>

#include <string>
#include <luajit/lua.hpp>

namespace bstorm
{
class Env;
// 環境を与えた場合は環境の作成だけを行い、Luaスレッドへの関数の登録はしない
void RegisterStandardAPI(lua_State* L, ScriptType type, const std::wstring& version, const NullableSharedPtr<Env>& env);

class Script;
Script* GetScript(lua_State* L);
void SetScript(lua_State* L, Script*);
}