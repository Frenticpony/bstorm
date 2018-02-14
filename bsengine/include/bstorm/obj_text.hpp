#pragma once

#include <bstorm/obj_render.hpp>
#include <d3dx9.h>

namespace bstorm {
  class Font;
  class FontCache;
  class ObjText : public ObjRender {
  public:
    ObjText(const std::shared_ptr<GameState>& state);
    ~ObjText();
    void update() override;
    void render() override;
    const std::wstring& getText() const;
    void setText(const std::wstring& t);
    const std::wstring& getFontName() const;
    void setFontName(const std::wstring& name);
    int getFontSize() const;
    void setFontSize(int s);
    bool isFontBold() const;
    void setFontBold(bool b);
    const ColorRGB& getFontColorTop() const;
    void setFontColorTop(int r, int g, int b);
    const ColorRGB& getFontColorBottom() const;
    void setFontColorBottom(int r, int g, int b);
    int getFontBorderType() const;
    void setFontBorderType(int t);
    int getFontBorderWidth() const;
    void setFontBorderWidth(int w);
    const ColorRGB& getFontBorderColor() const;
    void setFontBorderColor(int r, int g, int b);
    int getLinePitch() const;
    void setLinePitch(int pitch) { linePitch = pitch; }
    int getSidePitch() const;
    void setSidePitch(int pitch) { sidePitch = pitch; }
    bool isAutoTransCenterEnabled() const;
    void setAutoTransCenter(bool enable) { autoTransCenterEnable = enable; }
    bool isSyntacticAnalysisEnabled() const;
    void setSyntacticAnalysis(bool enable);
    int getHorizontalAlignment() const;
    void setHorizontalAlignment(int alignmentType);
    float getTransCenterX() const;
    float getTransCenterY() const;
    void setTransCenter(float x, float y) { transCenterX = x; transCenterY = y; }
    int getMaxWidth() const;
    void setMaxWidth(int w) { maxWidth = w; }
    int getMaxHeight() const;
    void setMaxHeight(int h) { maxHeight = h; }
    int getTotalWidth() const;
    int getTotalHeight() const;
    int getTextLength() const;
    int getTextLengthCU() const;
    std::vector<int> getTextLengthCUL() const;
    bool isFontParamModified() const;
    void generateFonts();
    const std::vector<std::shared_ptr<Font>>& getBodyFonts() const;
    template <typename T>
    struct Ruby {
      Ruby(int begin, int end, const T& text) :
        begin(begin),
        end(end),
        text(text) {}
      int begin;
      int end;
      T text;
    };
    const std::vector<Ruby<std::vector<std::shared_ptr<Font>>>>& getRubyFonts() const;
    static void parseRubiedString(const std::wstring& src, std::wstring& bodyText, std::vector<Ruby<std::wstring>>& rubies);
  protected:
    int getNextLineOffsetY() const;
    void renderFont(const std::shared_ptr<Font>& font, const D3DXMATRIX& worldMatrix);
    std::wstring text;
    std::wstring bodyText;
    std::vector<Ruby<std::wstring>> rubies;
    std::wstring fontName;
    int size;
    bool bold;
    ColorRGB topColor;
    ColorRGB bottomColor;
    int borderType;
    int borderWidth;
    ColorRGB borderColor;
    int linePitch;
    int sidePitch;
    float transCenterX;
    float transCenterY;
    bool autoTransCenterEnable;
    int horizontalAlignment;
    bool syntacticAnalysisEnable;
    int maxWidth;
    int maxHeight;
    bool fontParamModified;
    std::vector<std::shared_ptr<Font>> bodyFonts;
    std::vector<Ruby<std::vector<std::shared_ptr<Font>>>> rubyFonts;
  };
}
