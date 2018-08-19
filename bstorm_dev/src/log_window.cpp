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

static const char* getLogLevelIcon(LogLevel level)
{
    switch (level)
    {
        case LogLevel::LV_INFO: return ICON_FA_INFO_CIRCLE;
        case LogLevel::LV_WARN: return ICON_FA_EXCLAMATION_TRIANGLE;
        case LogLevel::LV_ERROR: return ICON_FA_TIMES_CIRCLE;
        case LogLevel::LV_SUCCESS: return ICON_FA_CHECK_CIRCLE;
        case LogLevel::LV_DEBUG: return ICON_FA_COG;
        case LogLevel::LV_USER: return ICON_FA_COMMENT;
    }
    return ICON_FA_QUESTION_CIRCLE_O;
}

static ImU32 getLogLevelColor(LogLevel level)
{
    switch (level)
    {
        case LogLevel::LV_WARN: return IM_COL32(0xF8, 0xFF, 0x00, 0xFF);
        case LogLevel::LV_ERROR: return IM_COL32(0xFF, 0x00, 0x19, 0xFF);
        case LogLevel::LV_SUCCESS: return IM_COL32(0x0B, 0xFF, 0x00, 0xFF);
        case LogLevel::LV_DEBUG: return IM_COL32(0x00, 0xFF, 0xE1, 0xFF);
        case LogLevel::LV_USER: return IM_COL32(0xFF, 0x6E, 0x00, 0xFF);
    }
    return IM_COL32_WHITE;
}

static void showLevelMenu(LogLevel level, bool* selected)
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
    if (IsMatchString(searchText, log.Msg())) return true;
    if (log.Param() && IsMatchString(searchText, log.Param()->GetText())) return true;
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
                showLevelMenu(LogLevel::LV_INFO, &showInfoLevel);
                showLevelMenu(LogLevel::LV_WARN, &showWarnLevel);
                showLevelMenu(LogLevel::LV_ERROR, &showErrorLevel);
                showLevelMenu(LogLevel::LV_USER, &showUserLevel);
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
                switch (log.Level())
                {
                    // level filter
                    case LogLevel::LV_INFO: if (!showInfoLevel) continue; break;
                    case LogLevel::LV_WARN: if (!showWarnLevel) continue; break;
                    case LogLevel::LV_ERROR: if (!showErrorLevel) continue; break;
                    case LogLevel::LV_SUCCESS: if (!showInfoLevel) continue; break;
                    case LogLevel::LV_DEBUG: if (!showInfoLevel) continue; break;
                    case LogLevel::LV_USER: if (!showUserLevel) continue; break;
                }
                ImGui::PushID(logIdx); // ここ以降にbreak, continueを書かない
                ImGui::Columns(5, NULL, false);
                const float iconColWidth = defaultIconColWidth;
                const float srcPosColWidth = log.GetSourcePosStack().empty() ? 0.0f : defaultSrcPosColWidth;
                const float copyColWidth = defaultCopyColWidth;
                const float msgColWidth = log.Param() ?
                    (availWidth - iconColWidth - defaultSrcPosColWidth - copyColWidth) * 0.3f :
                    (availWidth - iconColWidth - srcPosColWidth - copyColWidth);
                const float paramColWidth = log.Param() ? (availWidth - iconColWidth - msgColWidth - srcPosColWidth - copyColWidth) : 0.0f;
                ImGui::SetColumnWidth(0, iconColWidth);
                ImGui::SetColumnWidth(1, msgColWidth);
                ImGui::SetColumnWidth(2, paramColWidth);
                ImGui::SetColumnWidth(3, srcPosColWidth);
                ImGui::SetColumnWidth(4, copyColWidth);
                ImGui::PushStyleColor(ImGuiCol_Text, getLogLevelColor(log.Level()));
                ImGui::Text(getLogLevelIcon(log.Level()));
                ImGui::NextColumn();
                {
                    // msg
                    ImGui::TextWrapped(log.Msg().c_str());
                }
                ImGui::PopStyleColor();
                ImGui::NextColumn();
                {
                    // param
                    if (log.Param())
                    {
                        ImU32 color;
                        switch (log.Param()->GetTag())
                        {
                            case LogParam::Tag::TEXTURE:
                            case LogParam::Tag::PLAYER_SHOT_DATA:
                            case LogParam::Tag::ENEMY_SHOT_DATA:
                            case LogParam::Tag::ITEM_DATA:
                            case LogParam::Tag::MESH:
                            case LogParam::Tag::SCRIPT:
                            case LogParam::Tag::SOUND:
                            case LogParam::Tag::RENDER_TARGET:
                            case LogParam::Tag::SHADER:
                                color = IM_COL32(0x00, 0x85, 0xFF, 0xFF);
                                break;
                            default:
                                color = IM_COL32_WHITE;
                                break;
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                        ImGui::TextWrapped(log.Param()->GetText().c_str());
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
