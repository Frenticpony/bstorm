#include <bstorm/util.hpp>
#include <bstorm/static_script_error.hpp>

namespace bstorm {
  static_script_error::static_script_error() : std::runtime_error("") {}
  static_script_error::static_script_error(const std::wstring& filePath, int line, int column, const std::wstring& msg) :
    std::runtime_error(toUTF8(getFileName(filePath)) + ":L" + std::to_string(line) + "," + std::to_string(column) + ": " + toUTF8(msg)),
    filePath(filePath),
    line(line),
    column(column),
    msg(msg)
  {
  }
}