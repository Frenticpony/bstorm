#pragma once

#include <bstorm/non_copyable.hpp>

#include <map>
#include <unordered_map>
#include <memory>

namespace bstorm
{
class DnhValue;
class ObjectTable;
class GameState;
class Obj : private NonCopyable
{
public:
    using Type = int;
    Obj(const std::shared_ptr<GameState>& state);
    virtual ~Obj();
    virtual void Update() = 0;
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
    void Die() { isDead_ = true; };
    std::shared_ptr<GameState> GetGameState() const;
private:
    int id_;
    Type type_;
    bool isDead_;
    std::unordered_map<std::wstring, std::unique_ptr<DnhValue>> properties_;
    std::weak_ptr<GameState> gameState_;
    bool isStgSceneObj_;
    friend class ObjectTable;
};

class ObjectTable
{
public:
    ObjectTable();
    ~ObjectTable();
    template <class T>
    std::shared_ptr<T> Get(int id)
    {
        auto it = table_.find(id);
        if (it != table_.end() && !it->second->IsDead())
        {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    void Add(const std::shared_ptr<Obj>& obj);
    template <class T, class... Args>
    std::shared_ptr<T> Create(Args... args)
    {
        std::shared_ptr<T> obj = std::make_shared<T>(args...);
        Add(obj);
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
    bool isUpdating_;
};
}