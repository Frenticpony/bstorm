#pragma once

#include <bstorm/type.hpp>
#include <bstorm/non_copyable.hpp>
#include <bstorm/color_rgb.hpp>

#include <windows.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <d3d9.h>

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

namespace bstorm
{
class Font : private NonCopyable
{
public:
    Font(const FontParams& params, HWND hWnd, IDirect3DDevice9* d3DDevice_, int borderedFontQuality);
    ~Font();
    int GetWidth() const { return width_; }
    int GetHeight() const { return height_; }
    int GetTextureWidth() const { return textureWidth_; }
    int GetTextureHeight() const { return textureHeight_; }
    int GetPrintOffsetX() const { return printOffsetX_; }
    int GetPrintOffsetY() const { return printOffsetY_; }
    int GetRightCharOffsetX() const { return rightCharOffsetX_; }
    int GetNextLineOffsetY() const { return nextLineOffsetY_; }
    IDirect3DTexture9* GetTexture() const { return texture_; }
    const FontParams& GetParams() const { return params_; }
private:
    FontParams params_;
    IDirect3DTexture9 *texture_; // セル(文字画像)を格納したテクスチャ, 綺麗に出力できるように高さと幅は2のn乗に揃える
    int width_;  // セルの幅
    int height_; // セルの高さ
    /* セルの左上を置いた位置から描画開始位置までの距離 */
    int printOffsetX_;
    int printOffsetY_;
    int rightCharOffsetX_; // 右に来る文字のまでの距離
    int nextLineOffsetY_; // 次の行までの距離
    int textureWidth_; // テクスチャの幅(2のn乗)
    int textureHeight_; // テクスチャの高さ(2のn乗)
};

class FontCache
{
public:
    FontCache(HWND hWnd, IDirect3DDevice9* d3DDevice_);
    std::shared_ptr<Font> Create(const FontParams& params);
    void SetBorderedFontQuality(int q); // 1 ~ 4
    void ReleaseUnusedFont();
    const std::unordered_map<FontParams, std::shared_ptr<Font>>& GetFontMap() const { return fontMap_; }
private:
    HWND hWnd_;
    IDirect3DDevice9* d3DDevice_;
    std::unordered_map<FontParams, std::shared_ptr<Font>> fontMap_;
    int borderedFontQuality_;
};

bool InstallFont(const std::wstring& path);
}