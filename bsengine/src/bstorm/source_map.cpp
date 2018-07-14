#include <bstorm/source_map.hpp>

#include <bstorm/util.hpp>

#include "../../json/single_include/nlohmann/json.hpp"

#include <deque>

namespace bstorm
{
std::string SourcePos::ToString() const
{
    return ToUTF8(*filename) + ":" + std::to_string(line) + ((column >= 0) ? (":" + std::to_string(column)) : "");
}
using nlohmann::json;

// シリアライズ用の中間表現
// <srcPath, <outputLine, srcLine>>
using CompactSourceMap = std::map<std::string, std::deque<std::pair<int32_t, int32_t>>>;

SourceMap::SourceMap() {}

SourceMap::SourceMap(const std::vector<uint8_t>& data)
{
    CompactSourceMap compact = json::from_msgpack(data);
    for (const auto & entry : compact)
    {
        auto srcPath = std::make_shared<std::wstring>(ToUnicode(entry.first));
        for (const auto & pair : entry.second)
        {
            int outputLine = pair.first;
            int srcLine = pair.second;
            LogSourcePos(outputLine, srcPath, srcLine);
        }
    }
}

void SourceMap::LogSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine)
{
    srcMap_.emplace(std::piecewise_construct, std::forward_as_tuple(outputLine), std::forward_as_tuple(srcLine, -1, path));
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

void SourceMap::Serialize(std::vector<uint8_t>& dst) const
{
    CompactSourceMap compact;
    for (auto& entry : srcMap_)
    {
        int outputLine = entry.first;
        int srcLine = entry.second.line;
        const std::string& srcPath = ToUTF8(*entry.second.filename);

        if (compact.count(srcPath) == 0)
        {
            compact.emplace(std::piecewise_construct, std::make_tuple(srcPath), std::make_tuple());
        }
        compact.at(srcPath).emplace_back(outputLine, srcLine);
    }
    dst = json::to_msgpack(compact);
}
}