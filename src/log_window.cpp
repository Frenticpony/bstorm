#include <bstorm/util.hpp>

#include "log_window.hpp"

namespace bstorm {
  LogWindow::LogWindow() :
    iniLeft(0),
    iniTop(0),
    iniWidth(0),
    iniHeight(0),
    scrollToBottom(false),
    doCopy(false)
  {
  }

  LogWindow::~LogWindow() {}

  void LogWindow::setInitWindowPos(int left, int top, int width, int height) {
    iniLeft = left;
    iniTop = top;
    iniWidth = width;
    iniHeight = height;
  }

  void LogWindow::log(const std::wstring& tag, const std::wstring& text) {
    log(toUTF8(tag), toUTF8(text));
  }

  void LogWindow::log(const std::string& tag, const std::string& text) {
    int oldSize = buf.size();
    buf.append(tag.c_str());
    buf.append(" ");
    buf.append(text.c_str());
    buf.append("\n");
    for (int newSize = buf.size(); oldSize < newSize; oldSize++) {
      if (buf[oldSize] == '\n') lineOffsets.push_back(oldSize);
    }
#ifdef _DEBUG
    OutputDebugStringA((tag + " " + text + "\n").c_str());
#endif
    scrollToBottom = true;
  }

  void LogWindow::draw() {
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiSetCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiSetCond_FirstUseEver);
    if (ImGui::Begin("Log")) {
      ImGui::BeginChild("LogTextArea", ImVec2(0, -ImGui::GetTextLineHeightWithSpacing()*1.3), false, ImGuiWindowFlags_HorizontalScrollbar);
      if (doCopy) ImGui::LogToClipboard();
      doCopy = false;
      if (filter.IsActive()) {
        const char* bufBegin = buf.begin();
        const char* line = bufBegin;
        for (int lineNum = 0; line; lineNum++) {
          const char* lineEnd = (lineNum < lineOffsets.size()) ? bufBegin + lineOffsets[lineNum] : NULL;
          if (filter.PassFilter(line, lineEnd)) {
            ImGui::TextUnformatted(line, lineEnd);
          }
          line = lineEnd && lineEnd[1] ? lineEnd + 1 : NULL;
        }
      } else {
        ImGui::TextUnformatted(buf.begin());
      }
      if (scrollToBottom) {
        ImGui::SetScrollHere(1.0f);
      }
      scrollToBottom = false;
      ImGui::LogFinish();
      ImGui::EndChild();
      ImGui::Separator();
      if (ImGui::Button("Clear")) {
        buf.clear();
        lineOffsets.clear();
      }
      ImGui::SameLine();
      if (ImGui::Button("Copy")) {
        doCopy = true;
      }
      ImGui::SameLine();
      filter.Draw("Filter", -100);
    }
    ImGui::End();
  }
}
