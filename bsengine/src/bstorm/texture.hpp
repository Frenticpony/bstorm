#pragma once

#include <bstorm/non_copyable.hpp>

#include <string>
#include <memory>
#include <unordered_map>
#include <d3d9.h>
#include <mutex>
#include <thread>
#include <atomic>

namespace bstorm
{
IDirect3DTexture9* LoadTextureFromFile(const std::wstring& path, IDirect3DDevice9*) noexcept(true);

struct SourcePos;
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
    void Reload(IDirect3DTexture9* d3DTex);
private:
    const std::wstring path_;
    int width_;
    int height_;
    std::atomic<bool> reservedFlag_;
    IDirect3DTexture9* d3DTexture_;
};

class TextureCache
{
public:
    TextureCache(IDirect3DDevice9* d3DDevice_);
    ~TextureCache();
    std::shared_ptr<Texture> Load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void LoadInThread(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void Reload(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void RemoveReservedFlag(const std::wstring& path);
    void ReleaseUnusedTexture();
    // backdoor
    template <typename T>
    void BackDoor() {} // NOTE: 排他制御忘れないように
private:
    std::shared_ptr<Texture> GetTexture(const std::wstring& uniqPath) const;
    std::shared_ptr<Texture> LoadFirst(const std::wstring& uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    IDirect3DDevice9* d3DDevice_;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> textureMap_;
    mutable std::recursive_mutex textureLoadSection_; // lock1
    mutable std::recursive_mutex memberAccessSection_; // lock2 この順でlock
    std::unordered_map<std::wstring, std::thread> loadThreads_;
};

void GetD3DTextureSize(IDirect3DTexture9* texture, int* width, int* height);
int GetD3DTextureWidth(IDirect3DTexture9* texture);
int GetD3DTextureHeight(IDirect3DTexture9* texture);
}