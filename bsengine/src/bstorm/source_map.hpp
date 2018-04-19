#pragma once

#include <string>
#include <memory>
#include <map>

namespace bstorm
{
struct SourcePos
{
    int line;
    int column;
    std::shared_ptr<std::wstring> filename;
    std::string toString() const;
};

struct SourceLoc
{
    SourcePos begin;
    SourcePos end;
};

class SourceMap
{
public:
    void logSourcePos(int outputLine, const std::shared_ptr<std::wstring>& path, int srcLine);
    std::shared_ptr<SourcePos> getSourcePos(int outputLine) const;
private:
    std::map<int, SourcePos> srcMap;
};
}