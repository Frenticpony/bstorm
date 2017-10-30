#include <bstorm/source_map.hpp>

namespace bstorm {
  void SourceMap::logSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine) {
    srcMap[outputLine] = { srcLine, -1, path };
  }

  SourcePos SourceMap::getSourcePos(int outputLine) const {
    auto it = srcMap.find(outputLine);
    if (it != srcMap.end()) {
      return it->second;
    }
    return{ -1, -1, std::shared_ptr<std::wstring>() };
  }
}
