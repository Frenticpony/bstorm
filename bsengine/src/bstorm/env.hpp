#pragma once

#include <bstorm/nullable_shared_ptr.hpp>

#include <unordered_map>
#include <memory>

namespace bstorm
{
struct NodeDef;
using NameTable = std::unordered_map<std::string, std::shared_ptr<NodeDef>>;

struct Env
{
    NullableSharedPtr<NodeDef> FindDef(const std::string& name) const;
    bool IsRoot() const;
    NameTable table;
    std::shared_ptr<Env> parent;
};
}