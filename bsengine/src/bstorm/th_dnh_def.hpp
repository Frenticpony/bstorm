#pragma once

#include <string>

namespace bstorm
{
struct ThDnhDef
{
    std::wstring packageScriptMain;
    std::wstring windowTitle;
    int screenWidth = 640;
    int screenHeight = 480;
};

ThDnhDef loadThDnhDef(const std::string& path);
}