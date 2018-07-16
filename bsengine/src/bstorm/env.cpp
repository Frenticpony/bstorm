#include <bstorm/env.hpp>

namespace bstorm
{
Env::Env() : parent_(nullptr) {}

Env::Env(const std::shared_ptr<Env>& parent) : parent_(parent) {}

void Env::AddDef(const std::string & name, const std::shared_ptr<NodeDef>& def)
{
    AddDef(std::move(std::string(name)), std::move(std::shared_ptr<NodeDef>(def)));
}

void Env::AddDef(std::string && name, std::shared_ptr<NodeDef>&& def)
{
    table_.emplace(std::move(name), std::move(def));
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