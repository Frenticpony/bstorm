#include <bstorm/texture.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

#include <d3dx9.h>

namespace bstorm
{
Texture::Texture(const std::wstring& path, IDirect3DTexture9* d3DTexture) :
    d3DTexture_(d3DTexture),
    path_(path),
    reservedFlag_(false)
{
    GetD3DTextureSize(d3DTexture, &width_, &height_);
}

Texture::~Texture()
{
    safe_release(d3DTexture_);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("release texture.")
        .SetParam(Log::Param(Log::Param::Tag::TEXTURE, path_))));
}

const std::wstring& Texture::GetPath() const
{
    return path_;
}

int Texture::GetWidth() const
{
    return width_;
}

int Texture::GetHeight() const
{
    return height_;
}

void Texture::SetReservedFlag(bool flag)
{
    reservedFlag_ = flag;
}

bool Texture::IsReserved() const
{
    return reservedFlag_;
}

IDirect3DTexture9* Texture::GetTexture() const
{
    return d3DTexture_;
}

void Texture::Reload(IDirect3DTexture9 * d3DTex)
{
    if (d3DTex)
    {
        safe_release(d3DTexture_);
        d3DTexture_ = d3DTex;
        GetD3DTextureSize(d3DTexture_, &width_, &height_);
    }
}

IDirect3DTexture9 * LoadTextureFromFile(const std::wstring & path, IDirect3DDevice9* d3DDevice_) noexcept(true)
{
    IDirect3DTexture9* d3DTexture = NULL;
    if (FAILED(D3DXCreateTextureFromFileEx(
        d3DDevice_,
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

std::shared_ptr<Texture> TextureCache::Load(const std::wstring& p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    auto uniqPath = GetCanonicalPath(p);
    if (auto texture = GetTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->SetReservedFlag(true);
        return texture;
    }
    return LoadFirst(uniqPath, reserve, srcPos);
}

std::shared_ptr<Texture> TextureCache::LoadFirst(const std::wstring & uniqPath, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    IDirect3DTexture9* d3DTexture = NULL;
    {
        std::lock_guard<std::recursive_mutex> lock(textureLoadSection_);
        if (auto texture = GetTexture(uniqPath))
        {
            if (reserve) texture->SetReservedFlag(true);
            return texture;
        }
        // load
        d3DTexture = LoadTextureFromFile(uniqPath, d3DDevice_);
    }
    if (d3DTexture)
    {
        auto texture = std::make_shared<Texture>(uniqPath, d3DTexture);
        texture->SetReservedFlag(reserve);
        {
            std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
            textureMap_[uniqPath] = texture;
        }
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_INFO).SetMessage(std::string("load texture") + (reserve ? " (reserved)." : "."))
            .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
            .AddSourcePos(srcPos)));
        return texture;
    }
    throw Log(Log::Level::LV_ERROR)
        .SetMessage("failed to load texture.")
        .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath));
}

void TextureCache::LoadInThread(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    const std::wstring& uniqPath = GetCanonicalPath(p);
    if (auto texture = GetTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->SetReservedFlag(true);
        return;
    }
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
        if (loadThreads_.count(uniqPath) != 0) return; // ロード中
        loadThreads_[uniqPath] = std::thread([this, uniqPath, reserve, srcPos]()
        {
            try
            {
                LoadFirst(uniqPath, reserve, srcPos);
            } catch (Log& log)
            {
                log.SetLevel(Log::Level::LV_WARN);
                log.AddSourcePos(srcPos);
                Logger::WriteLog(log);
            }
            {
                std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
                loadThreads_[uniqPath].detach();
                loadThreads_.erase(uniqPath);
            }
        });
    }
}

void TextureCache::Reload(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    const std::wstring& uniqPath = GetCanonicalPath(p);
    if (auto texture = GetTexture(uniqPath))
    {
        // cache hit
        if (reserve) texture->SetReservedFlag(true);
        IDirect3DTexture9* d3DTexture = NULL;
        d3DTexture = LoadTextureFromFile(uniqPath, d3DDevice_);
        if (d3DTexture)
        {
            texture->Reload(d3DTexture);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_INFO).SetMessage(std::string("reload texture") + (reserve ? " (reserved)." : "."))
                .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
                .AddSourcePos(srcPos)));
        } else
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_WARN).SetMessage(std::string("failed to reload texture."))
                .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
                .AddSourcePos(srcPos)));
        }
    } else
    {
        LoadFirst(uniqPath, reserve, srcPos);
    }
}

void TextureCache::RemoveReservedFlag(const std::wstring & p)
{
    auto uniqPath = GetCanonicalPath(p);
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
        auto it = textureMap_.find(uniqPath);
        if (it != textureMap_.end())
        {
            it->second->SetReservedFlag(false);
        }
    }
}

void TextureCache::ReleaseUnusedTexture()
{
    std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
    auto it = textureMap_.begin();
    while (it != textureMap_.end())
    {
        auto& texture = it->second;
        if (!texture->IsReserved() && texture.use_count() <= 1)
        {
            textureMap_.erase(it++);
        } else ++it;
    }
}

std::shared_ptr<Texture> TextureCache::GetTexture(const std::wstring& uniqPath) const
{
    {
        std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
        auto it = textureMap_.find(uniqPath);
        if (it != textureMap_.end())
        {
            return it->second;
        }
    }
    return nullptr;
}

TextureCache::TextureCache(IDirect3DDevice9* d3DDevice_) :
    d3DDevice_(d3DDevice_)
{
}

TextureCache::~TextureCache()
{
    while (true)
    {
        bool allThreadFinished = true;
        {
            std::lock_guard<std::recursive_mutex> lock(memberAccessSection_);
            for (const auto& thread : loadThreads_)
            {
                allThreadFinished = allThreadFinished && !thread.second.joinable();
            }
        }
        if (allThreadFinished) break;
        Sleep(1);
    }
}

void GetD3DTextureSize(IDirect3DTexture9 * texture, int * width, int * height)
{
    D3DSURFACE_DESC desc;
    if (FAILED(texture->GetLevelDesc(0, &desc)))
    {
        *width = *height = 0;
    } else
    {
        *width = desc.Width;
        *height = desc.Height;
    }
}

int GetD3DTextureWidth(IDirect3DTexture9 * texture)
{
    int w, h;
    GetD3DTextureSize(texture, &w, &h);
    return w;
}

int GetD3DTextureHeight(IDirect3DTexture9 * texture)
{
    int w, h;
    GetD3DTextureSize(texture, &w, &h);
    return h;
}
}