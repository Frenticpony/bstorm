#pragma once

#include <windows.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <d3d9.h>

#include <bstorm/type.hpp>
#include <bstorm/non_copyable.hpp>

namespace bstorm {
  struct FontParams {
    FontParams();
    FontParams(const std::wstring& fontName, int size, int weight, const ColorRGB& topColor, const ColorRGB& bottomColor, int borderType, int borderWidth, const ColorRGB& borderColor, wchar_t c);
    std::wstring fontName;
    int size;
    int weight;
    ColorRGB topColor;
    ColorRGB bottomColor;
    int borderType;
    int borderWidth;
    ColorRGB borderColor;
    wchar_t c;
    bool operator==(const FontParams& params) const;
    bool operator!=(const FontParams& params) const;
    size_t hashValue() const;
  };
}

namespace std {
  template<>
  struct hash<bstorm::FontParams> {
    size_t operator()(const bstorm::FontParams& params) const {
      return params.hashValue();
    }
  };
}

namespace bstorm {
  class Font : private NonCopyable {
  public:
    Font(const FontParams& params, HWND hWnd, IDirect3DDevice9* d3DDevice, int borderedFontQuality);
    ~Font();
    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int getTextureWidth() const { return textureWidth; }
    int getTextureHeight() const { return textureHeight; }
    int getPrintOffsetX() const { return printOffsetX; }
    int getPrintOffsetY() const { return printOffsetY; }
    int getRightCharOffsetX() const { return rightCharOffsetX; }
    int getNextLineOffsetY() const { return nextLineOffsetY; }
    IDirect3DTexture9* getTexture() const { return texture; }
    const FontParams& getParams() const { return params; }
  private:
    FontParams params;
    IDirect3DTexture9 *texture; // セル
    int width;  // セルの幅
    int height; // セルの高さ
    /* セルの左上を置いた位置から描画開始位置までの距離 */
    int printOffsetX;
    int printOffsetY;
    int rightCharOffsetX; // 右に来る文字のまでの距離
    int nextLineOffsetY; // 次の行までの距離
    int textureWidth;
    int textureHeight;
  };

  class FontCache {
  public:
    FontCache(HWND hWnd, IDirect3DDevice9* d3DDevice);
    std::shared_ptr<Font> create(const FontParams& params);
    void setBorderedFontQuality(int q); // 1 ~ 4
    void releaseUnusedFont();
    // backdoor
    template <typename T>
    void backDoor() const {}
  private:
    HWND hWnd;
    IDirect3DDevice9* d3DDevice;
    std::unordered_map<FontParams, std::shared_ptr<Font>> fontMap;
    int borderedFontQuality;
  };

  bool installFont(const std::wstring& path);
}