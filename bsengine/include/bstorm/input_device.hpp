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
  class KeyConfig {
  public:
    void addVirtualKey(VirtualKey vkey, Key key, PadButton padButton);
    Key getAssignedKey(VirtualKey vkey) const;
    PadButton getAssignedPadButton(VirtualKey vkey) const;
  private:
    std::unordered_map<VirtualKey, std::pair<Key, PadButton>> keyAssign;
  };

  class InputDevice : private NonCopyable {
  public:
    InputDevice(HWND hWnd, const KeyConfig& keyConfig, const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY, int screenWidth, int screenHeight, const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight);
    ~InputDevice();
    void updateInputState();
    void resetInputState();
    KeyState getKeyState(Key key);
    KeyState getVirtualKeyState(VirtualKey vkey);
    void setVirtualKeyState(VirtualKey vkey, KeyState state);
    void addVirtualKey(VirtualKey vkey, Key key, PadButton padButton);
    KeyState getMouseState(MouseButton btn) const;
    KeyState getPadButtonState(PadButton btn) const;
    void setScreenPos(const std::shared_ptr<int>& posX, const std::shared_ptr<int>& posY);
    void setGameViewSize(const std::shared_ptr<int>& width, const std::shared_ptr<int>& height);
    void getMousePos(int& x, int& y) const;
    int getMouseX() const;
    int getMouseY() const;
    int getMouseMoveZ() const;
  private:
    void initPadDevice();
    HWND hWnd;
    IDirectInput8* dInput;
    IDirectInputDevice8* keyboardDevice;
    IDirectInputDevice8* mouseDevice;
    IDirectInputDevice8* padDevice; // Nullable
    std::array<BYTE, 256> keyInputStates;
    std::array<BYTE, 256> prevKeyInputStates;
    std::array<BYTE, 4> mouseButtonInputStates;
    std::array<BYTE, 4> prevMouseButtonInputStates;
    std::unordered_map<VirtualKey, KeyState> directSettedVirtualKeyStates;
    DIJOYSTATE* padInputState;
    DIJOYSTATE* prevPadInputState;
    int mouseMoveZ;
    KeyConfig keyConfig;
    std::shared_ptr<int> screenPosX;
    std::shared_ptr<int> screenPosY;
    int screenWidth;
    int screenHeight;
    std::shared_ptr<int> gameViewWidth;
    std::shared_ptr<int> gameViewHeight;
  };
}