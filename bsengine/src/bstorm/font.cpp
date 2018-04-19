#include <bstorm/font.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>

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
        /* キャッシュ効率化 */
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

size_t FontParams::hashValue() const
{
    size_t h = 0;
    hash_combine(h, fontName);
    hash_combine(h, size);
    hash_combine(h, weight);
    hash_combine(h, bstorm::toD3DCOLOR(topColor, 0xff));
    hash_combine(h, bstorm::toD3DCOLOR(bottomColor, 0xff));
    hash_combine(h, borderType);
    hash_combine(h, borderWidth);
    hash_combine(h, bstorm::toD3DCOLOR(borderColor, 0xff));
    hash_combine(h, c);
    return h;
}

static ColorRGB lerpColor(float y, float height, const ColorRGB& topColor, const ColorRGB& bottomColor)
{
    const float k = 1.0f - y / (height - 1.0f);
    int fontR = (int)(topColor.getR() * k + bottomColor.getR() * (1 - k));
    int fontG = (int)(topColor.getG() * k + bottomColor.getG() * (1 - k));
    int fontB = (int)(topColor.getB() * k + bottomColor.getB() * (1 - k));
    return ColorRGB(fontR, fontG, fontB);
}

// FUTURE : error check
Font::Font(const FontParams& params, HWND hWnd, IDirect3DDevice9* d3DDevice, int quality) :
    texture(NULL),
    params(params)
{
    if (params.borderType == BORDER_NONE)
    {
        HFONT hFont = CreateFont(params.size, 0, 0, 0, params.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params.fontName.c_str());
        HDC hDC = GetDC(hWnd);
        HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
        TEXTMETRIC tm;
        GetTextMetrics(hDC, &tm);
        UINT code = (UINT)params.c;
        MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } }; // 回転行列
        GLYPHMETRICS gm;
        int bmpFormat = GGO_GRAY8_BITMAP; // 65階調
        const BYTE grad = 65; // 階調
        DWORD bmpSize = GetGlyphOutline(hDC, code, bmpFormat, &gm, 0, NULL, &mat);
        BYTE *fontBmp = new BYTE[bmpSize];
        GetGlyphOutline(hDC, code, bmpFormat, &gm, bmpSize, fontBmp, &mat);

        SelectObject(hDC, hOldFont); DeleteObject(hFont);
        ReleaseDC(hWnd, hDC);

        const int fontWidth = gm.gmBlackBoxX;
        const int fontHeight = gm.gmBlackBoxY;
        const int texWidth = nextPow2(fontWidth);
        const int texHeight = nextPow2(fontHeight);
        d3DDevice->CreateTexture(texWidth, texHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, NULL);
        D3DLOCKED_RECT texRect;
        texture->LockRect(0, &texRect, NULL, D3DLOCK_DISCARD);
        D3DCOLOR* texMem = (D3DCOLOR*)texRect.pBits;

        int bmpWidth = (fontWidth + 3) & ~3; // DWORD-align
        for (int y = 0; y < fontHeight; y++)
        {
            for (int x = 0; x < fontWidth; x++)
            {
                int bmpPos = y * bmpWidth + x;
                int texPos = y * texWidth + x;
                BYTE alpha = (BYTE)(fontBmp[bmpPos] * 255.0 / (grad - 1));
                if (alpha != 0)
                {
                    texMem[texPos] = toD3DCOLOR(lerpColor(y, fontHeight, params.topColor, params.bottomColor), alpha);
                } else
                {
                    // BLEND_ADD_RGB時に色が加算されないようにするため0
                    // BLEND_MULTIPLYの時は色が黒になってしまうが、アルファテストでa=0を描画しないようにしているので大丈夫
                    texMem[texPos] = 0;
                }
            }
        }

        texture->UnlockRect(0);
        delete[] fontBmp;

        this->width = fontWidth;
        this->height = fontHeight;
        this->printOffsetX = gm.gmptGlyphOrigin.x;
        this->printOffsetY = tm.tmAscent - gm.gmptGlyphOrigin.y;
        this->rightCharOffsetX = gm.gmCellIncX;
        this->nextLineOffsetY = tm.tmHeight;
        this->textureWidth = texWidth;
        this->textureHeight = texHeight;
    } else
    {
        // BORDER_SHADOWは廃止

        /* GDIで大きく描いてから、SSAAしてテクスチャに書き込む */
        HDC hDC = GetDC(hWnd);
        HDC memDC = CreateCompatibleDC(hDC);

        /* フォントの作成 */
        /** quality倍の大きさで取得 */
        HFONT hFont = CreateFont(params.size * quality, 0, 0, 0, params.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params.fontName.c_str());
        HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);

        /* フォントのパラメータ取得 */
        TEXTMETRIC tm;
        GLYPHMETRICS gm;
        const MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
        GetTextMetrics(memDC, &tm);
        GetGlyphOutline(memDC, (UINT)params.c, GGO_METRICS, &gm, 0, NULL, &mat);

        const int penSize = 2 * params.borderWidth * quality;

        /* フォント矩形 */
        /** 幅と高さがqualityの倍数になるように調整 */
        RECT fontRect = { 0, 0, ((LONG)gm.gmBlackBoxX + penSize + quality - 1) / quality * quality, ((LONG)gm.gmBlackBoxY + penSize + quality - 1) / quality * quality };

        /* フォントビットマップ取得 */
        BITMAPINFO bmpInfo = {};
        bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmpInfo.bmiHeader.biWidth = fontRect.right;
        bmpInfo.bmiHeader.biHeight = -fontRect.bottom;
        bmpInfo.bmiHeader.biPlanes = 1;
        bmpInfo.bmiHeader.biBitCount = 24;
        BYTE* fontBmp = NULL;
        HBITMAP hBmp = CreateDIBSection(0, &bmpInfo, DIB_RGB_COLORS, (void**)&fontBmp, 0, 0);
        HBITMAP hOldBmp = (HBITMAP)SelectObject(memDC, hBmp);

        /* 描画準備、背景、縁取り、文字の色設定 */
        /** 背景：青 */
        HBRUSH hBgBrush = (HBRUSH)CreateSolidBrush(RGB(0, 0, 255));
        FillRect(memDC, &fontRect, hBgBrush);
        DeleteObject(hBgBrush);

        /** 縁取り：緑 */
        HPEN hPen = (HPEN)CreatePen(PS_SOLID, penSize, RGB(0, 255, 0));
        HPEN hOldPen = (HPEN)SelectObject(memDC, hPen);

        /** 文字 : 赤 */
        HBRUSH hBrush = (HBRUSH)CreateSolidBrush(RGB(255, 0, 0));
        HPEN hOldBrush = (HPEN)SelectObject(memDC, hBrush);

        /* 描画 */
        /** 描画開始位置からフォント矩形の左上位置までの距離 */
        const int drawOffsetX = gm.gmptGlyphOrigin.x - penSize / 2;
        const int drawOffsetY = tm.tmAscent - gm.gmptGlyphOrigin.y - penSize / 2;

        SetBkMode(memDC, TRANSPARENT);

        //縁取りを先に描く
        BeginPath(memDC);
        TextOut(memDC, -drawOffsetX, -drawOffsetY, (std::wstring{ params.c }).c_str(), 1);
        EndPath(memDC);
        StrokePath(memDC);

        //文字を塗りつぶす
        BeginPath(memDC);
        TextOut(memDC, -drawOffsetX, -drawOffsetY, (std::wstring{ params.c }).c_str(), 1);
        EndPath(memDC);
        FillPath(memDC);

        /* 後始末 */
        // 別のオブジェクトをSelectしてからDeleteしないと削除されない?
        SelectObject(memDC, hOldFont); DeleteObject(hFont);
        SelectObject(memDC, hOldPen); DeleteObject(hPen);
        SelectObject(memDC, hOldBrush); DeleteObject(hBrush);

        /* テクスチャ取得 */
        const int fontWidth = fontRect.right / quality;
        const int fontHeight = fontRect.bottom / quality;
        const int texWidth = nextPow2(fontWidth);
        const int texHeight = nextPow2(fontHeight);

        d3DDevice->CreateTexture(texWidth, texHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture, NULL);
        D3DLOCKED_RECT texRect;
        texture->LockRect(0, &texRect, NULL, D3DLOCK_DISCARD);
        D3DCOLOR* texMem = (D3DCOLOR*)texRect.pBits;

        /* フォントビットマップからテクスチャに書き込み */
        const int bmpWidth = (fontRect.right * 3 + 3) & ~3;
        const int q2 = quality * quality;
        for (int y = 0; y < fontHeight; y++)
        {
            for (int x = 0; x < fontWidth; x++)
            {
                const int texPos = y * texWidth + x;
                int ch = 0; // 文字
                int border = 0; // 縁取り
                int bg = 0; // 背景
                for (int dy = 0; dy < quality; dy++)
                {
                    for (int dx = 0; dx < quality; dx++)
                    {
                        const int bmpX = x * quality + dx;
                        const int bmpY = y * quality + dy;
                        const int bmpPos = bmpY * bmpWidth + 3 * bmpX;
                        ch += fontBmp[bmpPos + 2]; // R
                        border += fontBmp[bmpPos + 1]; // G
                        bg += fontBmp[bmpPos]; // B
                    }
                }
                ch /= q2;
                border /= q2;
                bg /= q2;

                BYTE alpha = 0xff - bg;

                if (alpha == 0)
                {
                    texMem[texPos] = 0;
                } else if (alpha < 0xff)
                {
                    texMem[texPos] = D3DCOLOR_ARGB(alpha, params.borderColor.getR(), params.borderColor.getG(), params.borderColor.getB());
                } else
                {
                    /* フォントの色を線形補間で生成 */
                    ColorRGB fontColor = lerpColor(y, fontHeight, params.topColor, params.bottomColor);
                    /* 混ぜる */
                    int r = ((fontColor.getR() * ch) >> 8) + ((params.borderColor.getR() * border) >> 8);
                    int g = ((fontColor.getG() * ch) >> 8) + ((params.borderColor.getG() * border) >> 8);
                    int b = ((fontColor.getB() * ch) >> 8) + ((params.borderColor.getB() * border) >> 8);
                    texMem[texPos] = D3DCOLOR_ARGB(alpha, (BYTE)r, (BYTE)g, (BYTE)b);
                }
            }
        }
        texture->UnlockRect(0);
        SelectObject(memDC, hOldBmp); DeleteObject(hBmp);
        DeleteDC(memDC);

        /* フィールド設定 */
        hFont = CreateFont(params.size, 0, 0, 0, params.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params.fontName.c_str());
        hOldFont = (HFONT)SelectObject(hDC, hFont);
        GetTextMetrics(hDC, &tm);
        GetGlyphOutline(hDC, (UINT)params.c, GGO_METRICS, &gm, 0, NULL, &mat);
        SelectObject(hDC, hOldFont); DeleteObject(hFont);
        ReleaseDC(hWnd, hDC);
        this->width = fontWidth;
        this->height = fontHeight;
        this->printOffsetX = gm.gmptGlyphOrigin.x;
        this->printOffsetY = tm.tmAscent - gm.gmptGlyphOrigin.y;
        this->rightCharOffsetX = gm.gmCellIncX + params.borderWidth;
        this->nextLineOffsetY = tm.tmHeight;
        this->textureWidth = texWidth;
        this->textureHeight = texHeight;
    }
}

Font::~Font()
{
    safe_release(texture);
}

FontCache::FontCache(HWND hWnd, IDirect3DDevice9 * d3DDevice) :
    hWnd(hWnd),
    d3DDevice(d3DDevice),
    borderedFontQuality(4)
{
}

std::shared_ptr<Font> FontCache::create(const FontParams& params)
{
    auto it = fontMap.find(params);
    if (it != fontMap.end())
    {
        return it->second;
    } else
    {
        return fontMap[params] = std::make_shared<Font>(params, hWnd, d3DDevice, borderedFontQuality);
    }
}

void FontCache::setBorderedFontQuality(int q)
{
    borderedFontQuality = q;
}

void FontCache::releaseUnusedFont()
{
    auto it = fontMap.begin();
    while (it != fontMap.end())
    {
        auto& font = it->second;
        if (font.use_count() <= 1)
        {
            fontMap.erase(it++);
        } else ++it;
    }
}

bool installFont(const std::wstring& path)
{
    int result = AddFontResourceEx(path.c_str(), FR_PRIVATE, NULL);
    return result != 0;
}
}