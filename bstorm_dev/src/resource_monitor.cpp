#include "resource_monitor.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/package.hpp>
#include <bstorm/package.hpp>

#include <algorithm>
#include <imgui.h>
#include <IconsFontAwesome_c.h>

namespace bstorm
{
static void drawRect(float offsetX, float offsetY, const Rect<int>& rect, ImU32 color)
{
    ImGui::GetWindowDrawList()->AddRect(ImVec2(offsetX + rect.left, offsetY + rect.top), ImVec2(offsetX + rect.right, offsetY + rect.bottom), color, 0.0f, -1, 2.0f);
}

void drawTextureInfo(const std::shared_ptr<Texture>& texture, const std::vector<Rect<int>>& rects)
{
    if (!texture) return;
    int width = texture->GetWidth();
    int height = texture->GetHeight();
    int useCount = texture.use_count() - 1;
    std::string path = ToUTF8(texture->GetPath());
    ImGui::BulletText("path      : %s", path.c_str());
    ImGui::BulletText("width     : %d", width);
    ImGui::BulletText("height    : %d", height);
    ImGui::BulletText("reserved  : %s", texture->IsReserved() ? "true" : "false");
    ImGui::BulletText("use-count : %d", useCount);
    if (ImGui::TreeNode("image##texture"))
    {
        float areaLeft = ImGui::GetCursorScreenPos().x;
        float areaTop = ImGui::GetCursorScreenPos().y;
        ImGui::Image(texture->GetTexture(), ImVec2(width, height));
        for (const auto& rect : rects)
        {
            drawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
        }
        ImGui::TreePop();
    }
}

void drawFontInfo(const std::shared_ptr<Font>& font)
{
    if (!font) return;
    {
        // font-params
        ImGui::BeginGroup();
        const FontParams& params_ = font->GetParams();
        bool hasBorder = params_.borderType != BORDER_NONE;
        ImGui::BulletText("char         : %s", ToUTF8(std::wstring{ params_.c }).c_str());
        ImGui::BulletText("font-name    : %s", ToUTF8(params_.fontName).c_str());
        ImGui::BulletText("size         : %d", params_.size);
        ImGui::BulletText("weight       : %d", params_.weight);
        ImGui::BulletText("border       : %s", hasBorder ? "enable" : "disable");
        if (hasBorder)
        {
            ImGui::BulletText("border-width : %d", params_.borderWidth);
        }
        {
            // top color
            float topColor[3] = { params_.topColor.GetR() / 255.0f, params_.topColor.GetG() / 255.0f, params_.topColor.GetB() / 255.0f };
            ImGui::ColorEdit3("top-color", topColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
        }
        {
            // bottom color
            float bottomColor[3] = { params_.bottomColor.GetR() / 255.0f, params_.bottomColor.GetG() / 255.0f, params_.bottomColor.GetB() / 255.0f };
            ImGui::ColorEdit3("bottom-color", bottomColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
        }
        if (hasBorder)
        { // border color
            float borderColor[3] = { params_.borderColor.GetR() / 255.0f, params_.borderColor.GetG() / 255.0f, params_.borderColor.GetB() / 255.0f };
            ImGui::ColorEdit3("border-color", borderColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
        }
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        // font texture info
        ImGui::BeginGroup();
        int width = font->GetWidth();
        int height = font->GetHeight();
        int texWidth = font->GetTextureWidth();
        int texHeight = font->GetTextureHeight();
        int useCount = font.use_count() - 1;
        ImGui::BulletText("width          : %d", width);
        ImGui::BulletText("height         : %d", height);
        ImGui::BulletText("texture-width  : %d", texWidth);
        ImGui::BulletText("texture-height : %d", texHeight);
        ImGui::BulletText("use-count      : %d", useCount);
        ImGui::Separator();
        ImGui::Image(font->GetTexture(), ImVec2(texWidth, texHeight));
        ImGui::EndGroup();
    }
}

void drawRenderTargetInfo(const std::shared_ptr<RenderTarget>& renderTarget, const std::vector<Rect<int>>& rects)
{
    int width = renderTarget->GetWidth();
    int height = renderTarget->GetHeight();
    int useCount = renderTarget.use_count() - 1;
    ImGui::BulletText("name      : %s", ToUTF8(renderTarget->GetName()).c_str());
    ImGui::BulletText("width     : %d", width);
    ImGui::BulletText("height    : %d", height);
    ImGui::BulletText("use-count : %d", useCount);
    if (ImGui::TreeNode("image##renderTarget"))
    {
        float areaLeft = ImGui::GetCursorScreenPos().x;
        float areaTop = ImGui::GetCursorScreenPos().y;
        ImGui::Image(renderTarget->GetTexture(), ImVec2(width, height));
        for (const auto& rect : rects)
        {
            drawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
        }
        ImGui::TreePop();
    }
}

template<>
void TextureCache::BackDoor<ResourceMonitor>()
{
    static std::wstring selectedTexturePath;
    const float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceTextureTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        for (const auto& entry : textureMap_)
        {
            std::wstring path = entry.first;
            auto pathU8 = ToUTF8(path);
            try
            {
                ImGui::PushID(pathU8.c_str());
                auto& texture = entry.second.get();
                ImGui::BeginGroup();
                if (ImGui::Button(ICON_FA_REFRESH))
                {
                    Reload(path, texture->IsReserved(), nullptr);
                }
                ImGui::SameLine();
                float iconWidth = std::min(sideBarWidth * 0.4f, 1.0f * texture->GetWidth());
                float iconHeight = std::max(iconWidth / texture->GetWidth() * texture->GetHeight(), ImGui::GetTextLineHeight());
                if (ImGui::ImageButton(texture->GetTexture(), ImVec2(iconWidth, iconHeight), ImVec2(0, 0), ImVec2(1, 1), 0, ImVec4(0, 0, 0, 1)))
                {
                    selectedTexturePath = path;
                }
                ImGui::SameLine();
                if (ImGui::Selectable(pathU8.c_str(), selectedTexturePath == path))
                {
                    selectedTexturePath = path;
                }
                ImGui::EndGroup();
                ImGui::PopID();
            } catch (...)
            {
            }
        }
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("ResourceTextureTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Texture Info");
        auto it = textureMap_.find(selectedTexturePath);
        if (it != textureMap_.end())
        {
            try
            {
                auto& texture = it->second.get();
                drawTextureInfo(texture, std::vector<Rect<int>>());
            } catch (...) {}
        }
    }
    ImGui::EndChild();
}

void DrawFontInfoTab(const std::unordered_map<FontParams, std::shared_ptr<Font>>& fontMap)
{
    static FontParams selectedFontParams;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceFontTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        int fontId = 0;
        for (const auto& entry : fontMap)
        {
            ImGui::PushID(fontId++);
            const auto& params_ = entry.first;
            const auto& font = entry.second;
            ImGui::BeginGroup();
            float iconWidth = ImGui::GetTextLineHeight();
            if (ImGui::ImageButton(font->GetTexture(), ImVec2(iconWidth, iconWidth), ImVec2(0, 0), ImVec2(1.0f*font->GetWidth() / font->GetTextureWidth(), 1.0f*font->GetHeight() / font->GetTextureHeight()), 0, ImVec4(0, 0, 0, 1)))
            {
                selectedFontParams = params_;
            }
            ImGui::SameLine();
            std::string label = ToUTF8(std::wstring{ params_.c }) + "(" + ToUTF8(params_.fontName) + ")";
            if (ImGui::Selectable(label.c_str(), selectedFontParams == params_))
            {
                selectedFontParams = params_;
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
    if (it != fontMap.end())
    {
        drawFontInfo(it->second);
    }
    ImGui::EndChild();
}

struct RenderTargetMonitor;
template <>
void Package::backDoor<RenderTargetMonitor>()
{
    static std::wstring selectedRenderTargetName;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceRenderTargetTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : renderTargets_)
    {
        const std::wstring& name = entry.first;
        if (ImGui::Selectable(ToUTF8(name).c_str(), selectedRenderTargetName == name))
        {
            selectedRenderTargetName = name;
        }
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ResourceRenderTargetTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    auto it = renderTargets_.find(selectedRenderTargetName);
    ImGui::Text("RenderTarget Info");
    if (it != renderTargets_.end())
    {
        drawRenderTargetInfo(it->second, std::vector<Rect<int>>());
    }
    ImGui::EndChild();
}

enum class Tab
{
    TEXTURE,
    FONT,
    RENDER_TARGET
};

template <>
void Package::backDoor<ResourceMonitor>()
{
    ImGui::Columns(3, "resource tab");
    ImGui::Separator();
    static Tab selectedTab = Tab::TEXTURE;
    if (ImGui::Selectable("Texture##ResourceTextureTab", selectedTab == Tab::TEXTURE))
    {
        selectedTab = Tab::TEXTURE;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Font##ResourceFontTab", selectedTab == Tab::FONT))
    {
        selectedTab = Tab::FONT;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("RenderTarget##ResourceRenderTargetTab", selectedTab == Tab::RENDER_TARGET))
    {
        selectedTab = Tab::RENDER_TARGET;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (selectedTab == Tab::TEXTURE)
    {
        textureCache_->BackDoor<ResourceMonitor>();
    } else if (selectedTab == Tab::FONT)
    {
        DrawFontInfoTab(GetFontMap());
    } else if (selectedTab == Tab::RENDER_TARGET)
    {
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

void ResourceMonitor::draw(const std::shared_ptr<Package>& package)
{
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Resource", &openFlag, ImGuiWindowFlags_ResizeFromAnySide))
    {
        if (package) package->backDoor<ResourceMonitor>();
    }
    ImGui::End();
}
}