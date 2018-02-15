#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <d3d9.h>
#include <mutex>
#include <thread>
#include <atomic>

#include <bstorm/non_copyable.hpp>

namespace bstorm {
  class Texture : private NonCopyable {
  public:
    Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture);
    ~Texture();
    const std::wstring& getPath() const;
    int getWidth() const;
    int getHeight() const;
    void setReservedFlag(bool flag);
    bool isReserved() const;
    IDirect3DTexture9* getTexture() const;
  private:
    std::wstring path;
    int width;
    int height;
    std::atomic<bool> reservedFlag;
    IDirect3DTexture9* d3DTexture;
  };

  class TextureLoader {
  public:
    virtual ~TextureLoader() {}
    virtual IDirect3DTexture9* loadTexture(const std::wstring& path, IDirect3DDevice9*) = 0;
  };

  class TextureLoaderFromImageFile : public TextureLoader {
    IDirect3DTexture9* loadTexture(const std::wstring& path, IDirect3DDevice9*) override;
  };

  struct SourcePos;
  class TextureCache {
  public:
    TextureCache(IDirect3DDevice9* d3DDevice);
    ~TextureCache();
    void setLoader(const std::shared_ptr<TextureLoader>& loader);
    std::shared_ptr<Texture> load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void loadInThread(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void removeReservedFlag(const std::wstring& path);
    void releaseUnusedTexture();
    // backdoor
    template <typename T>
    void backDoor() const {} // NOTE: 排他制御忘れないように
  private:
    std::shared_ptr<Texture> execLoad(const std::wstring& uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    IDirect3DDevice9* d3DDevice;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> m_textureMap;
    std::shared_ptr<TextureLoader> m_loader;
    mutable std::mutex textureLoadSection; // lock1
    mutable std::mutex memberAccessSection; // lock2 この順でlock
    std::unordered_map<std::wstring, std::thread> m_loadThreads;
  };

  void getD3DTextureSize(IDirect3DTexture9* texture, int& width, int& height);
  int getD3DTextureWidth(IDirect3DTexture9* texture);
  int getD3DTextureHeight(IDirect3DTexture9* texture);
}