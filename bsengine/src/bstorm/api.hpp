#pragma once

#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/script_info.hpp>

#include <string>
#include <luajit/lua.hpp>

namespace bstorm
{
class Env;
// Lがnullptrなら関数を登録せずに環境の作成のみを行う
std::shared_ptr<Env> CreateInitRootEnv(ScriptType type, const std::wstring& version, lua_State* L);

class Script;
Script* GetScript(lua_State* L);
void SetScript(lua_State* L, Script*);
}