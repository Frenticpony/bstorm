#pragma once

#include <bstorm/non_copyable.hpp>
#include <bstorm/type.hpp>
#include <bstorm/key_types.hpp>

#include <windows.h>
#include <unordered_map>
#include <array>
#include <memory>
#define	DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace bstorm
{
class MousePositionProvider
{
public:
    virtual ~MousePositionProvider() {};
    virtual void GetMousePos(int screenWidth, int screenHeight, int &x, int &y) = 0;
};

class WinMousePositionProvider : public MousePositionProvider
{
public:
    WinMousePositionProvider(HWND hWnd);
    void GetMousePos(int screenWidth, int screenHeight, int &x, int &y) override;
private:
    HWND hWnd_;
};

class InputDevice : private NonCopyable
{
public:
    InputDevice(HWND hWnd, const std::shared_ptr<MousePositionProvider>& mousePosProvider);
    ~InputDevice();
    void UpdateInputState();
    void ResetInputState();
    KeyState GetKeyState(Key key);
    KeyState GetMouseState(MouseButton btn) const;
    KeyState GetPadButtonState(PadButton btn) const;
    int GetMouseX(int screenWidth, int screenHeight) const;
    int GetMouseY(int screenWidth, int screenHeight) const;
    int GetMouseMoveZ() const;
    void SetMousePositionProvider(const std::shared_ptr<MousePositionProvider>& provider);
    void SetInputEnable(bool enable);
    static constexpr int MaxKey = 255;
    static constexpr int MaxPadButton = 255;
private:
    void InitPadDevice();
    HWND hWnd_;
    IDirectInput8* dInput_;
    IDirectInputDevice8* keyboardDevice_;
    IDirectInputDevice8* mouseDevice_;
    IDirectInputDevice8* padDevice_; // Nullable
    std::array<BYTE, MaxKey + 1> keyInputStates_;
    std::array<BYTE, MaxKey + 1> prevKeyInputStates_;
    std::array<BYTE, 4> mouseButtonInputStates_;
    std::array<BYTE, 4> prevMouseButtonInputStates_;
    DIJOYSTATE* padInputState_;
    DIJOYSTATE* prevPadInputState_;
    int mouseMoveZ_;
    std::shared_ptr<MousePositionProvider> mousePosProvider_;
    bool inputEnable_;
};
}