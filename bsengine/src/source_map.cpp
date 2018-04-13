#include <bstorm/source_map.hpp>

#include <bstorm/util.hpp>

namespace bstorm {
  std::string SourcePos::toString() const {
    return toUTF8(*filename) + ":" + std::to_string(line) + ((column >= 0) ? (":" + std::to_string(column)) : "");
  }

  void SourceMap::logSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine) {
    srcMap[outputLine] = { srcLine, -1, path };
  }

  std::shared_ptr<SourcePos> SourceMap::getSourcePos(int outputLine) const {
    auto it = srcMap.find(outputLine);
    if (it != srcMap.end()) {
      return std::make_shared<SourcePos>(it->second);
    }
    return nullptr;
  }
}
