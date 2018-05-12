#include <bstorm/item_data.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/logger.hpp>

#include <algorithm>

namespace bstorm
{
ItemData::ItemData() :
    id(ID_INVALID),
    type(ID_INVALID),
    rect(0, 0, 0, 0),
    out(0, 0, 0, 0),
    render(BLEND_ALPHA)
{
}

ItemDataTable::ItemDataTable(const std::shared_ptr<TextureCache>& textureCache, const std::shared_ptr<FileLoader>& fileLoader) :
    textureCache_(textureCache),
    fileLoader_(fileLoader)
{
}

ItemDataTable::~ItemDataTable()
{
}

void ItemDataTable::Add(const std::shared_ptr<ItemData>& data)
{
    if (data->texture)
    {
        table_[data->id] = data;
    }
}

void ItemDataTable::Load(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<SourcePos>& srcPos)
{
    if (IsLoaded(path))
    {
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("item data already loaded.")
            .SetParam(Log::Param(Log::Param::Tag::ITEM_DATA, path))
            .AddSourcePos(srcPos)));
    } else
    {
        Reload(path, loader, srcPos);
    }
}

void ItemDataTable::Reload(const std::wstring & path, const std::shared_ptr<FileLoader>& loader, const std::shared_ptr<SourcePos>& srcPos)
{
    std::wstring uniqPath = GetCanonicalPath(path);
    auto userItemData = ParseUserItemData(uniqPath, loader);
    auto texture = textureCache_->Load(userItemData->imagePath, false, srcPos);
    for (auto& entry : userItemData->dataMap)
    {
        auto& data = entry.second;
        data.texture = texture;
        table_[data.id] = std::make_shared<ItemData>(data);
    }
    loadedPaths_.insert(uniqPath);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("load item data.")
        .SetParam(Log::Param(Log::Param::Tag::ITEM_DATA, path))
        .AddSourcePos(srcPos)));
}

bool ItemDataTable::IsLoaded(const std::wstring & path) const
{
    return loadedPaths_.count(GetCanonicalPath(path)) != 0;
}

std::shared_ptr<ItemData> ItemDataTable::Get(int id) const
{
    auto it = table_.find(id);
    if (it != table_.end())
    {
        return it->second;
    }
    return nullptr;
}
}