#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <map>
#include <unordered_map>
#include <memory>

namespace bstorm
{
class DnhValue;
class ObjectTable;
class Package;
class Obj : private NonCopyable
{
public:
    using Type = uint8_t;
    Obj(const std::shared_ptr<Package>& state);
    virtual ~Obj();
    virtual void Update() {}
    const std::unique_ptr<DnhValue>& GetValue(const std::wstring& key) const;
    const std::unique_ptr<DnhValue>& GetValueD(const std::wstring& key, const std::unique_ptr<DnhValue>& defaultValue) const;
    void SetValue(const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    void DeleteValue(const std::wstring& key);
    bool IsValueExists(const std::wstring& key) const;
    int GetID() const { return id_; }
    Type GetType() const { return type_; }
    bool IsDead() const { return isDead_; }
    bool IsStgSceneObject() const { return isStgSceneObj_; }
    void SetStgSceneObject(bool b) { isStgSceneObj_ = b; }
    const std::unordered_map<std::wstring, std::unique_ptr<DnhValue>>& GetProperties() const;
protected:
    void SetType(Type t) { type_ = t; }
    void Die() noexcept
    {
        if (isDead_) return;
        OnDead();
        isDead_ = true;
    };
    virtual void OnDead() noexcept {}
    const std::weak_ptr<Package>& GetPackage() const { return package_; }
private:
    int id_;
    Type type_;
    bool isDead_;
    std::unordered_map<std::wstring, std::unique_ptr<DnhValue>> properties_;
    std::weak_ptr<Package> package_;
    bool isStgSceneObj_;
    friend class ObjectTable;
};

class ObjectTable
{
public:
    ObjectTable();
    ~ObjectTable();
    template <class T>
    NullableSharedPtr<T> Get(int id)
    {
        if (objCache_->GetID() == id && !objCache_->IsDead())
        {
            return std::dynamic_pointer_cast<T>(objCache_);
        }

        auto it = table_.find(id);
        if (it != table_.end() && !it->second->IsDead())
        {
            objCache_ = it->second;
            return std::dynamic_pointer_cast<T>(objCache_);
        }
        return nullptr;
    }
    template <>
    NullableSharedPtr<Obj> Get(int id)
    {
        if (objCache_->GetID() == id && !objCache_->IsDead())
        {
            return objCache_;
        }

        auto it = table_.find(id);
        if (it != table_.end() && !it->second->IsDead())
        {
            return objCache_ = it->second;
        }
        return nullptr;
    }
    template <class T, class... Args>
    std::shared_ptr<T> Create(Args&&... args)
    {
        std::shared_ptr<T> obj = std::make_shared<T>(std::forward<Args>(args)...);
        obj->id_ = idGen_++;
        table_.emplace_hint(table_.end(), obj->id_, obj);
        return obj;
    }
    void Delete(int id);
    bool IsDeleted(int id);
    void UpdateAll(bool ignoreStgSceneObj);
    void DeleteStgSceneObject();
    const std::map<int, std::shared_ptr<Obj>>& GetAll();
private:
    int idGen_;
    std::map<int, std::shared_ptr<Obj>> table_;
    std::shared_ptr<Obj> objCache_;
    bool isUpdating_;
};
}