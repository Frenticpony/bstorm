#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/cache_store.hpp> 

#include <string>
#include <memory>
#include <unordered_map>
#include <future>
#include <d3d9.h>

namespace bstorm
{
class GraphicDevice;

class Texture : private NonCopyable
{
public:
    Texture(const std::wstring& path, const std::shared_ptr<GraphicDevice>& graphicDevice);
    ~Texture();
    const std::wstring& GetPath() const;
    int GetWidth() const;
    int GetHeight() const;
    bool IsReserved() const;
    IDirect3DTexture9* GetTexture() const;
    void Reload();
private:
    const std::wstring path_;
    int width_;
    int height_;
    IDirect3DTexture9* d3DTexture_;
    const std::shared_ptr<GraphicDevice> graphicDevice_;
};

struct SourcePos;
class TextureStore
{
public:
    TextureStore(const std::shared_ptr<GraphicDevice>& graphicDevice);
    ~TextureStore();
    const std::shared_ptr<Texture>& Load(const std::wstring& path);
    void LoadInThread(const std::wstring & path);
    void SetReserveFlag(const std::wstring& path, bool reserve);
    bool IsReserved(const std::wstring& path) const;
    void RemoveUnusedTexture();
    bool IsLoadCompleted(const std::wstring & path) const;
    template <class Fn>
    void ForEach(Fn func)
    {
        cacheStore_.ForEach(func);
    }
private:
    CacheStore<std::wstring, Texture> cacheStore_;
    const std::shared_ptr<GraphicDevice> graphicDevice_;
};

void GetD3DTextureSize(IDirect3DTexture9* texture, int* width, int* height);
int GetD3DTextureWidth(IDirect3DTexture9* texture);
int GetD3DTextureHeight(IDirect3DTexture9* texture);
}