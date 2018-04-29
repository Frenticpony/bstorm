#include <bstorm/source_map.hpp>

#include <bstorm/util.hpp>

namespace bstorm
{
std::string SourcePos::ToString() const
{
    return ToUTF8(*filename) + ":" + std::to_string(line) + ((column >= 0) ? (":" + std::to_string(column)) : "");
}

void SourceMap::LogSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine)
{
    srcMap_[outputLine] = { srcLine, -1, path };
}

std::shared_ptr<SourcePos> SourceMap::GetSourcePos(int outputLine) const
{
    auto it = srcMap_.find(outputLine);
    if (it != srcMap_.end())
    {
        return std::make_shared<SourcePos>(it->second);
    }
    return nullptr;
}
}