#pragma once

#include <bstorm/color_rgb.hpp>

#include <string>

namespace bstorm
{
class FontParams
{
public:
    FontParams();
    FontParams(const std::wstring& fontName, int size, int weight, const ColorRGB& topColor, const ColorRGB& bottomColor, int borderType, int borderWidth, const ColorRGB& borderColor, wchar_t c);
    bool operator==(const FontParams& params) const;
    bool operator!=(const FontParams& params) const;
    size_t hashValue() const;
    std::wstring fontName;
    int size;
    int weight;
    ColorRGB topColor;
    ColorRGB bottomColor;
    int borderType;
    int borderWidth;
    ColorRGB borderColor;
    wchar_t c;
};
}

namespace std
{
template<>
struct hash<bstorm::FontParams>
{
    size_t operator()(const bstorm::FontParams& params) const
    {
        return params.hashValue();
    }
};
}

