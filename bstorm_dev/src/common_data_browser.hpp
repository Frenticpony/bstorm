#pragma once

#include <memory>

namespace bstorm
{
class Engine;
class CommonDataBrowser
{
public:
    CommonDataBrowser(int x, int y, int width, int height);
    ~CommonDataBrowser();
    void draw(const std::shared_ptr<Engine>& engine);
    bool isOpened() const { return openFlag; }
    void setOpen(bool b) { openFlag = b; }
private:
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool openFlag;
};
}