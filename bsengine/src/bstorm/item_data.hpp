﻿#pragma once

#include <bstorm/rect.hpp>
#include <bstorm/animation.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <string>
#include <memory>
#include <map>
#include <unordered_set>
#include <unordered_map>

namespace bstorm
{
struct SourcePos;
class Texture;
class FileLoader;
class ItemData
{
public:
    ItemData();
    int id;
    int type;
    Rect<int> rect;
    Rect<int> out;
    int render;
    int filter; //FP FILTER
    AnimationData animationData;
    std::shared_ptr<Texture> texture;
};

class UserItemData
{
public:
    std::wstring path;
    std::wstring imagePath;
    std::unordered_map<int, ItemData> dataMap;
};

class TextureStore;
class ItemDataTable
{
public:
    ItemDataTable(const std::shared_ptr<TextureStore>& textureStore, const std::shared_ptr<FileLoader>& fileLoader);
    ~ItemDataTable();
    void Add(const std::shared_ptr<ItemData>& data);
    void Load(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void Reload(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    bool IsLoaded(const std::wstring& path) const;
    NullableSharedPtr<ItemData> Get(int id) const;
    /* backdoor */
    template <typename T>
    void BackDoor() const {}
private:
    const std::shared_ptr<TextureStore> textureStore_;
    const std::shared_ptr<FileLoader> fileLoader_;
    std::unordered_set<std::wstring> loadedPaths_;
    std::map<int, std::shared_ptr<ItemData>> table_;
};
}