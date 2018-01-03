#pragma once

#include <string>
#include <memory>
#include <map>

namespace bstorm {
  struct SourcePos {
    int line;
    int column;
    std::shared_ptr<std::wstring> filename;
  };

  struct SourceLoc {
    SourcePos begin;
    SourcePos end;
  };

  class SourceMap {
  public:
    void logSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine);
    SourcePos getSourcePos(int outputLine) const;
  private:
    std::map<int, SourcePos> srcMap;
  };
}
