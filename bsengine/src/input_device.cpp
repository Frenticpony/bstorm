#define	DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/input_device.hpp>

namespace bstorm {
  static BOOL CALLBACK enumAxesCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID dev) {
    DIPROPRANGE diprg;

    diprg.diph.dwSize = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow = DIPH_BYID;
    diprg.diph.dwObj = pdidoi->dwType;
    diprg.lMin = -1000;
    diprg.lMax = +1000;

    IDirectInputDevice8* padDevice = (IDirectInputDevice8*)dev;

    if (FAILED(padDevice->SetProperty(DIPROP_RANGE, &diprg.diph))) {
      return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
  }

  InputDevice::InputDevice(HWND hWnd, const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY, int screenWidth, int screenHeight, const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight) :
    hWnd(hWnd),
    dInput(NULL),
    keyboardDevice(NULL),
    mouseDevice(NULL),
    padDevice(NULL),
    padInputState(NULL),
    prevPadInputState(NULL),
    mouseMoveZ(0),
    screenPosX(screenPosX),
    screenPosY(screenPosY),
    screenWidth(screenWidth),
    screenHeight(screenHeight),
    gameViewWidth(gameViewWidth),
    gameViewHeight(gameViewHeight)
  {
    try {
      if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&dInput, NULL))) {
        throw std::runtime_error("failed to init input device.");
      }

      auto keyboardInitFailed = std::runtime_error("failed to init keyboard.");
      auto mouseInitFailed = std::runtime_error("failed to init mouse.");

      // init keyboard
      if (FAILED(dInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL))) {
        throw keyboardInitFailed;
      }

      if (FAILED(keyboardDevice->SetDataFormat(&c_dfDIKeyboard))) {
        throw keyboardInitFailed;
      }

      if (FAILED(keyboardDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        throw keyboardInitFailed;
      }

      // init mouse
      if (FAILED(dInput->CreateDevice(GUID_SysMouse, &mouseDevice, NULL))) {
        throw mouseInitFailed;
      }

      if (FAILED(mouseDevice->SetDataFormat(&c_dfDIMouse))) {
        throw mouseInitFailed;
      }

      if (FAILED(mouseDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
        throw mouseInitFailed;
      }

      {
        DIPROPDWORD diprop;
        diprop.diph.dwSize = sizeof(diprop);
        diprop.diph.dwHeaderSize = sizeof(diprop.diph);
        diprop.diph.dwObj = 0;
        diprop.diph.dwHow = DIPH_DEVICE;
        diprop.dwData = DIPROPAXISMODE_REL;	// 相対軸

        if (FAILED(mouseDevice->SetProperty(DIPROP_AXISMODE, &diprop.diph))) {
          throw mouseInitFailed;
        }
      }

      initPadDevice();
      resetInputState();
    } catch (...) {
      safe_delete(padInputState);
      safe_delete(prevPadInputState);
      safe_release(keyboardDevice);
      safe_release(mouseDevice);
      safe_release(padDevice);
      safe_release(dInput);
      throw;
    }
  }

  InputDevice::~InputDevice() {
    delete padInputState;
    delete prevPadInputState;
    keyboardDevice->Release();
    mouseDevice->Release();
    safe_release(padDevice);
    dInput->Release();
  }

  void InputDevice::updateInputState() {
    keyboardDevice->Poll();
    while (keyboardDevice->Acquire() == DIERR_INPUTLOST);

    std::swap(prevKeyInputStates, keyInputStates);
    keyboardDevice->GetDeviceState(keyInputStates.size(), keyInputStates.data());

    mouseDevice->Poll();
    while (mouseDevice->Acquire() == DIERR_INPUTLOST);

    DIMOUSESTATE mouseInputState;
    mouseDevice->GetDeviceState(sizeof(mouseInputState), &mouseInputState);
    std::swap(prevMouseButtonInputStates, mouseButtonInputStates);
    for (int i = 0; i < 4; i++) {
      mouseButtonInputStates[i] = mouseInputState.rgbButtons[i];
    }
    mouseMoveZ = mouseInputState.lZ;

    if (padDevice) {
      padDevice->Poll();
      while (padDevice->Acquire() == DIERR_INPUTLOST);
      std::swap(prevPadInputState, padInputState);
      padDevice->GetDeviceState(sizeof(DIJOYSTATE), padInputState);
    } else {
      initPadDevice();
    }
  }

  void InputDevice::resetInputState() {
    keyInputStates.fill(0);
    prevKeyInputStates.fill(0);
    mouseButtonInputStates.fill(0);
    prevMouseButtonInputStates.fill(0);
    safe_delete(padInputState);
    safe_delete(prevPadInputState);
    padInputState = new DIJOYSTATE();
    prevPadInputState = new DIJOYSTATE();
    memset(padInputState, 0, sizeof(DIJOYSTATE));
    memset(prevPadInputState, 0, sizeof(DIJOYSTATE));
    mouseMoveZ = 0;
  }

  KeyState InputDevice::getKeyState(Key k) {
    if (k < 0 || k > 255) return KEY_FREE;
    return ((prevKeyInputStates[k] >> 6) | (keyInputStates[k] >> 7));
  }

  KeyState InputDevice::getMouseState(MouseButton btn) const {
    if (btn < 0 || btn > 2) return KEY_FREE;
    //MOUSE_LEFT = 0;
    //MOUSE_RIGHT = 1;
    //MOUSE_MIDDLE = 2;
    return ((prevMouseButtonInputStates[btn] >> 6) | (mouseButtonInputStates[btn] >> 7));
  }

  KeyState InputDevice::getPadButtonState(PadButton btn) const {
    // 0-3は十字キー用
    switch (btn) {
      case 0: // left
        return ((prevPadInputState->lX < 0) << 1) | (padInputState->lX < 0);
      case 1: // right
        return ((prevPadInputState->lX > 0) << 1) | (padInputState->lX > 0);
      case 2: // up
        return ((prevPadInputState->lY < 0) << 1) | (padInputState->lY < 0);
      case 3: // down
        return ((prevPadInputState->lY > 0) << 1) | (padInputState->lY > 0);
      default:
        btn -= 4;
        if (btn < 0 || btn > 31) return KEY_FREE;
        return ((prevPadInputState->rgbButtons[btn] >> 6) | (padInputState->rgbButtons[btn] >> 7));
    }
  }

  void InputDevice::setScreenPos(const std::shared_ptr<int>& posX, const std::shared_ptr<int>& posY) {
    screenPosX = posX;
    screenPosY = posY;
  }

  void InputDevice::setGameViewSize(const std::shared_ptr<int>& width, const std::shared_ptr<int>& height) {
    gameViewWidth = width;
    gameViewHeight = height;
  }

  void InputDevice::getMousePos(int& x, int& y) const {
    POINT point;
    x = 0; y = 0;
    if (GetCursorPos(&point)) {
      if (ScreenToClient(hWnd, &point)) {
        if (screenPosX) { point.x -= *screenPosX; }
        if (screenPosY) { point.y -= *screenPosY; }
        int windowWidth = screenWidth;
        int windowHeight = screenHeight;
        if (gameViewWidth && gameViewHeight) {
          windowWidth = *gameViewWidth;
          windowHeight = *gameViewHeight;
        } else {
          RECT clientRect;
          if (GetClientRect(hWnd, &clientRect)) {
            windowWidth = clientRect.right;
            windowHeight = clientRect.bottom;
          }
        }
        point.x = 1.0f * point.x * screenWidth / windowWidth;
        point.y = 1.0f * point.y * screenHeight / windowHeight;
        x = point.x; y = point.y;
      }
    }
  }

  int InputDevice::getMouseX() const {
    int x, y;
    getMousePos(x, y);
    return x;
  }

  int InputDevice::getMouseY() const {
    int x, y;
    getMousePos(x, y);
    return y;
  }
  int InputDevice::getMouseMoveZ() const {
    return mouseMoveZ;
  }

  void InputDevice::initPadDevice() {
    safe_release(padDevice);
    // init pad
    if (FAILED(dInput->CreateDevice(GUID_Joystick, &padDevice, NULL))) {
      safe_release(padDevice);
      return;
    }

    if (FAILED(padDevice->SetDataFormat(&c_dfDIJoystick))) {
      safe_release(padDevice);
      return;
    }

    if (FAILED(padDevice->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND))) {
      safe_release(padDevice);
      return;
    }

    {
      DIPROPDWORD diprop;
      diprop.diph.dwSize = sizeof(diprop);
      diprop.diph.dwHeaderSize = sizeof(diprop.diph);
      diprop.diph.dwObj = 0;
      diprop.diph.dwHow = DIPH_DEVICE;
      diprop.dwData = DIPROPAXISMODE_ABS;	// 絶対軸

      if (FAILED(padDevice->SetProperty(DIPROP_AXISMODE, &diprop.diph))) {
        safe_release(padDevice);
        return;
      }
    }

    if (FAILED(padDevice->EnumObjects(enumAxesCallback, padDevice, DIDFT_AXIS))) {
      safe_release(padDevice);
      return;
    }
  }
}