#include "game_view_mouse_position_provider.hpp"

namespace bstorm
{
GameViewMousePositionProvider::GameViewMousePositionProvider(HWND hWnd) : hWnd(hWnd)
{
}

void GameViewMousePositionProvider::getMousePos(int screenWidth, int screenHeight, int & x, int & y)
{
    POINT point;
    x = 0; y = 0;
    if (GetCursorPos(&point))
    {
        if (ScreenToClient(hWnd, &point))
        {
            if (screenPosX) { point.x -= *screenPosX; }
            if (screenPosY) { point.y -= *screenPosY; }
            int windowWidth = screenWidth;
            int windowHeight = screenHeight;
            if (gameViewWidth && gameViewHeight)
            {
                windowWidth = *gameViewWidth;
                windowHeight = *gameViewHeight;
            } else
            {
                RECT clientRect;
                if (GetClientRect(hWnd, &clientRect))
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

void GameViewMousePositionProvider::setScreenPos(const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY)
{
    this->screenPosX = screenPosX;
    this->screenPosY = screenPosY;
}

void GameViewMousePositionProvider::setGameViewSize(const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight)
{
    this->gameViewWidth = gameViewWidth;
    this->gameViewHeight = gameViewHeight;
}
}
