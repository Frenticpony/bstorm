#pragma once

#include <bstorm/logger.hpp>
#include <array>

namespace bstorm {
  class LogWindow : public Logger {
  public:
    using Logger::log;
    LogWindow();
    ~LogWindow();
    void setInitWindowPos(int left, int top, int width, int height);
    void log(Log& lg) override;
    void log(Log&& lg) override;
    void draw();
    void clear();
  private:
    static constexpr int MaxFilterInputSize = 65;
    std::array<char, MaxFilterInputSize> filterInput;
    int validIdx(int idx) const;
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool scrollToBottom;
    int logCnt;
    bool showInfoLevel;
    bool showWarnLevel;
    bool showErrorLevel;
    bool showUserLevel;
    bool showDetailLevel;
    static constexpr int MaxLogCnt = 1 << 11; // power of 2
    int headIdx;
    std::array<Log, MaxLogCnt> logs;
  };
}