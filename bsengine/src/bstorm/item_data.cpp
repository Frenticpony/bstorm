#include <bstorm/item_data.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/file_util.hpp>
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

ItemDataTable::ItemDataTable(const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader) :
    textureStore_(textureStore),
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
        table_.emplace(data->id, data);
    }
}

void ItemDataTable::Load(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (IsLoaded(path))
    {
        Logger::Write(std::move(
            Log(LogLevel::LV_WARN)
            .Msg("item data already loaded.")
            .Param(LogParam(LogParam::Tag::ITEM_DATA, path))
            .AddSourcePos(srcPos)));
    } else
    {
        Reload(path, srcPos);
    }
}

void ItemDataTable::Reload(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    std::wstring uniqPath = GetCanonicalPath(path);
    auto userItemData = ParseUserItemData(uniqPath, fileLoader_);
    auto& texture = textureStore_->Load(userItemData->imagePath);
    for (auto& entry : userItemData->dataMap)
    {
        auto& data = entry.second;
        data.texture = texture;
        table_.emplace(data.id, std::make_shared<ItemData>(data));
    }
    loadedPaths_.insert(uniqPath);
    Logger::Write(std::move(
        Log(LogLevel::LV_INFO)
        .Msg("load item data.")
        .Param(LogParam(LogParam::Tag::ITEM_DATA, path))
        .AddSourcePos(srcPos)));
}

bool ItemDataTable::IsLoaded(const std::wstring & path) const
{
    return loadedPaths_.count(GetCanonicalPath(path)) != 0;
}

NullableSharedPtr<ItemData> ItemDataTable::Get(int id) const
{
    auto it = table_.find(id);
    if (it != table_.end())
    {
        return it->second;
    }
    return nullptr;
}
}