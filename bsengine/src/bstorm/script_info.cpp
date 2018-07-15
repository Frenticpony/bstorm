#include <bstorm/script_info.hpp>

#include <bstorm/dnh_const.hpp>

namespace bstorm
{
const char * ScriptType::GetName() const
{
    switch (value)
    {
        case ScriptType::Value::PLAYER: return "Player";
        case ScriptType::Value::SINGLE: return "Single";
        case ScriptType::Value::PLURAL: return "Plural";
        case ScriptType::Value::STAGE: return "Stage";
        case ScriptType::Value::PACKAGE: return "Package";
        case ScriptType::Value::SHOT_CUSTOM: return "ShotCustom";
        case ScriptType::Value::ITEM_CUSTOM: return "ItemCustom";
    }
    return "Unknown";
}
int ScriptType::GetScriptConst() const
{
    switch (value)
    {
        case ScriptType::Value::PLAYER:
        case ScriptType::Value::SINGLE:
        case ScriptType::Value::PLURAL:
        case ScriptType::Value::STAGE:
        case ScriptType::Value::PACKAGE:
            return static_cast<int>(value);
    }
    return 0;
}
bool ScriptType::IsStgSceneScript() const
{
    return value != ScriptType::Value::PACKAGE;
}
ScriptType ScriptType::FromScriptConst(int c)
{
    switch (c)
    {
        case TYPE_SCRIPT_PLAYER:
            return ScriptType::Value::PLAYER;
        case TYPE_SCRIPT_SINGLE:
            return ScriptType::Value::SINGLE;
        case TYPE_SCRIPT_PLURAL:
            return ScriptType::Value::PLURAL;
        case TYPE_SCRIPT_STAGE:
            return ScriptType::Value::STAGE;
        case TYPE_SCRIPT_PACKAGE:
            return ScriptType::Value::PACKAGE;
    }
    return ScriptType::Value::UNKNOWN;
}
ScriptType ScriptType::FromName(const std::string & name)
{
    if (name == "Player") return ScriptType::Value::PLAYER;
    if (name == "Single") return ScriptType::Value::SINGLE;
    if (name == "Plural") return ScriptType::Value::PLURAL;
    if (name == "Stage") return ScriptType::Value::STAGE;
    if (name == "Package") return ScriptType::Value::PACKAGE;
    if (name == "ShotCustom") return ScriptType::Value::SHOT_CUSTOM;
    if (name == "ItemCustom") return ScriptType::Value::ITEM_CUSTOM;
    return ScriptType::Value::UNKNOWN;
}
}