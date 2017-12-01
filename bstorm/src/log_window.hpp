#pragma once

#include <imgui.h>
#include <bstorm/logger.hpp>

namespace bstorm {
  class LogWindow : public Logger {
  public:
    LogWindow();
    ~LogWindow();
    void setInitWindowPos(int left, int top, int width, int height);
    void log(const std::string& tag, const std::string& text)  override;
    void log(const std::wstring& tag, const std::wstring& text)  override;
    void draw();
  private:
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool scrollToBottom;
    bool doCopy;
    ImGuiTextFilter filter;
    ImGuiTextBuffer buf;
    ImVector<int> lineOffsets;
  };
}