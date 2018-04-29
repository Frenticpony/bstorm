#include <bstorm/real_device_input_source.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/input_device.hpp>

namespace bstorm
{
void KeyAssign::AddVirtualKey(VirtualKey vkey, Key key, PadButton padButton)
{
    keyMap_[vkey] = std::make_pair(key, padButton);
}

Key KeyAssign::GetAssignedKey(VirtualKey vkey) const
{
    auto it = keyMap_.find(vkey);
    if (it != keyMap_.end()) return it->second.first;
    return KEY_INVALID;
}

PadButton KeyAssign::GetAssignedPadButton(VirtualKey vkey) const
{
    auto it = keyMap_.find(vkey);
    if (it != keyMap_.end()) return it->second.second;
    return KEY_INVALID;
}

RealDeviceInputSource::RealDeviceInputSource(const std::shared_ptr<InputDevice>& inputDevice, const std::shared_ptr<KeyAssign>& keyAssign) :
    inputDevice_(inputDevice),
    keyAssign_(keyAssign)
{
}

RealDeviceInputSource::~RealDeviceInputSource() {}

KeyState RealDeviceInputSource::GetVirtualKeyState(VirtualKey vk)
{
    // 優先順
    // 1. SetVirtualKeyStateでセットされた状態
    // 2. 割り当てられたキーボードのキーの状態
    // 3. 割り当てられたパッドのボタンの状態
    {
        auto it = directSettedVirtualKeyStates_.find(vk);
        if (it != directSettedVirtualKeyStates_.end())
        {
            return it->second;
        }
    }
    auto keyState = inputDevice_->GetKeyState(keyAssign_->GetAssignedKey(vk));
    if (keyState != KEY_FREE)
    {
        return keyState;
    }
    return inputDevice_->GetPadButtonState(keyAssign_->GetAssignedPadButton(vk));
}

void RealDeviceInputSource::SetVirtualKeyState(VirtualKey vkey, KeyState state)
{
    directSettedVirtualKeyStates_[vkey] = state;
}
}
