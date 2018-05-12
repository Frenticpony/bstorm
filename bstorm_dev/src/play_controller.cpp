#include "play_controller.hpp"

#include <bstorm/const.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/engine.hpp>

#include <algorithm>

namespace bstorm
{
PlayController::PlayController(const std::shared_ptr<Engine>& engine) :
    engine(engine),
    playSpeed(1),
    paused(true),
    screenWidth(engine->GetScreenWidth()),
    screenHeight(engine->GetScreenHeight()),
    renderIntersectionEnable(false),
    playerInvincibleEnable(false),
    inputEnable(false)
{
}

void PlayController::tick()
{
    try
    {
        for (int i = 0; i < playSpeed; i++)
        {
            if (engine->IsPackageFinished()) break;
            engine->TickFrame();
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
        engine->Reset(screenWidth, screenHeight);
    }
}

void PlayController::pause(bool doPause)
{
    if (paused == true && doPause == false && engine->IsPackageFinished())
    {
        reload();
        if (!engine->IsPackageFinished())
        {
            paused = false;
        }
    } else
    {
        paused = doPause;
    }
}

void PlayController::close()
{
    engine->Reset(screenWidth, screenHeight);
    pause(true);
}

void PlayController::reload()
{
    try
    {
        if (mainScript.type == SCRIPT_TYPE_PACKAGE)
        {
            engine->Reset(screenWidth, screenHeight);
            engine->SetPackageMainScript(mainScript);
            engine->StartPackage();
        } else
        {
            if (mainScript.path.empty())
            {
                Logger::WriteLog(Log::Level::LV_ERROR, "main script is not selected.");
            }
            if (playerScript.path.empty())
            {
                Logger::WriteLog(Log::Level::LV_ERROR, "player script is not selected.");
            }
            if (!mainScript.path.empty() && !playerScript.path.empty())
            {
                if (mainScript.type == SCRIPT_TYPE_UNKNOWN)
                {
                    mainScript.type = SCRIPT_TYPE_SINGLE;
                }
                engine->Reset(screenWidth, screenHeight);
                engine->SetStageMainScript(mainScript);
                engine->SetStagePlayerScript(playerScript);
                ScriptInfo defaultPackageMainScript;
                defaultPackageMainScript.path = DEFAULT_PACKAGE_PATH;
                defaultPackageMainScript.type = SCRIPT_TYPE_PACKAGE;
                defaultPackageMainScript.version = SCRIPT_VERSION_PH3;
                engine->SetPackageMainScript(defaultPackageMainScript);
                engine->StartPackage();
            }
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
        engine->Reset(screenWidth, screenHeight);
    }
    engine->SetRenderIntersectionEnable(renderIntersectionEnable);
    engine->SetForcePlayerInvincibleEnable(playerInvincibleEnable);
    engine->SetInputEnable(inputEnable);
}

bool PlayController::isPaused() const
{
    return paused;
}

int PlayController::getPlaySpeed() const
{
    return playSpeed;
}

void PlayController::setPlaySpeed(int speed)
{
    playSpeed = std::max(speed, 1);
}

void PlayController::setScript(const ScriptInfo & mainScript, const ScriptInfo & playerScript)
{
    this->mainScript = mainScript;
    this->playerScript = playerScript;
}

int64_t PlayController::getElapsedFrame() const
{
    return engine->GetElapsedFrame();
}

void PlayController::setScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

bool PlayController::isPackageFinished() const
{
    return engine->IsPackageFinished();
}

bool PlayController::isRenderIntersectionEnabled() const
{
    return renderIntersectionEnable;
}

void PlayController::setRenderIntersectionEnable(bool enable)
{
    renderIntersectionEnable = enable;
    engine->SetRenderIntersectionEnable(enable);
}

bool PlayController::isPlayerInvincibleEnabled() const
{
    return playerInvincibleEnable;
}

void PlayController::setPlayerInvincibleEnable(bool enable)
{
    playerInvincibleEnable = enable;
    engine->SetForcePlayerInvincibleEnable(enable);
}

void PlayController::setInputEnable(bool enable)
{
    inputEnable = enable;
    engine->SetInputEnable(enable);
}

const ScriptInfo& PlayController::getMainScriptInfo() const
{
    return mainScript;
}
}