#include <bstorm/source_map.hpp>

#include <bstorm/util.hpp>

#include <yas/mem_streams.hpp>
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/std_types.hpp>
#include <yas/buffers.hpp>

#include <deque>

namespace bstorm
{
std::string SourcePos::ToString() const
{
    return ToUTF8(*filename) + ":" + std::to_string(line) + ((column >= 0) ? (":" + std::to_string(column)) : "");
}

// シリアライズ用の中間表現
// <srcPath, <outputLine, srcLine>>
using CompactSourceMap = std::map<std::string, std::deque<std::pair<int32_t, int32_t>>>;
constexpr auto yas_option = yas::binary | yas::no_header | yas::compacted;

SourceMap::SourceMap() {}

SourceMap::SourceMap(const std::string& data)
{
    CompactSourceMap compact;
    yas::mem_istream is(data.data(), data.size());
    yas::binary_iarchive<yas::mem_istream, yas_option> ia(is);
    ia & compact;
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

void SourceMap::Serialize(std::string& data) const
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
    yas::mem_ostream os;
    yas::binary_oarchive<yas::mem_ostream, yas_option> oa(os);
    oa & compact;
    auto buf = os.get_intrusive_buffer();
    data.assign(buf.data, buf.size);
}
}