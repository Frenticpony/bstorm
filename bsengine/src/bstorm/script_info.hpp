#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace bstorm
{
class ScriptType
{
public:
    enum class Value : uint8_t
    {
        PLAYER = 1,
        SINGLE = 2,
        PLURAL = 3,
        STAGE = 4,
        PACKAGE = 5,
        SHOT_CUSTOM = 6,
        ITEM_CUSTOM = 7,
        UNKNOWN = 0xff
    } value;
    ScriptType() : value(Value::UNKNOWN) {};
    ScriptType(Value v) : value(v) {};
    bool operator==(const ScriptType& type) { return value == type.value; }
    bool operator!=(const ScriptType& type) { return !(*this == type); }
    const char* GetName() const;
    int GetScriptConst() const;
    bool IsStgSceneScript() const;
    static ScriptType FromScriptConst(int c);
    static ScriptType FromName(const std::string& name);
};

constexpr wchar_t* SCRIPT_VERSION_PH3 = L"3";

class ScriptInfo
{
public:
    std::wstring path;
    ScriptType type;
    std::wstring version;
    std::wstring id;
    std::wstring title;
    std::wstring text;
    std::wstring imagePath;
    std::wstring systemPath;
    std::wstring backgroundPath;
    std::wstring bgmPath;
    std::vector<std::wstring> playerScripts;
    std::wstring replayName;
};
}