#include "play_controller.hpp"

#include <bstorm/const.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/package.hpp>

#include <algorithm>

namespace bstorm
{
PlayController::PlayController(const std::shared_ptr<Package>& package) :
    package(package),
    playSpeed(1),
    paused(true),
    screenWidth(package->GetScreenWidth()),
    screenHeight(package->GetScreenHeight()),
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
            if (package->IsPackageFinished()) break;
            package->TickFrame();
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
#ifndef _DEBUG
        FIXME!
       package->Reset(screenWidth, screenHeight);
#endif
    }
}

void PlayController::pause(bool doPause)
{
    if (paused == true && doPause == false && package->IsPackageFinished())
    {
        reload();
        if (!package->IsPackageFinished())
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
#ifndef _DEBUG
    FIXME!
        package->Reset(screenWidth, screenHeight);
#endif
    pause(true);
}

void PlayController::reload()
{
    try
    {
        if (mainScript.type == SCRIPT_TYPE_PACKAGE)
        {

#ifndef _DEBUG
            FIXME!
                package->Reset(screenWidth, screenHeight);
#endif
            package->SetPackageMainScript(mainScript);
            package->StartPackage();
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
#ifndef _DEBUG
                package->Reset(screenWidth, screenHeight);
#endif
                package->SetStageMainScript(mainScript);
                package->SetStagePlayerScript(playerScript);
                ScriptInfo defaultPackageMainScript;
                defaultPackageMainScript.path = DEFAULT_PACKAGE_PATH;
                defaultPackageMainScript.type = SCRIPT_TYPE_PACKAGE;
                defaultPackageMainScript.version = SCRIPT_VERSION_PH3;
                package->SetPackageMainScript(defaultPackageMainScript);
                package->StartPackage();
            }
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
#ifndef _DEBUG
        package->Reset(screenWidth, screenHeight);
#endif
    }
    package->SetRenderIntersectionEnable(renderIntersectionEnable);
    package->SetForcePlayerInvincibleEnable(playerInvincibleEnable);
    package->SetInputEnable(inputEnable);
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
    return package->GetElapsedFrame();
}

void PlayController::setScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

bool PlayController::isPackageFinished() const
{
    return package->IsPackageFinished();
}

bool PlayController::isRenderIntersectionEnabled() const
{
    return renderIntersectionEnable;
}

void PlayController::setRenderIntersectionEnable(bool enable)
{
    renderIntersectionEnable = enable;
    package->SetRenderIntersectionEnable(enable);
}

bool PlayController::isPlayerInvincibleEnabled() const
{
    return playerInvincibleEnable;
}

void PlayController::setPlayerInvincibleEnable(bool enable)
{
    playerInvincibleEnable = enable;
    package->SetForcePlayerInvincibleEnable(enable);
}

void PlayController::setInputEnable(bool enable)
{
    inputEnable = enable;
    package->SetInputEnable(enable);
}

const ScriptInfo& PlayController::getMainScriptInfo() const
{
    return mainScript;
}
}