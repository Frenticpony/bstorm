#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <string>
#include <memory>
#include <unordered_map>
#include <future>
#include <d3d9.h>

namespace bstorm
{
IDirect3DTexture9* LoadTextureFromFile(const std::wstring& path, IDirect3DDevice9*) noexcept(true);

class Texture : private NonCopyable
{
public:
    Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture);
    ~Texture();
    const std::wstring& GetPath() const;
    int GetWidth() const;
    int GetHeight() const;
    void SetReservedFlag(bool flag);
    bool IsReserved() const;
    IDirect3DTexture9* GetTexture() const;
    void Reset(IDirect3DTexture9 * d3DTex);
private:
    const std::wstring path_;
    int width_;
    int height_;
    bool reservedFlag_;
    IDirect3DTexture9* d3DTexture_;
};

struct SourcePos;
class TextureCache
{
public:
    TextureCache(IDirect3DDevice9* d3DDevice_);
    ~TextureCache();
    std::shared_ptr<Texture> Load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_future<std::shared_ptr<Texture>> LoadInThread(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void Reload(const std::wstring& p, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void RemoveReservedFlag(const std::wstring& path);
    void ReleaseUnusedTexture();
    // backdoor
    template <typename T>
    void BackDoor() {}
private:
    IDirect3DDevice9* d3DDevice_;
    std::unordered_map<std::wstring, std::shared_future<std::shared_ptr<Texture>>> textureMap_;
};

void GetD3DTextureSize(IDirect3DTexture9* texture, int* width, int* height);
int GetD3DTextureWidth(IDirect3DTexture9* texture);
int GetD3DTextureHeight(IDirect3DTexture9* texture);
}