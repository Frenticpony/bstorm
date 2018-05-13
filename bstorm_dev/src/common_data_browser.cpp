#pragma once

#include <imgui.h>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/package.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/common_data_db.hpp>
#include <bstorm/package.hpp>

#include "common_data_browser.hpp"

namespace bstorm
{
static void DrawCommonDataInfo(const std::map<CommonDataDB::DataAreaName, CommonDataDB::CommonDataArea>& areaTable)
{
    static CommonDataDB::DataAreaName selectedArea;
    {
        // common data area list
        ImGui::Text("Data Area");
        ImGui::BeginChild("CommonDataAreaList", ImVec2(-1, ImGui::GetContentRegionAvail().y * 0.1), true, ImGuiWindowFlags_HorizontalScrollbar);
        for (const auto& entry : areaTable)
        {
            const CommonDataDB::DataAreaName& areaName = entry.first;
            std::string label = areaName.empty() ? "[default]" : ToUTF8(areaName);
            if (ImGui::Selectable(label.c_str(), selectedArea == areaName))
            {
                selectedArea = areaName;
            }
        }
        ImGui::EndChild();
    }
    {
        // key-value list
        ImGui::BeginChild("CommonDataKeyValueList", ImVec2(-1, -1), true, ImGuiWindowFlags_HorizontalScrollbar);
        auto it = areaTable.find(selectedArea);
        if (it != areaTable.end())
        {
            const auto& area = it->second;
            ImGui::Columns(2, "key-value-list");
            ImGui::Text("Key"); ImGui::NextColumn(); ImGui::Text("Value"); ImGui::NextColumn();
            ImGui::Separator();
            for (auto& entry : area)
            {
                const auto& key = entry.first;
                const auto& value = entry.second;
                if (ImGui::GetColumnIndex() == 0)
                {
                    ImGui::Separator();
                }
                ImGui::Text(ToUTF8(key).c_str()); ImGui::NextColumn(); ImGui::Text(ToUTF8(value->ToString()).c_str()); ImGui::NextColumn();
            }
            ImGui::Columns(1);
            ImGui::Separator();
        }
        ImGui::EndChild();
    }
}

template <>
void Package::backDoor<CommonDataBrowser>()
{
    const auto& areaTable = commonDataDB->GetCommonDataAreaTable();
    DrawCommonDataInfo(areaTable);
}

CommonDataBrowser::CommonDataBrowser(int left, int top, int width, int height) :
    iniLeft(left),
    iniTop(top),
    iniWidth(width),
    iniHeight(height),
    openFlag(false)
{
}
CommonDataBrowser::~CommonDataBrowser() {}

void CommonDataBrowser::draw(const std::shared_ptr<Package>& package)
{
    if (!isOpened()) return;
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Common Data", &openFlag, ImGuiWindowFlags_ResizeFromAnySide))
    {
        package->backDoor<CommonDataBrowser>();
    }
    ImGui::End();
}
}
