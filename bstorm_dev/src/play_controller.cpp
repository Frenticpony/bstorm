#include "play_controller.hpp"

#include <bstorm/path_const.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/package.hpp>
#include <bstorm/engine.hpp>
#include <bstorm/engine_develop_options.hpp>

#include <algorithm>

namespace bstorm
{
PlayController::PlayController(const std::shared_ptr<Engine>& engine) :
    engine_(engine),
    package_(nullptr),
    playSpeed_(1),
    isPaused_(true),
    screenWidth_(640),
    screenHeight_(480)
{
}

void PlayController::Tick()
{
    if (NotExistPackage()) return;
    if (package_->IsClosed())
    {
        package_->Finalize();
        package_ = nullptr;
        return;
    }
    try
    {
        for (int i = 0; i < playSpeed_; i++)
        {
            if (package_->IsClosed())
            {
                package_->Finalize();
                package_ = nullptr;
                break;
            }
            package_->TickFrame();
        }
    } catch (Log& log)
    {
        package_ = nullptr;
        Logger::Write(log);
    }
}

void PlayController::Pause(bool doPause)
{
    if (isPaused_ == true && doPause == false && NotExistPackage())
    {
        // パッケージ再生開始
        Reload();
        if (!NotExistPackage())
        {
            isPaused_ = false;
        }
    } else
    {
        isPaused_ = doPause;
    }
}

void PlayController::Stop()
{
    Pause(true);
    if (package_)
    {
        package_->Finalize();
    }
    package_ = nullptr;
}

void PlayController::Reload()
{
    try
    {
        if (mainScript_.type == ScriptType::Value::PACKAGE)
        {
            if (package_)
            {
                package_->Finalize();
                package_ = nullptr;
            }
            package_ = engine_->CreatePackage(screenWidth_, screenHeight_, mainScript_.path);
            package_->Start();
        } else
        {
            if (mainScript_.path.empty())
            {
                Logger::Write(LogLevel::LV_ERROR, "main script is not selected.");
            }
            if (playerScript_.path.empty())
            {
                Logger::Write(LogLevel::LV_ERROR, "player script is not selected.");
            }
            if (!mainScript_.path.empty() && !playerScript_.path.empty())
            {
                if (mainScript_.type == ScriptType::Value::UNKNOWN)
                {
                    mainScript_.type = ScriptType::Value::SINGLE;
                }

                package_ = engine_->CreatePackage(screenWidth_, screenHeight_, DEFAULT_PACKAGE_PATH);
                package_->SetStageMainScript(mainScript_);
                package_->SetStagePlayerScript(playerScript_);
                package_->Start();
            }
        }
    } catch (Log& log)
    {
        package_ = nullptr;
        Logger::Write(log);
    }
}

bool PlayController::IsPaused() const
{
    return isPaused_;
}

int PlayController::GetPlaySpeed() const
{
    return playSpeed_;
}

void PlayController::SetPlaySpeed(int speed)
{
    playSpeed_ = std::max(speed, 1);
}

void PlayController::SetScript(const ScriptInfo & mainScript, const ScriptInfo & playerScript)
{
    mainScript_ = mainScript;
    playerScript_ = playerScript;
}

int PlayController::GetElapsedFrame() const
{
    if (NotExistPackage()) return 0;
    return package_->GetElapsedFrame();
}

void PlayController::SetScreenSize(int width, int height)
{
    screenWidth_ = width;
    screenHeight_ = height;
}

bool PlayController::NotExistPackage() const
{
    return !package_;
}

bool PlayController::IsRenderIntersectionEnabled() const
{
    return engine_->GetDevelopOptions()->renderIntersectionEnable;
}

void PlayController::SetRenderIntersectionEnable(bool enable)
{
    engine_->GetDevelopOptions()->renderIntersectionEnable = enable;
}

bool PlayController::IsPlayerInvincibleEnabled() const
{
    return engine_->GetDevelopOptions()->forcePlayerInvincibleEnable;
}

void PlayController::SetPlayerInvincibleEnable(bool enable)
{
    engine_->GetDevelopOptions()->forcePlayerInvincibleEnable = enable;
}

void PlayController::SetInputEnable(bool enable)
{
    engine_->SetInputEnable(enable);
}

const ScriptInfo& PlayController::GetMainScriptInfo() const
{
    return mainScript_;
}
}