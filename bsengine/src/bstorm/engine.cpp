#include <bstorm/engine.hpp>

#include <bstorm/graphic_device.hpp>
#include <bstorm/input_device.hpp>
#include <bstorm/fps_counter.hpp>
#include <bstorm/lostable_graphic_resource.hpp>
#include <bstorm/engine_develop_options.hpp>
#include <bstorm/config.hpp>
#include <bstorm/package.hpp>

#include <cassert>

namespace bstorm
{
Engine::Engine(HWND hWnd, const conf::KeyConfig* keyConfig) :
    hWnd_(hWnd),
    graphicDevice_(std::make_unique<GraphicDevice>(hWnd)),
    inputDevice_(std::make_shared<InputDevice>(hWnd)),
    fpsCounter_(std::make_shared<FpsCounter>()),
    lostableGraphicResourceManager_(std::make_shared<LostableGraphicResourceManager>()),
    engineDevelopOptions_(std::make_shared<EngineDevelopOptions>()),
    keyConfig_(std::make_shared<conf::KeyConfig>(*keyConfig))
{
}

std::shared_ptr<Package> Engine::CreatePackage(int screenWidth, int screenHeight, const std::wstring & packageMainScriptPath) noexcept(false)
{
    return std::make_shared<Package>(hWnd_, screenWidth, screenHeight, packageMainScriptPath, keyConfig_, graphicDevice_, inputDevice_, fpsCounter_, lostableGraphicResourceManager_, engineDevelopOptions_);
}

IDirect3DDevice9* Engine::GetDirect3DDevice() const
{
    return graphicDevice_->GetDevice();
}

void Engine::ResetGraphicDevice()
{
    graphicDevice_->Reset();
}

void Engine::ReleaseLostableGraphicResource()
{
    lostableGraphicResourceManager_->OnLostDeviceAll();
}

void Engine::RestoreLostableGraphicDevice()
{
    lostableGraphicResourceManager_->OnResetDeviceAll();
}

void Engine::RemoveUnusedLostableGraphicResource()
{
    lostableGraphicResourceManager_->RemoveUnusedResource();
}

void Engine::SwitchRenderTargetToBackBuffer()
{
    graphicDevice_->SwitchRenderTargetToBackBuffer();
}

void Engine::SetInputEnable(bool enable)
{
    inputDevice_->SetInputEnable(enable);
}

void Engine::SetMousePositionProvider(const std::shared_ptr<MousePositionProvider>& mousePosProvider)
{
    inputDevice_->SetMousePositionProvider(mousePosProvider);
}

void Engine::UpdateFpsCounter()
{
    fpsCounter_->Update();
}

const std::shared_ptr<EngineDevelopOptions>& Engine::GetDevelopOptions() const
{
    return engineDevelopOptions_;
}
}