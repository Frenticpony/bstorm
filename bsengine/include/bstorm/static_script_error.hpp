#pragma once

#include <string>
#include <exception>
#include <memory>

namespace bstorm {
  struct static_script_error : public std::runtime_error {
    static_script_error();
    static_script_error(const std::wstring& filePath, int line, int column, const std::wstring& msg);
    std::wstring filePath;
    int line;
    int column;
    std::wstring msg;
  };
};