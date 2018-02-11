#pragma once

#include <algorithm>
#include <imgui.h>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/engine.hpp>

#include "resource_monitor.hpp"

namespace bstorm {
  static void drawRect(float offsetX, float offsetY, const Rect<int>& rect, ImU32 color) {
    ImGui::GetWindowDrawList()->AddRect(ImVec2(offsetX + rect.left, offsetY + rect.top), ImVec2(offsetX + rect.right, offsetY + rect.bottom), color, 0.0f, -1, 2.0f);
  }

  void drawTextureInfo(const std::shared_ptr<Texture>& texture, const std::vector<Rect<int>>& rects) {
    if (!texture) return;
    int width = texture->getWidth();
    int height = texture->getHeight();
    int useCount = texture.use_count() - 1;
    std::string path = toUTF8(texture->getPath());
    ImGui::BulletText("path      : %s", path.c_str());
    ImGui::BulletText("width     : %d", width);
    ImGui::BulletText("height    : %d", height);
    ImGui::BulletText("reserved  : %s", texture->isReserved() ? "true" : "false");
    ImGui::BulletText("use-count : %d", useCount);
    if (ImGui::TreeNode("image##texture")) {
      float areaLeft = ImGui::GetCursorScreenPos().x;
      float areaTop = ImGui::GetCursorScreenPos().y;
      ImGui::Image(texture->getTexture(), ImVec2(width, height));
      for (const auto& rect : rects) {
        drawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
      }
      ImGui::TreePop();
    }
  }

  void drawFontInfo(const std::shared_ptr<Font>& font) {
    if (!font) return;
    {
      // font-params
      ImGui::BeginGroup();
      const FontParams& params = font->getParams();
      bool hasBorder = params.borderType != BORDER_NONE;
      ImGui::BulletText("char         : %s", toUTF8(std::wstring{ params.c }).c_str());
      ImGui::BulletText("font-name    : %s", toUTF8(params.fontName).c_str());
      ImGui::BulletText("size         : %d", params.size);
      ImGui::BulletText("weight       : %d", params.weight);
      ImGui::BulletText("border       : %s", hasBorder ? "enable" : "disable");
      if (hasBorder) {
        ImGui::BulletText("border-width : %d", params.borderWidth);
      }
      {
        // top color
        float topColor[3] = { params.topColor.getR() / 255.0f, params.topColor.getG() / 255.0f, params.topColor.getB() / 255.0f };
        ImGui::ColorEdit3("top-color", topColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
      }
      {
        // bottom color
        float bottomColor[3] = { params.bottomColor.getR() / 255.0f, params.bottomColor.getG() / 255.0f, params.bottomColor.getB() / 255.0f };
        ImGui::ColorEdit3("bottom-color", bottomColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
      }
      if (hasBorder) { // border color
        float borderColor[3] = { params.borderColor.getR() / 255.0f, params.borderColor.getG() / 255.0f, params.borderColor.getB() / 255.0f };
        ImGui::ColorEdit3("border-color", borderColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
      }
      ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
      // font texture info
      ImGui::BeginGroup();
      int width = font->getWidth();
      int height = font->getHeight();
      int texWidth = font->getTextureWidth();
      int texHeight = font->getTextureHeight();
      int useCount = font.use_count() - 1;
      ImGui::BulletText("width          : %d", width);
      ImGui::BulletText("height         : %d", height);
      ImGui::BulletText("texture-width  : %d", texWidth);
      ImGui::BulletText("texture-height : %d", texHeight);
      ImGui::BulletText("use-count      : %d", useCount);
      ImGui::Separator();
      ImGui::Image(font->getTexture(), ImVec2(texWidth, texHeight));
      ImGui::EndGroup();
    }
  }

  void drawRenderTargetInfo(const std::shared_ptr<RenderTarget>& renderTarget, const std::vector<Rect<int>>& rects) {
    int width = renderTarget->getWidth();
    int height = renderTarget->getHeight();
    int useCount = renderTarget.use_count() - 1;
    ImGui::BulletText("name      : %s", toUTF8(renderTarget->getName()).c_str());
    ImGui::BulletText("width     : %d", width);
    ImGui::BulletText("height    : %d", height);
    ImGui::BulletText("use-count : %d", useCount);
    if (ImGui::TreeNode("image##renderTarget")) {
      float areaLeft = ImGui::GetCursorScreenPos().x;
      float areaTop = ImGui::GetCursorScreenPos().y;
      ImGui::Image(renderTarget->getTexture(), ImVec2(width, height));
      for (const auto& rect : rects) {
        drawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
      }
      ImGui::TreePop();
    }
  }

  template<>
  void TextureCache::backDoor<ResourceMonitor>() const {
    static std::wstring selectedTexturePath;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceTextureTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : textureMap) {
      std::wstring path = entry.first;
      const auto& texture = entry.second;
      ImGui::BeginGroup();
      float iconWidth = std::min(sideBarWidth * 0.4f, 1.0f * texture->getWidth());
      float iconHeight = std::max(iconWidth / texture->getWidth() * texture->getHeight(), ImGui::GetTextLineHeight());
      if (ImGui::ImageButton(texture->getTexture(), ImVec2(iconWidth, iconHeight), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 1))) {
        selectedTexturePath = path;
      }
      ImGui::SameLine();
      if (ImGui::Selectable(toUTF8(path).c_str(), selectedTexturePath == path)) {
        selectedTexturePath = path;
      }
      ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ResourceTextureTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Texture Info");
    auto it = textureMap.find(selectedTexturePath);
    if (it != textureMap.end()) {
      drawTextureInfo(it->second, std::vector<Rect<int>>());
    }
    ImGui::EndChild();
  }

  template<>
  void FontCache::backDoor<ResourceMonitor>() const {
    static FontParams selectedFontParams;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceFontTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
      int fontId = 0;
      for (const auto& entry : fontMap) {
        ImGui::PushID(fontId++);
        const auto& params = entry.first;
        const auto& font = entry.second;
        ImGui::BeginGroup();
        float iconWidth = ImGui::GetTextLineHeight();
        if (ImGui::ImageButton(font->getTexture(), ImVec2(iconWidth, iconWidth), ImVec2(0, 0), ImVec2(1.0f*font->getWidth() / font->getTextureWidth(), 1.0f*font->getHeight() / font->getTextureHeight()), 0, ImVec4(0, 0, 0, 1))) {
          selectedFontParams = params;
        }
        ImGui::SameLine();
        std::string label = toUTF8(std::wstring{ params.c }) + "(" + toUTF8(params.fontName) + ")";
        if (ImGui::Selectable(label.c_str(), selectedFontParams == params)) {
          selectedFontParams = params;
        }
        ImGui::EndGroup();
        ImGui::PopID();
      }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ResourceFontTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    auto it = fontMap.find(selectedFontParams);
    ImGui::Text("Font Info");
    if (it != fontMap.end()) {
      drawFontInfo(it->second);
    }
    ImGui::EndChild();
  }

  struct RenderTargetMonitor;
  template <>
  void Engine::backDoor<RenderTargetMonitor>() {
    static std::wstring selectedRenderTargetName;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceRenderTargetTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : renderTargets) {
      const std::wstring& name = entry.first;
      if (ImGui::Selectable(toUTF8(name).c_str(), selectedRenderTargetName == name)) {
        selectedRenderTargetName = name;
      }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ResourceRenderTargetTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    auto it = renderTargets.find(selectedRenderTargetName);
    ImGui::Text("RenderTarget Info");
    if (it != renderTargets.end()) {
      drawRenderTargetInfo(it->second, std::vector<Rect<int>>());
    }
    ImGui::EndChild();
  }

  enum class Tab {
    TEXTURE,
    FONT,
    RENDER_TARGET
  };

  template <>
  void Engine::backDoor<ResourceMonitor>() {
    ImGui::Columns(3, "resource tab");
    ImGui::Separator();
    static Tab selectedTab = Tab::TEXTURE;
    if (ImGui::Selectable("Texture##ResourceTextureTab", selectedTab == Tab::TEXTURE)) {
      selectedTab = Tab::TEXTURE;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Font##ResourceFontTab", selectedTab == Tab::FONT)) {
      selectedTab = Tab::FONT;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("RenderTarget##ResourceRenderTargetTab", selectedTab == Tab::RENDER_TARGET)) {
      selectedTab = Tab::RENDER_TARGET;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (selectedTab == Tab::TEXTURE) {
      gameState->textureCache->backDoor<ResourceMonitor>();
    } else if (selectedTab == Tab::FONT) {
      gameState->fontCache->backDoor<ResourceMonitor>();
    } else if (selectedTab == Tab::RENDER_TARGET) {
      backDoor<RenderTargetMonitor>();
    }
  }

  ResourceMonitor::ResourceMonitor(int left, int top, int width, int height) :
    iniLeft(left),
    iniTop(top),
    iniWidth(width),
    iniHeight(height),
    openFlag(false)
  {
  }

  ResourceMonitor::~ResourceMonitor() {}

  void ResourceMonitor::draw(const std::shared_ptr<Engine>& engine) {
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Resource", &openFlag, ImGuiWindowFlags_ResizeFromAnySide)) {
      engine->backDoor<ResourceMonitor>();
    }
    ImGui::End();
  }
}