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
    typedef int Type;
    Obj(const std::shared_ptr<GameState>& state);
    virtual ~Obj();
    virtual void update() = 0;
    std::unique_ptr<DnhValue> getValue(const std::wstring& key) const;
    std::unique_ptr<DnhValue> getValueD(const std::wstring& key, std::unique_ptr<DnhValue>&& defaultValue) const;
    void setValue(const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    void deleteValue(const std::wstring& key);
    bool isValueExists(const std::wstring& key) const;
    int getID() const { return id; }
    Type GetType() const { return type; }
    bool isDead() const { return deadFlag; }
    bool isStgSceneObject() const { return stgSceneObj; }
    void setStgSceneObject(bool b) { stgSceneObj = b; }
    const std::unordered_map<std::wstring, std::unique_ptr<DnhValue>>& getProperties() const;
protected:
    void setType(Type t) { type = t; }
    void die() { deadFlag = true; };
    std::shared_ptr<GameState> getGameState() const;
private:
    int id;
    Type type;
    bool deadFlag;
    std::unordered_map<std::wstring, std::unique_ptr<DnhValue>> properties;
    std::weak_ptr<GameState> gameState;
    bool stgSceneObj;
    friend ObjectTable;
};

class ObjectTable
{
public:
    ObjectTable();
    ~ObjectTable();
    template <class T>
    std::shared_ptr<T> Get(int id)
    {
        auto it = table.find(id);
        if (it != table.end() && !it->second->isDead())
        {
            return std::dynamic_pointer_cast<T>(it->second);
        }
        return nullptr;
    }
    void add(const std::shared_ptr<Obj>& obj);
    template <class T, class... Args>
    std::shared_ptr<T> create(Args... args)
    {
        std::shared_ptr<T> obj = std::make_shared<T>(args...);
        add(obj);
        return obj;
    }
    void del(int id);
    bool isDeleted(int id);
    void updateAll(bool ignoreStgSceneObj);
    void deleteStgSceneObject();
    const std::map<int, std::shared_ptr<Obj>>& getAll();
private:
    int idGen;
    std::map<int, std::shared_ptr<Obj>> table;
    bool isUpdating;
};
}