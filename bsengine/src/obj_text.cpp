#include <algorithm>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/font.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_text.hpp>

namespace bstorm {
  ObjText::ObjText(const std::shared_ptr<GameState>& state) :
    ObjRender(state),
    fontName(L"ＭＳ ゴシック"),
    size(20),
    bold(false),
    topColor({ 0xff, 0xff, 0xff }),
    bottomColor({ 0xff, 0xff, 0xff }),
    borderType(BORDER_NONE),
    borderWidth(0),
    borderColor({ 0xff, 0xff, 0xff }),
    maxWidth(INT_MAX),
    maxHeight(INT_MAX),
    linePitch(4),
    sidePitch(0),
    transCenterX(0),
    transCenterY(0),
    autoTransCenterEnable(true),
    horizontalAlignment(ALIGNMENT_LEFT),
    syntacticAnalysisEnable(true),
    fontParamModified(true)
  {
    setType(OBJ_TEXT);
  }

  ObjText::~ObjText() {
  }

  void ObjText::update() {}

  void ObjText::generateFonts() {
    if (fontParamModified) {
      if (auto state = getGameState()) {
        bodyFonts.clear();
        rubyFonts.clear();
        for (wchar_t c : bodyText) {
          if (c == L'\n') {
            bodyFonts.push_back(std::shared_ptr<Font>());
          } else {
            if (auto fc = state->fontCache) {
              bodyFonts.push_back(fc->create(FontParams(fontName, size, bold ? FW_BOLD : FW_DONTCARE, topColor, bottomColor, borderType, borderWidth, borderColor, c)));
            }
          }
        }
        for (const Ruby<std::wstring>& ruby : rubies) {
          std::vector<std::shared_ptr<Font>> fonts;
          for (wchar_t c : ruby.text) {
            if (auto fc = state->fontCache) {
              fonts.push_back(fc->create(FontParams(fontName, size / 2, FW_BOLD, topColor, bottomColor, borderType, borderWidth / 2, borderColor, c)));
            }
          }
          rubyFonts.emplace_back(ruby.begin, ruby.end, fonts);
        }
      }
      fontParamModified = false;
    }
  }

  const std::vector<std::shared_ptr<Font>>& ObjText::getBodyFonts() const {
    return bodyFonts;
  }

  int ObjText::getNextLineOffsetY() const {
    auto it = std::find_if(bodyFonts.begin(), bodyFonts.end(), [](const std::shared_ptr<Font>& font) { return font; });
    if (it == bodyFonts.end()) return 0;
    auto& font = *it;
    return font->getNextLineOffsetY() + linePitch;
  }

  void ObjText::renderFont(const std::shared_ptr<Font>& font, const D3DXMATRIX& world) {
    std::array<Vertex, 4> vertices;
    D3DCOLOR color = getD3DCOLOR();
    for (auto& vertex : vertices) { vertex.color = color; }
    vertices[1].u = vertices[3].u = 1.0f * font->getWidth() / font->getTextureWidth();
    vertices[2].v = vertices[3].v = 1.0f * font->getHeight() / font->getTextureHeight();

    vertices[0].x = vertices[2].x = 0;
    vertices[0].y = vertices[1].y = 0;
    vertices[1].x = vertices[3].x = font->getWidth();
    vertices[2].y = vertices[3].y = font->getHeight();

    if (auto state = getGameState()) {
      state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), font->getTexture(), getBlendType(), world, getAppliedShader(), isPermitCamera());
    }
  }

  void ObjText::setText(const std::wstring& t) {
    if (text == t) return;
    text = t;
    if (syntacticAnalysisEnable) {
      parseRubiedString(t, bodyText, rubies);
    } else {
      bodyText = t;
      rubies.clear();
    }
    fontParamModified = true;
  }

  const std::wstring & ObjText::getFontName() const {
    return fontName;
  }

  void ObjText::setFontName(const std::wstring& name) {
    if (fontName == name) return;
    fontName = name;
    fontParamModified = true;
  }

  int ObjText::getFontSize() const {
    return size;
  }

  void ObjText::setFontSize(int s) {
    if (size == s) return;
    size = s;
    fontParamModified = true;
  }

  bool ObjText::isFontBold() const {
    return bold;
  }

  void ObjText::setFontBold(bool b) {
    if (bold == b) return;
    bold = b;
    fontParamModified = true;
  }

  const ColorRGB & ObjText::getFontColorTop() const {
    return topColor;
  }

  void ObjText::setFontColorTop(int r, int g, int b) {
    ColorRGB color(r, g, b);
    if (topColor == color) return;
    topColor = color;
    fontParamModified = true;
  }

  const ColorRGB & ObjText::getFontColorBottom() const {
    return bottomColor;
  }

  void ObjText::setFontColorBottom(int r, int g, int b) {
    ColorRGB color(r, g, b);
    if (bottomColor == color) return;
    bottomColor = color;
    fontParamModified = true;
  }

  int ObjText::getFontBorderType() const {
    return borderType;
  }

  void ObjText::setFontBorderType(int t) {
    if (borderType == t) return;
    borderType = t;
    fontParamModified = true;
  }

  int ObjText::getFontBorderWidth() const {
    return borderWidth;
  }

  void ObjText::setFontBorderWidth(int w) {
    if (borderWidth == w) return;
    borderWidth = w;
    fontParamModified = true;
  }

  const ColorRGB & ObjText::getFontBorderColor() const {
    return borderColor;
  }

  void ObjText::setFontBorderColor(int r, int g, int b) {
    ColorRGB color(r, g, b);
    if (borderColor == color) return;
    borderColor = color;
    fontParamModified = true;
  }

  int ObjText::getLinePitch() const {
    return linePitch;
  }

  int ObjText::getSidePitch() const {
    return sidePitch;
  }

  bool ObjText::isAutoTransCenterEnabled() const {
    return autoTransCenterEnable;
  }

  bool ObjText::isSyntacticAnalysisEnabled() const {
    return syntacticAnalysisEnable;
  }

  void ObjText::setSyntacticAnalysis(bool enable) {
    if (syntacticAnalysisEnable != enable) {
      syntacticAnalysisEnable = enable;
      setText(text);
    }
  }

  int ObjText::getHorizontalAlignment() const {
    return horizontalAlignment;
  }

  void ObjText::setHorizontalAlignment(int alignmentType) {
    horizontalAlignment = alignmentType;
  }

  float ObjText::getTransCenterX() const {
    return transCenterX;
  }

  float ObjText::getTransCenterY() const {
    return transCenterY;
  }

  int ObjText::getMaxWidth() const {
    return maxWidth;
  }

  int ObjText::getMaxHeight() const {
    return maxHeight;
  }

  void ObjText::render() {
    if (auto state = getGameState()) {
      generateFonts();
      int idx = 0;
      float lineY = getY(); // 現在の行のy座標
      const int centerX = getX() + (autoTransCenterEnable ? std::max(0, (getTotalWidth() - sidePitch) / 2) : transCenterX);
      const int centerY = getY() + (autoTransCenterEnable ? getTotalHeight() / 2 + (borderType != BORDER_NONE ? borderWidth / 2 : 0) : transCenterY);
      const int nextLineOffsetY = getNextLineOffsetY();
      auto ruby = rubyFonts.begin();
      for (auto cnt : getTextLengthCUL()) { // 行ごとの文字数を取得
        float colX = getX(); // 現在の列のx座標
        if (cnt != 0) {
          if (!bodyFonts[idx]) idx++;
          if (horizontalAlignment != ALIGNMENT_LEFT) {
            // alignment
            int lineBodyWidth = 0;
            for (int k = 0; k < cnt; k++) {
              lineBodyWidth += bodyFonts[idx + k]->getRightCharOffsetX() + sidePitch;
            }
            if (borderType != BORDER_NONE) {
              lineBodyWidth += borderWidth;
            }
            if (horizontalAlignment == ALIGNMENT_RIGHT) {
              colX += maxWidth - lineBodyWidth;
            } else if (horizontalAlignment == ALIGNMENT_CENTER) {
              colX += (maxWidth - lineBodyWidth) / 2;
            }
          }
          for (int k = 0; k < cnt; k++) {
            auto bodyFont = bodyFonts[idx];
            // 初めに中心座標を原点に持ってきてから回転・拡大したのち元の位置に戻す
            D3DXMATRIX trans = rotScaleTrans(colX + bodyFont->getPrintOffsetX() - centerX, lineY + bodyFont->getPrintOffsetY() - centerY, getZ(), 0, 0, 0, 1, 1, 1);
            D3DXMATRIX rotScale = rotScaleTrans(centerX, centerY, 0, getAngleX(), getAngleY(), getAngleZ(), getScaleX(), getScaleY(), getScaleZ());
            D3DXMATRIX world = trans * rotScale;
            renderFont(bodyFont, world);
            if (ruby != rubyFonts.end()) {
              if (ruby->begin == idx) {
                if (!ruby->text.empty()) {
                  int bodyWidth = 0;
                  for (int i = ruby->begin; i < ruby->end && i < bodyFonts.size(); i++) {
                    if (bodyFonts[i]) {
                      bodyWidth += bodyFonts[i]->getRightCharOffsetX() + sidePitch;
                    }
                  }
                  int rubyOffsetSum = 0;
                  for (auto& font : ruby->text) {
                    rubyOffsetSum += font->getRightCharOffsetX();
                  }
                  const int rubySidePitch = std::max(0, (bodyWidth - rubyOffsetSum)) / (ruby->text.size());
                  const int rubyY = lineY - ruby->text[0]->getNextLineOffsetY();
                  float rubyX = colX;
                  for (auto rubyFont : ruby->text) {
                    D3DXMATRIX trans = rotScaleTrans(rubyX + rubyFont->getPrintOffsetX() - centerX, rubyY + rubyFont->getPrintOffsetY() - centerY, getZ(), 0, 0, 0, 1, 1, 1);
                    D3DXMATRIX world = trans * rotScale;
                    renderFont(rubyFont, world);
                    rubyX += rubyFont->getRightCharOffsetX() + rubySidePitch;
                  }
                }
                ruby++;
              }
            }
            colX += bodyFont->getRightCharOffsetX() + sidePitch; // 文字の間隔空け
            idx++;
          }
        } else idx++;
        lineY += nextLineOffsetY; // 行の間隔空け
      }
    }
  }

  const std::wstring & ObjText::getText() const {
    return text;
  }

  int ObjText::getTotalWidth() const {
    int idx = 0;
    int maxTotalWidth = 0;
    for (auto cnt : getTextLengthCUL()) {
      int totalWidth = 0;
      if (cnt != 0) {
        if (!bodyFonts[idx]) idx++;
        for (int k = 0; k < cnt; k++) {
          totalWidth += bodyFonts[idx]->getRightCharOffsetX() + sidePitch;
          idx++;
        }
        if (borderType != BORDER_NONE) {
          totalWidth += borderWidth;
        }
        maxTotalWidth = std::max(maxTotalWidth, totalWidth);
      } else idx++;
    }
    return maxTotalWidth;
  }

  int ObjText::getTotalHeight() const {
    if (bodyFonts.empty()) {
      return 0;
    } else {
      int lineCnt = getTextLengthCUL().size(); // 行数
      // NULLでないフォントを探す
      int h = lineCnt * getNextLineOffsetY() - linePitch;
      if (borderType != BORDER_NONE) h += borderWidth;
      return h;
    }
  }

  int ObjText::getTextLength() const {
    int cnt = 0;
    for (wchar_t c : text) {
      if ((int)c < 256) {
        cnt++;
      } else {
        cnt += 2;
      }
    }
    return cnt;
  }

  int ObjText::getTextLengthCU() const {
    int cnt = 0;
    for (wchar_t c : bodyText) {
      if (c != L'\n') {
        cnt++;
      }
    }
    return cnt;
  }

  std::vector<int> ObjText::getTextLengthCUL() const {
    std::vector<int> cnts;
    int idx = 0;
    int cnt = 0;
    int sumWidth = 0;
    while (idx < bodyFonts.size()) {
      if (!bodyFonts[idx]) {
        // 改行文字
        cnts.push_back(cnt);
        cnt = 0;
        sumWidth = 0;
      } else {
        int charWidth = bodyFonts[idx]->getRightCharOffsetX() + sidePitch;
        sumWidth += charWidth;
        if (maxWidth <= sumWidth) {
          // 幅に収まらずに改行
          cnts.push_back(cnt);
          // 行に1文字も入らない場合は1文字分飛ばす
          if (cnt == 0) idx++;
          cnt = 0;
          sumWidth = 0;
          continue;
        }
        cnt++;
      }
      idx++;
    }
    if (cnt != 0) cnts.push_back(cnt);
    return cnts;
  }

  bool ObjText::isFontParamModified() const {
    return fontParamModified;
  }

  // FUTURE: パーサの引数に返り値を持たせないと使いにくいので直す(要求度：低)

  static bool parseChar(const std::wstring& src, int& i, wchar_t c) {
    if (i < src.size() && src[i] == c) {
      i++;
      return true;
    }
    return false;
  }

  static bool parseString(const std::wstring& src, int& i, const std::wstring& s) {
    if (i < src.size() && s == src.substr(i, s.size())) {
      i += s.size();
      return true;
    }
    return false;
  }

  static bool parseNewLine(const std::wstring& src, int& i) {
    return parseString(src, i, L"[r]");
  }

  static void skipSpace(const std::wstring& src, int& i) {
    while (i < src.size() && isSpace(src[i])) i++;
  }

  static bool parseRuby(const std::wstring& src, int& i, std::wstring& rb, std::wstring& rt) {
    int prevIdx = i;
    if (!parseString(src, i, L"[ruby")) goto parse_failed;
    while (i < src.size() && src[i] != L']') {
      skipSpace(src, i);
      std::wstring propName;
      if (parseString(src, i, L"rb")) {
        propName = L"rb";
      } else if (parseString(src, i, L"rt")) {
        propName = L"rt";
      } else goto parse_failed;
      skipSpace(src, i);
      if (!parseChar(src, i, L'=')) goto parse_failed;
      skipSpace(src, i);
      if (!parseChar(src, i, L'"')) goto parse_failed;
      std::wstring& dst = (propName == L"rb") ? rb : rt;
      while (i < src.size() && src[i] != L'"') {
        if (parseString(src, i, L"&nbsp;")) {
          dst += L" ";
        } else if (parseString(src, i, L"&quot;")) {
          dst += L"\"";
        } else if (parseString(src, i, L"&osb;")) {
          dst += L"[";
        } else if (parseString(src, i, L"&csb;")) {
          dst += L"]";
        } else {
          dst += src[i++];
        }
      }
      if (!parseChar(src, i, L'"')) goto parse_failed;
      skipSpace(src, i);
      parseChar(src, i, L','); // カンマをセパレータにできる。(NOTE: 本家は他にも書けるが未対応
      skipSpace(src, i);
    }
    if (!parseChar(src, i, L']')) goto parse_failed;
    return true;
parse_failed:
    i = prevIdx;
    rb.clear();
    rt.clear();
    return false;
  }

  const std::vector<ObjText::Ruby<std::vector<std::shared_ptr<Font>>>>& ObjText::getRubyFonts() const {
    return rubyFonts;
  }

  void ObjText::parseRubiedString(const std::wstring& src, std::wstring& bodyText, std::vector<Ruby<std::wstring>>& rubies) {
    bodyText.clear();
    rubies.clear();
    for (int i = 0; i < src.size();) {
      if (parseNewLine(src, i)) {
        bodyText += L'\n';
      } else if (parseString(src, i, L"&nbsp;")) {
        bodyText += L" ";
      } else if (parseString(src, i, L"&quot;")) {
        bodyText += L"\"";
      } else if (parseString(src, i, L"&osb;")) {
        bodyText += L"[";
      } else if (parseString(src, i, L"&csb;")) {
        bodyText += L"]";
      } else {
        std::wstring rb;
        std::wstring rt;
        if (parseRuby(src, i, rb, rt)) {
          rubies.emplace_back(bodyText.size(), bodyText.size() + rb.size(), rt);
          bodyText += rb;
        } else {
        // 改行は除去
          if (src[i] != L'\r' && src[i] != '\n') bodyText += src[i];
          i++;
        }
      }
    }
  }
}