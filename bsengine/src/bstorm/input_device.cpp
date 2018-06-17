#include <bstorm/input_device.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>

namespace bstorm
{
static BOOL CALLBACK enumAxesCallback(LPCDIDEVICEOBJECTINSTANCE pdidoi, LPVOID dev)
{
    DIPROPRANGE diprg;

    diprg.diph.dwSize = sizeof(DIPROPRANGE);
    diprg.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprg.diph.dwHow = DIPH_BYID;
    diprg.diph.dwObj = pdidoi->dwType;
    diprg.lMin = -1000;
    diprg.lMax = +1000;

    IDirectInputDevice8* padDevice = (IDirectInputDevice8*)dev;

    if (FAILED(padDevice->SetProperty(DIPROP_RANGE, &diprg.diph)))
    {
        return DIENUM_STOP;
    }
    return DIENUM_CONTINUE;
}

InputDevice::InputDevice(HWND hWnd) :
    hWnd_(hWnd),
    dInput_(NULL),
    keyboardDevice_(NULL),
    mouseDevice_(NULL),
    padDevice_(NULL),
    padInputState_(NULL),
    prevPadInputState_(NULL),
    mouseMoveZ_(0),
    mousePosProvider_(nullptr),
    inputEnable_(true)
{
    try
    {
        if (FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *)&dInput_, NULL)))
        {
            throw Log(Log::Level::LV_ERROR).SetMessage("failed to init input device.");
        }

        auto keyboardInitFailed = Log(Log::Level::LV_ERROR).SetMessage("failed to init keyboard.");
        auto mouseInitFailed = Log(Log::Level::LV_ERROR).SetMessage("failed to init mouse.");

        // init keyboard
        if (FAILED(dInput_->CreateDevice(GUID_SysKeyboard, &keyboardDevice_, NULL)))
        {
            throw keyboardInitFailed;
        }

        if (FAILED(keyboardDevice_->SetDataFormat(&c_dfDIKeyboard)))
        {
            throw keyboardInitFailed;
        }

        if (FAILED(keyboardDevice_->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
        {
            throw keyboardInitFailed;
        }

        // init mouse
        if (FAILED(dInput_->CreateDevice(GUID_SysMouse, &mouseDevice_, NULL)))
        {
            throw mouseInitFailed;
        }

        if (FAILED(mouseDevice_->SetDataFormat(&c_dfDIMouse)))
        {
            throw mouseInitFailed;
        }

        if (FAILED(mouseDevice_->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
        {
            throw mouseInitFailed;
        }

        {
            DIPROPDWORD diprop;
            diprop.diph.dwSize = sizeof(diprop);
            diprop.diph.dwHeaderSize = sizeof(diprop.diph);
            diprop.diph.dwObj = 0;
            diprop.diph.dwHow = DIPH_DEVICE;
            diprop.dwData = DIPROPAXISMODE_REL;	// 相対軸

            if (FAILED(mouseDevice_->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
            {
                throw mouseInitFailed;
            }
        }

        InitPadDevice();
        ResetInputState();
    } catch (...)
    {
        safe_delete(padInputState_);
        safe_delete(prevPadInputState_);
        safe_release(keyboardDevice_);
        safe_release(mouseDevice_);
        safe_release(padDevice_);
        safe_release(dInput_);
        throw;
    }
}

InputDevice::~InputDevice()
{
    delete padInputState_;
    delete prevPadInputState_;
    keyboardDevice_->Release();
    mouseDevice_->Release();
    safe_release(padDevice_);
    dInput_->Release();
}

void InputDevice::UpdateInputState()
{
    keyboardDevice_->Poll();
    while (keyboardDevice_->Acquire() == DIERR_INPUTLOST);

    std::swap(prevKeyInputStates_, keyInputStates_);
    keyboardDevice_->GetDeviceState(keyInputStates_.size(), keyInputStates_.data());

    mouseDevice_->Poll();
    while (mouseDevice_->Acquire() == DIERR_INPUTLOST);

    DIMOUSESTATE mouseInputState;
    mouseDevice_->GetDeviceState(sizeof(mouseInputState), &mouseInputState);
    std::swap(prevMouseButtonInputStates_, mouseButtonInputStates_);
    for (int i = 0; i < 4; i++)
    {
        mouseButtonInputStates_[i] = mouseInputState.rgbButtons[i];
    }
    mouseMoveZ_ = mouseInputState.lZ;

    if (padDevice_)
    {
        padDevice_->Poll();
        while (padDevice_->Acquire() == DIERR_INPUTLOST);
        std::swap(prevPadInputState_, padInputState_);
        padDevice_->GetDeviceState(sizeof(DIJOYSTATE), padInputState_);
    } else
    {
        InitPadDevice();
    }
}

void InputDevice::ResetInputState()
{
    keyInputStates_.fill(0);
    prevKeyInputStates_.fill(0);
    mouseButtonInputStates_.fill(0);
    prevMouseButtonInputStates_.fill(0);
    safe_delete(padInputState_);
    safe_delete(prevPadInputState_);
    padInputState_ = new DIJOYSTATE();
    prevPadInputState_ = new DIJOYSTATE();
    memset(padInputState_, 0, sizeof(DIJOYSTATE));
    memset(prevPadInputState_, 0, sizeof(DIJOYSTATE));
    mouseMoveZ_ = 0;
}

KeyState InputDevice::GetKeyState(Key k) const
{
    if (!inputEnable_) return KEY_FREE;
    if (k < 0 || k > 255) return KEY_FREE;
    return ((prevKeyInputStates_[k] >> 6) | (keyInputStates_[k] >> 7));
}

KeyState InputDevice::GetMouseState(MouseButton btn) const
{
    if (!inputEnable_) return KEY_FREE;
    if (btn < 0 || btn > 2) return KEY_FREE;
    //MOUSE_LEFT : 0
    //MOUSE_RIGHT : 1
    //MOUSE_MIDDLE : 2
    return ((prevMouseButtonInputStates_[btn] >> 6) | (mouseButtonInputStates_[btn] >> 7));
}

KeyState InputDevice::GetPadButtonState(PadButton btn) const
{
    if (!inputEnable_) return KEY_FREE;
    // 0-3は十字キー用
    switch (btn)
    {
        case 0: // left
            return ((prevPadInputState_->lX < 0) << 1) | (padInputState_->lX < 0);
        case 1: // right
            return ((prevPadInputState_->lX > 0) << 1) | (padInputState_->lX > 0);
        case 2: // up
            return ((prevPadInputState_->lY < 0) << 1) | (padInputState_->lY < 0);
        case 3: // down
            return ((prevPadInputState_->lY > 0) << 1) | (padInputState_->lY > 0);
        default:
            btn -= 4;
            if (btn < 0 || btn > 31) return KEY_FREE;
            return ((prevPadInputState_->rgbButtons[btn] >> 6) | (padInputState_->rgbButtons[btn] >> 7));
    }
}

int InputDevice::GetMouseX(int screenWidth, int screenHeight) const
{
    if (!inputEnable_) return 0;
    int x = 0;
    int y;
    if (mousePosProvider_)
    {
        mousePosProvider_->GetMousePos(screenWidth, screenHeight, x, y);
    }
    return x;
}

int InputDevice::GetMouseY(int screenWidth, int screenHeight) const
{
    if (!inputEnable_) return 0;
    int x;
    int y = 0;
    if (mousePosProvider_)
    {
        mousePosProvider_->GetMousePos(screenWidth, screenHeight, x, y);
    }
    return y;
}

int InputDevice::GetMouseMoveZ() const
{
    if (!inputEnable_) return 0;
    return mouseMoveZ_;
}

void InputDevice::SetMousePositionProvider(const std::shared_ptr<MousePositionProvider>& provider)
{
    mousePosProvider_ = provider;
}

void InputDevice::SetInputEnable(bool enable)
{
    inputEnable_ = enable;
}

void InputDevice::InitPadDevice()
{
    safe_release(padDevice_);
    // init pad
    if (FAILED(dInput_->CreateDevice(GUID_Joystick, &padDevice_, NULL)))
    {
        safe_release(padDevice_);
        return;
    }

    if (FAILED(padDevice_->SetDataFormat(&c_dfDIJoystick)))
    {
        safe_release(padDevice_);
        return;
    }

    if (FAILED(padDevice_->SetCooperativeLevel(hWnd_, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
    {
        safe_release(padDevice_);
        return;
    }

    {
        DIPROPDWORD diprop;
        diprop.diph.dwSize = sizeof(diprop);
        diprop.diph.dwHeaderSize = sizeof(diprop.diph);
        diprop.diph.dwObj = 0;
        diprop.diph.dwHow = DIPH_DEVICE;
        diprop.dwData = DIPROPAXISMODE_ABS;	// 絶対軸

        if (FAILED(padDevice_->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
        {
            safe_release(padDevice_);
            return;
        }
    }

    if (FAILED(padDevice_->EnumObjects(enumAxesCallback, padDevice_, DIDFT_AXIS)))
    {
        safe_release(padDevice_);
        return;
    }
}

WinMousePositionProvider::WinMousePositionProvider(HWND hWnd) : hWnd_(hWnd)
{
}

void WinMousePositionProvider::GetMousePos(int screenWidth, int screenHeight, int & x, int & y)
{
    POINT point;
    x = 0; y = 0;
    if (GetCursorPos(&point))
    {
        if (ScreenToClient(hWnd_, &point))
        {
            int windowWidth = screenWidth;
            int windowHeight = screenHeight;
            RECT clientRect;
            if (GetClientRect(hWnd_, &clientRect))
            {
                windowWidth = clientRect.right;
                windowHeight = clientRect.bottom;
            }
            x = 1.0f * point.x * screenWidth / windowWidth;
            y = 1.0f * point.y * screenHeight / windowHeight;
        }
    }
}
}