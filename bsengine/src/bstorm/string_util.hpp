#pragma once

#include <string>
#include <vector>
#include <windows.h>

namespace bstorm
{
template <UINT cp>
std::string ToMultiByte(const std::wstring& ws)
{
    if (ws.empty()) return std::string();
    int neededSize = WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), NULL, 0, NULL, NULL);
    std::string s(neededSize, 0);
    WideCharToMultiByte(cp, 0, &ws[0], (int)ws.size(), &s[0], neededSize, NULL, NULL);
    return s;
}

template <UINT cp>
std::wstring FromMultiByte(const std::string& s)
{
    if (s.empty()) return std::wstring();
    int neededSize = MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), NULL, 0);
    std::wstring ws(neededSize, 0);
    MultiByteToWideChar(cp, 0, &s[0], (int)s.size(), &ws[0], neededSize);
    return ws;
}

inline std::string ToUTF8(const std::wstring& ws)
{
    return ToMultiByte<CP_UTF8>(ws);
}

inline std::wstring ToUnicode(const std::string& s)
{
    return FromMultiByte<CP_UTF8>(s);
}

std::vector<std::wstring> Split(const std::wstring& s, wchar_t delimiter);
std::vector<std::wstring> Split(const std::wstring& s, const std::wstring& delimiter);

bool IsMatchString(const std::string& searchText, const std::string& searchTarget);

bool IsSpace(wchar_t c);

void TrimSpace(std::wstring* s);
}