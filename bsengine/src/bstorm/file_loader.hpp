#pragma once

#include <cstdio>
#include <string>

namespace bstorm
{
// should be thread safe.
class FileLoader
{
public:
    FILE * OpenFile(const std::wstring& path);
    void CloseFile(const std::wstring& path, FILE* fp);
};
}