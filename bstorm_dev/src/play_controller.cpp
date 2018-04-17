#include "play_controller.hpp"

#include <bstorm/const.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/engine.hpp>

namespace bstorm
{
PlayController::PlayController(const std::shared_ptr<Engine>& engine) :
    engine(engine),
    playSpeed(1),
    paused(true),
    screenWidth(engine->getScreenWidth()),
    screenHeight(engine->getScreenHeight()),
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
            if (engine->isPackageFinished()) break;
            engine->tickFrame();
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
        engine->reset(screenWidth, screenHeight);
    }
}

void PlayController::pause(bool doPause)
{
    if (paused == true && doPause == false && engine->isPackageFinished())
    {
        reload();
        if (!engine->isPackageFinished())
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
    engine->reset(screenWidth, screenHeight);
    pause(true);
}

void PlayController::reload()
{
    try
    {
        if (mainScript.type == SCRIPT_TYPE_PACKAGE)
        {
            engine->reset(screenWidth, screenHeight);
            engine->setPackageMainScript(mainScript);
            engine->startPackage();
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
                engine->reset(screenWidth, screenHeight);
                engine->setStageMainScript(mainScript);
                engine->setStagePlayerScript(playerScript);
                ScriptInfo defaultPackageMainScript;
                defaultPackageMainScript.path = DEFAULT_PACKAGE_PATH;
                defaultPackageMainScript.type = SCRIPT_TYPE_PACKAGE;
                defaultPackageMainScript.version = SCRIPT_VERSION_PH3;
                engine->setPackageMainScript(defaultPackageMainScript);
                engine->startPackage();
            }
        }
    } catch (Log& log)
    {
        Logger::WriteLog(log);
        engine->reset(screenWidth, screenHeight);
    }
    engine->setRenderIntersectionEnable(renderIntersectionEnable);
    engine->setForcePlayerInvincibleEnable(playerInvincibleEnable);
    engine->setInputEnable(inputEnable);
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
    return engine->getElapsedFrame();
}

void PlayController::setScreenSize(int width, int height)
{
    screenWidth = width;
    screenHeight = height;
}

bool PlayController::isPackageFinished() const
{
    return engine->isPackageFinished();
}

bool PlayController::isRenderIntersectionEnabled() const
{
    return renderIntersectionEnable;
}

void PlayController::setRenderIntersectionEnable(bool enable)
{
    renderIntersectionEnable = enable;
    engine->setRenderIntersectionEnable(enable);
}

bool PlayController::isPlayerInvincibleEnabled() const
{
    return playerInvincibleEnable;
}

void PlayController::setPlayerInvincibleEnable(bool enable)
{
    playerInvincibleEnable = enable;
    engine->setForcePlayerInvincibleEnable(enable);
}

void PlayController::setInputEnable(bool enable)
{
    inputEnable = enable;
    engine->setInputEnable(enable);
}

const ScriptInfo& PlayController::getMainScriptInfo() const
{
    return mainScript;
}
}