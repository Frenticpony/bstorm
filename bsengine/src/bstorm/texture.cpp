#include <bstorm/texture.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/graphic_device.hpp>

#include <d3dx9.h>

namespace bstorm
{
static IDirect3DTexture9 * LoadTextureFromFile(const std::wstring & path, GraphicDevice& graphicDevice) noexcept(true)
{
    IDirect3DTexture9* d3DTexture = NULL;
    if (FAILED(D3DXCreateTextureFromFileEx(
        graphicDevice.GetDevice(),
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

Texture::Texture(const std::wstring & path, const std::shared_ptr<GraphicDevice> & graphicDevice) :
    path_(path),
    d3DTexture_(nullptr),
    graphicDevice_(graphicDevice)
{
    Reload();
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

IDirect3DTexture9* Texture::GetTexture() const
{
    return d3DTexture_;
}

void Texture::Reload()
{
    auto texture = LoadTextureFromFile(path_, *graphicDevice_);
    if (texture == nullptr)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("failed to load texture.")
            .SetParam(Log::Param(Log::Param::Tag::TEXTURE, path_));
    }
    d3DTexture_ = texture;
    GetD3DTextureSize(d3DTexture_, &width_, &height_);
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

TextureStore::TextureStore(const std::shared_ptr<GraphicDevice>& graphicDevice) :
    graphicDevice_(graphicDevice)
{
}

TextureStore::~TextureStore()
{
}

const std::shared_ptr<Texture>& TextureStore::Load(const std::wstring & path)
{
    auto uniqPath = GetCanonicalPath(path);
    if (cacheStore_.Contains(uniqPath))
    {
        return cacheStore_.Get(uniqPath);
    }
    auto& texture = cacheStore_.Load(uniqPath, uniqPath, graphicDevice_);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO).SetMessage(std::string("load texture."))
        .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))));
    return texture;
}

void TextureStore::LoadInThread(const std::wstring & path)
{
    auto uniqPath = GetCanonicalPath(path);
    if (cacheStore_.Contains(uniqPath)) return;
    cacheStore_.LoadAsync(uniqPath, uniqPath, graphicDevice_);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO).SetMessage(std::string("load texture (async)."))
        .SetParam(Log::Param(Log::Param::Tag::TEXTURE, uniqPath))));
}

void TextureStore::SetReserveFlag(const std::wstring & path, bool reserve)
{
    auto uniqPath = GetCanonicalPath(path);
    cacheStore_.SetReserveFlag(uniqPath, reserve);
}

bool TextureStore::IsReserved(const std::wstring & path) const
{
    return cacheStore_.IsReserved(path);
}

void TextureStore::RemoveUnusedTexture()
{
    cacheStore_.RemoveUnused();
}
bool TextureStore::IsLoadCompleted(const std::wstring& path) const
{
    auto uniqPath = GetCanonicalPath(path);
    return cacheStore_.IsLoadCompleted(uniqPath);
}
}