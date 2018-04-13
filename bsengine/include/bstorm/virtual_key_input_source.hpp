#pragma once

#include <bstorm/type.hpp>

namespace bstorm
{
class VirtualKeyInputSource
{
public:
    virtual ~VirtualKeyInputSource() {};
    virtual KeyState getVirtualKeyState(VirtualKey vkey) = 0;
    virtual void setVirtualKeyState(VirtualKey vkey, KeyState state) = 0;
};
}