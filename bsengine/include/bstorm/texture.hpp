#pragma once

#include <string>
#include <memory>
#include <unordered_map>
#include <d3d9.h>

#include <bstorm/non_copyable.hpp>

namespace bstorm {
  class Logger;
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
    bool reservedFlag;
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

  class TextureCache {
  public:
    TextureCache(IDirect3DDevice9* d3DDevice);
    void setLoader(const std::shared_ptr<TextureLoader>& loader);
    std::shared_ptr<Texture> load(const std::wstring& path, bool reserve);
    void removeReservedFlag(const std::wstring& path);
    void releaseUnusedTexture(const std::shared_ptr<Logger>& logger);
    // backdoor
    template <typename T>
    void backDoor() const {}
  private:
    IDirect3DDevice9* d3DDevice;
    std::unordered_map<std::wstring, std::shared_ptr<Texture>> textureMap;
    std::shared_ptr<TextureLoader> loader;
  };

  void getD3DTextureSize(IDirect3DTexture9* texture, int& width, int& height);
  int getD3DTextureWidth(IDirect3DTexture9* texture);
  int getD3DTextureHeight(IDirect3DTexture9* texture);
}