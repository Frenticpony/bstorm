#pragma once

#include <d3d9.h>

namespace bstorm
{
class ColorRGB
{
public:
    ColorRGB();
    ColorRGB(int r, int g, int b);
    bool operator==(const ColorRGB& other) const;
    bool operator!=(const ColorRGB& other) const;
    int GetR() const { return r_; }
    int GetG() const { return g_; }
    int GetB() const { return b_; }
    D3DCOLOR ToD3DCOLOR(int alpha) const;
private:
    int r_;
    int g_;
    int b_;
};

}