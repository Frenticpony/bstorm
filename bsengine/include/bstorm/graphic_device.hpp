#pragma once

#include <bstorm/non_copyable.hpp>

#include <windows.h>
#include <d3d9.h>

namespace bstorm
{
class GraphicDevice : private NonCopyable
{
public:
    GraphicDevice(HWND hWnd);
    ~GraphicDevice();
    void reset();
    IDirect3DDevice9* getDevice() const;
    void setBackbufferRenderTarget();
    DWORD getBackBufferWidth() const;
    DWORD getBackBufferHeight() const;
private:
    IDirect3D9 * d3D;
    IDirect3DDevice9* d3DDevice;
    IDirect3DSurface9* backBufferSurface;
    IDirect3DSurface9* backBufferDepthStencilSurface;
    D3DPRESENT_PARAMETERS presentParams;
};
}