#pragma once

#include <windows.h>
#include <memory>
#include <bstorm/input_device.hpp>

namespace bstorm
{
class GameViewMousePositionProvider : public MousePositionProvider
{
public:
    GameViewMousePositionProvider(HWND hWnd);
    void getMousePos(int screenWidth, int screenHeight, int &x, int &y) override;
    void setScreenPos(const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY);
    void setGameViewSize(const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight);
private:
    HWND hWnd;
    std::shared_ptr<int> screenPosX;
    std::shared_ptr<int> screenPosY;
    std::shared_ptr<int> gameViewWidth;
    std::shared_ptr<int> gameViewHeight;
};
}