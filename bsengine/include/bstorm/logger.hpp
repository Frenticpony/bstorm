#pragma once

#include <string>
#include <memory>
#include <windows.h>

namespace bstorm {
  class Logger {
  public:
    virtual ~Logger() {}
    // NOTE : use UTF8
    virtual void log(const std::string& tag, const std::string& text) = 0;
    virtual void log(const std::wstring& tag, const std::wstring& text) = 0;
    void logInfo(const std::string& text) { log("[info]", text); }
    void logInfo(const std::wstring& text) { log(L"[info]", text); }
    void logWarn(const std::string& text) { log("[warn]", text); }
    void logWarn(const std::wstring& text) { log(L"[warn]", text); }
    void logError(const std::string& text) { log("[error]", text); }
    void logError(const std::wstring& text) { log(L"[error]", text); }
    void logDebug(const std::string& text) {
#ifdef _DEBUG
      log("[debug]", text);
#endif
    }
    void logDebug(const std::wstring& text) {
#ifdef _DEBUG
      log(L"[debug]", text);
#endif
    }
  };

  class DummyLogger : public Logger {
  public:
    DummyLogger() {}
    void log(const std::string& tag, const std::string& text) {
#ifdef _DEBUG
      OutputDebugStringA((tag + " " + text + "\n").c_str());
#endif
    }
    void log(const std::wstring& tag, const std::wstring& text) {
#ifdef _DEBUG
      OutputDebugStringW((tag + L" " + text + L"\n").c_str());
#endif
    }
  };
}