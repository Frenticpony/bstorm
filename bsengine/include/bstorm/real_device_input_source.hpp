#pragma once

#include <bstorm/virtual_key_input_source.hpp>

namespace bstorm {
  class InputDevice;

  class KeyAssign {
  public:
    void addVirtualKey(VirtualKey vkey, Key key, PadButton padButton);
    Key getAssignedKey(VirtualKey vkey) const;
    PadButton getAssignedPadButton(VirtualKey vkey) const;
  private:
    std::unordered_map<VirtualKey, std::pair<Key, PadButton>> keyMap;
  };

  class RealDeviceInputSource : public VirtualKeyInputSource {
  public:
    RealDeviceInputSource(const std::shared_ptr<InputDevice>& inputDevice, const std::shared_ptr<KeyAssign>& keyAssign);
    ~RealDeviceInputSource() override;
    KeyState getVirtualKeyState(VirtualKey vkey) override;
    void setVirtualKeyState(VirtualKey vkey, KeyState state) override;
    std::unordered_map<VirtualKey, KeyState> directSettedVirtualKeyStates;
    std::shared_ptr<KeyAssign> keyAssign;
    std::shared_ptr<InputDevice> inputDevice;
  };
}