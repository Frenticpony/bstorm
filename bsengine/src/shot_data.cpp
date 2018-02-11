#include <algorithm>

#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/logger.hpp>
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

  ShotDataTable::ShotDataTable(Type type) :
    type(type)
  {
  }

  ShotDataTable::~ShotDataTable() {
  }

  void ShotDataTable::add(const std::shared_ptr<ShotData>& data) {
    if (data->texture) {
      table[data->id] = data;
    }
  }

  const char * ShotDataTable::getTypeName(Type type) {
    if (type == Type::PLAYER) return "player";
    if (type == Type::ENEMY) return "enemy";
    return "unknown";
  }

  static Log::Param::Tag getElemTag(ShotDataTable::Type type) {
    if (type == ShotDataTable::Type::PLAYER) return Log::Param::Tag::PLAYER_SHOT_DATA;
    if (type == ShotDataTable::Type::ENEMY) return Log::Param::Tag::ENEMY_SHOT_DATA;
    return Log::Param::Tag::TEXT;
  }

  void ShotDataTable::load(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos) {
    if (isLoaded(path)) {
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage(std::string(getTypeName(type)) + " shot data already loaded.")
        .setParam(Log::Param(getElemTag(type), path))
        .addSourcePos(srcPos)));
    } else {
      reload(path, loader, textureCache, srcPos);
    }
  }

  void ShotDataTable::reload(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<SourcePos>& srcPos) {
    std::wstring uniqPath = canonicalPath(path);
    auto userShotData = parseUserShotData(uniqPath, loader);
    auto texture = textureCache->load(userShotData->imagePath, false, srcPos);
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
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("load " + std::string(getTypeName(type)) + " shot data.")
      .setParam(Log::Param(getElemTag(type), path))
      .addSourcePos(srcPos)));
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

  ShotDataTable::Type ShotDataTable::getType() const {
    return type;
  }
}