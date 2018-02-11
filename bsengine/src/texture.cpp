#include <d3dx9.h>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/texture.hpp>

namespace bstorm {
  Texture::Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture) :
    d3DTexture(d3DTexture),
    path(path),
    reservedFlag(false)
  {
    getD3DTextureSize(d3DTexture, width, height);
  }

  Texture::~Texture() {
    d3DTexture->Release();
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("release texture.")
      .setParam(Log::Param(Log::Param::Tag::TEXTURE, path))));
  }

  const std::wstring& Texture::getPath() const {
    return path;
  }

  int Texture::getWidth() const {
    return width;
  }

  int Texture::getHeight() const {
    return height;
  }

  void Texture::setReservedFlag(bool flag) {
    reservedFlag = flag;
  }

  bool Texture::isReserved() const {
    return reservedFlag;
  }

  IDirect3DTexture9 * Texture::getTexture() const {
    return d3DTexture;
  }

  IDirect3DTexture9 * TextureLoaderFromImageFile::loadTexture(const std::wstring & path, IDirect3DDevice9* d3DDevice) {
    IDirect3DTexture9* d3DTexture = NULL;
    if (FAILED(D3DXCreateTextureFromFileEx(
      d3DDevice,
      path.c_str(),
      D3DX_DEFAULT_NONPOW2, // 画像の横幅
      D3DX_DEFAULT_NONPOW2, // 画像の縦幅
      0, // MipsLevel
      0, // Usage
      D3DFMT_UNKNOWN, // Format
      D3DPOOL_MANAGED, // Pool
      D3DX_FILTER_NONE, // Filter
      D3DX_FILTER_NONE, // MipMapFilter
      0, //  透明色
      NULL, // ImageInfo
      NULL, // Pallete
      &d3DTexture))) {
      return NULL;
    }
    return d3DTexture;
  }

  std::shared_ptr<Texture> TextureCache::load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) {
    auto uniqPath = canonicalPath(path);
    auto it = textureMap.find(uniqPath);
    if (it != textureMap.end()) {
      if (reserve) it->second->setReservedFlag(true);
      return it->second;
    } else {
      IDirect3DTexture9* d3DTexture = loader->loadTexture(uniqPath, d3DDevice);
      if (d3DTexture) {
        auto texture = std::make_shared<Texture>(uniqPath, d3DTexture);
        texture->setReservedFlag(reserve);
        textureMap[uniqPath] = texture;
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_INFO).setMessage(std::string("load texture") + (reserve ? " (reserved)." : "."))
          .setParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
          .addSourcePos(srcPos)));
        return texture;
      }
    }
    throw Log(Log::Level::LV_ERROR)
      .setMessage("failed to load texture.")
      .setParam(Log::Param(Log::Param::Tag::TEXTURE, path));
  }

  void TextureCache::removeReservedFlag(const std::wstring & path) {
    auto uniqPath = canonicalPath(path);
    auto it = textureMap.find(uniqPath);
    if (it != textureMap.end()) {
      it->second->setReservedFlag(false);
    }
  }

  void TextureCache::releaseUnusedTexture() {
    auto it = textureMap.begin();
    while (it != textureMap.end()) {
      auto& texture = it->second;
      if (!texture->isReserved() && texture.use_count() <= 1) {
        textureMap.erase(it++);
      } else ++it;
    }
  }

  TextureCache::TextureCache(IDirect3DDevice9* d3DDevice) :
    d3DDevice(d3DDevice),
    loader(std::make_shared<TextureLoaderFromImageFile>())
  {
  }

  void TextureCache::setLoader(const std::shared_ptr<TextureLoader>& ld) {
    if (ld) {
      loader = ld;
    } else {
      loader = std::make_shared<TextureLoaderFromImageFile>();
    }
  }

  void getD3DTextureSize(IDirect3DTexture9 * texture, int & width, int & height) {
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0, &desc))) {
      width = height = 0;
    } else {
      width = desc.Width;
      height = desc.Height;
    }
  }

  int getD3DTextureWidth(IDirect3DTexture9 * texture) {
    int w, h;
    getD3DTextureSize(texture, w, h);
    return w;
  }

  int getD3DTextureHeight(IDirect3DTexture9 * texture) {
    int w, h;
    getD3DTextureSize(texture, w, h);
    return h;
  }
}