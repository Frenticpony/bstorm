#pragma once

#include <string>
#include <vector>

namespace bstorm
{
constexpr wchar_t* SCRIPT_TYPE_PLAYER = L"Player";
constexpr wchar_t* SCRIPT_TYPE_SINGLE = L"Single";
constexpr wchar_t* SCRIPT_TYPE_PLURAL = L"Plural";
constexpr wchar_t* SCRIPT_TYPE_STAGE = L"Stage";
constexpr wchar_t* SCRIPT_TYPE_PACKAGE = L"Package";
constexpr wchar_t* SCRIPT_TYPE_UNKNOWN = L"Unknown";
constexpr wchar_t* SCRIPT_TYPE_SHOT_CUSTOM = L"ShotCustom";
constexpr wchar_t* SCRIPT_TYPE_ITEM_CUSTOM = L"ItemCustom";
constexpr wchar_t* SCRIPT_VERSION_PH3 = L"3";

class ScriptInfo
{
public:
    std::wstring path;
    std::wstring type;
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

int GetScriptTypeConstFromName(const std::wstring& name);
std::wstring GetScriptTypeNameFromConst(int c);
}