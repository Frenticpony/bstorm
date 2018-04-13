#include <bstorm/obj.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/dnh_value.hpp>

namespace bstorm {
  Obj::Obj(const std::shared_ptr<GameState>& state) :
    id(ID_INVALID),
    type(-1),
    deadFlag(false),
    stgSceneObj(true),
    gameState(state)
  {
  }

  Obj::~Obj() {}

  std::unique_ptr<DnhValue> Obj::getValue(const std::wstring& key) const {
    return getValueD(key, std::make_unique<DnhNil>());
  }

  std::unique_ptr<DnhValue> Obj::getValueD(const std::wstring& key, std::unique_ptr<DnhValue>&& defaultValue) const {
    auto it = properties.find(key);
    if (it != properties.end()) {
      return it->second->clone();
    }
    return std::move(defaultValue);
  }

  void Obj::setValue(const std::wstring& key, std::unique_ptr<DnhValue>&& value) {
    properties[key] = std::move(value);
  }

  void Obj::deleteValue(const std::wstring& key) {
    properties.erase(key);
  }

  bool Obj::isValueExists(const std::wstring& key) const {
    return properties.count(key) != 0;
  }

  const std::unordered_map<std::wstring, std::unique_ptr<DnhValue>>& Obj::getProperties() const {
    return properties;
  }

  std::shared_ptr<GameState> Obj::getGameState() const { return gameState.lock(); }

  ObjectTable::ObjectTable() :
    idGen(0),
    isUpdating(false)
  {
  }

  ObjectTable::~ObjectTable() {}

  void ObjectTable::add(const std::shared_ptr<Obj>& obj) {
    if (obj->id != ID_INVALID) return;
    table[idGen] = obj;
    obj->id = idGen;
    idGen++;
  }

  void ObjectTable::del(int id) {
    auto it = table.find(id);
    if (it != table.end()) {
      it->second->die();
      if (!isUpdating) {
        table.erase(it);
      }
    }
  }

  bool ObjectTable::isDeleted(int id) {
    auto it = table.find(id);
    if (it != table.end()) {
      return it->second->isDead();
    } else {
      return true;
    }
  }

  void ObjectTable::updateAll(bool ignoreStgSceneObj) {
    isUpdating = true;
    auto it = table.begin();
    while (it != table.end()) {
      auto& obj = it->second;
      if (!obj->isDead()) {
        if (!ignoreStgSceneObj || !obj->isStgSceneObject()) {
          obj->update();
        }
      }
      if (obj->isDead()) {
        table.erase(it++);
      } else {
        ++it;
      }
    }
    isUpdating = false;
  }

  void ObjectTable::deleteStgSceneObject() {
    auto it = table.begin();
    while (it != table.end()) {
      if (it->second->isStgSceneObject()) {
        table.erase(it++);
      } else {
        ++it;
      }
    }
  }
  const std::map<int, std::shared_ptr<Obj>>& ObjectTable::getAll() {
    return table;
  }
}