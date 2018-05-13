#pragma once

#include <bstorm/script_info.hpp>

#include <string>
#include <memory>

namespace bstorm
{
class Package;
class PlayController
{
public:
    PlayController(const std::shared_ptr<Package>& package);
    void tick();
    void pause(bool doPause);
    void close();
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
    void setInputEnable(bool enable);
    const ScriptInfo& getMainScriptInfo() const;
private:
    std::shared_ptr<Package> package;
    ScriptInfo mainScript;
    ScriptInfo playerScript;
    int64_t elapsedFrame;
    int playSpeed;
    bool paused;
    int screenWidth;
    int screenHeight;
    bool renderIntersectionEnable;
    bool playerInvincibleEnable;
    bool inputEnable;
};
}
