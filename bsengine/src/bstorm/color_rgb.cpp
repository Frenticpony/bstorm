#include <bstorm/color_rgb.hpp>
#include <bstorm/math_util.hpp>

#include <algorithm>

namespace bstorm
{
ColorRGB::ColorRGB() : r_(0xff), g_(0xff), b_(0xff) {}

ColorRGB::ColorRGB(int r, int g, int b) :
    r_(constrain(r, 0, 0xff)),
    g_(constrain(g, 0, 0xff)),
    b_(constrain(b, 0, 0xff))
{
}

bool ColorRGB::operator==(const ColorRGB& other) const
{
    return r_ == other.r_ && g_ == other.g_ && b_ == other.b_;
}

bool ColorRGB::operator!=(const ColorRGB& other) const
{
    return !(*this == other);
}

D3DCOLOR ColorRGB::ToD3DCOLOR(int alpha) const
{
    alpha = constrain(alpha, 0, 0xff);
    return D3DCOLOR_ARGB(alpha, GetR(), GetG(), GetB());
}
}