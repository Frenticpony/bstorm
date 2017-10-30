#pragma once

#include <string>
#include <memory>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <bstorm/type.hpp>
#include <bstorm/file_loader.hpp>

namespace bstorm {
  class Texture;
  class ItemData {
  public:
    ItemData();
    int id;
    int type;
    Rect<int> rect;
    Rect<int> out;
    int render;
    AnimationData animationData;
    std::shared_ptr<Texture> texture;
  };

  class UserItemData {
  public:
    std::wstring path;
    std::wstring imagePath;
    std::unordered_map<int, ItemData> dataMap;
  };

  class TextureCache;
  class ItemDataTable {
  public:
    ItemDataTable();
    ~ItemDataTable();
    void add(const std::shared_ptr<ItemData>& data);
    void reload(const std::wstring& path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache);
    bool isLoaded(const std::wstring& path) const;
    std::shared_ptr<ItemData> get(int id) const;
    /* backdoor */
    template <typename T>
    void backDoor() const {}
  private:
    std::unordered_set<std::wstring> loadedPaths;
    std::map<int, std::shared_ptr<ItemData>> table;
  };
}