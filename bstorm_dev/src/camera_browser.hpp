#pragma once

#include <memory>

namespace bstorm
{
class Engine;
class CameraBrowser
{
public:
    CameraBrowser(int x, int y, int width, int height);
    ~CameraBrowser();
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
