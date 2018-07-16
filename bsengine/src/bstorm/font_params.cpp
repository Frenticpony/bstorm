#include <bstorm/font_params.hpp>

#include <bstorm/dnh_const.hpp>

namespace bstorm
{
FontParams::FontParams() :
    size(0),
    weight(0),
    borderType(BORDER_NONE),
    borderWidth(0),
    c(L'\0')
{
}

FontParams::FontParams(const std::wstring& fontName, int size, int weight, const ColorRGB& topColor, const ColorRGB& bottomColor, int borderType, int borderWidth, const ColorRGB& borderColor, wchar_t c) :
    fontName(fontName),
    size(size),
    weight(weight),
    topColor(topColor),
    bottomColor(bottomColor),
    borderType(borderType),
    borderWidth(borderWidth),
    borderColor(borderColor),
    c(c)
{
    if (borderType == BORDER_NONE)
    {
        /* ƒLƒƒƒbƒVƒ…Œø—¦‰» */
        this->borderWidth = 0;
        this->borderColor = ColorRGB(0, 0, 0);
    }
}

bool FontParams::operator==(const FontParams& params) const
{
    return c == params.c &&
        fontName == params.fontName &&
        size == params.size &&
        weight == params.weight &&
        topColor == params.topColor &&
        bottomColor == params.bottomColor &&
        borderType == params.borderType &&
        borderWidth == params.borderWidth &&
        borderColor == params.borderColor;
}

bool FontParams::operator!=(const FontParams& params) const
{
    return !(*this == params);
}

template <class T>
void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

size_t FontParams::hashValue() const
{
    size_t h = 0;
    hash_combine(h, fontName);
    hash_combine(h, size);
    hash_combine(h, weight);
    hash_combine(h, topColor.ToD3DCOLOR(0xff));
    hash_combine(h, bottomColor.ToD3DCOLOR(0xff));
    hash_combine(h, borderType);
    hash_combine(h, borderWidth);
    hash_combine(h, borderColor.ToD3DCOLOR(0xff));
    hash_combine(h, c);
    return h;
}
}