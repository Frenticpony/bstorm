#include <bstorm/env.hpp>

#include <bstorm/node.hpp>

#include <cassert>

namespace bstorm
{
Env::Env() : parent_(nullptr), depth_(0) {}

Env::Env(const std::shared_ptr<Env>& parent) : parent_(parent), depth_(parent->depth_ + 1) {}

const std::shared_ptr<NodeDef>& Env::AddDef(const std::string & name, const std::shared_ptr<NodeDef>& def)
{
    return AddDef(std::move(std::string(name)), std::move(std::shared_ptr<NodeDef>(def)));
}

static std::string getShortName(size_t i, int depth)
{
    constexpr char chars[33] = "_aAbBcCdDeEfghijklmnopqrstuvwxyz"; // 32-chars
    constexpr size_t charCnt = sizeof(chars) - 1;
    std::string name;
    while (true)
    {
        name += chars[i & (charCnt - 1)];
        i /= charCnt;
        if (i == 0)
        {
            break;
        }
    }
    if (depth > 0)
    {
        name += std::to_string(depth);
    }
    return name;
}

const std::shared_ptr<NodeDef>& Env::AddDef(std::string && name, std::shared_ptr<NodeDef>&& def)
{
    if (table_.count(name) != 0) { return table_[name]; }
#ifndef _DEBUG
    def->convertedName = std::move(getShortName(table_.size(), depth_));
#endif
    return table_[std::move(name)] = std::move(def);
}

NullableSharedPtr<NodeDef> Env::FindDef(const std::string & name) const
{
    auto it = table_.find(name);
    if (it != table_.end())
    {
        return it->second;
    } else
    {
        if (parent_)
        {
            return parent_->FindDef(name);
        } else
        {
            return nullptr;
        }
    }
}

bool Env::IsRoot() const
{
    if (parent_) return false;
    return true;
}
}