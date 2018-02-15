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

  std::shared_ptr<Texture> TextureCache::load(const std::wstring& p, bool reserve, const std::shared_ptr<SourcePos>& srcPos) {
    auto uniqPath = canonicalPath(p);
    {
      // cache hit
      std::lock_guard<std::mutex> lock(memberAccessSection);
      auto it = m_textureMap.find(uniqPath);
      if (it != m_textureMap.end()) {
        if (reserve) it->second->setReservedFlag(true);
        return it->second;
      }
    }
    return execLoad(uniqPath, reserve, srcPos);
  }

  std::shared_ptr<Texture> TextureCache::execLoad(const std::wstring & uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos) {
    IDirect3DTexture9* d3DTexture = NULL;
    {
      std::lock_guard<std::mutex> lock(textureLoadSection);
      {
        // 他のスレッドのtextureLoadSectionでロードされたテクスチャがあればそれを返す
        std::lock_guard<std::mutex> lock(memberAccessSection);
        auto it = m_textureMap.find(uniqPath);
        if (it != m_textureMap.end()) {
          if (reserve) it->second->setReservedFlag(true);
          return it->second;
        }
      }
      // load
      d3DTexture = m_loader->loadTexture(uniqPath, d3DDevice);
    }
    if (d3DTexture) {
      auto texture = std::make_shared<Texture>(uniqPath, d3DTexture);
      texture->setReservedFlag(reserve);
      {
        std::lock_guard<std::mutex> lock(memberAccessSection);
        m_textureMap[uniqPath] = texture;
      }
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO).setMessage(std::string("load texture") + (reserve ? " (reserved)." : "."))
        .setParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
        .addSourcePos(srcPos)));
      return texture;
    }
    throw Log(Log::Level::LV_ERROR)
      .setMessage("failed to load texture.")
      .setParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath));
  }

  void TextureCache::loadInThread(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos) {
    const std::wstring& uniqPath = canonicalPath(p);
    {
      std::lock_guard<std::mutex> lock(memberAccessSection);
      {
        // cache hit
        auto it = m_textureMap.find(uniqPath);
        if (it != m_textureMap.end()) {
          if (reserve) it->second->setReservedFlag(true);
          return;
        }
      }
      if (m_loadThreads.count(uniqPath) != 0) return; // ロード中
      m_loadThreads[uniqPath] = std::thread([this, uniqPath, reserve, srcPos]() {
        try {
          execLoad(uniqPath, reserve, srcPos);
        } catch (Log& log) {
          log.setLevel(Log::Level::LV_WARN);
          log.addSourcePos(srcPos);
          Logger::WriteLog(log);
        }
        {
          std::lock_guard<std::mutex> lock(memberAccessSection);
          m_loadThreads[uniqPath].detach();
          m_loadThreads.erase(uniqPath);
        }
      });
    }
  }

  void TextureCache::removeReservedFlag(const std::wstring & p) {
    auto uniqPath = canonicalPath(p);
    {
      std::lock_guard<std::mutex> lock(memberAccessSection);
      auto it = m_textureMap.find(uniqPath);
      if (it != m_textureMap.end()) {
        it->second->setReservedFlag(false);
      }
    }
  }

  void TextureCache::releaseUnusedTexture() {
    std::lock_guard<std::mutex> lock(memberAccessSection);
    auto it = m_textureMap.begin();
    while (it != m_textureMap.end()) {
      auto& texture = it->second;
      if (!texture->isReserved() && texture.use_count() <= 1) {
        m_textureMap.erase(it++);
      } else ++it;
    }
  }

  TextureCache::TextureCache(IDirect3DDevice9* d3DDevice) :
    d3DDevice(d3DDevice),
    m_loader(std::make_shared<TextureLoaderFromImageFile>())
  {
  }

  TextureCache::~TextureCache() {
    while (true) {
      bool allThreadFinished = true;
      {
        std::lock_guard<std::mutex> lock(memberAccessSection);
        for (const auto& thread : m_loadThreads) {
          allThreadFinished = allThreadFinished && !thread.second.joinable();
        }
      }
      if (allThreadFinished) break;
      Sleep(1);
    }
  }

  void TextureCache::setLoader(const std::shared_ptr<TextureLoader>& ld) {
    std::lock_guard<std::mutex> lock(memberAccessSection);
    if (ld) {
      m_loader = ld;
    } else {
      m_loader = std::make_shared<TextureLoaderFromImageFile>();
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