#pragma once

#include <bstorm/virtual_key_input_source.hpp>

#include <unordered_map>
#include <memory>

namespace bstorm
{
class InputDevice;

class KeyAssign
{
public:
    void AddVirtualKey(VirtualKey vkey, Key key, PadButton padButton);
    Key GetAssignedKey(VirtualKey vkey) const;
    PadButton GetAssignedPadButton(VirtualKey vkey) const;
private:
    std::unordered_map<VirtualKey, std::pair<Key, PadButton>> keyMap_;
};

class RealDeviceInputSource : public VirtualKeyInputSource
{
public:
    RealDeviceInputSource(const std::shared_ptr<InputDevice>& inputDevice, const std::shared_ptr<KeyAssign>& keyAssign);
    ~RealDeviceInputSource() override;
    KeyState GetVirtualKeyState(VirtualKey vkey) override;
    void SetVirtualKeyState(VirtualKey vkey, KeyState state) override;
private:
    std::unordered_map<VirtualKey, KeyState> directSettedVirtualKeyStates_;
    std::shared_ptr<KeyAssign> keyAssign_;
    std::shared_ptr<InputDevice> inputDevice_;
};
}