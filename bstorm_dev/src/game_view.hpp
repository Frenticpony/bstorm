#pragma once

#include <memory>

namespace bstorm
{
class RenderTarget;
class PlayController;
class GameView
{
public:
    GameView(int iniLeft, int iniTop, int iniWidth, int iniHeight, const std::shared_ptr<PlayController>& playController);
    ~GameView();
    void draw();
    std::shared_ptr<int> getViewPosX() const;
    std::shared_ptr<int> getViewPosY() const;
    std::shared_ptr<int> getViewWidth() const;
    std::shared_ptr<int> getViewHeight() const;
private:
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    std::shared_ptr<int> viewPosX;
    std::shared_ptr<int> viewPosY;
    std::shared_ptr<int> viewWidth;
    std::shared_ptr<int> viewHeight;
    std::shared_ptr<PlayController> playController;
};
}