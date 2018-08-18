#include <imgui.h>
#include <imgui_internal.h>
#include <IconsFontAwesome_c.h>

#include <bstorm/file_util.hpp>
#include <bstorm/string_util.hpp>
#include <bstorm/source_map.hpp>

#include "log_window.hpp"

namespace bstorm
{
LogWindow::LogWindow() :
    iniLeft(0),
    iniTop(0),
    iniWidth(0),
    iniHeight(0),
    scrollToBottom(false),
    showInfoLevel(false),
    showWarnLevel(true),
    showErrorLevel(true),
    showUserLevel(true),
    headIdx(0),
    logCnt(0)
{
    filterInput.fill('\0');
}

LogWindow::~LogWindow() {}

void LogWindow::setInitWindowPos(int left, int top, int width, int height)
{
    iniLeft = left;
    iniTop = top;
    iniWidth = width;
    iniHeight = height;
}

void LogWindow::log(Log& lg) noexcept(false)
{
    log(std::move(Log(lg)));
}

void LogWindow::log(Log&& lg) noexcept(false)
{
    logs[validIdx(headIdx + logCnt)] = std::move(lg);
    if (logCnt != MaxLogCnt)
    {
        ++logCnt;
    } else
    {
        headIdx = validIdx(headIdx + 1);
    }
    scrollToBottom = true;
}

static const char* getLogLevelIcon(Log::Level level)
{
    switch (level)
    {
        case Log::Level::LV_INFO: return ICON_FA_INFO_CIRCLE;
        case Log::Level::LV_WARN: return ICON_FA_EXCLAMATION_TRIANGLE;
        case Log::Level::LV_ERROR: return ICON_FA_TIMES_CIRCLE;
        case Log::Level::LV_SUCCESS: return ICON_FA_CHECK_CIRCLE;
        case Log::Level::LV_DEBUG: return ICON_FA_COG;
        case Log::Level::LV_USER: return ICON_FA_COMMENT;
    }
    return ICON_FA_QUESTION_CIRCLE_O;
}

static ImU32 getLogLevelColor(Log::Level level)
{
    switch (level)
    {
        case Log::Level::LV_WARN: return IM_COL32(0xF8, 0xFF, 0x00, 0xFF);
        case Log::Level::LV_ERROR: return IM_COL32(0xFF, 0x00, 0x19, 0xFF);
        case Log::Level::LV_SUCCESS: return IM_COL32(0x0B, 0xFF, 0x00, 0xFF);
        case Log::Level::LV_DEBUG: return IM_COL32(0x00, 0xFF, 0xE1, 0xFF);
        case Log::Level::LV_USER: return IM_COL32(0xFF, 0x6E, 0x00, 0xFF);
    }
    return IM_COL32_WHITE;
}

static void showLevelMenu(Log::Level level, bool* selected)
{
    std::string icon = getLogLevelIcon(level);
    std::string name = Log::GetLevelName(level);
    name[0] = toupper(name[0]);
    std::string text = icon + " " + name;
    ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
    ImGui::MenuItem(text.c_str(), NULL, selected);
    ImGui::PopItemFlag();
}

static bool matchLog(const std::string& searchText, const Log& log)
{
    if (IsMatchString(searchText, log.GetMessage())) return true;
    if (log.GetParam() && IsMatchString(searchText, log.GetParam()->GetText())) return true;
    return false;
}

void LogWindow::draw()
{
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight), ImGuiCond_FirstUseEver);
    constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_ResizeFromAnySide |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_MenuBar;
    if (ImGui::Begin("Log", NULL, windowFlags))
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu(ICON_FA_EYE" Levels"))
            {
                {
                    // All
                    ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
                    if (ImGui::MenuItem(ICON_FA_ASTERISK" All", NULL))
                    {
                        bool show = true;
                        if (showInfoLevel && showWarnLevel && showErrorLevel && showUserLevel)
                        {
                            show = false;
                        }
                        showInfoLevel = showWarnLevel = showErrorLevel = showUserLevel = show;
                    }
                    ImGui::PopItemFlag();
                }
                showLevelMenu(Log::Level::LV_INFO, &showInfoLevel);
                showLevelMenu(Log::Level::LV_WARN, &showWarnLevel);
                showLevelMenu(Log::Level::LV_ERROR, &showErrorLevel);
                showLevelMenu(Log::Level::LV_USER, &showUserLevel);
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
        int showCnt = 0;
        {
            // log area
            ImGui::BeginChild("LogArea", ImVec2(-1, -ImGui::GetTextLineHeightWithSpacing() * 1.3), false, ImGuiWindowFlags_HorizontalScrollbar);

            // column width
            const float availWidth = ImGui::GetContentRegionAvailWidth();
            const float defaultIconColWidth = ImGui::GetTextLineHeightWithSpacing() * 1.3;
            const float defaultSrcPosColWidth = ImGui::GetTextLineHeightWithSpacing() * 10 > availWidth * 0.3 ?
                ImGui::GetTextLineHeightWithSpacing() * 4 :
                ImGui::GetTextLineHeightWithSpacing() * 10;
            const float defaultCopyColWidth = defaultIconColWidth;

            std::string searchText(filterInput.data()); // delete null sequence
            for (int i = 0; i < logCnt; i++)
            {
                const int logIdx = validIdx(headIdx + i);
                const auto& log = logs.at(logIdx);
                if (!searchText.empty())
                {
                    // filter
                    if (!matchLog(searchText, log))
                    {
                        continue;
                    }
                }
                switch (log.GetLevel())
                {
                    // level filter
                    case Log::Level::LV_INFO: if (!showInfoLevel) continue; break;
                    case Log::Level::LV_WARN: if (!showWarnLevel) continue; break;
                    case Log::Level::LV_ERROR: if (!showErrorLevel) continue; break;
                    case Log::Level::LV_SUCCESS: if (!showInfoLevel) continue; break;
                    case Log::Level::LV_DEBUG: if (!showInfoLevel) continue; break;
                    case Log::Level::LV_USER: if (!showUserLevel) continue; break;
                }
                ImGui::PushID(logIdx); // ここ以降にbreak, continueを書かない
                ImGui::Columns(5, NULL, false);
                const float iconColWidth = defaultIconColWidth;
                const float srcPosColWidth = log.GetSourcePosStack().empty() ? 0.0f : defaultSrcPosColWidth;
                const float copyColWidth = defaultCopyColWidth;
                const float msgColWidth = log.GetParam() ?
                    (availWidth - iconColWidth - defaultSrcPosColWidth - copyColWidth) * 0.3f :
                    (availWidth - iconColWidth - srcPosColWidth - copyColWidth);
                const float paramColWidth = log.GetParam() ? (availWidth - iconColWidth - msgColWidth - srcPosColWidth - copyColWidth) : 0.0f;
                ImGui::SetColumnWidth(0, iconColWidth);
                ImGui::SetColumnWidth(1, msgColWidth);
                ImGui::SetColumnWidth(2, paramColWidth);
                ImGui::SetColumnWidth(3, srcPosColWidth);
                ImGui::SetColumnWidth(4, copyColWidth);
                ImGui::PushStyleColor(ImGuiCol_Text, getLogLevelColor(log.GetLevel()));
                ImGui::Text(getLogLevelIcon(log.GetLevel()));
                ImGui::NextColumn();
                {
                    // msg
                    ImGui::TextWrapped(log.GetMessage().c_str());
                }
                ImGui::PopStyleColor();
                ImGui::NextColumn();
                {
                    // param
                    if (log.GetParam())
                    {
                        ImU32 color;
                        switch (log.GetParam()->GetTag())
                        {
                            case Log::Param::Tag::TEXTURE:
                            case Log::Param::Tag::PLAYER_SHOT_DATA:
                            case Log::Param::Tag::ENEMY_SHOT_DATA:
                            case Log::Param::Tag::ITEM_DATA:
                            case Log::Param::Tag::MESH:
                            case Log::Param::Tag::SCRIPT:
                            case Log::Param::Tag::SOUND:
                            case Log::Param::Tag::RENDER_TARGET:
                            case Log::Param::Tag::SHADER:
                                color = IM_COL32(0x00, 0x85, 0xFF, 0xFF);
                                break;
                            default:
                                color = IM_COL32_WHITE;
                                break;
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::TextWrapped(log.GetParam()->GetText().c_str());
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::NextColumn();
                {
                    // source pos
                    const auto& srcPosStack = log.GetSourcePosStack();
                    if (!srcPosStack.empty())
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0xfb, 0xed, 0xed, 0xff));
                        auto omittedPos = ToUTF8(GetOmittedFileName(*srcPosStack[0].filename, 15)) + ":" + std::to_string(srcPosStack[0].line);
                        auto omittedPosWidth = ImGui::CalcTextSize(omittedPos.c_str()).x;
                        ImGui::Text("");
                        ImGui::SameLine(0.0f, srcPosColWidth - omittedPosWidth - 12.0f);
                        ImGui::TextWrapped(omittedPos.c_str());
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::BeginTooltip();
                            for (const auto& srcPos : srcPosStack)
                            {
                                ImGui::Text(srcPos.ToString().c_str());
                            }
                            ImGui::EndTooltip();
                        }
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::NextColumn();
                {
                    // copy button
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    if (ImGui::Button(ICON_FA_CLIPBOARD))
                    {
                        ImGui::LogToClipboard();
                        ImGui::LogText(log.ToString().c_str());
                        ImGui::LogFinish();
                    }
                    ImGui::PopStyleVar();
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("copy");
                    }
                }
                ImGui::NextColumn();
                ImGui::Separator();
                ImGui::PopID();
                ImGui::Columns(1);
                showCnt++;
            }
            if (scrollToBottom)
            {
                ImGui::SetScrollHere(0.99999f);
                scrollToBottom = false;
            }
            ImGui::EndChild();
        }
        ImGui::Separator();
        {
            const float countWidth = 120.0f;
            // tool space
            if (ImGui::Button(ICON_FA_TRASH))
            {
                clear();
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - countWidth);
            ImGui::InputText(ICON_FA_SEARCH"##logfilter", filterInput.data(), filterInput.size());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text(ICON_FA_EYE" Count: %d", showCnt);
        }
    }
    ImGui::End();
}

void LogWindow::clear()
{
    headIdx = logCnt = 0;
}

int LogWindow::validIdx(int idx) const
{
    return idx & (MaxLogCnt - 1);
}
}
