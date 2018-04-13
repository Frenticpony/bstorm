#include <bstorm/item_data.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/logger.hpp>

#include <algorithm>

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

  void ItemDataTable::load(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos) {
    if (isLoaded(path)) {
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("item data already loaded.")
        .setParam(Log::Param(Log::Param::Tag::ITEM_DATA, path))
        .addSourcePos(srcPos)));
    } else {
      reload(path, loader, textureCache, srcPos);
    }
  }

  void ItemDataTable::reload(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos) {
    std::wstring uniqPath = canonicalPath(path);
    auto userItemData = parseUserItemData(uniqPath, loader);
    auto texture = textureCache->load(userItemData->imagePath, false, srcPos);
    for (auto& entry : userItemData->dataMap) {
      auto& data = entry.second;
      data.texture = texture;
      table[data.id] = std::make_shared<ItemData>(data);
    }
    loadedPaths.insert(uniqPath);
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("load item data.")
      .setParam(Log::Param(Log::Param::Tag::ITEM_DATA, path))
      .addSourcePos(srcPos)));
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