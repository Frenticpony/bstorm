#include <bstorm/env.hpp>

#include <bstorm/node.hpp>

namespace bstorm
{
std::shared_ptr<NodeDef> Env::FindDef(const std::string & name) const
{
    auto it = table.find(name);
    if (it != table.end())
    {
        return it->second;
    } else
    {
        if (parent)
        {
            return parent->FindDef(name);
        } else
        {
            return nullptr;
        }
    }
}

bool Env::IsRoot() const
{
    if (parent) return false;
    return true;
}
}