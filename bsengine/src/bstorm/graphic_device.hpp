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
    void Reset();
    IDirect3DDevice9* GetDevice() const;
    void SetBackbufferRenderTarget();
    DWORD GetBackBufferWidth() const;
    DWORD GetBackBufferHeight() const;
private:
    IDirect3D9 * d3D_;
    IDirect3DDevice9* d3DDevice_;
    IDirect3DSurface9* backBufferSurface_;
    IDirect3DSurface9* backBufferDepthStencilSurface_;
    D3DPRESENT_PARAMETERS presentParams_;
};
}