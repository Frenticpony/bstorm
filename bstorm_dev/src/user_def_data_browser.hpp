#pragma once

#include <memory>

namespace bstorm {
  class Engine;
  class ShotData;
  class ItemData;
  class UserDefDataBrowser {
  public :
    UserDefDataBrowser(int left, int top, int width, int height);
    ~UserDefDataBrowser();
    void draw(const std::shared_ptr<Engine>& engine);
    bool isOpened() const { return openFlag; }
    void setOpen(bool b) { openFlag = b; }
  private :
    int iniLeft;
    int iniTop;
    int iniWidth;
    int iniHeight;
    bool openFlag;
  };

  void drawShotDataInfo(const std::shared_ptr<ShotData>& shotData);
  void drawItemDataInfo(const std::shared_ptr<ItemData>& itemData);
}