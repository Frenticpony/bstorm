#include <bstorm/graphic_device.hpp>

#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
GraphicDevice::GraphicDevice(HWND hWnd) :
    d3D(NULL),
    d3DDevice(NULL),
    backBufferSurface(NULL),
    backBufferDepthStencilSurface(NULL)
{
    RECT screen;
    GetClientRect(hWnd, &screen);
    BOOL isWindowMode = IsWindow(hWnd);

    d3D = Direct3DCreate9(D3D_SDK_VERSION);

    presentParams = {
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

    if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams, &d3DDevice)))
    {
        if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams, &d3DDevice)))
        {
            if (FAILED(d3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_FPU_PRESERVE | D3DCREATE_MULTITHREADED, &presentParams, &d3DDevice)))
            {
                d3D->Release();
                throw Log(Log::Level::LV_ERROR)
                    .setMessage("failed to init graphic device.");
            }
        }
    }

    d3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferSurface);
    d3DDevice->GetDepthStencilSurface(&backBufferDepthStencilSurface);
}

GraphicDevice::~GraphicDevice()
{
    safe_release(backBufferSurface);
    safe_release(backBufferDepthStencilSurface);
    d3DDevice->Release();
    d3D->Release();
}

void GraphicDevice::reset()
{
    safe_release(backBufferSurface);
    safe_release(backBufferDepthStencilSurface);
    if (FAILED(d3DDevice->Reset(&presentParams)))
    {
        throw Log(Log::Level::LV_ERROR).setMessage("failed to reset graphic device.");
    }
    d3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &backBufferSurface);
    d3DDevice->GetDepthStencilSurface(&backBufferDepthStencilSurface);
}

IDirect3DDevice9 * GraphicDevice::getDevice() const
{
    return d3DDevice;
}

void GraphicDevice::setBackbufferRenderTarget()
{
    D3DVIEWPORT9 viewport = { 0, 0, presentParams.BackBufferWidth, presentParams.BackBufferHeight, 0.0f, 1.0f };
    d3DDevice->SetViewport(&viewport);
    d3DDevice->SetRenderTarget(0, backBufferSurface);
    d3DDevice->SetDepthStencilSurface(backBufferDepthStencilSurface);
}

DWORD GraphicDevice::getBackBufferWidth() const
{
    return presentParams.BackBufferWidth;
}

DWORD GraphicDevice::getBackBufferHeight() const
{
    return presentParams.BackBufferHeight;
}
}