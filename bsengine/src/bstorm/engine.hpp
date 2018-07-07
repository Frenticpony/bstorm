#pragma once

#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/non_copyable.hpp>

#include <memory>
#include <string>
#include <windows.h>
#include <d3d9.h>

namespace bstorm
{
class GraphicDevice;
class InputDevice;
class FpsCounter;
class LostableGraphicResourceManager;
class MousePositionProvider;
class Package;
class EngineDevelopOptions;
namespace conf { struct KeyConfig; }

class Engine : private NonCopyable
{
public:
    Engine(HWND hWnd, const conf::KeyConfig* keyConfig);

    /* package */
    std::shared_ptr<Package> CreatePackage(int screenWidth, int screenHeight, const std::wstring& packageMainScriptPath) noexcept(false);

    /* graphic control */
    IDirect3DDevice9* GetDirect3DDevice() const;
    void ResetGraphicDevice();
    void ReleaseLostableGraphicResource();
    void RestoreLostableGraphicDevice();
    void RemoveUnusedLostableGraphicResource();
    void SwitchRenderTargetToBackBuffer();

    /* input control */
    void SetInputEnable(bool enable);
    void SetMousePositionProvider(const std::shared_ptr<MousePositionProvider>& mousePosProvider);

    /* time control */
    void UpdateFpsCounter();

    /* development */
    const std::shared_ptr<EngineDevelopOptions>& GetDevelopOptions() const;
private:
    HWND hWnd_;
    std::shared_ptr<GraphicDevice> graphicDevice_;
    std::shared_ptr<InputDevice> inputDevice_;
    std::shared_ptr<FpsCounter> fpsCounter_;
    std::shared_ptr<LostableGraphicResourceManager> lostableGraphicResourceManager_;
    std::shared_ptr<EngineDevelopOptions> engineDevelopOptions_;
    std::shared_ptr<conf::KeyConfig> keyConfig_;
};
}