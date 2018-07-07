#include <bstorm/font.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/thread_util.hpp>

namespace bstorm
{
static ColorRGB LerpColor(float y, float height, const ColorRGB& topColor, const ColorRGB& bottomColor)
{
    const float k = 1.0f - y / (height - 1.0f);
    int fontR = (int)(topColor.GetR() * k + bottomColor.GetR() * (1 - k));
    int fontG = (int)(topColor.GetG() * k + bottomColor.GetG() * (1 - k));
    int fontB = (int)(topColor.GetB() * k + bottomColor.GetB() * (1 - k));
    return ColorRGB(fontR, fontG, fontB);
}

// FUTURE : error check
Font::Font(const FontParams& params, HWND hWnd, IDirect3DDevice9* d3DDevice_, int quality) :
    texture_(NULL),
    params_(params)
{
    if (params_.borderType == BORDER_NONE)
    {
        HFONT hFont = CreateFont(params_.size, 0, 0, 0, params_.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params_.fontName.c_str());
        HDC hDC = GetDC(hWnd);
        HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
        TEXTMETRIC tm;
        GetTextMetrics(hDC, &tm);
        UINT code = (UINT)params_.c;
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
        const int texWidth = NextPow2(fontWidth);
        const int texHeight = NextPow2(fontHeight);
        d3DDevice_->CreateTexture(texWidth, texHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture_, NULL);
        D3DLOCKED_RECT texRect;
        texture_->LockRect(0, &texRect, NULL, D3DLOCK_DISCARD);
        D3DCOLOR* texMem = (D3DCOLOR*)texRect.pBits;

        const int bmpWidth = (fontWidth + 3) & ~3; // DWORD-align
        ParallelTimes(fontHeight, [&](int y)
        {
            for (int x = 0; x < fontWidth; x++)
            {
                const int bmpPos = y * bmpWidth + x;
                const int texPos = y * texWidth + x;
                const BYTE alpha = (BYTE)(fontBmp[bmpPos] * 255.0 / (grad - 1));
                if (alpha != 0)
                {
                    texMem[texPos] = LerpColor(y, fontHeight, params_.topColor, params_.bottomColor).ToD3DCOLOR(alpha);
                } else
                {
                    // BLEND_ADD_RGB時に色が加算されないようにするため0
                    // BLEND_MULTIPLYの時は色が黒になってしまうが、アルファテストでa=0を描画しないようにしているので大丈夫
                    texMem[texPos] = 0;
                }
            }
        });

        texture_->UnlockRect(0);
        delete[] fontBmp;

        this->width_ = fontWidth;
        this->height_ = fontHeight;
        this->printOffsetX_ = gm.gmptGlyphOrigin.x;
        this->printOffsetY_ = tm.tmAscent - gm.gmptGlyphOrigin.y;
        this->rightCharOffsetX_ = gm.gmCellIncX;
        this->nextLineOffsetY_ = tm.tmHeight;
        this->textureWidth_ = texWidth;
        this->textureHeight_ = texHeight;
    } else
    {
        // BORDER_SHADOWは廃止

        /* GDIで大きく描いてから、SSAAしてテクスチャに書き込む */
        HDC hDC = GetDC(hWnd);
        HDC memDC = CreateCompatibleDC(hDC);

        /* フォントの作成 */
        /** quality倍の大きさで取得 */
        HFONT hFont = CreateFont(params_.size * quality, 0, 0, 0, params_.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params_.fontName.c_str());
        HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);

        /* フォントのパラメータ取得 */
        TEXTMETRIC tm;
        GLYPHMETRICS gm;
        const MAT2 mat = { { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 1 } };
        GetTextMetrics(memDC, &tm);
        GetGlyphOutline(memDC, (UINT)params_.c, GGO_METRICS, &gm, 0, NULL, &mat);

        const int penSize = 2 * params_.borderWidth * quality;

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
        TextOut(memDC, -drawOffsetX, -drawOffsetY, (std::wstring{ params_.c }).c_str(), 1);
        EndPath(memDC);
        StrokePath(memDC);

        //文字を塗りつぶす
        BeginPath(memDC);
        TextOut(memDC, -drawOffsetX, -drawOffsetY, (std::wstring{ params_.c }).c_str(), 1);
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
        const int texWidth = NextPow2(fontWidth);
        const int texHeight = NextPow2(fontHeight);

        d3DDevice_->CreateTexture(texWidth, texHeight, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &texture_, NULL);
        D3DLOCKED_RECT texRect;
        texture_->LockRect(0, &texRect, NULL, D3DLOCK_DISCARD);
        D3DCOLOR* texMem = (D3DCOLOR*)texRect.pBits;

        /* フォントビットマップからテクスチャに書き込み */
        const int bmpWidth = (fontRect.right * 3 + 3) & ~3;
        const int q2 = quality * quality;
        ParallelTimes(fontHeight, [&](int y)
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

                const BYTE alpha = 0xff - bg;

                if (alpha == 0)
                {
                    texMem[texPos] = 0;
                } else if (alpha < 0xff)
                {
                    texMem[texPos] = D3DCOLOR_ARGB(alpha, params_.borderColor.GetR(), params_.borderColor.GetG(), params_.borderColor.GetB());
                } else
                {
                    /* フォントの色を線形補間で生成 */
                    const ColorRGB fontColor = LerpColor(y, fontHeight, params_.topColor, params_.bottomColor);
                    /* 混ぜる */
                    const int r = ((fontColor.GetR() * ch) >> 8) + ((params_.borderColor.GetR() * border) >> 8);
                    const int g = ((fontColor.GetG() * ch) >> 8) + ((params_.borderColor.GetG() * border) >> 8);
                    const int b = ((fontColor.GetB() * ch) >> 8) + ((params_.borderColor.GetB() * border) >> 8);
                    texMem[texPos] = D3DCOLOR_ARGB(alpha, (BYTE)r, (BYTE)g, (BYTE)b);
                }
            }
        });
        texture_->UnlockRect(0);
        SelectObject(memDC, hOldBmp); DeleteObject(hBmp);
        DeleteDC(memDC);

        /* フィールド設定 */
        hFont = CreateFont(params_.size, 0, 0, 0, params_.weight, 0, 0, 0, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, params_.fontName.c_str());
        hOldFont = (HFONT)SelectObject(hDC, hFont);
        GetTextMetrics(hDC, &tm);
        GetGlyphOutline(hDC, (UINT)params_.c, GGO_METRICS, &gm, 0, NULL, &mat);
        SelectObject(hDC, hOldFont); DeleteObject(hFont);
        ReleaseDC(hWnd, hDC);
        this->width_ = fontWidth;
        this->height_ = fontHeight;
        this->printOffsetX_ = gm.gmptGlyphOrigin.x;
        this->printOffsetY_ = tm.tmAscent - gm.gmptGlyphOrigin.y;
        this->rightCharOffsetX_ = gm.gmCellIncX + params_.borderWidth;
        this->nextLineOffsetY_ = tm.tmHeight;
        this->textureWidth_ = texWidth;
        this->textureHeight_ = texHeight;
    }
}

Font::~Font()
{
    safe_release(texture_);
}

FontCache::FontCache(HWND hWnd, IDirect3DDevice9 * d3DDevice_) :
    hWnd_(hWnd),
    d3DDevice_(d3DDevice_),
    borderedFontQuality_(4)
{
}

std::shared_ptr<Font> FontCache::Create(const FontParams& params)
{
    auto it = fontMap_.find(params);
    if (it != fontMap_.end())
    {
        return it->second;
    } else
    {
        return fontMap_[params] = std::make_shared<Font>(params, hWnd_, d3DDevice_, borderedFontQuality_);
    }
}

void FontCache::SetBorderedFontQuality(int q)
{
    borderedFontQuality_ = q;
}

void FontCache::ReleaseUnusedFont()
{
    auto it = fontMap_.begin();
    while (it != fontMap_.end())
    {
        auto& font = it->second;
        if (font.use_count() <= 1)
        {
            fontMap_.erase(it++);
        } else ++it;
    }
}

bool InstallFont(const std::wstring& path)
{
    int result = AddFontResourceEx(path.c_str(), FR_PRIVATE, NULL);
    return result != 0;
}
}