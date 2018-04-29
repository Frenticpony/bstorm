#include "game_view_mouse_position_provider.hpp"

namespace bstorm
{
GameViewMousePositionProvider::GameViewMousePositionProvider(HWND hWnd) : hWnd_(hWnd)
{
}

void GameViewMousePositionProvider::GetMousePos(int screenWidth, int screenHeight, int & x, int & y)
{
    POINT point;
    x = 0; y = 0;
    if (GetCursorPos(&point))
    {
        if (ScreenToClient(hWnd_, &point))
        {
            if (screenPosX_) { point.x -= *screenPosX_; }
            if (screenPosY_) { point.y -= *screenPosY_; }
            int windowWidth = screenWidth;
            int windowHeight = screenHeight;
            if (gameViewWidth_ && gameViewHeight_)
            {
                windowWidth = *gameViewWidth_;
                windowHeight = *gameViewHeight_;
            } else
            {
                RECT clientRect;
                if (GetClientRect(hWnd_, &clientRect))
                {
                    windowWidth = clientRect.right;
                    windowHeight = clientRect.bottom;
                }
            }
            point.x = 1.0f * point.x * screenWidth / windowWidth;
            point.y = 1.0f * point.y * screenHeight / windowHeight;
            x = point.x; y = point.y;
        }
    }
}

void GameViewMousePositionProvider::SetScreenPos(const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY)
{
    this->screenPosX_ = screenPosX;
    this->screenPosY_ = screenPosY;
}

void GameViewMousePositionProvider::SetGameViewSize(const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight)
{
    this->gameViewWidth_ = gameViewWidth;
    this->gameViewHeight_ = gameViewHeight;
}
}
