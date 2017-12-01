#pragma once

#include <fstream>

#include <bstorm/logger.hpp>

namespace bstorm {
  class FileLogger : public Logger {
  public :
    FileLogger(const std::wstring& filePath);
    ~FileLogger();
    void log(const std::string& tag, const std::string& text) override;
    void log(const std::wstring& tag, const std::wstring& text) override;
  private :
    std::ofstream file;
  };
}
