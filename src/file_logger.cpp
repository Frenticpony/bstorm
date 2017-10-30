#include <windows.h>
#include <bstorm/util.hpp>

#include "file_logger.hpp"

namespace bstorm {
  FileLogger::FileLogger(const std::wstring & filePath) {
    file.open(filePath, std::ios::app);
    if (!file.good()) {
      throw std::runtime_error("can't open log file: " + toUTF8(filePath));
    }
  }

  FileLogger::~FileLogger() {
    file.close();
  }

  void FileLogger::log(const std::string & tag, const std::string & text) {
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::string date(23, '\0');
    sprintf(&date[0], "%04d/%02d/%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    file << date + " " + tag + " " + text << std::endl;
  }

  void FileLogger::log(const std::wstring & tag, const std::wstring & text) {
    log(toUTF8(tag), toUTF8(text));
  }
}
