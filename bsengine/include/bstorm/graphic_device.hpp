#pragma once

#include <windows.h>
#include <d3d9.h>

#include <bstorm/non_copyable.hpp>

namespace bstorm {
  class GraphicDevice {
  public:
    GraphicDevice(HWND hWnd);
    ~GraphicDevice();
    void reset();
    IDirect3DDevice9* getDevice() const;
    void setBackbufferRenderTarget();
  private:
    IDirect3D9 *d3D;
    IDirect3DDevice9 *d3DDevice;
    IDirect3DSurface9* backBufferSurface;
    IDirect3DSurface9* backBufferDepthStencilSurface;
    D3DPRESENT_PARAMETERS presentParams;
  };
}