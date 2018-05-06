#pragma once

#include <bstorm/obj_render.hpp>

#include <d3dx9.h>

namespace bstorm
{
class Font;
class FontCache;
class ObjText : public ObjRender
{
public:
    ObjText(const std::shared_ptr<GameState>& state);
    ~ObjText();
    void Update() override;
    void Render(const std::unique_ptr<Renderer>& renderer) override;
    const std::wstring& GetText() const;
    void SetText(const std::wstring& t);
    const std::wstring& GetFontName() const;
    void SetFontName(const std::wstring& name);
    int GetFontSize() const;
    void SetFontSize(int s);
    bool IsFontBold() const;
    void SetFontBold(bool b);
    const ColorRGB& GetFontColorTop() const;
    void SetFontColorTop(int r, int g, int b);
    const ColorRGB& GetFontColorBottom() const;
    void SetFontColorBottom(int r, int g, int b);
    int GetFontBorderType() const;
    void SetFontBorderType(int t);
    int GetFontBorderWidth() const;
    void SetFontBorderWidth(int w);
    const ColorRGB& GetFontBorderColor() const;
    void SetFontBorderColor(int r, int g, int b);
    int GetLinePitch() const;
    void SetLinePitch(int pitch) { linePitch_ = pitch; }
    int GetSidePitch() const;
    void SetSidePitch(int pitch) { sidePitch_ = pitch; }
    bool IsAutoTransCenterEnabled() const;
    void SetAutoTransCenter(bool enable) { autoTransCenterEnable_ = enable; }
    bool IsSyntacticAnalysisEnabled() const;
    void SetSyntacticAnalysis(bool enable);
    int GetHorizontalAlignment() const;
    void SetHorizontalAlignment(int alignmentType);
    float GetTransCenterX() const;
    float GetTransCenterY() const;
    void SetTransCenter(float x, float y) { transCenterX_ = x; transCenterY_ = y; }
    int GetMaxWidth() const;
    void SetMaxWidth(int w) { maxWidth_ = w; }
    int GetMaxHeight() const;
    void SetMaxHeight(int h) { maxHeight_ = h; }
    int GetTotalWidth() const;
    int GetTotalHeight() const;
    int GetTextLength() const;
    int GetTextLengthCU() const;
    std::vector<int> GetTextLengthCUL() const;
    bool IsFontParamModified() const;
    void GenerateFonts();
    const std::vector<std::shared_ptr<Font>>& GetBodyFonts() const;
    template <typename T>
    struct Ruby
    {
        Ruby(int begin, int end, const T& text) :
            begin(begin),
            end(end),
            text(text)
        {
        }
        int begin;
        int end;
        T text;
    };
    const std::vector<Ruby<std::vector<std::shared_ptr<Font>>>>& GetRubyFonts() const;
    static void ParseRubiedString(const std::wstring& src, std::wstring& bodyText, std::vector<Ruby<std::wstring>>& rubies);
private:
    int GetNextLineOffsetY() const;
    void RenderFont(const std::shared_ptr<Font>& font, const D3DXMATRIX& worldMatrix, const std::unique_ptr<Renderer>& renderer);
    std::wstring text_;
    std::wstring bodyText_;
    std::vector<Ruby<std::wstring>> rubies_;
    std::wstring fontName_;
    int size_;
    bool isBold_;
    ColorRGB topColor_;
    ColorRGB bottomColor_;
    int borderType_;
    int borderWidth_;
    ColorRGB borderColor_;
    int linePitch_;
    int sidePitch_;
    float transCenterX_;
    float transCenterY_;
    bool autoTransCenterEnable_;
    int horizontalAlignment_;
    bool syntacticAnalysisEnable_;
    int maxWidth_;
    int maxHeight_;
    bool isFontParamModified_;
    std::vector<std::shared_ptr<Font>> bodyFonts_;
    std::vector<Ruby<std::vector<std::shared_ptr<Font>>>> rubyFonts_;
};
}
