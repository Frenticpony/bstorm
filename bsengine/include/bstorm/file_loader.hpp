#pragma once

#include <cstdio>
#include <string>

namespace bstorm {
  class FileLoader {
  public :
    FileLoader() {}
    virtual ~FileLoader(){}
    virtual FILE* openFile(const std::wstring& path) = 0;
    virtual void closeFile(const std::wstring& path, FILE* fp) = 0;
  };

  class FileLoaderFromTextFile : public FileLoader {
  public:
    FILE* openFile(const std::wstring& path) override;
    void closeFile(const std::wstring& path, FILE* fp) override;
  };
}