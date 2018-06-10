#include <algorithm>
#include <imgui.h>
#include <IconsFontAwesome_c.h>

#include <bstorm/render_target.hpp>
#include <bstorm/package.hpp>
#include <bstorm/util.hpp>

#include "play_controller.hpp"
#include "game_view.hpp"

namespace bstorm
{
GameView::GameView(int iniLeft, int iniTop, int iniWidth, int iniHeight, const std::shared_ptr<PlayController>& playController) :
    iniLeft(iniLeft),
    iniTop(iniTop),
    iniWidth(iniWidth),
    iniHeight(iniHeight),
    viewPosX(std::make_shared<int>(-1)),
    viewPosY(std::make_shared<int>(-1)),
    viewWidth(std::make_shared<int>(-1)),
    viewHeight(std::make_shared<int>(-1)),
    playController(playController)
{
}

GameView::~GameView() {}

void GameView::draw()
{
    ImGui::SetNextWindowPos(ImVec2(iniLeft, iniTop), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(iniWidth, iniHeight + 65), ImGuiCond_FirstUseEver);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    auto windowFlags = ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::Begin("Game View", NULL, windowFlags))
    {
        playController->SetInputEnable(ImGui::IsWindowFocused());
        {
            // menu bar
            ImGui::PopStyleVar();
            if (ImGui::BeginMenuBar())
            {
                if (ImGui::BeginMenu(ICON_FA_TELEVISION" Window"))
                {
                    if (ImGui::MenuItem("640 x 480"))
                    {
                        *viewWidth = 640; *viewHeight = 480;
                    }
                    if (ImGui::MenuItem("800 x 600"))
                    {
                        *viewWidth = 800; *viewHeight = 600;
                    }
                    if (ImGui::MenuItem("1024 x 768"))
                    {
                        *viewWidth = 1024; *viewHeight = 768;
                    }
                    if (ImGui::MenuItem("1280 x 960"))
                    {
                        *viewWidth = 1280; *viewHeight = 960;
                    }
                    if (ImGui::MenuItem("1024 x 576"))
                    {
                        *viewWidth = 1024; *viewHeight = 576;
                    }
                    if (ImGui::MenuItem("1280 x 720"))
                    {
                        *viewWidth = 1280; *viewHeight = 720;
                    }
                    if (ImGui::MenuItem("1600 x 900"))
                    {
                        *viewWidth = 1600; *viewHeight = 900;
                    }
                    if (ImGui::MenuItem("1920 x 1080"))
                    {
                        *viewWidth = 1920; *viewHeight = 1080;
                    }
                    if (*viewWidth >= 1 && *viewHeight >= 1)
                    {
                        ImGui::Separator();
                        if (ImGui::BeginMenu("Custom Size"))
                        {
                            ImGui::InputInt("width##gameViewWidth", viewWidth.get(), 1, 100);
                            ImGui::InputInt("height##gameViewHeight", viewHeight.get(), 1, 100);
                            *viewWidth = std::max(*viewWidth, 1);
                            *viewHeight = std::max(*viewHeight, 1);
                            ImGui::EndMenu();
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            }
        }
        {
            ImGui::PopStyleVar();
            const float optionSpace = 150.0f;
            const float controllerSpace = 94.0f;
            // 調整用
            //ImGui::DragFloat("option Space", &optionSpace, 0, 1000);
            //ImGui::DragFloat("controller Space", &controllerSpace, 0, 1000);

            // tool bar
            ImGui::BeginGroup();
            {
                // info text
                ImGui::BeginGroup();
                ImGui::Text(" elapsed: %d (%.1f fps)", playController->GetElapsedFrame(), ImGui::GetIO().Framerate);

                {
                    const ScriptInfo& mainScriptInfo = playController->GetMainScriptInfo();
                    auto scriptName = mainScriptInfo.title.empty() ? ToUTF8(mainScriptInfo.path) : ToUTF8(mainScriptInfo.title);
                    if (playController->HasPackage())
                    {
                        if (scriptName.empty())
                        {
                            ImGui::Text(" select script.");
                        } else
                        {
                            ImGui::Text((" ready: " + scriptName).c_str());
                        }
                    } else
                    {
                        if (playController->IsPaused())
                        {
                            ImGui::Text((" pause: " + scriptName).c_str());
                        } else
                        {
                            ImGui::Text((" running: " + scriptName).c_str());
                        }
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - optionSpace - controllerSpace);
            {
                // option
                ImGui::BeginGroup();
                {
                    bool renderIntersectionEnable = playController->IsRenderIntersectionEnabled();
                    ImGui::Checkbox("show intersection", &renderIntersectionEnable);
                    playController->SetRenderIntersectionEnable(renderIntersectionEnable);
                }
                {
                    bool playerInvincibleEnable = playController->IsPlayerInvincibleEnabled();
                    ImGui::Checkbox("never hit", &playerInvincibleEnable);
                    playController->SetPlayerInvincibleEnable(playerInvincibleEnable);
                }
                ImGui::EndGroup();
            }
            ImGui::SameLine(ImGui::GetContentRegionAvailWidth() - controllerSpace);
            {
                // controller
                ImGui::BeginGroup();
                {
                    // row 1
                    if (ImGui::Button(ICON_FA_REFRESH))
                    {
                        playController->Reload();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_STOP))
                    {
                        playController->Stop();
                    }
                    ImGui::SameLine();
                    if (playController->IsPaused())
                    {
                        if (ImGui::Button(ICON_FA_PLAY))
                        {
                            playController->Pause(false);
                        }
                    } else
                    {
                        if (ImGui::Button(ICON_FA_PAUSE))
                        {
                            playController->Pause(true);
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FA_STEP_FORWARD))
                    {
                        playController->Tick();
                    }
                }
                {
                    // row2
                    {
                        ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
                        int playSpeed = playController->GetPlaySpeed();
                        ImGui::InputInt("##playSpeed", &playSpeed, 1, 5);
                        playController->SetPlaySpeed(std::max(playSpeed, 1));
                        ImGui::PopItemWidth();
                    }
                    if (ImGui::IsItemHovered())
                    {
                        ImGui::SetTooltip("speed");
                    }
                }
                ImGui::EndGroup();
            }
            ImGui::EndGroup();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        }
        if (*viewWidth < 0 || *viewHeight < 0)
        {
            *viewWidth = ImGui::GetContentRegionAvailWidth();
            *viewHeight = ImGui::GetContentRegionAvail().y;
        }
        auto sideMargin = ImGui::GetWindowWidth() - ImGui::GetContentRegionAvailWidth();
        auto topMargin = ImGui::GetWindowSize().y - ImGui::GetContentRegionAvail().y;
        ImGui::SetWindowSize(ImVec2(*viewWidth + sideMargin, *viewHeight + topMargin), ImGuiCond_Always);
        if (auto package = playController->GetCurrentPackage())
        {
            constexpr wchar_t* GAME_VIEW_RENDER_TARGET = L"___GAME_VIEW_RENDER_TARGET___";
            auto gameViewRenderTarget = package->GetRenderTarget(GAME_VIEW_RENDER_TARGET);
            if (!gameViewRenderTarget)
            {
                gameViewRenderTarget = package->CreateRenderTarget(GAME_VIEW_RENDER_TARGET, package->GetScreenWidth(), package->GetScreenHeight(), nullptr);
            }
            package->Render(GAME_VIEW_RENDER_TARGET);
            ImGui::Image(gameViewRenderTarget->GetTexture(), ImVec2(*viewWidth, *viewHeight));
        } else
        {
            ImGui::Image(NULL, ImVec2(*viewWidth, *viewHeight), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0, 0, 0, 1));
        }
        {
            // set view pos to pointer
            auto windowPos = ImGui::GetWindowPos();
            *viewPosX = windowPos.x + sideMargin;
            *viewPosY = windowPos.y + topMargin;
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
std::shared_ptr<int> GameView::getViewPosX() const
{
    return viewPosX;
}
std::shared_ptr<int> GameView::getViewPosY() const
{
    return viewPosY;
}
std::shared_ptr<int> GameView::getViewWidth() const
{
    return viewWidth;
}
std::shared_ptr<int> GameView::getViewHeight() const
{
    return viewHeight;
}
}