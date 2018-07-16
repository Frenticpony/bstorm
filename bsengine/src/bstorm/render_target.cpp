#include <bstorm/render_target.hpp>

#include <bstorm/ptr_util.hpp>
#include <bstorm/logger.hpp>

#include <exception>

namespace bstorm
{
RenderTarget::RenderTarget(const std::wstring& name, int width, int height, IDirect3DDevice9* d3DDevice_) :
    LostableGraphicResource(),
    name_(name),
    width_(width),
    height_(height),
    d3DDevice_(d3DDevice_),
    texture_(nullptr),
    textureSurface_(nullptr),
    textureDepthStencilSurface_(nullptr),
    viewport_({ 0, 0, (DWORD)width, (DWORD)height, 0.0f, 1.0f })
{
    OnResetDevice();
}

RenderTarget::~RenderTarget()
{
    OnLostDevice();
}

void RenderTarget::SetRenderTarget()
{
    if (textureSurface_ != nullptr && textureDepthStencilSurface_ != nullptr)
    {
        d3DDevice_->SetRenderTarget(0, textureSurface_);
        d3DDevice_->SetDepthStencilSurface(textureDepthStencilSurface_);
        d3DDevice_->SetViewport(&viewport_);
    }
}

IDirect3DTexture9* RenderTarget::GetTexture() const
{
    return texture_;
}

IDirect3DSurface9 * RenderTarget::GetSurface() const
{
    return textureSurface_;
}

const std::wstring& RenderTarget::GetName() const
{
    return name_;
}

int RenderTarget::GetWidth() const
{
    return width_;
}

int RenderTarget::GetHeight() const
{
    return height_;
}

const D3DVIEWPORT9 & RenderTarget::GetViewport() const
{
    return viewport_;
}

void RenderTarget::SetViewport(int left, int top, int width, int height)
{
    viewport_ = { (DWORD)left, (DWORD)top, (DWORD)width, (DWORD)height, 0.0f, 1.0f };
}

void RenderTarget::OnResetDevice()
{
    if (textureSurface_ != nullptr || textureDepthStencilSurface_ != nullptr) return;

    Log err = Log(Log::Level::LV_ERROR)
        .SetMessage("failed to create render target.")
        .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name_));

    if (FAILED(d3DDevice_->CreateTexture(
        width_,
        height_,
        0,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &texture_,
        nullptr)))
    {
        throw err;
    }

    if (FAILED(texture_->GetSurfaceLevel(0, &textureSurface_)))
    {
        throw err;
    }

    if (FAILED(d3DDevice_->CreateDepthStencilSurface(
        width_,
        height_,
        D3DFMT_D16,
        D3DMULTISAMPLE_NONE,
        0,
        TRUE,
        &textureDepthStencilSurface_,
        nullptr)))
    {
        throw err;
    }
}

void RenderTarget::OnLostDevice()
{
    safe_release(textureDepthStencilSurface_);
    safe_release(textureSurface_);
    safe_release(texture_);
}
}