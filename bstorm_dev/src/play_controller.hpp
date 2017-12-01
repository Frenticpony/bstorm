#pragma once

#include <string>
#include <memory>

#include <bstorm/script_info.hpp>

namespace bstorm {
  class Engine;
  class PlayController {
  public:
    PlayController(const std::shared_ptr<Engine>& engine);
    void tick();
    void pause(bool doPause);
    void reload();
    bool isPaused() const;
    int getPlaySpeed() const;
    void setPlaySpeed(int speed);
    void setScript(const ScriptInfo& mainScript, const ScriptInfo& playerScript);
    int64_t getElapsedFrame() const;
    void setScreenSize(int width, int height);
    bool isPackageFinished() const;
    bool isRenderIntersectionEnabled() const;
    void setRenderIntersectionEnable(bool enable);
    bool isPlayerInvincibleEnabled() const;
    void setPlayerInvincibleEnable(bool enable);
  private:
    std::shared_ptr<Engine> engine;
    ScriptInfo mainScript;
    ScriptInfo playerScript;
    int64_t elapsedFrame;
    int playSpeed;
    bool paused;
    int screenWidth;
    int screenHeight;
    bool renderIntersectionEnable;
    bool playerInvincibleEnable;
  };
}
