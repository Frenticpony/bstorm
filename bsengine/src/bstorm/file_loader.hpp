#pragma once

#include <cstdio>
#include <string>

namespace bstorm
{
class FileLoader
{
public:
    FileLoader() {}
    virtual ~FileLoader() {}
    virtual FILE* OpenFile(const std::wstring& path) = 0;
    virtual void CloseFile(const std::wstring& path, FILE* fp) = 0;
};

class FileLoaderFromTextFile : public FileLoader
{
public:
    FILE * OpenFile(const std::wstring& path) override;
    void CloseFile(const std::wstring& path, FILE* fp) override;
};
}