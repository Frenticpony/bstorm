#pragma once

#include <string>
#include <memory>
#include <map>
#include <cstdint>
#include <vector>

namespace bstorm
{
struct SourcePos
{
    SourcePos() : line(-1), column(-1), filename(nullptr) { }
    SourcePos(int line, int column, const std::shared_ptr<std::wstring>& path) :
        line(line), column(column), filename(path) {}
    int line;
    int column;
    std::shared_ptr<std::wstring> filename;
    std::string ToString() const;
};

struct SourceLoc
{
    SourcePos begin;
    SourcePos end;
};

class SourceMap
{
public:
    SourceMap();
    SourceMap(const std::string& data); // from serialized data.
    void LogSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine);
    std::shared_ptr<SourcePos> GetSourcePos(int outputLine) const;
    void Serialize(std::string& data) const;
private:
    std::map<int, SourcePos> srcMap_;
};
}