#pragma once

#include <windows.h>
#include <unordered_map>
#include <array>
#include <memory>

#include <bstorm/non_copyable.hpp>
#include <bstorm/type.hpp>

struct IDirectInput8;
struct IDirectInputDevice8;
struct DIJOYSTATE;

namespace bstorm {
  class InputDevice : private NonCopyable {
  public:
    InputDevice(HWND hWnd, const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY, int screenWidth, int screenHeight, const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight);
    ~InputDevice();
    void updateInputState();
    void resetInputState();
    KeyState getKeyState(Key key);
    KeyState getMouseState(MouseButton btn) const;
    KeyState getPadButtonState(PadButton btn) const;
    void setScreenPos(const std::shared_ptr<int>& posX, const std::shared_ptr<int>& posY);
    void setGameViewSize(const std::shared_ptr<int>& width, const std::shared_ptr<int>& height);
    void getMousePos(int& x, int& y) const;
    int getMouseX() const;
    int getMouseY() const;
    int getMouseMoveZ() const;
    static constexpr int MaxKey = 255;
    static constexpr int MaxPadButton = 255;
  private:
    void initPadDevice();
    HWND hWnd;
    IDirectInput8* dInput;
    IDirectInputDevice8* keyboardDevice;
    IDirectInputDevice8* mouseDevice;
    IDirectInputDevice8* padDevice; // Nullable
    std::array<BYTE, MaxKey + 1> keyInputStates;
    std::array<BYTE, MaxKey + 1> prevKeyInputStates;
    std::array<BYTE, 4> mouseButtonInputStates;
    std::array<BYTE, 4> prevMouseButtonInputStates;
    DIJOYSTATE* padInputState;
    DIJOYSTATE* prevPadInputState;
    int mouseMoveZ;
    std::shared_ptr<int> screenPosX;
    std::shared_ptr<int> screenPosY;
    int screenWidth;
    int screenHeight;
    std::shared_ptr<int> gameViewWidth;
    std::shared_ptr<int> gameViewHeight;
  };
}