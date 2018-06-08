#pragma once

#include <bstorm/script_info.hpp>
#include <bstorm/nullable_shared_ptr.hpp>

#include <string>
#include <memory>

namespace bstorm
{
class Package;
class Engine;
class PlayController
{
public:
    PlayController(const std::shared_ptr<Engine>& engine);
    void Tick();
    void Pause(bool doPause);
    void Stop();
    void Reload();
    bool IsPaused() const;
    int GetPlaySpeed() const;
    void SetPlaySpeed(int speed);
    void SetScript(const ScriptInfo& mainScript, const ScriptInfo& playerScript);
    int GetElapsedFrame() const;
    void SetScreenSize(int width, int height);
    bool IsPackageFinished() const;
    bool IsRenderIntersectionEnabled() const;
    void SetRenderIntersectionEnable(bool enable);
    bool IsPlayerInvincibleEnabled() const;
    void SetPlayerInvincibleEnable(bool enable);
    void SetInputEnable(bool enable);
    const ScriptInfo& GetMainScriptInfo() const;
    const NullableSharedPtr<Package>& GetCurrentPackage() const { return package_; }
private:
    std::shared_ptr<Engine> engine_;
    NullableSharedPtr<Package> package_;
    ScriptInfo mainScript_;
    ScriptInfo playerScript_;
    int playSpeed_;
    bool isPaused_;
    int screenWidth_;
    int screenHeight_;
};
}
