#pragma once

#include <memory>

namespace bstorm
{
class Package;
class ObjectBrowser
{
public:
    ObjectBrowser(int x, int y, int width, int height);
    ~ObjectBrowser();
    void draw(const std::shared_ptr<Package>& package);
    bool isOpened() const { return openFlag; }
    void setOpen(bool b) { openFlag = b; }
private:
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool openFlag;
};

class Obj;
class ObjectLayerList;
void drawObjEditArea(const std::shared_ptr<Obj>& obj, std::shared_ptr<ObjectLayerList>& layerList);
}