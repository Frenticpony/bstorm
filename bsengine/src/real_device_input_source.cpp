#include <bstorm/real_device_input_source.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/input_device.hpp>

namespace bstorm {
  void KeyAssign::addVirtualKey(VirtualKey vkey, Key key, PadButton padButton) {
    keyMap[vkey] = std::make_pair(key, padButton);
  }

  Key KeyAssign::getAssignedKey(VirtualKey vkey) const {
    auto it = keyMap.find(vkey);
    if (it != keyMap.end()) return it->second.first;
    return KEY_INVALID;
  }

  PadButton KeyAssign::getAssignedPadButton(VirtualKey vkey) const {
    auto it = keyMap.find(vkey);
    if (it != keyMap.end()) return it->second.second;
    return KEY_INVALID;
  }

  RealDeviceInputSource::RealDeviceInputSource(const std::shared_ptr<InputDevice>& inputDevice, const std::shared_ptr<KeyAssign>& keyAssign) :
    inputDevice(inputDevice),
    keyAssign(keyAssign)
  {
  }

  RealDeviceInputSource::~RealDeviceInputSource() { }

  KeyState RealDeviceInputSource::getVirtualKeyState(VirtualKey vk) {
    // 優先順
    // 1. SetVirtualKeyStateでセットされた状態
    // 2. 割り当てられたキーボードのキーの状態
    // 3. 割り当てられたパッドのボタンの状態
    {
      auto it = directSettedVirtualKeyStates.find(vk);
      if (it != directSettedVirtualKeyStates.end()) {
        return it->second;
      }
    }
    auto keyState = inputDevice->getKeyState(keyAssign->getAssignedKey(vk));
    if (keyState != KEY_FREE) {
      return keyState;
    }
    return inputDevice->getPadButtonState(keyAssign->getAssignedPadButton(vk));
  }

  void RealDeviceInputSource::setVirtualKeyState(VirtualKey vkey, KeyState state) {
    directSettedVirtualKeyStates[vkey] = state;
  }
}
