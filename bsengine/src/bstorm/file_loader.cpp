#include <bstorm/file_loader.hpp>

#include <windows.h>

namespace bstorm
{
FILE* FileLoaderFromTextFile::OpenFile(const std::wstring& path)
{
    OutputDebugStringW(L"OpenFile: ");
    OutputDebugStringW(path.c_str());
    OutputDebugStringW(L"\n");
    return _wfopen(path.c_str(), L"rb");
}

void FileLoaderFromTextFile::CloseFile(const std::wstring& path, FILE* fp)
{
    OutputDebugStringW(L"CloseFile: ");
    OutputDebugStringW(path.c_str());
    OutputDebugStringW(L"\n");
    fclose(fp);
}
}