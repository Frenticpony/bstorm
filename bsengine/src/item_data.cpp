#include <algorithm>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/item_data.hpp>

namespace bstorm {
  ItemData::ItemData() :
    id(ID_INVALID),
    type(ID_INVALID),
    rect(0, 0, 0, 0),
    out(0, 0, 0, 0),
    render(BLEND_ALPHA)
  {
  }

  ItemDataTable::ItemDataTable() {
  }

  ItemDataTable::~ItemDataTable() {
  }

  void ItemDataTable::add(const std::shared_ptr<ItemData>& data) {
    if (data->texture) {
      table[data->id] = data;
    }
  }

  void ItemDataTable::reload(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache) {
    std::wstring uniqPath = canonicalPath(path);
    auto userItemData = parseUserItemData(uniqPath, loader);
    auto texture = textureCache->load(userItemData->imagePath, false);
    for (auto& entry : userItemData->dataMap) {
      auto& data = entry.second;
      data.texture = texture;
      table[data.id] = std::make_shared<ItemData>(data);
    }
    loadedPaths.insert(uniqPath);
  }

  bool ItemDataTable::isLoaded(const std::wstring & path) const {
    return loadedPaths.count(canonicalPath(path)) != 0;
  }

  std::shared_ptr<ItemData> ItemDataTable::get(int id) const {
    auto it = table.find(id);
    if (it != table.end()) {
      return it->second;
    }
    return nullptr;
  }
}