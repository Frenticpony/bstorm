#include <exception>

#include <bstorm/util.hpp>
#include <bstorm/render_target.hpp>

namespace bstorm {
  RenderTarget::RenderTarget(const std::wstring& name, int width, int height, IDirect3DDevice9* d3DDevice) :
    LostableGraphicResource(),
    name(name),
    width(width),
    height(height),
    d3DDevice(d3DDevice),
    texture(NULL),
    textureSurface(NULL),
    textureDepthStencilSurface(NULL),
    viewport({ 0, 0, (DWORD)width, (DWORD)height, 0.0f, 1.0f })
  {
    onResetDevice();
  }

  RenderTarget::~RenderTarget() {
    onLostDevice();
  }

  void RenderTarget::setRenderTarget() {
    if (textureSurface != NULL && textureDepthStencilSurface != NULL) {
      d3DDevice->SetRenderTarget(0, textureSurface);
      d3DDevice->SetDepthStencilSurface(textureDepthStencilSurface);
      d3DDevice->SetViewport(&viewport);
    }
  }

  IDirect3DTexture9* RenderTarget::getTexture() const {
    return texture;
  }

  IDirect3DSurface9 * RenderTarget::getSurface() const {
    return textureSurface;
  }

  std::wstring RenderTarget::getName() const {
    return name;
  }

  int RenderTarget::getWidth() const {
    return width;
  }

  int RenderTarget::getHeight() const {
    return height;
  }

  const D3DVIEWPORT9 & RenderTarget::getViewport() const {
    return viewport;
  }

  void RenderTarget::setViewport(int left, int top, int width, int height) {
    viewport = { (DWORD)left, (DWORD)top, (DWORD)width, (DWORD)height, 0.0f, 1.0f };
  }

  void RenderTarget::onResetDevice() {
    if (textureSurface != NULL || textureDepthStencilSurface != NULL) return;

    std::runtime_error err("failed to create render target: " + toUTF8(name));

    if (FAILED(d3DDevice->CreateTexture(
      width,
      height,
      0,
      D3DUSAGE_RENDERTARGET,
      D3DFMT_A8R8G8B8,
      D3DPOOL_DEFAULT,
      &(this->texture),
      NULL))) {
      throw err;
    }

    if (FAILED(texture->GetSurfaceLevel(0, &(this->textureSurface)))) {
      throw err;
    }

    if (FAILED(d3DDevice->CreateDepthStencilSurface(
      width,
      height,
      D3DFMT_D16,
      D3DMULTISAMPLE_NONE,
      0,
      TRUE,
      &(this->textureDepthStencilSurface),
      NULL))) {
      throw err;
    }
  }

  void RenderTarget::onLostDevice() {
    safe_release(textureDepthStencilSurface);
    safe_release(textureSurface);
    safe_release(texture);
  }
}