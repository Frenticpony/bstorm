#include "resource_monitor.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/string_util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/serialized_script.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/package.hpp>

#include <algorithm>
#include <imgui.h>
#include <IconsFontAwesome_c.h>

namespace bstorm
{
static void DrawRect(float offsetX, float offsetY, const Rect<int>& rect, ImU32 color)
{
    ImGui::GetWindowDrawList()->AddRect(ImVec2(offsetX + rect.left, offsetY + rect.top), ImVec2(offsetX + rect.right, offsetY + rect.bottom), color, 0.0f, -1, 2.0f);
}

void DrawTextureInfo(const std::shared_ptr<Texture>& texture, const std::vector<Rect<int>>& rects, bool* reserved)
{
    if (!texture) return;
    int width = texture->GetWidth();
    int height = texture->GetHeight();
    int useCount = texture.use_count() - 1;
    std::string path = ToUTF8(texture->GetPath());
    ImGui::BulletText("path      : %s", path.c_str());
    ImGui::BulletText("width     : %d", width);
    ImGui::BulletText("height    : %d", height);
    if (reserved != nullptr)
    {
        ImGui::Checkbox(("reserved##" + path).c_str(), reserved);
    }
    //    ImGui::BulletText("reserved  : %s", texture->IsReserved() ? "true" : "false");
    ImGui::BulletText("use-count : %d", useCount);
    if (ImGui::TreeNode("image##texture"))
    {
        float areaLeft = ImGui::GetCursorScreenPos().x;
        float areaTop = ImGui::GetCursorScreenPos().y;
        ImGui::Image(texture->GetTexture(), ImVec2(width, height));
        for (const auto& rect : rects)
        {
            DrawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
        }
        ImGui::TreePop();
    }
}

void DrawFontInfo(const std::shared_ptr<Font>& font)
{
    if (!font) return;
    {
        // font-params
        ImGui::BeginGroup();
        const FontParams& params = font->GetParams();
        bool hasBorder = params.borderType != BORDER_NONE;
        ImGui::BulletText("char         : %s", ToUTF8(std::wstring{ params.c }).c_str());
        ImGui::BulletText("font-name    : %s", ToUTF8(params.fontName).c_str());
        ImGui::BulletText("size         : %d", params.size);
        ImGui::BulletText("weight       : %d", params.weight);
        ImGui::BulletText("border       : %s", hasBorder ? "enable" : "disable");
        if (hasBorder)
        {
            ImGui::BulletText("border-width : %d", params.borderWidth);
        }
        {
            // top color
            float topColor[3] = { params.topColor.GetR() / 255.0f, params.topColor.GetG() / 255.0f, params.topColor.GetB() / 255.0f };
            ImGui::ColorEdit3("top-color", topColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
        }
        {
            // bottom color
            float bottomColor[3] = { params.bottomColor.GetR() / 255.0f, params.bottomColor.GetG() / 255.0f, params.bottomColor.GetB() / 255.0f };
            ImGui::ColorEdit3("bottom-color", bottomColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
        }
        if (hasBorder)
        { // border color
            float borderColor[3] = { params.borderColor.GetR() / 255.0f, params.borderColor.GetG() / 255.0f, params.borderColor.GetB() / 255.0f };
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

void DrawRenderTargetInfo(const std::shared_ptr<RenderTarget>& renderTarget, const std::vector<Rect<int>>& rects)
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
            DrawRect(areaLeft, areaTop, rect, IM_COL32(0x00, 0xff, 0x00, 0xff));
        }
        ImGui::TreePop();
    }
}

void DrawTextureInfoTab(const std::shared_ptr<TextureStore>& textureStore)
{
    static std::wstring selectedTexturePath;
    const float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceTextureTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        textureStore->ForEach([&](const auto& path, bool& isReserved, auto& texture)
        {
            auto pathU8 = ToUTF8(path);
            ImGui::PushID(pathU8.c_str());
            ImGui::BeginGroup();
            if (ImGui::Button(ICON_FA_REFRESH))
            {
                try
                {
                    texture->Reload();
                    Logger::WriteLog(std::move(
                        Log(Log::Level::LV_INFO).SetMessage(std::string("reload texture."))
                        .SetParam(Log::Param(Log::Param::Tag::TEXTURE, path))));
                } catch (Log& log)
                {
                    Logger::WriteLog(log);
                }
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
        });
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ResourceTextureTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    {
        ImGui::Text("Texture Info");
        if (textureStore->IsLoadCompleted(selectedTexturePath))
        {
            try
            {
                auto& texture = textureStore->Load(selectedTexturePath);
                bool isReserved = textureStore->IsReserved(selectedTexturePath);
                DrawTextureInfo(texture, std::vector<Rect<int>>(), &isReserved);
                textureStore->SetReserveFlag(selectedTexturePath, isReserved);
            } catch (...) {}
        }
    }
    ImGui::EndChild();
}

void DrawFontInfoTab(const std::shared_ptr<FontStore>& fontStore)
{
    static FontParams selectedFontParams;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceFontTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        int fontId = 0;
        fontStore->ForEach([&](const auto& params, auto& isReserved, auto& font)
        {
            ImGui::PushID(fontId++);
            ImGui::BeginGroup();
            float iconWidth = ImGui::GetTextLineHeight();
            if (ImGui::ImageButton(font->GetTexture(), ImVec2(iconWidth, iconWidth), ImVec2(0, 0), ImVec2(1.0f*font->GetWidth() / font->GetTextureWidth(), 1.0f*font->GetHeight() / font->GetTextureHeight()), 0, ImVec4(0, 0, 0, 1)))
            {
                selectedFontParams = params;
            }
            ImGui::SameLine();
            std::string label = ToUTF8(std::wstring{ params.c }) + "(" + ToUTF8(params.fontName) + ")";
            if (ImGui::Selectable(label.c_str(), selectedFontParams == params))
            {
                selectedFontParams = params;
            }
            ImGui::EndGroup();
            ImGui::PopID();
        });
    }
    ImGui::EndChild();
    ImGui::SameLine();
    {
        ImGui::BeginChild("ResourceFontTabInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text("Font Info");
        if (fontStore->Contains(selectedFontParams))
        {
            DrawFontInfo(fontStore->Create(selectedFontParams));
        }
        ImGui::EndChild();
    }
}

void DrawScriptCacheInfoTab(const std::shared_ptr<SerializedScriptStore>& serializedScriptStore)
{
    static SerializedScriptSignature selectedSignature(L"", ScriptType::Value::UNKNOWN, L"", TIME_STAMP_NONE);
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ResourceScriptCacheSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    {
        serializedScriptStore->ForEach([&](const auto& signature, bool& isReserved, auto& texture)
        {
            auto id = std::to_string(signature.lastUpdateTime);
            ImGui::PushID(id.c_str());
            if (ImGui::Selectable(ToUTF8(signature.path).c_str(), selectedSignature == signature))
            {
                selectedSignature = signature;
            }
            ImGui::PopID();
        });
        ImGui::EndChild();
    }
    ImGui::SameLine();
    ImGui::BeginChild("ResourceScriptCacheInfoArea", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    {
        ImGui::Text("Script Cache Info");
        if (serializedScriptStore->IsLoadCompleted(selectedSignature))
        {
            if (auto serializedScript = serializedScriptStore->Get(selectedSignature))
            {
                ImGui::BulletText("path       : %s", ToUTF8(selectedSignature.path).c_str());
                ImGui::BulletText("type       : %s", selectedSignature.type.GetName());
                ImGui::BulletText("code-size  : %d [byte]", serializedScript->GetByteCodeSize());
                ImGui::BulletText("info-size  : %d [byte]", serializedScript->GetScriptInfo().size());
                ImGui::BulletText("source-map : %d [byte]", serializedScript->GetSourceMap().size());
                ImGui::BulletText("use-count  : %d", serializedScript.use_count() - 2);
#ifdef _DEBUG
                ImGui::BeginChild("Source Code", ImVec2(-1, -1), false, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::TextUnformatted(serializedScript->GetSourceCode().c_str());
                ImGui::EndChild();
#endif
            }
        }
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
        DrawRenderTargetInfo(it->second, std::vector<Rect<int>>());
    }
    ImGui::EndChild();
}

enum class Tab
{
    TEXTURE,
    FONT,
    RENDER_TARGET,
    SCRIPT_CACHE
};

template <>
void Package::backDoor<ResourceMonitor>()
{
    ImGui::Columns(4, "resource tab");
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
    ImGui::NextColumn();
    if (ImGui::Selectable("ScriptCache##ScriptCacheTab", selectedTab == Tab::SCRIPT_CACHE))
    {
        selectedTab = Tab::SCRIPT_CACHE;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    switch (selectedTab)
    {
        case Tab::TEXTURE:
            DrawTextureInfoTab(textureStore_);
            break;
        case Tab::FONT:
            DrawFontInfoTab(fontStore_);
            break;
        case Tab::RENDER_TARGET:
            backDoor<RenderTargetMonitor>();
            break;
        case Tab::SCRIPT_CACHE:
            DrawScriptCacheInfoTab(serializedScriptStore_);
            break;

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