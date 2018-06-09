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

void Texture::Reset(IDirect3DTexture9 * d3DTex)
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

std::shared_ptr<Texture> TextureCache::Load(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    return LoadInThread(path, reserve, srcPos).get();
}

std::shared_future<std::shared_ptr<Texture>> TextureCache::LoadInThread(const std::wstring & p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    auto uniqPath = GetCanonicalPath(p);
    {
        auto it = textureMap_.find(uniqPath);
        if (it != textureMap_.end())
        {
            // ロード中 or ロード済
            return it->second;
        }
    }
    return textureMap_[uniqPath] = std::async(std::launch::async, [this, uniqPath, reserve, srcPos]()
    {
        // load
        if (auto d3DTexture = LoadTextureFromFile(uniqPath, d3DDevice_))
        {
            auto texture = std::make_shared<Texture>(uniqPath, d3DTexture);
            texture->SetReservedFlag(reserve);
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_INFO).SetMessage(std::string("load texture") + (reserve ? " (reserved)." : "."))
                .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))
                .AddSourcePos(srcPos)));
            return texture;
        }
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to load texture.")
            .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath));
    }).share();
}

void TextureCache::Reload(const std::wstring& p, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    const std::wstring& uniqPath = GetCanonicalPath(p);
    auto it = textureMap_.find(uniqPath);
    if (it != textureMap_.end())
    {
        // cache hit
        try
        {
            // ロードが完了していない場合は完了させる
            auto& texture = it->second.get();

            if (reserve) texture->SetReservedFlag(true);
            if (auto d3DTexture = LoadTextureFromFile(uniqPath, d3DDevice_))
            {
                // リロード
                texture->Reset(d3DTexture);
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
            return;
        } catch (...)
        {
            // ロード失敗
        }
    }
    // 未ロードかロードに失敗していた場合は新しくロード
    LoadInThread(uniqPath, reserve, srcPos);
}

void TextureCache::RemoveReservedFlag(const std::wstring & p)
{
    auto uniqPath = GetCanonicalPath(p);
    auto it = textureMap_.find(uniqPath);
    if (it != textureMap_.end())
    {
        try
        {
            auto texture = it->second.get();
            texture->SetReservedFlag(false);
        } catch (...)
        {
        }
    }
}

void TextureCache::ReleaseUnusedTexture()
{
    auto it = textureMap_.begin();
    while (it != textureMap_.end())
    {
        try
        {
            auto& texture = it->second.get();
            if (!texture->IsReserved() && texture.use_count() <= 1)
            {
                textureMap_.erase(it++);
            } else ++it;
        } catch (...)
        {
            ++it;
        }
    }
}

TextureCache::TextureCache(IDirect3DDevice9* d3DDevice_) :
    d3DDevice_(d3DDevice_)
{
}

TextureCache::~TextureCache()
{
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