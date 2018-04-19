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
    RenderTarget(const std::wstring& name, int width, int height, IDirect3DDevice9* d3DDevice);
    ~RenderTarget();
    void setRenderTarget();
    IDirect3DTexture9* getTexture() const;
    IDirect3DSurface9* getSurface() const;
    std::wstring getName() const;
    int getWidth() const;
    int getHeight() const;
    const D3DVIEWPORT9& getViewport() const;
    void setViewport(int left, int top, int width, int height);
    void onLostDevice() override;
    void onResetDevice() override;
private:
    std::wstring name;
    int width;
    int height;
    IDirect3DDevice9* d3DDevice;
    IDirect3DTexture9* texture;
    IDirect3DSurface9* textureSurface;
    IDirect3DSurface9* textureDepthStencilSurface;
    D3DVIEWPORT9 viewport;
};
}
