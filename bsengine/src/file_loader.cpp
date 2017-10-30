#pragma once

#include <windows.h>
#include <bstorm/file_loader.hpp>

namespace bstorm {
  FILE* FileLoaderFromTextFile::openFile(const std::wstring& path) {
    OutputDebugStringW(L"OpenFile: ");
    OutputDebugStringW(path.c_str());
    OutputDebugStringW(L"\n");
    return _wfopen(path.c_str(), L"rb");
  }

  void FileLoaderFromTextFile::closeFile(const std::wstring& path, FILE* fp) {
    OutputDebugStringW(L"CloseFile: ");
    OutputDebugStringW(path.c_str());
    OutputDebugStringW(L"\n");
    fclose(fp);
  }
}