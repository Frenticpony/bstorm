#include "util.hpp"
#include "resource_monitor.hpp"
#include "user_def_data_browser.hpp"

#include <bstorm/dnh_const.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/util.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/engine.hpp>

#include <algorithm>
#include <imgui.h>

namespace bstorm
{
void drawShotDataInfo(const std::shared_ptr<ShotData>& shotData)
{
    if (!shotData) return;
    {
        ImGui::BeginGroup();
        // param view area
        ImGui::BulletText("id               : %d", shotData->id);
        ImGui::BulletText("use-count        : %d", shotData.use_count() - 1);
        if (shotData->animationData.empty())
        {
            ImGui::BulletText("rect             : (%d %d %d %d)", shotData->rect.left, shotData->rect.top, shotData->rect.right, shotData->rect.bottom);
        } else
        {
            // animation-data
            if (ImGui::TreeNode("animation-data"))
            {
                for (const auto& clip : shotData->animationData)
                {
                    const auto& rect = clip.rect;
                    ImGui::BulletText("%d-frame : (%d %d %d %d)", clip.frame, rect.left, rect.top, rect.right, rect.bottom);
                }
                ImGui::TreePop();
            }
        }
        ImGui::BulletText("render           : %s", getBlendTypeName(shotData->render));
        ImGui::BulletText("alpha            : %d", shotData->alpha);
        ImGui::BulletText("delay-rect       : (%d %d %d %d) %s", shotData->delayRect.left, shotData->delayRect.top, shotData->delayRect.right, shotData->delayRect.bottom, shotData->useDelayRect ? "" : "[derived]");
        {
            // delay color
            float delayColor[3] = { shotData->delayColor.GetR() / 255.0f, shotData->delayColor.GetG() / 255.0f, shotData->delayColor.GetB() / 255.0f };
            if (shotData->useDelayColor)
            {
                ImGui::ColorEdit3("delay-color", delayColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
            } else
            {
                ImGui::ColorEdit3("delay-color [derived]", delayColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs);
            }
        }
        ImGui::BulletText("delay-render     : %s", getBlendTypeName(shotData->delayRender));
        if (shotData->useAngularVelocityRand)
        {
            ImGui::BulletText("angular-velocity : rand(%f, %f)", shotData->angularVelocityRandMin, shotData->angularVelocityRandMax);
        } else
        {
            ImGui::BulletText("angular-velocity : %f", shotData->angularVelocity);
        }
        ImGui::BulletText("fixed-angle      : %s", shotData->fixedAngle ? "true" : "false");
        {
            // collisions
            if (ImGui::TreeNode("collisions"))
            {
                for (const auto& col : shotData->collisions)
                {
                    ImGui::BulletText("(r:%f x:%f y:%f)", col.r, col.x, col.y);
                }
                {
                    ImGui::BeginGroup();
                    // collision view
                    float areaLeft = ImGui::GetCursorScreenPos().x;
                    float areaTop = ImGui::GetCursorScreenPos().y;
                    if (shotData->animationData.empty())
                    {
                        drawCroppedImage(shotData->rect, shotData->texture);
                    } else
                    {
                        drawCroppedImage(shotData->animationData[0].rect, shotData->texture);
                    }
                    float rectCenterX = std::abs(shotData->rect.right - shotData->rect.left) / 2.0f;
                    float rectCenterY = std::abs(shotData->rect.bottom - shotData->rect.top) / 2.0f;
                    for (const auto& col : shotData->collisions)
                    {
                        ImGui::GetWindowDrawList()->AddCircle(ImVec2(areaLeft + col.x + rectCenterX, areaTop + col.y + rectCenterY), col.r, 0xff0000ff, 24);
                    }
                    ImGui::EndGroup();
                }
                ImGui::TreePop();
            }
        }
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        // rect view area
        ImGui::Columns(2, "shotdata rectview");
        ImGui::Separator();
        ImGui::Text("normal");
        ImGui::NextColumn();
        ImGui::Text("delay");
        ImGui::NextColumn();
        ImGui::Separator();
        {
            // normal
            if (shotData->animationData.empty())
            {
                drawCroppedImage(shotData->rect, shotData->texture);
            } else
            {
                drawCroppedImage(shotData->animationData[0].rect, shotData->texture);
            }
        }
        ImGui::NextColumn();
        {
            // delay
            drawCroppedImage(shotData->delayRect, shotData->texture);
        }
        ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::EndGroup();
    }
    if (ImGui::TreeNodeEx("Texture Info"))
    {
        std::vector<Rect<int>> rects;
        if (shotData->animationData.empty())
        {
            rects.push_back(shotData->rect);
        } else
        {
            for (const auto& clip : shotData->animationData)
            {
                rects.push_back(clip.rect);
            }
        }
        rects.push_back(shotData->delayRect);
        drawTextureInfo(shotData->texture, rects);
        ImGui::TreePop();
    }
}

void drawItemDataInfo(const std::shared_ptr<ItemData>& itemData)
{
    if (!itemData) return;
    {
        ImGui::BeginGroup();
        // param view area
        ImGui::BulletText("id               : %d", itemData->id);
        ImGui::BulletText("type             : %d", itemData->type);
        ImGui::BulletText("use-count        : %d", itemData.use_count() - 1);
        if (itemData->animationData.empty())
        {
            ImGui::BulletText("rect             : (%d %d %d %d)", itemData->rect.left, itemData->rect.top, itemData->rect.right, itemData->rect.bottom);
        } else
        {
            // animation-data
            if (ImGui::TreeNode("animation-data"))
            {
                for (const auto& clip : itemData->animationData)
                {
                    const auto& rect = clip.rect;
                    ImGui::BulletText("%d-frame : (%d %d %d %d)", clip.frame, rect.left, rect.top, rect.right, rect.bottom);
                }
                ImGui::TreePop();
            }
        }
        ImGui::BulletText("out              : (%d %d %d %d)", itemData->out.left, itemData->out.top, itemData->out.right, itemData->out.bottom);
        ImGui::BulletText("render           : %s", getBlendTypeName(itemData->render));
        ImGui::EndGroup();
    }
    ImGui::SameLine();
    {
        ImGui::BeginGroup();
        // rect view area
        ImGui::Columns(2, "itemdata rectview");
        ImGui::Separator();
        ImGui::Text("normal");
        ImGui::NextColumn();
        ImGui::Text("out");
        ImGui::NextColumn();
        ImGui::Separator();
        {
            // normal
            if (itemData->animationData.empty())
            {
                drawCroppedImage(itemData->rect, itemData->texture);
            } else
            {
                drawCroppedImage(itemData->animationData[0].rect, itemData->texture);
            }
        }
        ImGui::NextColumn();
        {
            // delay
            drawCroppedImage(itemData->out, itemData->texture);
        }
        ImGui::NextColumn();
        ImGui::Columns(1);
        ImGui::Separator();
        ImGui::EndGroup();
    }
    if (ImGui::TreeNodeEx("Texture Info"))
    {
        std::vector<Rect<int>> rects;
        if (itemData->animationData.empty())
        {
            rects.push_back(itemData->rect);
        } else
        {
            for (const auto& clip : itemData->animationData)
            {
                rects.push_back(clip.rect);
            }
        }
        rects.push_back(itemData->out);
        drawTextureInfo(itemData->texture, rects);
        ImGui::TreePop();
    }
}


void drawShotDataTab(const std::string& name, int& selectedId, const std::map<int, std::shared_ptr<ShotData>>& table)
{
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild((name + "ShotDataTabSideBar").c_str(), ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : table)
    {
        int id = entry.first;
        const auto& data = entry.second;
        ImGui::BeginGroup();
        const auto& rect = data->animationData.empty() ? data->rect : data->animationData[0].rect;
        float rectWidth = std::abs(rect.right - rect.left);
        float rectHeight = std::abs(rect.bottom - rect.top);
        float iconWidth = std::min(sideBarWidth * 0.3f, rectWidth);
        float iconHeight = std::max(iconWidth / rectWidth * rectHeight, ImGui::GetTextLineHeight());

        float u1 = 1.0f * rect.left / data->texture->GetWidth();
        float v1 = 1.0f * rect.top / data->texture->GetHeight();
        float u2 = 1.0f * rect.right / data->texture->GetWidth();
        float v2 = 1.0f * rect.bottom / data->texture->GetHeight();

        ImGui::PushID(id);
        if (ImGui::ImageButton(data->texture->GetTexture(), ImVec2(iconWidth, iconHeight), ImVec2(u1, v1), ImVec2(u2, v2), 0, ImVec4(0, 0, 0, 1)))
        {
            selectedId = id;
        }
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Selectable(std::to_string(id).c_str(), selectedId == id))
        {
            selectedId = id;
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild((name + "ShotDataTabInfoArea").c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Shot Data Info");
    auto it = table.find(selectedId);
    if (it != table.end())
    {
        drawShotDataInfo(it->second);
    }
    ImGui::EndChild();
}

struct PlayerShotData;
struct EnemyShotData;

template<>
void ShotDataTable::BackDoor<PlayerShotData>() const
{
    static int selectedId = -1;
    drawShotDataTab("Player", selectedId, table_);
}

template<>
void ShotDataTable::BackDoor<EnemyShotData>() const
{
    static int selectedId = -1;
    drawShotDataTab("Enemy", selectedId, table_);
}

template <>
void ItemDataTable::BackDoor<UserDefDataBrowser>() const
{
    static int selectedId = -1;
    float sideBarWidth = ImGui::GetContentRegionAvailWidth() * 0.2;
    ImGui::BeginChild("ItemDataTabSideBar", ImVec2(sideBarWidth, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& entry : table_)
    {
        int id = entry.first;
        const auto& data = entry.second;
        const auto& texture = data->texture;
        ImGui::BeginGroup();
        const auto& rect = data->animationData.empty() ? data->rect : data->animationData[0].rect;
        float rectWidth = std::abs(rect.right - rect.left);
        float rectHeight = std::abs(rect.bottom - rect.top);
        float iconWidth = std::min(sideBarWidth * 0.3f, rectWidth);
        float iconHeight = std::max(iconWidth / rectWidth * rectHeight, ImGui::GetTextLineHeight());

        float u1 = 1.0f * rect.left / texture->GetWidth();
        float v1 = 1.0f * rect.top / texture->GetHeight();
        float u2 = 1.0f * rect.right / texture->GetWidth();
        float v2 = 1.0f * rect.bottom / texture->GetHeight();

        ImGui::PushID(id);
        if (ImGui::ImageButton(texture->GetTexture(), ImVec2(iconWidth, iconHeight), ImVec2(u1, v1), ImVec2(u2, v2), 0, ImVec4(0, 0, 0, 1)))
        {
            selectedId = id;
        }
        ImGui::PopID();
        ImGui::SameLine();
        if (ImGui::Selectable(std::to_string(id).c_str(), selectedId == id))
        {
            selectedId = id;
        }
        ImGui::EndGroup();
    }
    ImGui::EndChild();
    ImGui::SameLine();
    ImGui::BeginChild("ItemDataTabInfoArea", ImVec2(ImGui::GetContentRegionAvailWidth(), -1), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::Text("Item Data Info");
    auto it = table_.find(selectedId);
    if (it != table_.end())
    {
        drawItemDataInfo(it->second);
    }
    ImGui::EndChild();
}

enum class Tab
{
    PLAYER_SHOT,
    ENEMY_SHOT,
    ITEM
};

template <>
void Engine::backDoor<UserDefDataBrowser>()
{
    ImGui::Columns(3, "userdefdata tab");
    ImGui::Separator();
    static Tab selectedTab = Tab::PLAYER_SHOT;
    if (ImGui::Selectable("PlayerShot##UserDefDataPlayerShotTab", selectedTab == Tab::PLAYER_SHOT))
    {
        selectedTab = Tab::PLAYER_SHOT;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("EnemyShot##UserDefDataEnemyShotTab", selectedTab == Tab::ENEMY_SHOT))
    {
        selectedTab = Tab::ENEMY_SHOT;
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Item##UserDefDataItemTab", selectedTab == Tab::ITEM))
    {
        selectedTab = Tab::ITEM;
    }
    ImGui::Columns(1);
    ImGui::Separator();
    if (selectedTab == Tab::PLAYER_SHOT)
    {
        gameState->playerShotDataTable->BackDoor<PlayerShotData>();
    } else if (selectedTab == Tab::ENEMY_SHOT)
    {
        gameState->enemyShotDataTable->BackDoor<EnemyShotData>();
    } else if (selectedTab == Tab::ITEM)
    {
        gameState->itemDataTable->BackDoor<UserDefDataBrowser>();
    }
}

UserDefDataBrowser::UserDefDataBrowser(int left, int top, int width, int height) :
    iniLeft(left),
    iniTop(top),
    iniWidth(width),
    iniHeight(height),
    openFlag(false)
{
}

UserDefDataBrowser::~UserDefDataBrowser() {}

void UserDefDataBrowser::draw(const std::shared_ptr<Engine>& engine)
{
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("UserDefData", &openFlag, ImGuiWindowFlags_ResizeFromAnySide))
    {
        engine->backDoor<UserDefDataBrowser>();
    }
    ImGui::End();
}
}