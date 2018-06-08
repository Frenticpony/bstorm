#include <bstorm/obj.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/dnh_value.hpp>

namespace bstorm
{
Obj::Obj(const std::shared_ptr<Package>& state) :
    id_(ID_INVALID),
    type_(-1),
    isDead_(false),
    isStgSceneObj_(true),
    package_(state)
{
}

Obj::~Obj()
{
    Die();
}

const std::unique_ptr<DnhValue>& Obj::GetValue(const std::wstring& key) const
{
    return GetValueD(key, DnhValue::Nil());
}

const std::unique_ptr<DnhValue>& Obj::GetValueD(const std::wstring& key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    auto it = properties_.find(key);
    if (it != properties_.end())
    {
        return it->second;
    }
    return defaultValue;
}

void Obj::SetValue(const std::wstring& key, std::unique_ptr<DnhValue>&& value)
{
    properties_[key] = std::move(value);
}

void Obj::DeleteValue(const std::wstring& key)
{
    properties_.erase(key);
}

bool Obj::IsValueExists(const std::wstring& key) const
{
    return properties_.count(key) != 0;
}

const std::unordered_map<std::wstring, std::unique_ptr<DnhValue>>& Obj::GetProperties() const
{
    return properties_;
}

ObjectTable::ObjectTable() :
    idGen_(0),
    isUpdating_(false)
{
}

ObjectTable::~ObjectTable()
{
}

void ObjectTable::Add(const std::shared_ptr<Obj>& obj)
{
    if (obj->id_ != ID_INVALID) return;
    table_[idGen_] = obj;
    obj->id_ = idGen_;
    idGen_++;
}

void ObjectTable::Delete(int id)
{
    auto it = table_.find(id);
    if (it != table_.end())
    {
        it->second->Die();
        if (!isUpdating_)
        {
            table_.erase(it);
        }
    }
}

bool ObjectTable::IsDeleted(int id)
{
    auto it = table_.find(id);
    if (it != table_.end())
    {
        return it->second->IsDead();
    } else
    {
        return true;
    }
}

void ObjectTable::UpdateAll(bool ignoreStgSceneObj)
{
    isUpdating_ = true;
    auto it = table_.begin();
    while (it != table_.end())
    {
        auto& obj = it->second;
        if (!obj->IsDead())
        {
            if (!ignoreStgSceneObj || !obj->IsStgSceneObject())
            {
                obj->Update();
            }
        }
        if (obj->IsDead())
        {
            table_.erase(it++);
        } else
        {
            ++it;
        }
    }
    isUpdating_ = false;
}

void ObjectTable::DeleteStgSceneObject()
{
    auto it = table_.begin();
    while (it != table_.end())
    {
        if (it->second->IsStgSceneObject())
        {
            table_.erase(it++);
        } else
        {
            ++it;
        }
    }
}
const std::map<int, std::shared_ptr<Obj>>& ObjectTable::GetAll()
{
    return table_;
}
}