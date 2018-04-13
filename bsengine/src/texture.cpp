#include <bstorm/texture.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

#include <d3dx9.h>

namespace bstorm
{
Texture::Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture) :
    d3DTexture(d3DTexture),
    path(path),
    reservedFlag(false)
{
    getD3DTextureSize(d3DTexture, width, height);
}

Texture::~Texture()
{
    safe_release(d3DTexture);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .setMessage("release texture.")
        .setParam(Log::Param(Log::Param::Tag::TEXTURE, path))));
}

const std::wstring& Texture::getPath() const
{
    return path;
}

int Texture::getWidth() const
{
    return width;
}

int Texture::getHeight() const
{
    return height;
}

void Texture::setReservedFlag(bool flag)
{
    reservedFlag = flag;
}

bool Texture::isReserved() const
{
    return reservedFlag;
}

IDirect3DTexture9* Texture::getTexture() const
{
    return d3DTexture;
}

void Texture::reload(IDirect3DTexture9 * d3DTex)
{
    if (d3DTex)
    {
        safe_release(d3DTexture);
        d3DTexture = d3DTex;
        getD3DTextureSize(d3DTexture, width, height);
    }
}

IDirect3DTexture9 * loadTextureFromFile(const std::wstring & path, IDirect3DDevice9* d3DDevice) noexcept(true)
{
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
        &d3DTexture)))
    {
        return NULL;
    }
    return d3DTexture;
}

std::shared_ptr<Texture> TextureCache::load(const std::wstring& p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    auto uniqPath = canonicalPath(p);
    if (auto texture = getTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->setReservedFlag(true);
        return texture;
    }
    return loadFirst(uniqPath, reserve, srcPos);
}

std::shared_ptr<Texture> TextureCache::loadFirst(const std::wstring & uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    IDirect3DTexture9* d3DTexture = NULL;
    {
        std::lock_guard<std::recursive_mutex> lock(textureLoadSection);
        if (auto texture = getTexture(uniqPath))
        {
            if (reserve) texture->setReservedFlag(true);
            return texture;
        }
        // load
        d3DTexture = loadTextureFromFile(uniqPath, d3DDevice);
    }
    if (d3DTexture)
    {
        auto texture = std::make_shared<Texture>(uniqPath, d3DTexture);
        texture->setReservedFlag(reserve);
        {
            std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
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

void TextureCache::loadInThread(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    const std::wstring& uniqPath = canonicalPath(p);
    if (auto texture = getTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->setReservedFlag(true);
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
        if (m_loadThreads.count(uniqPath) != 0) return; // ロード中
        m_loadThreads[uniqPath] = std::thread([this, uniqPath, reserve, srcPos]()
        {
            try
            {
                loadFirst(uniqPath, reserve, srcPos);
            } catch (Log& log)
            {
                log.setLevel(Log::Level::LV_WARN);
                log.addSourcePos(srcPos);
                Logger::WriteLog(log);
            }
            {
                std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
                m_loadThreads[uniqPath].detach();
                m_loadThreads.erase(uniqPath);
            }
        });
    }
}

void TextureCache::reload(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    const std::wstring& uniqPath = canonicalPath(p);
    if (auto texture = getTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->setReservedFlag(true);
        IDirect3DTexture9* d3DTexture = NULL;
        d3DTexture = loadTextureFromFile(uniqPath, d3DDevice);
        if (d3DTexture)
        {
            texture->reload(d3DTexture);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_INFO).setMessage(std::string("reload texture") + (reserve ? " (reserved)." : "."))
                .setParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
                .addSourcePos(srcPos)));
        } else
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_WARN).setMessage(std::string("failed to reload texture."))
                .setParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
                .addSourcePos(srcPos)));
        }
    } else
    {
        loadFirst(uniqPath, reserve, srcPos);
    }
}

void TextureCache::removeReservedFlag(const std::wstring & p)
{
    auto uniqPath = canonicalPath(p);
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
        auto it = m_textureMap.find(uniqPath);
        if (it != m_textureMap.end())
        {
            it->second->setReservedFlag(false);
        }
    }
}

void TextureCache::releaseUnusedTexture()
{
    std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
    auto it = m_textureMap.begin();
    while (it != m_textureMap.end())
    {
        auto& texture = it->second;
        if (!texture->isReserved() && texture.use_count() <= 1)
        {
            m_textureMap.erase(it++);
        } else ++it;
    }
}

std::shared_ptr<Texture> TextureCache::getTexture(const std::wstring& uniqPath) const
{
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
        auto it = m_textureMap.find(uniqPath);
        if (it != m_textureMap.end())
        {
            return it->second;
        }
    }
    return nullptr;
}

TextureCache::TextureCache(IDirect3DDevice9* d3DDevice) :
    d3DDevice(d3DDevice)
{
}

TextureCache::~TextureCache()
{
    while (true)
    {
        bool allThreadFinished = true;
        {
            std::lock_guard<std::recursive_mutex> lock(memberAccessSection);
            for (const auto& thread : m_loadThreads)
            {
                allThreadFinished = allThreadFinished && !thread.second.joinable();
            }
        }
        if (allThreadFinished) break;
        Sleep(1);
    }
}

void getD3DTextureSize(IDirect3DTexture9 * texture, int & width, int & height)
{
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0, &desc)))
    {
        width = height = 0;
    } else
    {
        width = desc.Width;
        height = desc.Height;
    }
}

int getD3DTextureWidth(IDirect3DTexture9 * texture)
{
    int w, h;
    getD3DTextureSize(texture, w, h);
    return w;
}

int getD3DTextureHeight(IDirect3DTexture9 * texture)
{
    int w, h;
    getD3DTextureSize(texture, w, h);
    return h;
}
}