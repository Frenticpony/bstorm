#pragma once

#include <bstorm/type.hpp>

#include <string>
#include <memory>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace bstorm {
  struct SourcePos;
  class Texture;
  class FileLoader;
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
    void load(const std::wstring& path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);
    void reload(const std::wstring& path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos);
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