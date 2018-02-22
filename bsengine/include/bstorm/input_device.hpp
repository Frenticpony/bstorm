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
  class MousePositionProvider {
  public:
    virtual ~MousePositionProvider() {};
    virtual void getMousePos(int screenWidth, int screenHeight, int &x, int &y) = 0;
  };

  class WinMousePositionProvider : public MousePositionProvider {
  public:
    WinMousePositionProvider(HWND hWnd);
    void getMousePos(int screenWidth, int screenHeight, int &x, int &y) override;
  private:
    HWND hWnd;
  };

  class InputDevice : private NonCopyable {
  public:
    InputDevice(HWND hWnd, const std::shared_ptr<MousePositionProvider>& mousePosProvider);
    ~InputDevice();
    void updateInputState();
    void resetInputState();
    KeyState getKeyState(Key key);
    KeyState getMouseState(MouseButton btn) const;
    KeyState getPadButtonState(PadButton btn) const;
    int getMouseX(int screenWidth, int screenHeight) const;
    int getMouseY(int screenWidth, int screenHeight) const;
    int getMouseMoveZ() const;
    void setMousePositionProvider(const std::shared_ptr<MousePositionProvider>& provider);
    void setInputEnable(bool enable);
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
    std::shared_ptr<MousePositionProvider> mousePosProvider;
    bool inputEnable;
  };
}