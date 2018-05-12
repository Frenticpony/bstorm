#pragma once

#include <bstorm/key_types.hpp>

namespace bstorm
{
class VirtualKeyInputSource
{
public:
    virtual ~VirtualKeyInputSource() {};
    virtual KeyState GetVirtualKeyState(VirtualKey vkey) = 0;
    virtual void SetVirtualKeyState(VirtualKey vkey, KeyState state) = 0;
};
}