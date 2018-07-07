#include <bstorm/file_loader.hpp>

#include <windows.h>

namespace bstorm
{
FILE* FileLoader::OpenFile(const std::wstring& path)
{
    return _wfopen(path.c_str(), L"rb");
}

void FileLoader::CloseFile(const std::wstring& path, FILE* fp)
{
    fclose(fp);
}
}