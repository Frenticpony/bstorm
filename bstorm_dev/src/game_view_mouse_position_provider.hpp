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
    void GetMousePos(int screenWidth, int screenHeight, int &x, int &y) override;
    void SetScreenPos(const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY);
    void SetGameViewSize(const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight);
private:
    HWND hWnd_;
    std::shared_ptr<int> screenPosX_;
    std::shared_ptr<int> screenPosY_;
    std::shared_ptr<int> gameViewWidth_;
    std::shared_ptr<int> gameViewHeight_;
};
}