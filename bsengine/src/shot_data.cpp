#include <algorithm>

#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/shot_data.hpp>

namespace bstorm {
  ShotData::ShotData() :
    id(ID_INVALID),
    rect(0, 0, 0, 0),
    render(BLEND_ALPHA),
    alpha(0xff),
    delayRect(0, 0, 0, 0),
    useDelayRect(false),
    delayColor(0, 0, 0),
    useDelayColor(false),
    delayRender(BLEND_ADD_ARGB),
    angularVelocity(0),
    useAngularVelocityRand(false),
    angularVelocityRandMin(0),
    angularVelocityRandMax(0),
    fixedAngle(false)
  {
  }

  ShotDataTable::ShotDataTable() {
  }

  ShotDataTable::~ShotDataTable() {
  }

  void ShotDataTable::add(const std::shared_ptr<ShotData>& data) {
    if (data->texture) {
      table[data->id] = data;
    }
  }

  void ShotDataTable::reload(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache) {
    std::wstring uniqPath = canonicalPath(path);
    auto userShotData = parseUserShotData(uniqPath, loader);
    auto texture = textureCache->load(userShotData->imagePath, false);
    for (auto& entry : userShotData->dataMap) {
      auto& data = entry.second;
      if (!data.useDelayRect) {
        data.delayRect = userShotData->delayRect;
      }

      if (!data.useDelayColor) {
        data.delayColor = userShotData->delayColor;
      }
      if (data.collisions.empty()) {
        float width = std::abs(data.rect.right - data.rect.left);
        float height = std::abs(data.rect.bottom - data.rect.top);
        float r = std::max(std::max(width, height) / 1.5f - 1.5f, 3.0f) / 2.0f;
        data.collisions.push_back({ r, 0.0f, 0.0f });
      }
      data.texture = texture;
      table[data.id] = std::make_shared<ShotData>(data);
    }
    loadedPaths.insert(uniqPath);
  }

  bool ShotDataTable::isLoaded(const std::wstring & path) const {
    return loadedPaths.count(canonicalPath(path)) != 0;
  }

  std::shared_ptr<ShotData> ShotDataTable::get(int id) const {
    auto it = table.find(id);
    if (it != table.end()) {
      return it->second;
    }
    return nullptr;
  }
}