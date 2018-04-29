#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/lostable_graphic_resource.hpp>

#include <string>
#include <d3d9.h>

namespace bstorm
{
class RenderTarget : private NonCopyable, public LostableGraphicResource
{
public:
    RenderTarget(const std::wstring& name, int width, int height, IDirect3DDevice9* d3DDevice_);
    ~RenderTarget();
    void SetRenderTarget();
    IDirect3DTexture9* GetTexture() const;
    IDirect3DSurface9* GetSurface() const;
    const std::wstring& GetName() const;
    int GetWidth() const;
    int GetHeight() const;
    const D3DVIEWPORT9& GetViewport() const;
    void SetViewport(int left, int top, int width, int height);
    void OnLostDevice() override;
    void OnResetDevice() override;
private:
    std::wstring name_;
    int width_;
    int height_;
    IDirect3DDevice9* d3DDevice_;
    IDirect3DTexture9* texture_;
    IDirect3DSurface9* textureSurface_;
    IDirect3DSurface9* textureDepthStencilSurface_;
    D3DVIEWPORT9 viewport_;
};
}
