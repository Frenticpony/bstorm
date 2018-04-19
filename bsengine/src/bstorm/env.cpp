#include <bstorm/env.hpp>

#include <bstorm/node.hpp>

namespace bstorm
{
std::shared_ptr<NodeDef> Env::findDef(const std::string & name) const
{
    auto it = table.find(name);
    if (it != table.end())
    {
        return it->second;
    } else
    {
        if (parent)
        {
            return parent->findDef(name);
        } else
        {
            return nullptr;
        }
    }
}

bool Env::isRoot() const
{
    if (parent) return false;
    return true;
}
}