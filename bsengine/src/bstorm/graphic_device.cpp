#include <bstorm/graphic_device.hpp>

#include <bstorm/ptr_util.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
GraphicDevice::GraphicDevice(HWND hWnd) :
    d3D_(nullptr),
    d3DDevice_(nullptr),
    backBufferSurface_(nullptr),
    backBufferDepthStencilSurface_(nullptr)
{
    RECT screen;
    GetClientRect(hWnd, &screen);
    BOOL isWindowMode = IsWindow(hWnd);

    d3D_ = Direct3DCreate9(D3D_SDK_VERSION);

    presentParams_ = {
        (UINT)screen.right,
        (UINT)screen.bottom,
        isWindowMode ? D3DFMT_UNKNOWN : D3DFMT_X8R8G8B8,
        1,
        D3DMULTISAMPLE_NONE,
        0,
        D3DSWAPEFFECT_DISCARD,
        hWnd,
        isWindowMode,
        TRUE,
        D3DFMT_D24S8,
        0,
        D3DPRESENT_RATE_DEFAULT,
        D3DPRESENT_INTERVAL_ONE
    };

    if (FAILED(d3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams_, &d3DDevice_)))
    {
        if (FAILED(d3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams_, &d3DDevice_)))
        {
            if (FAILED(d3D_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams_, &d3DDevice_)))
            {
                d3D_->Release();
                throw Log(LogLevel::LV_ERROR)
                    .Msg("Failed to initialize D3D graphic device.");
            }
        }
    }

    d3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferSurface_);
    d3DDevice_->GetDepthStencilSurface(&backBufferDepthStencilSurface_);
}

GraphicDevice::~GraphicDevice()
{
    safe_release(backBufferSurface_);
    safe_release(backBufferDepthStencilSurface_);
    d3DDevice_->Release();
    d3D_->Release();
}

void GraphicDevice::Reset()
{
    safe_release(backBufferSurface_);
    safe_release(backBufferDepthStencilSurface_);
    if (FAILED(d3DDevice_->Reset(&presentParams_)))
    {
        throw Log(LogLevel::LV_ERROR).Msg("Failed to reset D3D graphic device.");
    }
    d3DDevice_->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferSurface_);
    d3DDevice_->GetDepthStencilSurface(&backBufferDepthStencilSurface_);
}

IDirect3DDevice9 * GraphicDevice::GetDevice() const
{
    return d3DDevice_;
}

void GraphicDevice::SwitchRenderTargetToBackBuffer()
{
    D3DVIEWPORT9 viewport = { 0, 0, presentParams_.BackBufferWidth, presentParams_.BackBufferHeight, 0.0f, 1.0f };
    d3DDevice_->SetViewport(&viewport);
    d3DDevice_->SetRenderTarget(0, backBufferSurface_);
    d3DDevice_->SetDepthStencilSurface(backBufferDepthStencilSurface_);
}

DWORD GraphicDevice::GetBackBufferWidth() const
{
    return presentParams_.BackBufferWidth;
}

DWORD GraphicDevice::GetBackBufferHeight() const
{
    return presentParams_.BackBufferHeight;
}
}