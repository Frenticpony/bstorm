#pragma once

#include <unordered_map>
#include <memory>

namespace bstorm
{
struct NodeDef;
typedef std::unordered_map<std::string, std::shared_ptr<NodeDef>> NameTable;

struct Env
{
    std::shared_ptr<NodeDef> findDef(const std::string& name) const;
    bool isRoot() const;
    NameTable table;
    std::shared_ptr<Env> parent;
};
}