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
IDirect3DTexture9* loadTextureFromFile(const std::wstring& path, IDirect3DDevice9*) noexcept(true);

struct SourcePos;
class Texture : private NonCopyable
{
public:
    Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture);
    ~Texture();
    const std::wstring& getPath() const;
    int getWidth() const;
    int getHeight() const;
    void setReservedFlag(bool flag);
    bool isReserved() const;
    IDirect3DTexture9* getTexture() const;
    void reload(IDirect3DTexture9* d3DTex);
private:
    const std::wstring path;
    int width;
    int height;
    std::atomic<bool> reservedFlag;
    IDirect3DTexture9* d3DTexture;
};

class TextureCache
{
public:
    TextureCache(IDirect3DDevice9* d3DDevice);
    ~TextureCache();
    std::shared_ptr<Texture> load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void loadInThread(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void reload(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void removeReservedFlag(const std::wstring& path);
    void releaseUnusedTexture();
    // backdoor
    template <typename T>
    void backDoor() {} // NOTE: 排他制御忘れないように
private:
    std::shared_ptr<Texture> getTexture(const std::wstring& uniqPath) const;
    std::shared_ptr<Texture> loadFirst(const std::wstring& uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    IDirect3DDevice9* d3DDevice;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> m_textureMap;
    mutable std::recursive_mutex textureLoadSection; // lock1
    mutable std::recursive_mutex memberAccessSection; // lock2 この順でlock
    std::unordered_map<std::wstring, std::thread> m_loadThreads;
};

void getD3DTextureSize(IDirect3DTexture9* texture, int& width, int& height);
int getD3DTextureWidth(IDirect3DTexture9* texture);
int getD3DTextureHeight(IDirect3DTexture9* texture);
}