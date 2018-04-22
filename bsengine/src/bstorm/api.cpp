#include <bstorm/api.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_mesh.hpp>
#include <bstorm/obj_text.hpp>
#include <bstorm/obj_sound.hpp>
#include <bstorm/obj_file.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/obj_spell.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/script.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/engine.hpp>

#include <exception>

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
static int GetModuleDirectory(lua_State* L)
{
    DnhArray(L"./").push(L);
    return 1;
}

static int GetMainStgScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getMainStgScriptPath()).push(L);
    return 1;
}

static int GetMainStgScriptDirectory(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getMainStgScriptDirectory()).push(L);
    return 1;
}

static int GetMainPackageScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getMainPackageScriptPath()).push(L);
    return 1;
}

static int GetScriptPathList(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto dirPath = DnhValue::toString(L, 1);
    int scriptType = DnhValue::toInt(L, 2);
    DnhArray pathList;
    for (const auto& info : engine->getScriptList(dirPath, scriptType, false))
    {
        pathList.pushBack(std::make_unique<DnhArray>(info.path));
    }
    pathList.push(L);
    return 1;
}

static int GetCurrentDateTimeS(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getCurrentDateTimeS()).push(L);
    return 1;
}

static int GetStageTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStageTime());
    return 1;
}

static int GetPackageTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getPackageTime());
    return 1;
}

static int GetCurrentFps(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getCurrentFps());
    return 1;
}

// API
static int InstallFont(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    lua_pushboolean(L, engine->installFont(path, getSourcePos(L)));
    return 1;
}

static int ToString(lua_State* L)
{
    auto str = DnhValue::toString(L, 1);
    DnhArray(str).push(L);
    return 1;
}

static int GetReplayFps(lua_State* L)
{
    // FUTURE : impl
    lua_pushnumber(L, 0);
    return 1;
}

static int WriteLog(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::string msg = DnhValue::toStringU8(L, 1);
    engine->writeLog(std::move(msg), getSourcePos(L));
    return 0;
}

static int RaiseError(lua_State* L)
{
    std::string msg = DnhValue::toStringU8(L, 1);
    throw Log(Log::Level::LV_ERROR)
        .setMessage("RaiseError.")
        .setParam(Log::Param(Log::Param::Tag::TEXT, std::move(msg)));
    return 0;
}

static int assert(lua_State* L)
{
    bool cond = DnhValue::toBool(L, 1);
    std::string msg = DnhValue::toStringU8(L, 2);
    if (!cond)
    {
        throw Log(Log::Level::LV_ERROR)
            .setMessage("assertion failed.")
            .setParam(Log::Param(Log::Param::Tag::TEXT, std::move(msg)));
    }
    return 0;
}

static int SetCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::toString(L, 1);
    auto value = DnhValue::get(L, 2);
    engine->setCommonData(key, std::move(value));
    return 0;
}

static int GetCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::toString(L, 1);
    auto defaultValue = DnhValue::get(L, 2);
    engine->getCommonData(key, std::move(defaultValue))->push(L);
    return 1;
}

static int ClearCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->clearCommonData();
    return 0;
}

static int DeleteCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::toString(L, 1);
    engine->deleteCommonData(key);
    return 0;
}

static int SetAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    auto value = DnhValue::get(L, 3);
    engine->setAreaCommonData(area, key, std::move(value));
    return 0;
}

static int GetAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    auto defaultValue = DnhValue::get(L, 3);
    engine->getAreaCommonData(area, key, std::move(defaultValue))->push(L);
    return 1;
}

static int ClearAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    engine->clearAreaCommonData(area);
    return 0;
}

static int DeleteAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    engine->deleteAreaCommonData(area, key);
    return 0;
}

static int CreateCommonDataArea(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    engine->createCommonDataArea(area);
    return 0;
}

static int IsCommonDataAreaExists(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    lua_pushboolean(L, engine->isCommonDataAreaExists(area));
    return 1;
}

static int CopyCommonDataArea(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring dst = DnhValue::toString(L, 1);
    std::wstring src = DnhValue::toString(L, 2);
    engine->copyCommonDataArea(dst, src);
    return 0;
}

static int GetCommonDataAreaKeyList(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray ret;
    for (const auto& key : engine->getCommonDataAreaKeyList())
    {
        ret.pushBack(std::make_unique<DnhArray>(key));
    }
    ret.push(L);
    return 1;
}

static int GetCommonDataValueKeyList(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    DnhArray ret;
    for (const auto& key : engine->getCommonDataValueKeyList(area))
    {
        ret.pushBack(std::make_unique<DnhArray>(key));
    }
    ret.push(L);
    return 1;
}

static int SaveCommonDataAreaA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    lua_pushboolean(L, engine->saveCommonDataAreaA1(area));
    return 1;
}

static int LoadCommonDataAreaA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    lua_pushboolean(L, engine->loadCommonDataAreaA1(area));
    return 1;
}

static int SaveCommonDataAreaA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    std::wstring saveFilePath = DnhValue::toString(L, 2);
    lua_pushboolean(L, engine->saveCommonDataAreaA2(area, saveFilePath));
    return 1;
}

static int LoadCommonDataAreaA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::toString(L, 1);
    std::wstring saveFilePath = DnhValue::toString(L, 2);
    lua_pushboolean(L, engine->loadCommonDataAreaA2(area, saveFilePath));
    return 1;
}

static int SaveCommonDataAreaToReplayFile(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushboolean(L, false);
    return 1;
}

static int LoadCommonDataAreaFromReplayFile(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushboolean(L, false);
    return 1;
}

static int LoadSound(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->loadOrphanSound(path, getSourcePos(L));
    return 0;
}

static int RemoveSound(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->removeOrphanSound(path);
    return 0;
}

static int PlayBGM(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    double loopStartSec = DnhValue::toNum(L, 2);
    double loopEndSec = DnhValue::toNum(L, 3);
    engine->playBGM(path, loopStartSec, loopEndSec);
    return 0;
}

static int PlaySE(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->playSE(path);
    return 0;
}

static int StopSound(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->stopOrphanSound(path);
    return 0;
}

static int GetVirtualKeyState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::toInt(L, 1);
    lua_pushnumber(L, engine->getVirtualKeyState(vk));
    return 1;
}

static int SetVirtualKeyState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::toInt(L, 1);
    int state = DnhValue::toInt(L, 2);
    engine->setVirtualKeyState(vk, state);
    return 0;
}

static int AddVirtualKey(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::toInt(L, 1);
    int k = DnhValue::toInt(L, 2);
    int btn = DnhValue::toInt(L, 3);
    engine->addVirtualKey(vk, k, btn);
    return 0;
}

static int AddReplayTargetVirtualKey(lua_State* L)
{
    // FUTURE : impl
    Engine* engine = getEngine(L);
    return 0;
}

static int GetKeyState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int k = DnhValue::toInt(L, 1);
    lua_pushnumber(L, engine->getKeyState(k));
    return 1;
}

static int GetMouseState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int btn = DnhValue::toInt(L, 1);
    lua_pushnumber(L, engine->getMouseState(btn));
    return 1;
}

static int GetMouseX(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getMouseX());
    return 1;
}

static int GetMouseY(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getMouseY());
    return 1;
}

static int GetMouseMoveZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getMouseMoveZ());
    return 1;
}

static int SetSkipModeKey(lua_State* L)
{
    // 廃止
    return 0;
}

static int CreateRenderTarget(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    try
    {
        engine->createRenderTarget(name, 1024, 512, getSourcePos(L));
        lua_pushboolean(L, true);
    } catch (Log& log)
    {
        log.setLevel(Log::Level::LV_WARN)
            .addSourcePos(getSourcePos(L));
        Logger::WriteLog(log);
        lua_pushboolean(L, false);
    }
    return 1;
}

static int RenderToTextureA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    int begin = DnhValue::toInt(L, 2);
    int end = DnhValue::toInt(L, 3);
    bool doClear = DnhValue::toBool(L, 4);
    engine->renderToTextureA1(name, begin, end, doClear);
    return 0;
}

static int RenderToTextureB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    int objId = DnhValue::toInt(L, 2);
    bool doClear = DnhValue::toBool(L, 3);
    engine->renderToTextureB1(name, objId, doClear);
    return 0;
}

static int SaveRenderedTextureA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    auto path = DnhValue::toString(L, 2);
    engine->saveRenderedTextureA1(name, path, getSourcePos(L));
    return 0;
}

static int SaveRenderedTextureA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    auto path = DnhValue::toString(L, 2);
    int l = DnhValue::toInt(L, 3);
    int t = DnhValue::toInt(L, 4);
    int r = DnhValue::toInt(L, 5);
    int b = DnhValue::toInt(L, 6);
    engine->saveRenderedTextureA2(name, path, l, t, r, b, getSourcePos(L));
    return 0;
}

static int SaveSnapShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->saveSnapShotA1(path, getSourcePos(L));
    return 0;
}

static int SaveSnapShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    int l = DnhValue::toInt(L, 2);
    int t = DnhValue::toInt(L, 3);
    int r = DnhValue::toInt(L, 4);
    int b = DnhValue::toInt(L, 5);
    engine->saveSnapShotA2(path, l, t, r, b, getSourcePos(L));
    return 0;
}

static int IsPixelShaderSupported(lua_State* L)
{
    Engine* engine = getEngine(L);
    int major = DnhValue::toInt(L, 1);
    int minor = DnhValue::toInt(L, 2);
    lua_pushboolean(L, engine->isPixelShaderSupported(major, minor));
    return 1;
}

static int SetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double begin = DnhValue::toNum(L, 2);
    double end = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        engine->setShader((int)(begin * MAX_RENDER_PRIORITY), (int)(end * MAX_RENDER_PRIORITY), obj->getShader());
    }
    return 0;
}

static int SetShaderI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int begin = DnhValue::toInt(L, 2);
    int end = DnhValue::toInt(L, 3);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        engine->setShader(begin, end, obj->getShader());
    }
    return 0;
}

static int ResetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    double begin = DnhValue::toNum(L, 1);
    double end = DnhValue::toNum(L, 2);
    engine->resetShader((int)(begin * MAX_RENDER_PRIORITY), (int)(end * MAX_RENDER_PRIORITY));
    return 0;
}

static int ResetShaderI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int begin = DnhValue::toInt(L, 1);
    int end = DnhValue::toInt(L, 2);
    engine->resetShader(begin, end);
    return 0;
}

static int LoadTextureInLoadThread(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->loadTextureInThread(path, true, getSourcePos(L));
    return 0;
}

static int LoadTexture(lua_State* L)
{
    return LoadTextureInLoadThread(L);
}

static int RemoveTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->removeTextureReservedFlag(path);
    return 0;
}

static int GetTextureWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    if (auto target = engine->getRenderTarget(name))
    {
        lua_pushnumber(L, target->getWidth());
    } else
    {
        try
        {
            lua_pushnumber(L, engine->loadTexture(name, false, getSourcePos(L))->getWidth());
        } catch (Log& log)
        {
            log.setLevel(Log::Level::LV_WARN)
                .addSourcePos(getSourcePos(L));
            Logger::WriteLog(log);
            lua_pushnumber(L, 0);
        }
    }
    return 1;
}

static int GetTextureHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::toString(L, 1);
    if (auto target = engine->getRenderTarget(name))
    {
        lua_pushnumber(L, target->getHeight());
    } else
    {
        try
        {
            lua_pushnumber(L, engine->loadTexture(name, false, getSourcePos(L))->getHeight());
        } catch (Log& log)
        {
            log.setLevel(Log::Level::LV_WARN)
                .addSourcePos(getSourcePos(L));
            Logger::WriteLog(log);
            lua_pushnumber(L, 0);
        }
    }
    return 1;
}

static int SetFogEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool enable = DnhValue::toBool(L, 1);
    engine->setFogEnable(enable);
    return 0;
}

static int SetFogParam(lua_State* L)
{
    Engine* engine = getEngine(L);
    double fogStart = DnhValue::toNum(L, 1);
    double fogEnd = DnhValue::toNum(L, 2);
    int r = DnhValue::toInt(L, 3);
    int g = DnhValue::toInt(L, 4);
    int b = DnhValue::toInt(L, 5);
    engine->setFogParam(fogStart, fogEnd, r, g, b);
    return 0;
}

static int ClearInvalidRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->clearInvalidRenderPriority();
    return 0;
}

static int SetInvalidRenderPriorityA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int begin = DnhValue::toInt(L, 1);
    int end = DnhValue::toInt(L, 2);
    engine->setInvalidRenderPriority(begin, end);
    return 0;
}

static int GetReservedRenderTargetName(lua_State* L)
{
    Engine* engine = getEngine(L);
    int n = DnhValue::toInt(L, 1);
    DnhArray(engine->getReservedRenderTargetName(n)).push(L);
    return 1;
}

template <void (Engine::*func)(float)>
static int SetCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    double v = DnhValue::toNum(L, 1);
    (engine->*func)(v);
    return 0;
}

static int SetCameraFocusX(lua_State* L) { return SetCamera<&Engine::setCameraFocusX>(L); }
static int SetCameraFocusY(lua_State* L) { return SetCamera<&Engine::setCameraFocusY>(L); }
static int SetCameraFocusZ(lua_State* L) { return SetCamera<&Engine::setCameraFocusZ>(L); }

static int SetCameraFocusXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double z = DnhValue::toNum(L, 3);
    engine->setCameraFocusXYZ(x, y, z);
    return 0;
}

static int SetCameraRadius(lua_State* L) { return SetCamera<&Engine::setCameraRadius>(L); }
static int SetCameraAzimuthAngle(lua_State* L) { return SetCamera<&Engine::setCameraAzimuthAngle>(L); }
static int SetCameraElevationAngle(lua_State* L) { return SetCamera<&Engine::setCameraElevationAngle>(L); }
static int SetCameraYaw(lua_State* L) { return SetCamera<&Engine::setCameraYaw>(L); }
static int SetCameraPitch(lua_State* L) { return SetCamera<&Engine::setCameraPitch>(L); }
static int SetCameraRoll(lua_State* L) { return SetCamera<&Engine::setCameraRoll>(L); }

template <float (Engine::*func)() const>
static int GetCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, (engine->*func)());
    return 1;
}

static int GetCameraX(lua_State* L) { return GetCamera<&Engine::getCameraX>(L); }
static int GetCameraY(lua_State* L) { return GetCamera<&Engine::getCameraY>(L); }
static int GetCameraZ(lua_State* L) { return GetCamera<&Engine::getCameraZ>(L); }
static int GetCameraFocusX(lua_State* L) { return GetCamera<&Engine::getCameraFocusX>(L); }
static int GetCameraFocusY(lua_State* L) { return GetCamera<&Engine::getCameraFocusY>(L); }
static int GetCameraFocusZ(lua_State* L) { return GetCamera<&Engine::getCameraFocusZ>(L); }
static int GetCameraRadius(lua_State* L) { return GetCamera<&Engine::getCameraRadius>(L); }
static int GetCameraAzimuthAngle(lua_State* L) { return GetCamera<&Engine::getCameraAzimuthAngle>(L); }
static int GetCameraElevationAngle(lua_State* L) { return GetCamera<&Engine::getCameraElevationAngle>(L); }
static int GetCameraYaw(lua_State* L) { return GetCamera<&Engine::getCameraYaw>(L); }
static int GetCameraPitch(lua_State* L) { return GetCamera<&Engine::getCameraPitch>(L); }
static int GetCameraRoll(lua_State* L) { return GetCamera<&Engine::getCameraRoll>(L); }

static int SetCameraPerspectiveClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double n = DnhValue::toNum(L, 1);
    double f = DnhValue::toNum(L, 2);
    engine->setCameraPerspectiveClip(n, f);
    return 0;
}

template <void (Engine::*func)(float)>
static int Set2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    float v = DnhValue::toNum(L, 1);
    (engine->*func)(v);
    return 0;
}

static int Set2DCameraFocusX(lua_State* L) { return Set2DCamera<&Engine::set2DCameraFocusX>(L); }
static int Set2DCameraFocusY(lua_State* L) { return Set2DCamera<&Engine::set2DCameraFocusY>(L); }
static int Set2DCameraAngleZ(lua_State* L) { return Set2DCamera<&Engine::set2DCameraAngleZ>(L); }
static int Set2DCameraRatio(lua_State* L) { return Set2DCamera<&Engine::set2DCameraRatio>(L); }
static int Set2DCameraRatioX(lua_State* L) { return Set2DCamera<&Engine::set2DCameraRatioX>(L); }
static int Set2DCameraRatioY(lua_State* L) { return Set2DCamera<&Engine::set2DCameraRatioY>(L); }

static int Reset2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->reset2DCamera();
    return 0;
}

template <float (Engine::*func)() const>
static int Get2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, (engine->*func)());
    return 1;
}

static int Get2DCameraX(lua_State* L) { return Get2DCamera<&Engine::get2DCameraX>(L); }
static int Get2DCameraY(lua_State* L) { return Get2DCamera<&Engine::get2DCameraY>(L); }
static int Get2DCameraAngleZ(lua_State* L) { return Get2DCamera<&Engine::get2DCameraAngleZ>(L); }
static int Get2DCameraRatio(lua_State* L) { return Get2DCamera<&Engine::get2DCameraRatio>(L); }
static int Get2DCameraRatioX(lua_State* L) { return Get2DCamera<&Engine::get2DCameraRatioX>(L); }
static int Get2DCameraRatioY(lua_State* L) { return Get2DCamera<&Engine::get2DCameraRatioY>(L); }

static int LoadScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    if (std::shared_ptr<Script> script = engine->loadScript(path, getScript(L)->getType(), SCRIPT_VERSION_PH3, getSourcePos(L)))
    {
        lua_pushnumber(L, (double)script->getID());
    }
    return 1;
}

static int LoadScriptInThread(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    if (std::shared_ptr<Script> script = engine->loadScriptInThread(path, getScript(L)->getType(), SCRIPT_VERSION_PH3, getSourcePos(L)))
    {
        lua_pushnumber(L, (double)script->getID());
    }
    return 1;
}

static int StartScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    if (auto script = engine->getScript(scriptId))
    {
        script->start();
        script->runInitialize();
    }
    return 0;
}

static int CloseScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    if (auto script = engine->getScript(scriptId))
    {
        script->close();
    }
    return 0;
}

static int IsCloseScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    if (auto script = engine->getScript(scriptId))
    {
        lua_pushboolean(L, script->isClosed());
    } else
    {
        lua_pushboolean(L, true);
    }
    return 1;
}

static int SetScriptArgument(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    int idx = DnhValue::toInt(L, 2);
    auto value = DnhValue::get(L, 3);
    if (auto script = engine->getScript(scriptId))
    {
        script->setScriptArgument(idx, std::move(value));
    }
    return 0;
}

static int GetScriptArgument(lua_State* L)
{
    Script* script = getScript(L);
    int idx = DnhValue::toInt(L, 1);
    script->getScriptArgument(idx)->push(L);
    return 1;
}

static int GetScriptArgumentCount(lua_State* L)
{
    Script* script = getScript(L);
    lua_pushnumber(L, script->getScriptArgumentount());
    return 1;
}

static int CloseStgScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->closeStgScene();
    return 0;
}

static int SetStgFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int l = DnhValue::toInt(L, 1);
    int t = DnhValue::toInt(L, 2);
    int r = DnhValue::toInt(L, 3);
    int b = DnhValue::toInt(L, 4);
    int priorityMin = DnhValue::toInt(L, 5);
    int priorityMax = DnhValue::toInt(L, 6);
    engine->setStgFrame(l, t, r, b);
    engine->reset2DCamera();
    engine->set2DCameraFocusX(engine->getStgFrameCenterWorldX());
    engine->set2DCameraFocusY(engine->getStgFrameCenterWorldY());
    engine->setStgFrameRenderPriorityMin(priorityMin);
    engine->setStgFrameRenderPriorityMax(priorityMax);
    return 0;
}

static int GetScore(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getScore());
    return 1;
}

static int AddScore(lua_State* L)
{
    Engine* engine = getEngine(L);
    int64_t score = DnhValue::toNum(L, 1);
    engine->addScore(score);
    return 0;
}

static int GetGraze(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getGraze());
    return 1;
}

static int AddGraze(lua_State* L)
{
    Engine* engine = getEngine(L);
    int64_t graze = DnhValue::toNum(L, 1);
    engine->addGraze(graze);
    return 0;
}

static int GetPoint(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getPoint());
    return 1;
}

static int AddPoint(lua_State* L)
{
    Engine* engine = getEngine(L);
    int64_t point = DnhValue::toNum(L, 1);
    engine->addPoint(point);
    return 0;
}

static int SetItemRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    int p = DnhValue::toInt(L, 1);
    engine->setItemRenderPriority(p);
    return 0;
}

static int SetShotRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    int p = DnhValue::toInt(L, 1);
    engine->setShotRenderPriority(p);
    return 0;
}

static int GetStgFrameRenderPriorityMinI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameRenderPriorityMin());
    return 1;
}

static int GetStgFrameRenderPriorityMaxI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameRenderPriorityMax());
    return 1;
}

static int GetItemRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getItemRenderPriority());
    return 1;
}

static int GetShotRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getShotRenderPriority());
    return 1;
}

static int GetPlayerRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getPlayerRenderPriority());
    return 1;
}

static int GetCameraFocusPermitPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getCameraFocusPermitRenderPriority());
    return 1;
}

static int GetStgFrameLeft(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameLeft());
    return 1;
}

static int GetStgFrameTop(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameTop());
    return 1;
}

static int GetStgFrameWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameWidth());
    return 1;
}

static int GetStgFrameHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStgFrameHeight());
    return 1;
}

static int GetScreenWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getScreenWidth());
    return 1;
}

static int GetScreenHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getScreenHeight());
    return 1;
}

static int SCREEN_WIDTH(lua_State* L) { return GetScreenWidth(L); }
static int SCREEN_HEIGHT(lua_State* L) { return GetScreenHeight(L); }

// FUTURE : impl
static int IsReplay(lua_State* L)
{
    // FUTURE : impl
    Engine* engine = getEngine(L);
    lua_pushboolean(L, false);
    return 1;
}

static int AddArchiveFile(lua_State* L)
{
    // FUTURE : impl
    Engine* engine = getEngine(L);
    lua_pushboolean(L, false);
    return 1;
}

static int GetPlayerObjectID(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, player ? player->getID() : ID_INVALID);
    return 1;
}

static int GetPlayerScriptID(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto playerScript = engine->getPlayerScript())
    {
        lua_pushnumber(L, playerScript->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int SetPlayerSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    double normalSpeed = DnhValue::toNum(L, 1);
    double slowSpeed = DnhValue::toNum(L, 2);
    if (auto player = engine->getPlayerObject())
    {
        player->setNormalSpeed(normalSpeed);
        player->setSlowSpeed(slowSpeed);
    }
    return 0;
}

static int SetPlayerClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double left = DnhValue::toNum(L, 1);
    double top = DnhValue::toNum(L, 2);
    double right = DnhValue::toNum(L, 3);
    double bottom = DnhValue::toNum(L, 4);
    if (auto player = engine->getPlayerObject())
    {
        player->setClip(left, top, right, bottom);
    }
    return 0;
}


static int SetPlayerLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    double life = DnhValue::toNum(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setLife(life);
    }
    return 0;
}

static int SetPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    double spell = DnhValue::toNum(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setSpell(spell);
    }
    return 0;
}

static int SetPlayerPower(lua_State* L)
{
    Engine* engine = getEngine(L);
    double power = DnhValue::toNum(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setPower(power);
    }
    return 0;
}

static int SetPlayerInvincibilityFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::toInt(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setInvincibilityFrame(frame);
    }
    return 0;
}

static int SetPlayerDownStateFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::toInt(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setDownStateFrame(frame);
    }
    return 0;
}

static int SetPlayerRebirthFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::toInt(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setRebirthFrame(frame);
    }
    return 0;
}

static int SetPlayerRebirthLossFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::toInt(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setRebirthLossFrame(frame);
    }
    return 0;
}

static int SetPlayerAutoItemCollectLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    double lineY = DnhValue::toNum(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setAutoItemCollectLineY(lineY);
    }
    return 0;
}

static int SetForbidPlayerShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool forbid = DnhValue::toBool(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setForbidPlayerShot(forbid);
    }
    return 0;
}

static int SetForbidPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool forbid = DnhValue::toBool(L, 1);
    if (auto player = engine->getPlayerObject())
    {
        player->setForbidPlayerSpell(forbid);
    }
    return 0;
}

static int GetPlayerState(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->getPlayerObject())
    {
        lua_pushnumber(L, (double)player->getState());
    } else
    {
        lua_pushnumber(L, (double)STATE_END);
    }
    return 1;
}

static int GetPlayerSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->getPlayerObject())
    {
        Point2D speeds{ player->getNormalSpeed(), player->getSlowSpeed() };
        DnhArray(speeds).push(L);
    } else
    {
        DnhArray(Point2D(0.0f, 0.0f)).push(L);
    }
    return 1;
}

static int GetPlayerClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    std::vector<double> clipRect;
    clipRect.push_back(!player ? 0 : player->getClipLeft());
    clipRect.push_back(!player ? 0 : player->getClipTop());
    clipRect.push_back(!player ? 0 : player->getClipRight());
    clipRect.push_back(!player ? 0 : player->getClipBottom());
    DnhArray(clipRect).push(L);
    return 1;
}

static int GetPlayerLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getLife());
    return 1;
}

static int GetPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getSpell());
    return 1;
}

static int GetPlayerPower(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getPower());
    return 1;
}

static int GetPlayerInvincibilityFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getInvincibilityFrame());
    return 1;
}

static int GetPlayerDownStateFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, player ? player->getDownStateFrame() : 0);
    return 1;
}

static int GetPlayerRebirthFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, player ? player->getRebirthFrame() : 0);
    return 1;
}

static int IsPermitPlayerShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushboolean(L, player && player->isPermitPlayerShot());
    return 1;
}

static int IsPermitPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushboolean(L, player && player->isPermitPlayerSpell());
    return 1;
}

static int IsPlayerLastSpellWait(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushboolean(L, player && player->isLastSpellWait());
    return 1;
}

static int IsPlayerSpellActive(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushboolean(L, player && player->isSpellActive());
    return 1;
}

static int GetPlayerX(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getX());
    return 1;
}
static int GetPlayerY(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->getPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->getY());
    return 1;
}

static int GetAngleToPlayer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        auto player = engine->getPlayerObject();
        double angle = !player ? 0 : D3DXToDegree(atan2(player->getMoveY() - obj->getY(), player->getMoveX() - obj->getX()));
        lua_pushnumber(L, angle);
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int GetPlayerID(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getPlayerID()).push(L);
    return 1;
}

static int GetPlayerReplayName(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getPlayerReplayName()).push(L);
    return 1;
}

inline static float qd(const Point2D &p, const Point2D &q)
{
    float dx = p.x - q.x;
    float dy = p.y - q.y;
    return dx * dx + dy * dy;
}

static void sortByDistanceFromPoint(std::vector<Point2D>& ps, const Point2D &from)
{
    std::sort(ps.begin(), ps.end(), [&from](const Point2D& p, const Point2D& q)
    {
        return qd(p, from) < qd(q, from);
    });
}

static void getEnemyIntersectionPositionFromPoint(std::vector<Point2D>& ps, const std::vector<std::shared_ptr<ObjEnemy>>& enemies, const Point2D& from)
{
    ps.clear();
    for (auto& enemy : enemies)
    {
        const std::vector<Point2D>& tmp = enemy->getAllIntersectionToShotPosition();
        // psの後ろに結合
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(ps));
    }
    sortByDistanceFromPoint(ps, from);
}

static int GetEnemyIntersectionPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    int n = DnhValue::toInt(L, 3);
    auto enemies = engine->getObjectAll<ObjEnemy>();
    std::vector<Point2D> ps;
    getEnemyIntersectionPositionFromPoint(ps, enemies, Point2D((float)x, (float)y));
    if (ps.size() > n) { ps.resize(n); }
    DnhArray(ps).push(L);
    return 1;
}

static int GetEnemyBossSceneObjectID(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto obj = engine->getEnemyBossSceneObject();
    lua_pushnumber(L, obj ? obj->getID() : ID_INVALID);
    return 1;
}

static int GetEnemyBossObjectID(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto boss = engine->getEnemyBossObject())
    {
        DnhArray(std::vector<double>{(double)boss->getID()}).push(L);
    } else
    {
        DnhArray().push(L);
    }
    return 1;
}

static int GetAllEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray ids;
    for (const auto& enemy : engine->getObjectAll<ObjEnemy>())
    {
        ids.pushBack(std::make_unique<DnhReal>((double)enemy->getID()));
    }
    ids.push(L);
    return 1;
}

static int GetIntersectionRegistedEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray ids;
    for (const auto& enemy : engine->getObjectAll<ObjEnemy>())
    {
        if (!enemy->getAllIntersectionToShotPosition().empty())
        {
            ids.pushBack(std::make_unique<DnhReal>((double)enemy->getID()));
        }
    }
    ids.push(L);
    return 1;
}

static int GetAllEnemyIntersectionPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray poss;
    for (const auto& enemy : engine->getObjectAll<ObjEnemy>())
    {
        for (const auto& pos : enemy->getAllIntersectionToShotPosition())
        {
            poss.pushBack(std::make_unique<DnhArray>(pos));
        }
    }
    poss.push(L);
    return 1;
}

static int GetEnemyIntersectionPositionByIdA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::vector<Point2D> ps;
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        getEnemyIntersectionPositionFromPoint(ps, { obj }, Point2D(obj->getX(), obj->getY()));
    }
    DnhArray(ps).push(L);
    return 1;
}

static int GetEnemyIntersectionPositionByIdA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float x = DnhValue::toNum(L, 2);
    float y = DnhValue::toNum(L, 3);
    std::vector<Point2D> ps;
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        getEnemyIntersectionPositionFromPoint(ps, { obj }, Point2D(x, y));
    }
    DnhArray(ps).push(L);
    return 1;
}

static int LoadEnemyShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->loadEnemyShotData(path, getSourcePos(L));
    return 0;
}

static int ReloadEnemyShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->reloadEnemyShotData(path, getSourcePos(L));
    return 0;
}

static int atoi(lua_State* L)
{
    auto str = DnhValue::toString(L, 1);
    lua_pushnumber(L, _wtoi64(str.c_str()));
    return 1;
}

static int ator(lua_State* L)
{
    double r = DnhValue::toNum(L, 1);
    lua_pushnumber(L, r);
    return 1;
}

static int TrimString(lua_State* L)
{
    auto str = DnhValue::toString(L, 1);
    trimSpace(str);
    DnhArray(str).push(L);
    return 1;
}

static int rtos(lua_State* L)
{
    std::wstring format = DnhValue::toString(L, 1);
    double num = DnhValue::toNum(L, 2);
    std::vector<std::wstring> ss = split(format, L'.');
    int zeroCnt1 = 0;
    int zeroCnt2 = 0;
    for (auto x : ss[0])
    {
        if (x == L'0') zeroCnt1++;
    }
    if (ss.size() > 1)
    {
        for (auto x : ss[1])
        {
            if (x == L'0') zeroCnt2++;
        }
    }
    std::wstring printfFormat(L"%0" + std::to_wstring(zeroCnt1) + L"." + std::to_wstring(zeroCnt2) + L"f");
    std::wstring buf(zeroCnt1 + zeroCnt2 + 32, L'\0'); // それなりに大きく
    swprintf_s(&buf[0], buf.size(), &printfFormat[0], num);
    // c_strして別のwstringを作ることで無駄なヌル文字を消去
    DnhArray(std::wstring(buf.c_str())).push(L);
    return 1;
}

static int vtos(lua_State* L)
{
    auto format = DnhValue::toString(L, 1);
    auto value = DnhValue::get(L, 2);
    std::remove_if(format.begin(), format.end(), [](const wchar_t& c)
    {
        switch (c)
        {
            case L'd': case L'f': case L's': case L'-': case L'.':
            case L'0': case L'1': case L'2': case L'3': case L'4':
            case L'5': case L'6': case L'7': case L'8': case L'9':
                return false;
            default:
                return true;
        }
    });
    format = L"%" + format;
    std::wstring buf(1024, L'\0');
    if (format.find(L'd') != std::string::npos)
    {
        swprintf_s(&buf[0], buf.size(), &format[0], value->toInt());
    } else if (format.find(L'f') != std::string::npos)
    {
        swprintf_s(&buf[0], buf.size(), &format[0], value->toNum());
    } else if (format.find(L's') != std::string::npos)
    {
        swprintf_s(&buf[0], buf.size(), &format[0], value->toString().c_str());
    } else
    {
        buf = L"error format";
    }
    DnhArray(std::wstring(buf.c_str())).push(L);
    return 1;
}

static int SplitString(lua_State* L)
{
    auto str = DnhValue::toString(L, 1);
    auto delim = DnhValue::toString(L, 2);
    DnhArray arr;
    for (auto& s : split(str, delim))
    {
        arr.pushBack(std::make_unique<DnhArray>(s));
    }
    arr.push(L);
    return 1;
}

static int GetFileDirectory(lua_State* L)
{
    auto path = DnhValue::toString(L, 1);
    DnhArray(parentPath(path) + L"/").push(L);
    return 1;
}

static int GetFilePathList(lua_State* L)
{
    auto dirPath = DnhValue::toString(L, 1);
    if (dirPath.empty())
    {
        DnhArray(L"").push(L);
        return 1;
    }

    if (dirPath.back() != L'/' && dirPath.back() != L'\\')
    {
        dirPath = parentPath(dirPath);
    }

    std::vector<std::wstring> pathList;
    getFilePaths(dirPath, pathList, {}, false);

    DnhArray ret;
    for (const auto& path : pathList)
    {
        ret.pushBack(std::make_unique<DnhArray>(path));
    }
    ret.push(L);
    return 1;
}

static int GetDirectoryList(lua_State* L)
{
    auto dirPath = DnhValue::toString(L, 1);
    if (dirPath.empty())
    {
        DnhArray(L"").push(L);
        return 1;
    }

    if (dirPath.back() != L'/' && dirPath.back() != L'\\')
    {
        dirPath = parentPath(dirPath);
    }

    std::vector<std::wstring> dirList;
    getDirs(dirPath, dirList, false);

    DnhArray ret;
    for (const auto& dir : dirList)
    {
        ret.pushBack(std::make_unique<DnhArray>(concatPath(dir, L"")));
    }
    ret.push(L);
    return 1;
}

static int DeleteShotAll(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::toInt(L, 1);
    int behavior = DnhValue::toInt(L, 2);
    engine->deleteShotAll(target, behavior);
    return 0;
}

static int DeleteShotInCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::toInt(L, 1);
    int behavior = DnhValue::toInt(L, 2);
    float x = DnhValue::toNum(L, 3);
    float y = DnhValue::toNum(L, 4);
    float r = DnhValue::toNum(L, 5);
    engine->deleteShotInCircle(target, behavior, x, y, r);
    return 0;
}

static int CreateShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speed = DnhValue::toNum(L, 3);
    double angle = DnhValue::toNum(L, 4);
    int graphic = DnhValue::toInt(L, 5);
    int delay = DnhValue::toInt(L, 6);
    if (auto shot = engine->createShotA1(x, y, speed, angle, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speed = DnhValue::toNum(L, 3);
    double angle = DnhValue::toNum(L, 4);
    double accel = DnhValue::toNum(L, 5);
    double maxSpeed = DnhValue::toNum(L, 6);
    int graphic = DnhValue::toInt(L, 7);
    int delay = DnhValue::toInt(L, 8);
    if (auto shot = engine->createShotA2(x, y, speed, angle, accel, maxSpeed, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotOA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int objId = DnhValue::toInt(L, 1);
    double speed = DnhValue::toNum(L, 2);
    double angle = DnhValue::toNum(L, 3);
    int graphic = DnhValue::toInt(L, 4);
    int delay = DnhValue::toInt(L, 5);
    if (auto shot = engine->createShotOA1(objId, speed, angle, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speedX = DnhValue::toNum(L, 3);
    double speedY = DnhValue::toNum(L, 4);
    int graphic = DnhValue::toInt(L, 5);
    int delay = DnhValue::toInt(L, 6);
    if (auto shot = engine->createShotB1(x, y, speedX, speedY, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotB2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speedX = DnhValue::toNum(L, 3);
    double speedY = DnhValue::toNum(L, 4);
    double accelX = DnhValue::toNum(L, 5);
    double accelY = DnhValue::toNum(L, 6);
    double maxSpeedX = DnhValue::toNum(L, 7);
    double maxSpeedY = DnhValue::toNum(L, 8);
    int graphic = DnhValue::toInt(L, 9);
    int delay = DnhValue::toInt(L, 10);
    if (auto shot = engine->createShotB2(x, y, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotOB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int objId = DnhValue::toInt(L, 1);
    double speedX = DnhValue::toNum(L, 2);
    double speedY = DnhValue::toNum(L, 3);
    int graphic = DnhValue::toInt(L, 4);
    int delay = DnhValue::toInt(L, 5);
    if (auto shot = engine->createShotOB1(objId, speedX, speedY, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(shot->getID());
        lua_pushnumber(L, shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateLooseLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speed = DnhValue::toNum(L, 3);
    double angle = DnhValue::toNum(L, 4);
    double laserLength = DnhValue::toNum(L, 5);
    double laserWidth = DnhValue::toNum(L, 6);
    int graphic = DnhValue::toInt(L, 7);
    int delay = DnhValue::toInt(L, 8);
    if (auto laser = engine->createLooseLaserA1(x, y, speed, angle, laserLength, laserWidth, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(laser->getID());
        lua_pushnumber(L, laser->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateStraightLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double angle = DnhValue::toNum(L, 3);
    double laserLength = DnhValue::toNum(L, 4);
    double laserWidth = DnhValue::toNum(L, 5);
    int deleteFrame = DnhValue::toInt(L, 6);
    int graphic = DnhValue::toInt(L, 7);
    int delay = DnhValue::toInt(L, 8);
    if (auto laser = engine->createStraightLaserA1(x, y, angle, laserLength, laserWidth, deleteFrame, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(laser->getID());
        lua_pushnumber(L, laser->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateCurveLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speed = DnhValue::toNum(L, 3);
    double angle = DnhValue::toNum(L, 4);
    double laserLength = DnhValue::toNum(L, 5);
    double laserWidth = DnhValue::toNum(L, 6);
    int graphic = DnhValue::toInt(L, 7);
    int delay = DnhValue::toInt(L, 8);
    if (auto laser = engine->createCurveLaserA1(x, y, speed, angle, laserLength, laserWidth, graphic, delay, script->getType() == SCRIPT_TYPE_PLAYER))
    {
        script->addAutoDeleteTargetObjectId(laser->getID());
        lua_pushnumber(L, laser->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int SetShotIntersectionCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double r = DnhValue::toNum(L, 3);
    engine->setShotIntersectoinCicle(x, y, r);
    return 0;
}

static int SetShotIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x1 = DnhValue::toNum(L, 1);
    double y1 = DnhValue::toNum(L, 2);
    double x2 = DnhValue::toNum(L, 3);
    double y2 = DnhValue::toNum(L, 4);
    double width = DnhValue::toNum(L, 5);
    engine->setShotIntersectoinLine(x1, y1, x2, y2, width);
    return 0;
}

static int GetShotIdInCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double r = DnhValue::toNum(L, 3);
    DnhArray ids;
    for (auto& shot : engine->getShotInCircle(x, y, r, script->getType() == SCRIPT_TYPE_PLAYER ? TARGET_ENEMY : TARGET_PLAYER))
    {
        if (shot)
        {
            ids.pushBack(std::make_unique<DnhReal>((double)shot->getID()));
        }
    }
    ids.push(L);
    return 1;
}

static int GetShotIdInCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double r = DnhValue::toNum(L, 3);
    int target = DnhValue::toInt(L, 4);
    DnhArray ids;
    for (auto& shot : engine->getShotInCircle(x, y, r, target))
    {
        if (shot)
        {
            ids.pushBack(std::make_unique<DnhReal>((double)shot->getID()));
        }
    }
    ids.push(L);
    return 1;
}

static int GetShotCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::toInt(L, 1);
    int cnt = 0;
    switch (target)
    {
        case TARGET_ALL:
            cnt = engine->getAllShotCount();
            break;
        case TARGET_ENEMY:
            cnt = engine->getEnemyShotCount();
            break;
        case TARGET_PLAYER:
            cnt = engine->getPlayerShotCount();
            break;
    }
    lua_pushnumber(L, cnt);
    return 1;
}

static int SetShotAutoDeleteClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double l = DnhValue::toNum(L, 1);
    double t = DnhValue::toNum(L, 2);
    double r = DnhValue::toNum(L, 3);
    double b = DnhValue::toNum(L, 4);
    engine->setShotAutoDeleteClip(l, t, r, b);
    return 0;
}

static int GetShotDataInfoA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int id = DnhValue::toInt(L, 1);
    bool isPlayerShot = DnhValue::toInt(L, 2) == TARGET_PLAYER;
    int infoType = DnhValue::toInt(L, 3);
    if (auto shotData = isPlayerShot ? engine->getPlayerShotData(id) : engine->getEnemyShotData(id))
    {
        switch (infoType)
        {
            case INFO_RECT:
                DnhArray(std::vector<double>{(double)shotData->rect.left, (double)shotData->rect.top, (double)shotData->rect.left, (double)shotData->rect.bottom}).push(L);
                break;
            case INFO_DELAY_COLOR:
                DnhArray(std::vector<double>{(double)shotData->delayColor.getR(), (double)shotData->delayColor.getG(), (double)shotData->delayColor.getB()}).push(L);
                break;
            case INFO_BLEND:
                lua_pushnumber(L, shotData->render);
                break;
            case INFO_COLLISION:
                if (shotData->collisions.empty())
                {
                    lua_pushnumber(L, 0);
                } else
                {
                    lua_pushnumber(L, shotData->collisions[0].r);
                }
                break;
            case INFO_COLLISION_LIST:
            {
                DnhArray colList;
                for (const auto& col : shotData->collisions)
                {
                    colList.pushBack(std::make_unique<DnhArray>(std::vector<double>{col.r, col.x, col.y}));
                }
                colList.push(L);
            }
            break;
            default:
                lua_pushnil(L);
                break;
        }
        return 1;
    }
    return 0;
}

static int StartShotScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->startShotScript(path, getSourcePos(L));
    return 0;
}

static int CreateItemA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int type = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    int64_t score = DnhValue::toNum(L, 4);
    if (auto item = engine->createItemA1(type, x, y, score))
    {
        script->addAutoDeleteTargetObjectId(item->getID());
        lua_pushnumber(L, item->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int type = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double destX = DnhValue::toNum(L, 4);
    double destY = DnhValue::toNum(L, 5);
    int64_t score = DnhValue::toNum(L, 6);
    if (auto item = engine->createItemA2(type, x, y, destX, destY, score))
    {
        script->addAutoDeleteTargetObjectId(item->getID());
        lua_pushnumber(L, item->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemU1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int itemDataId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    int64_t score = DnhValue::toNum(L, 4);
    if (auto item = engine->createItemU1(itemDataId, x, y, score))
    {
        script->addAutoDeleteTargetObjectId(item->getID());
        lua_pushnumber(L, item->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemU2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int itemDataId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double destX = DnhValue::toNum(L, 4);
    double destY = DnhValue::toNum(L, 5);
    int64_t score = DnhValue::toNum(L, 6);
    if (auto item = engine->createItemU2(itemDataId, x, y, destX, destY, score))
    {
        script->addAutoDeleteTargetObjectId(item->getID());
        lua_pushnumber(L, item->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CollectAllItems(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->collectAllItems();
    return 0;
}

static int CollectItemsByType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int type = DnhValue::toInt(L, 1);
    engine->collectItemsByType(type);
    return 0;
}

static int CollectItemsInCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double r = DnhValue::toNum(L, 3);
    engine->collectItemsInCircle(x, y, r);
    return 0;
}

static int CancelCollectItems(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->cancelCollectItems();
    return 0;
}

static int StartItemScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->startItemScript(path, getSourcePos(L));
    return 0;
}

static int SetDefaultBonusItemEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool enable = DnhValue::toBool(L, 1);
    engine->setDefaultBonusItemEnable(enable);
    return 0;
}

static int LoadItemData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->loadItemData(path, getSourcePos(L));
    return 0;
}

static int ReloadItemData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->reloadItemData(path, getSourcePos(L));
    return 0;
}

static int StartSlow(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int fps = DnhValue::toInt(L, 2);
    engine->startSlow(fps, script->getType() == SCRIPT_TYPE_PLAYER);
    return 0;
}

static int StopSlow(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    engine->stopSlow(script->getType() == SCRIPT_TYPE_PLAYER);
    return 0;
}

static int IsIntersected_Line_Circle(lua_State* L)
{
    double x1 = DnhValue::toNum(L, 1);
    double y1 = DnhValue::toNum(L, 2);
    double x2 = DnhValue::toNum(L, 3);
    double y2 = DnhValue::toNum(L, 4);
    double width = DnhValue::toNum(L, 5);
    double cx = DnhValue::toNum(L, 6);
    double cy = DnhValue::toNum(L, 7);
    double r = DnhValue::toNum(L, 8);
    lua_pushboolean(L, IsIntersectedLineCircle(x1, y1, x2, y2, width, cx, cy, r));
    return 1;
}

static int IsIntersected_Obj_Obj(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId1 = DnhValue::toInt(L, 1);
    int objId2 = DnhValue::toInt(L, 2);
    auto obj1 = engine->getObject<ObjCol>(objId1);
    auto obj2 = engine->getObject<ObjCol>(objId2);
    lua_pushboolean(L, obj1 && obj2 && obj1->isIntersected(obj2));
    return 1;
}

static int GetObjectDistance(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId1 = DnhValue::toInt(L, 1);
    int objId2 = DnhValue::toInt(L, 2);
    if (auto obj1 = engine->getObject<ObjRender>(objId1))
    {
        if (auto obj2 = engine->getObject<ObjRender>(objId2))
        {
            float dx = obj1->getX() - obj2->getX();
            float dy = obj1->getY() - obj2->getY();
            lua_pushnumber(L, sqrt(dx * dx + dy * dy));
            return 1;
        }
    }
    lua_pushnumber(L, -1);
    return 1;
}

static int GetObject2dPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        DnhArray(engine->get2DPosition(obj->getX(), obj->getY(), obj->getZ(), obj->isStgSceneObject())).push(L);
    } else
    {
        DnhArray(Point2D(0, 0)).push(L);
    }
    return 1;
}

static int Get2dPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    float x = DnhValue::toNum(L, 1);
    float y = DnhValue::toNum(L, 2);
    float z = DnhValue::toNum(L, 3);
    DnhArray(engine->get2DPosition(x, y, z, script->isStgSceneScript())).push(L);
    return 1;
}

static int GetOwnScriptID(lua_State* L)
{
    Script* script = getScript(L);
    lua_pushnumber(L, script->getID());
    return 1;
}

static int SetScriptResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    auto result = DnhValue::get(L, 1);
    engine->setScriptResult(script->getID(), std::move(result));
    return 0;
}

static int GetScriptResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    engine->getScriptResult(scriptId)->push(L);
    return 1;
}

static int SetAutoDeleteObject(lua_State* L)
{
    bool enable = DnhValue::toBool(L, 1);
    Script* script = getScript(L);
    script->setAutoDeleteObjectEnable(enable);
    return 0;
}

static int NotifyEvent(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::toInt(L, 1);
    int eventType = DnhValue::toInt(L, 2);
    auto arg = DnhValue::get(L, 3);
    if (auto script = engine->getScript(scriptId))
    {
        auto args = std::make_unique<DnhArray>();
        args->pushBack(std::move(arg));
        script->notifyEvent(eventType, args);
    }
    return 0;
}

static int NotifyEventAll(lua_State* L)
{
    Engine* engine = getEngine(L);
    int eventType = DnhValue::toInt(L, 1);
    auto arg = DnhValue::get(L, 2);
    auto args = std::make_unique<DnhArray>();
    args->pushBack(std::move(arg));
    engine->notifyEventAll(eventType, args);
    return 0;
}

static int GetScriptInfoA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    int infoType = DnhValue::toInt(L, 2);
    ScriptInfo info = engine->getScriptInfo(path, getSourcePos(L));
    switch (infoType)
    {
        case INFO_SCRIPT_TYPE:
            lua_pushnumber(L, getScriptTypeConstFromName(info.type));
            break;
        case INFO_SCRIPT_PATH:
            DnhArray(info.path).push(L);
            break;
        case INFO_SCRIPT_ID:
            DnhArray(info.id).push(L);
            break;
        case INFO_SCRIPT_TITLE:
            DnhArray(info.title).push(L);
            break;
        case INFO_SCRIPT_TEXT:
            DnhArray(info.text).push(L);
            break;
        case INFO_SCRIPT_IMAGE:
            DnhArray(info.imagePath).push(L);
            break;
        case INFO_SCRIPT_REPLAY_NAME:
            DnhArray(info.replayName).push(L);
            break;
        default:
            lua_pushnumber(L, -1);
            break;
    }
    return 1;
}

static int Obj_Delete(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    engine->deleteObject(objId);
    return 0;
}

static int Obj_IsDeleted(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    lua_pushboolean(L, engine->isObjectDeleted(objId));
    return 1;
}

static int Obj_SetVisible(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool b = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setVisible(b);
    }
    return 0;
}

static int Obj_IsVisible(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjRender>(objId);
    lua_pushboolean(L, obj && obj->isVisible());
    return 1;
}

static int Obj_SetRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double p = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        engine->setObjectRenderPriority(obj, (int)(p * MAX_RENDER_PRIORITY));
    }
    return 0;
}

static int Obj_SetRenderPriorityI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int p = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        engine->setObjectRenderPriority(obj, p);
    }
    return 0;
}

static int Obj_GetRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? 1.0 * obj->getRenderPriority() / MAX_RENDER_PRIORITY : 0);
    return 1;
}

static int Obj_GetRenderPriorityI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? obj->getRenderPriority() : 0);
    return 1;
}

static int Obj_GetValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->getValue(key)->push(L);
        return 1;
    }
    return 0;
}

static int Obj_GetValueD(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    auto defaultValue = DnhValue::get(L, 3);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->getValueD(key, std::move(defaultValue))->push(L);
    } else
    {
        defaultValue->push(L);
    }
    return 1;
}

static int Obj_SetValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    auto value = DnhValue::get(L, 3);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->setValue(key, std::move(value));
    }
    return 0;
}

static int Obj_DeleteValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->deleteValue(key);
    }
    return 0;
}

static int Obj_IsValueExists(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring key = DnhValue::toString(L, 2);
    auto obj = engine->getObject<Obj>(objId);
    lua_pushboolean(L, obj && obj->isValueExists(key));
    return 1;
}

static int Obj_GetType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        lua_pushnumber(L, obj->getType());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

template <void (ObjRender::*func)(float)>
static int ObjRender_Set(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float v = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        ((obj.get())->*func)(v);
    }
    return 0;
}

static int ObjRender_SetX(lua_State* L) { return ObjRender_Set<&ObjRender::setX>(L); }
static int ObjRender_SetY(lua_State* L) { return ObjRender_Set<&ObjRender::setY>(L); }
static int ObjRender_SetZ(lua_State* L) { return ObjRender_Set<&ObjRender::setZ>(L); }

static int ObjRender_SetPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double z = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setPosition(x, y, z); }
    return 0;
}

static int ObjRender_SetAngleX(lua_State* L) { return ObjRender_Set<&ObjRender::setAngleX>(L); }
static int ObjRender_SetAngleY(lua_State* L) { return ObjRender_Set<&ObjRender::setAngleY>(L); }
static int ObjRender_SetAngleZ(lua_State* L) { return ObjRender_Set<&ObjRender::setAngleZ>(L); }

static int ObjRender_SetAngleXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double rx = DnhValue::toNum(L, 2);
    double ry = DnhValue::toNum(L, 3);
    double rz = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setAngleXYZ(rx, ry, rz); }
    return 0;
}

static int ObjRender_SetScaleX(lua_State* L) { return ObjRender_Set<&ObjRender::setScaleX>(L); }
static int ObjRender_SetScaleY(lua_State* L) { return ObjRender_Set<&ObjRender::setScaleY>(L); }
static int ObjRender_SetScaleZ(lua_State* L) { return ObjRender_Set<&ObjRender::setScaleZ>(L); }

static int ObjRender_SetScaleXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double sx = DnhValue::toNum(L, 2);
    double sy = DnhValue::toNum(L, 3);
    double sz = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setScaleXYZ(sx, sy, sz); }
    return 0;
}

static int ObjRender_SetColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int r = DnhValue::toInt(L, 2);
    int g = DnhValue::toInt(L, 3);
    int b = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setColor(r, g, b);
        // ObjPrimでObjSpriteList2Dでなければ全ての頂点に設定
        if (auto prim = std::dynamic_pointer_cast<ObjPrim>(obj))
        {
            if (!std::dynamic_pointer_cast<ObjSpriteList2D>(prim))
            {
                int vertexCnt = prim->getVertexCount();
                for (int i = 0; i < vertexCnt; i++)
                {
                    prim->setVertexColor(i, r, g, b);
                }
            }
        }
    }
    return 0;
}

static int ObjRender_SetColorHSV(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int h = DnhValue::toInt(L, 2);
    int s = DnhValue::toInt(L, 3);
    int v = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setColorHSV(h, s, v);
        // ObjPrimでObjSpriteList2Dでなければ全ての頂点に設定
        if (auto prim = std::dynamic_pointer_cast<ObjPrim>(obj))
        {
            if (!std::dynamic_pointer_cast<ObjSpriteList2D>(prim))
            {
                int vertexCnt = prim->getVertexCount();
                const auto& color = prim->getColor();
                for (int i = 0; i < vertexCnt; i++)
                {
                    prim->setVertexColor(i, color.getR(), color.getG(), color.getB());
                }
            }
        }
    }
    return 0;
}

static int ObjRender_SetAlpha(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int a = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setAlpha(a); }
    if (auto obj = engine->getObject <ObjPrim>(objId))
    {
        int vertexCnt = obj->getVertexCount();
        const auto& color = obj->getColor();
        for (int i = 0; i < vertexCnt; i++)
        {
            obj->setVertexAlpha(i, a);
        }
    }
    return 0;
}

static int ObjRender_SetBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int blendType = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setBlendType(blendType); }
    return 0;
}

template <float (ObjRender::*func)() const>
static int ObjRender_Get(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? ((obj.get())->*func)() : 0);
    return 1;
}

static int ObjRender_GetX(lua_State* L) { return ObjRender_Get<&ObjRender::getX>(L); }
static int ObjRender_GetY(lua_State* L) { return ObjRender_Get<&ObjRender::getY>(L); }
static int ObjRender_GetZ(lua_State* L) { return ObjRender_Get<&ObjRender::getZ>(L); }
static int ObjRender_GetAngleX(lua_State* L) { return ObjRender_Get<&ObjRender::getAngleX>(L); }
static int ObjRender_GetAngleY(lua_State* L) { return ObjRender_Get<&ObjRender::getAngleY>(L); }
static int ObjRender_GetAngleZ(lua_State* L) { return ObjRender_Get<&ObjRender::getAngleZ>(L); }
static int ObjRender_GetScaleX(lua_State* L) { return ObjRender_Get<&ObjRender::getScaleX>(L); }
static int ObjRender_GetScaleY(lua_State* L) { return ObjRender_Get<&ObjRender::getScaleY>(L); }
static int ObjRender_GetScaleZ(lua_State* L) { return ObjRender_Get<&ObjRender::getScaleZ>(L); }

static int ObjRender_GetBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? obj->getBlendType() : BLEND_NONE);
    return 1;
}

static int ObjRender_SetZWrite(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setZWrite(enable); }
    return 0;
}

static int ObjRender_SetZTest(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setZTest(enable); }
    return 0;
}

static int ObjRender_SetFogEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setFogEnable(enable); }
    return 0;
}

static int ObjRender_SetPermitCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId)) { obj->setPermitCamera(enable); }
    return 0;
}

static int ObjRender_SetCullingMode(lua_State* L)
{
    return 0;
}

static int ObjPrim_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int type = DnhValue::toInt(L, 1);
    int objId = ID_INVALID;
    switch (type)
    {
        case OBJ_PRIMITIVE_2D:
            objId = engine->createObjPrim2D()->getID();
            break;
        case OBJ_SPRITE_2D:
            objId = engine->createObjSprite2D()->getID();
            break;
        case OBJ_SPRITE_LIST_2D:
            objId = engine->createObjSpriteList2D()->getID();
            break;
        case OBJ_PRIMITIVE_3D:
            objId = engine->createObjPrim3D()->getID();
            break;
        case OBJ_SPRITE_3D:
            objId = engine->createObjSprite3D()->getID();
            break;
    }
    script->addAutoDeleteTargetObjectId(objId);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->setStgSceneObject(script->isStgSceneScript());
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjPrim_SetPrimitiveType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int type = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjPrim>(objId))
    {
        obj->setPrimitiveType(type);
    }
    return 0;
}

static int ObjPrim_SetVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int cnt = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjPrim>(objId))
    {
        obj->setVertexCount(cnt);
    }
    return 0;
}

static int ObjPrim_GetVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjPrim>(objId);
    lua_pushnumber(L, obj ? obj->getVertexCount() : 0);
    return 1;
}

static int ObjPrim_SetTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring name = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjPrim>(objId))
    {
        if (auto renderTarget = engine->getRenderTarget(name))
        {
            obj->setRenderTarget(renderTarget);
        } else
        {
            obj->setTexture(engine->loadTexture(name, false, getSourcePos(L)));
        }
    }
    return 0;
}

static int ObjPrim_SetVertexPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    double x = DnhValue::toNum(L, 3);
    double y = DnhValue::toNum(L, 4);
    double z = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjPrim>(objId))
    {
        obj->setVertexPosition(vIdx, x, y, z);
    }
    return 0;
}

static int ObjPrim_GetVertexPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    auto obj = engine->getObject<ObjPrim>(objId);
    float x = obj ? obj->getVertexPositionX(vIdx) : 0;
    float y = obj ? obj->getVertexPositionY(vIdx) : 0;
    float z = obj ? obj->getVertexPositionZ(vIdx) : 0;
    DnhArray ret;
    ret.pushBack(std::make_unique<DnhReal>(x));
    ret.pushBack(std::make_unique<DnhReal>(y));
    ret.pushBack(std::make_unique<DnhReal>(z));
    ret.push(L);
    return 1;
}

static int ObjPrim_SetVertexUV(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    double u = DnhValue::toNum(L, 3);
    double v = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjPrim>(objId)) { obj->setVertexUV(vIdx, u, v); }
    return 0;
}

static int ObjPrim_SetVertexUVT(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    double u = DnhValue::toNum(L, 3);
    double v = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjPrim>(objId)) { obj->setVertexUVT(vIdx, u, v); }
    return 0;
}

static int ObjPrim_SetVertexColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    int r = DnhValue::toInt(L, 3);
    int g = DnhValue::toInt(L, 4);
    int b = DnhValue::toInt(L, 5);
    if (auto obj = engine->getObject<ObjPrim>(objId)) { obj->setVertexColor(vIdx, r, g, b); }
    return 0;
}

static int ObjPrim_SetVertexAlpha(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int vIdx = DnhValue::toInt(L, 2);
    int a = DnhValue::toInt(L, 3);
    if (auto obj = engine->getObject<ObjPrim>(objId)) { obj->setVertexAlpha(vIdx, a); }
    return 0;
}

static int ObjSprite2D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSprite2D>(objId))
    {
        obj->setSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite2D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSprite2D>(objId))
    {
        obj->setDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite2D_SetDestCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSprite2D>(objId))
    {
        obj->setDestCenter();
    }
    return 0;
}

static int ObjSpriteList2D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->setSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSpriteList2D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->setDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSpriteList2D_SetDestCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->setDestCenter();
    }
    return 0;
}

static int ObjSpriteList2D_AddVertex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->addVertex();
    }
    return 0;
}

static int ObjSpriteList2D_CloseVertex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->closeVertex();
    }
    return 0;
}

static int ObjSpriteList2D_ClearVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSpriteList2D>(objId))
    {
        obj->clearVerexCount();
    }
    return 0;
}

static int ObjSprite3D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSprite3D>(objId))
    {
        obj->setSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSprite3D>(objId))
    {
        obj->setDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetSourceDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double l = DnhValue::toNum(L, 2);
    double t = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double b = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjSprite3D>(objId))
    {
        obj->setSourceDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetBillboard(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjSprite3D>(objId))
    {
        obj->setBillboard(enable);
    }
    return 0;
}

static int ObjTrajectory3D_SetComplementCount(lua_State* L)
{
    return 0;
}

static int ObjTrajectory3D_SetAlphaVariation(lua_State* L)
{
    return 0;
}

static int ObjTrajectory3D_SetInitialPoint(lua_State* L)
{
    return 0;
}

static int ObjMesh_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjMesh())
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        obj->setStgSceneObject(script->isStgSceneScript());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjMesh_Load(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto path = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjMesh>(objId))
    {
        if (auto mesh = engine->loadMesh(path, getSourcePos(L)))
        {
            obj->setMesh(mesh);
        }
    }
    return 0;
}

static int ObjMesh_SetColor(lua_State* L)
{
    return ObjRender_SetColor(L);
}

static int ObjMesh_SetAlpha(lua_State* L)
{
    return ObjRender_SetAlpha(L);
}

static int ObjMesh_SetAnimation(lua_State* L)
{
    // deprecated
    return 0;
}

static int ObjMesh_SetCoordinate2D(lua_State* L)
{
    // FUTURE : impl
    return 0;
}

static int ObjMesh_GetPath(lua_State* L)
{
    // deprecated
    lua_pushnumber(L, 0);
    return 1;
}

static int ObjText_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjText())
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        obj->setStgSceneObject(script->isStgSceneScript());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjText_SetText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring text = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setText(text);
    }
    return 0;
}

static int ObjText_SetFontType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring name = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontName(name);
    }
    return 0;
}

static int ObjText_SetFontSize(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int size = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontSize(size);
    }
    return 0;
}

static int ObjText_SetFontBold(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool bold = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontBold(bold);
    }
    return 0;
}

static int ObjText_SetFontColorTop(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int r = DnhValue::toInt(L, 2);
    int g = DnhValue::toInt(L, 3);
    int b = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontColorTop(r, g, b);
    }
    return 0;
}

static int ObjText_SetFontColorBottom(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int r = DnhValue::toInt(L, 2);
    int g = DnhValue::toInt(L, 3);
    int b = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontColorBottom(r, g, b);
    }
    return 0;
}

static int ObjText_SetFontBorderWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int width = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontBorderWidth(width);
    }
    return 0;
}

static int ObjText_SetFontBorderType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int t = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontBorderType(t);
    }
    return 0;
}

static int ObjText_SetFontBorderColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int r = DnhValue::toInt(L, 2);
    int g = DnhValue::toInt(L, 3);
    int b = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setFontBorderColor(r, g, b);
    }
    return 0;
}

static int ObjText_SetMaxWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int width = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setMaxWidth(width);
    }
    return 0;
}

static int ObjText_SetMaxHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int height = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setMaxHeight(height);
    }
    return 0;
}

static int ObjText_SetLinePitch(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int pitch = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setLinePitch(pitch);
    }
    return 0;
}

static int ObjText_SetSidePitch(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int pitch = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setSidePitch(pitch);
    }
    return 0;
}

static int ObjText_SetTransCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setTransCenter(x, y);
    }
    return 0;
}

static int ObjText_SetAutoTransCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setAutoTransCenter(enable);
    }
    return 0;
}

static int ObjText_SetHorizontalAlignment(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int alignment = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setHorizontalAlignment(alignment);
    }
    return 0;
}

static int ObjText_SetSyntacticAnalysis(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->setSyntacticAnalysis(enable);
    }
    return 0;
}

static int ObjText_GetTextLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjText>(objId);
    lua_pushnumber(L, obj ? obj->getTextLength() : 0);
    return 1;
}

static int ObjText_GetTextLengthCU(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjText>(objId);
    lua_pushnumber(L, obj ? obj->getTextLengthCU() : 0);
    return 1;
}

static int ObjText_GetTextLengthCUL(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->generateFonts();
        auto cnts = obj->getTextLengthCUL();
        DnhArray ret;
        for (auto cnt : cnts)
        {
            ret.pushBack(std::make_unique<DnhReal>((double)cnt));
        }
        ret.push(L);
        return 1;
    } else
    {
        return 0;
    }
}

static int ObjText_GetTotalWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->generateFonts();
        lua_pushnumber(L, obj->getTotalWidth());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjText_GetTotalHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjText>(objId))
    {
        obj->generateFonts();
        lua_pushnumber(L, obj->getTotalHeight());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjShader_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjShader())
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        obj->setStgSceneObject(script->isStgSceneScript());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjShader_SetShaderF(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring path = DnhValue::toString(L, 2);

    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        auto shader = engine->createShader(path, false);
        if (shader)
        {
            obj->setShader(shader);
            lua_pushboolean(L, true);
        } else
        {
            lua_pushboolean(L, false);
        }
    } else
    {
        lua_pushboolean(L, false);
    }
    return 1;
}

static int ObjShader_SetShaderO(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int shaderObjId = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        auto shaderObj = engine->getObject<ObjRender>(shaderObjId);
        obj->setShaderO(shaderObj);
    }
    return 0;
}

static int ObjShader_ResetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->resetShader();
    }
    return 0;
}

static int ObjShader_SetTechnique(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::string technique = DnhValue::toStringU8(L, 2);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setShaderTechnique(technique);
    }
    return 0;
}

static int ObjShader_SetVector(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::string name = DnhValue::toStringU8(L, 2);
    double x = DnhValue::toNum(L, 3);
    double y = DnhValue::toNum(L, 4);
    double z = DnhValue::toNum(L, 5);
    double w = DnhValue::toNum(L, 6);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setShaderVector(name, x, y, z, w);
    }
    return 0;
}

static int ObjShader_SetFloat(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::string name = DnhValue::toStringU8(L, 2);
    double f = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        obj->setShaderFloat(name, f);
    }
    return 0;
}

static int ObjShader_SetFloatArray(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::string name = DnhValue::toStringU8(L, 2);
    auto value = DnhValue::get(L, 3);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        if (DnhArray* floatArray = dynamic_cast<DnhArray*>(value.get()))
        {
            size_t size = floatArray->getSize();
            std::vector<float> fs(size);
            for (int i = 0; i < size; i++)
            {
                fs[i] = (float)floatArray->index(i)->toNum();
            }
            obj->setShaderFloatArray(name, fs);
        }
    }
    return 0;
}

static int ObjShader_SetTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::string name = DnhValue::toStringU8(L, 2);
    std::wstring path = DnhValue::toString(L, 3);
    if (auto obj = engine->getObject<ObjRender>(objId))
    {
        if (auto renderTarget = engine->getRenderTarget(toUnicode(name)))
        {
            obj->setShaderTexture(name, renderTarget);
        } else
        {
            obj->setShaderTexture(name, engine->loadTexture(path, false, getSourcePos(L)));
        }
    }
    return 0;
}

static int ObjSound_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjSound())
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        obj->setStgSceneObject(script->isStgSceneScript());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjSound_Load(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto path = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setSound(nullptr);
        obj->setSound(engine->loadSound(path, getSourcePos(L)));
    }
    return 0;
}

static int ObjSound_Play(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->play();
    }
    return 0;
}

static int ObjSound_Stop(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->stop();
    }
    return 0;
}

static int ObjSound_SetVolumeRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float vol = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setVolumeRate(vol);
    }
    return 0;
}

static int ObjSound_SetPanRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float pan = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setPanRate(pan);
    }
    return 0;
}

static int ObjSound_SetFade(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float fade = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setFade(fade);
    }
    return 0;
}

static int ObjSound_SetLoopEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setLoopEnable(enable);
    }
    return 0;
}

static int ObjSound_SetLoopTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double start = (DWORD)DnhValue::toNum(L, 2);
    double end = (DWORD)DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setLoopTime(start, end);
    }
    return 0;
}

static int ObjSound_SetLoopSampleCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    DWORD start = (DWORD)DnhValue::toNum(L, 2);
    DWORD end = (DWORD)DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setLoopSampleCount(start, end);
    }
    return 0;
}

static int ObjSound_SetRestartEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setRestartEnable(enable);
    }
    return 0;
}

static int ObjSound_SetSoundDivision(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int division = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjSound>(objId))
    {
        obj->setSoundDivision(division == SOUND_SE ? ObjSound::SoundDivision::SE : ObjSound::SoundDivision::BGM);
    }
    return 0;
}

static int ObjSound_IsPlaying(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjSound>(objId);
    lua_pushboolean(L, obj && obj->isPlaying());
    return 1;
}

static int ObjSound_GetVolumeRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjSound>(objId);
    lua_pushnumber(L, obj ? obj->getVolumeRate() : 0);
    return 1;
}

static int ObjFile_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int type = DnhValue::toInt(L, 1);
    int objId = ID_INVALID;
    if (type == OBJ_FILE_TEXT)
    {
        objId = engine->createObjFileT()->getID();
    } else if (type == OBJ_FILE_BINARY)
    {
        objId = engine->createObjFileB()->getID();
    }
    script->addAutoDeleteTargetObjectId(objId);
    if (auto obj = engine->getObject<Obj>(objId))
    {
        obj->setStgSceneObject(script->isStgSceneScript());
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjFile_Open(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto path = DnhValue::toString(L, 2);
    auto obj = engine->getObject<ObjFile>(objId);
    lua_pushboolean(L, obj && obj->open(path));
    return 1;
}

static int ObjFile_OpenNW(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto path = DnhValue::toString(L, 2);
    auto obj = engine->getObject<ObjFile>(objId);
    lua_pushboolean(L, obj && obj->openNW(path));
    return 1;
}

static int ObjFile_Store(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFile>(objId))
    {
        obj->store();
    }
    return 0;
}

static int ObjFile_GetSize(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFile>(objId))
    {
        lua_pushnumber(L, obj->getSize());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjFileT_GetLineCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileT>(objId))
    {
        lua_pushnumber(L, obj->getLineCount());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjFileT_GetLineText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int lineNum = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjFileT>(objId))
    {
        DnhArray(obj->getLineText(lineNum)).push(L);
    } else
    {
        DnhArray(L"").push(L);
    }
    return 1;
}

static int ObjFileT_SplitLineText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int lineNum = DnhValue::toInt(L, 2);
    std::wstring delim = DnhValue::toString(L, 3);
    if (auto obj = engine->getObject<ObjFileT>(objId))
    {
        DnhArray ret;
        for (const auto& s : obj->splitLineText(lineNum, delim))
        {
            ret.pushBack(std::make_unique<DnhArray>(s));
        }
        ret.push(L);
    } else
    {
        DnhArray().push(L);
    }
    return 1;
}

static int ObjFileT_AddLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    std::wstring line = DnhValue::toString(L, 2);
    if (auto obj = engine->getObject<ObjFileT>(objId))
    {
        obj->addLine(line);
    }
    return 0;
}

static int ObjFileT_ClearLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileT>(objId))
    {
        obj->clearLine();
    }
    return 0;
}

static int ObjFileB_SetByteOrder(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int endian = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        if (endian == ENDIAN_LITTLE)
        {
            obj->setByteOrder(false);
        } else
        {
            obj->setByteOrder(true);
        }
    }
    return 0;
}
static int ObjFileB_SetCharacterCode(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int code = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        switch (code)
        {
            case CODE_ACP:
                obj->setCharacterCode(ObjFileB::Encoding::ACP);
                break;
            case CODE_UTF8:
                obj->setCharacterCode(ObjFileB::Encoding::UTF8);
                break;
            case CODE_UTF16LE:
                obj->setCharacterCode(ObjFileB::Encoding::UTF16LE);
                break;
            case CODE_UTF16BE:
                obj->setCharacterCode(ObjFileB::Encoding::UTF16BE);
                break;
        }
    }
    return 0;
}

static int ObjFileB_GetPointer(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->getPointer());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_Seek(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int pos = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        obj->seek(pos);
    }
    return 0;
}

static int ObjFileB_ReadBoolean(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjFileB>(objId);
    lua_pushboolean(L, obj && obj->readBoolean());
    return 1;
}

static int ObjFileB_ReadByte(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readByte());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadShort(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readShort());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadInteger(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readInteger());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadLong(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readLong());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadFloat(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readFloat());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadDouble(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->readDouble());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadString(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int size = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjFileB>(objId))
    {
        DnhArray(obj->readString(size)).push(L);
    } else
    {
        DnhArray(L"").push(L);
    }
    return 1;
}

template <void (ObjMove::*func)(float)>
static int ObjMove_Set(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    float v = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        ((obj.get())->*func)(v);
    }
    return 0;
}

static int ObjMove_SetX(lua_State* L) { return ObjMove_Set<&ObjMove::setMoveX>(L); }
static int ObjMove_SetY(lua_State* L) { return ObjMove_Set<&ObjMove::setMoveY>(L); }

static int ObjMove_SetPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->setMovePosition(x, y);
    }
    return 0;
}

static int ObjMove_SetSpeed(lua_State* L) { return ObjMove_Set<&ObjMove::setSpeed>(L); }
static int ObjMove_SetAngle(lua_State* L) { return ObjMove_Set<&ObjMove::setAngle>(L); }
static int ObjMove_SetAcceleration(lua_State* L) { return ObjMove_Set<&ObjMove::setAcceleration>(L); }
static int ObjMove_SetMaxSpeed(lua_State* L) { return ObjMove_Set<&ObjMove::setMaxSpeed>(L); }
static int ObjMove_SetAngularVelocity(lua_State* L) { return ObjMove_Set<&ObjMove::setAngularVelocity>(L); }

static int ObjMove_SetDestAtSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double speed = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->setDestAtSpeed(x, y, speed);
    }
    return 0;
}

static int ObjMove_SetDestAtFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    int frame = DnhValue::toInt(L, 4);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->setDestAtFrame(x, y, frame);
    }
    return 0;
}

static int ObjMove_SetDestAtWeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double w = DnhValue::toNum(L, 4);
    double maxSpeed = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->setDestAtWeight(x, y, w, maxSpeed);
    }
    return 0;
}

static int ObjMove_AddPatternA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speed = DnhValue::toNum(L, 3);
    float angle = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, 0.0f, 0.0f, 0.0f, std::shared_ptr<ObjMove>(), std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speed = DnhValue::toNum(L, 3);
    float angle = DnhValue::toNum(L, 4);
    float accel = DnhValue::toNum(L, 5);
    float angularVelocity = DnhValue::toNum(L, 6);
    float maxSpeed = DnhValue::toNum(L, 7);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, std::shared_ptr<ObjMove>(), std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternA3(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speed = DnhValue::toNum(L, 3);
    float angle = DnhValue::toNum(L, 4);
    float accel = DnhValue::toNum(L, 5);
    float angularVelocity = DnhValue::toNum(L, 6);
    float maxSpeed = DnhValue::toNum(L, 7);
    int shotDataId = DnhValue::toInt(L, 8);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        shotData = obj->isPlayerShot() ? engine->getPlayerShotData(shotDataId) : engine->getEnemyShotData(shotDataId);
    }
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, std::shared_ptr<ObjMove>(), shotData));
    }
    return 0;
}

static int ObjMove_AddPatternA4(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speed = DnhValue::toNum(L, 3);
    float angle = DnhValue::toNum(L, 4);
    float accel = DnhValue::toNum(L, 5);
    float angularVelocity = DnhValue::toNum(L, 6);
    float maxSpeed = DnhValue::toNum(L, 7);
    int baseObjId = DnhValue::toInt(L, 8);
    int shotDataId = DnhValue::toInt(L, 9);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        shotData = obj->isPlayerShot() ? engine->getPlayerShotData(shotDataId) : engine->getEnemyShotData(shotDataId);
    }
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, engine->getObject<ObjMove>(baseObjId), shotData));
    }
    return 0;
}

static int ObjMove_AddPatternB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speedX = DnhValue::toNum(L, 3);
    float speedY = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, 0.0f, 0.0f, 0.0f, 0.0f, std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternB2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speedX = DnhValue::toNum(L, 3);
    float speedY = DnhValue::toNum(L, 4);
    float accelX = DnhValue::toNum(L, 5);
    float accelY = DnhValue::toNum(L, 6);
    float maxSpeedX = DnhValue::toNum(L, 7);
    float maxSpeedY = DnhValue::toNum(L, 8);
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternB3(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    float speedX = DnhValue::toNum(L, 3);
    float speedY = DnhValue::toNum(L, 4);
    float accelX = DnhValue::toNum(L, 5);
    float accelY = DnhValue::toNum(L, 6);
    float maxSpeedX = DnhValue::toNum(L, 7);
    float maxSpeedY = DnhValue::toNum(L, 8);
    int shotDataId = DnhValue::toInt(L, 9);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        shotData = obj->isPlayerShot() ? engine->getPlayerShotData(shotDataId) : engine->getEnemyShotData(shotDataId);
    }
    if (auto obj = engine->getObject<ObjMove>(objId))
    {
        obj->addMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, shotData));
    }
    return 0;
}

template <float (ObjMove::*func)() const>
static int ObjMove_Get(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjMove>(objId);
    lua_pushnumber(L, obj ? ((obj.get())->*func)() : 0.0);
    return 1;
}

static int ObjMove_GetX(lua_State* L) { return ObjMove_Get<&ObjMove::getMoveX>(L); }
static int ObjMove_GetY(lua_State* L) { return ObjMove_Get<&ObjMove::getMoveY>(L); }
static int ObjMove_GetSpeed(lua_State* L) { return ObjMove_Get<&ObjMove::getSpeed>(L); }
static int ObjMove_GetAngle(lua_State* L) { return ObjMove_Get<&ObjMove::getAngle>(L); }

static int ObjEnemy_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int type = DnhValue::toInt(L, 1);
    int objId = ID_INVALID;
    if (type == OBJ_ENEMY)
    {
        if (auto enemy = engine->createObjEnemy())
        {
            objId = enemy->getID();
            script->addAutoDeleteTargetObjectId(objId);
        }
    } else if (type == OBJ_ENEMY_BOSS)
    {
        if (auto boss = engine->getEnemyBossObject())
        {
            objId = boss->getID();
        }
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjEnemy_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjEnemy>(objId)) { obj->regist(); }
    return 0;
}

static int ObjEnemy_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int info = DnhValue::toInt(L, 2);
    double ret = 0;
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        if (info == INFO_LIFE)
        {
            ret = obj->getLife();
        } else if (info == INFO_DAMAGE_RATE_SHOT)
        {
            ret = obj->getDamageRateShot();
        } else if (info == INFO_DAMAGE_RATE_SPELL)
        {
            ret = obj->getDamageRateSpell();
        } else if (info == INFO_SHOT_HIT_COUNT)
        {
            ret = obj->getPrevFrameShotHitCount();
        }
    }
    lua_pushnumber(L, ret);
    return 1;
}

static int ObjEnemy_SetLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double life = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        obj->setLife(life);
    }
    return 0;
}

static int ObjEnemy_AddLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double life = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        obj->addLife(life);
    }
    return 0;
}

static int ObjEnemy_SetDamageRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double damageRateShot = DnhValue::toNum(L, 2);
    double damageRateSpell = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        obj->setDamageRateShot(damageRateShot);
        obj->setDamageRateSpell(damageRateSpell);
    }
    return 0;
}

static int ObjEnemy_SetIntersectionCircleToShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        obj->addTempIntersectionCircleToShot(x, y, r);
    }
    return 0;
}

static int ObjEnemy_SetIntersectionCircleToPlayer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjEnemy>(objId))
    {
        obj->addTempIntersectionCircleToPlayer(x, y, r);
    }
    return 0;
}

static int ObjEnemyBossScene_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjEnemyBossScene(getSourcePos(L)))
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjEnemyBossScene_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjEnemyBossScene>(objId))
    {
        obj->regist(getSourcePos(L));
    }
    return 0;
}

static int ObjEnemyBossScene_Add(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int step = DnhValue::toInt(L, 2);
    auto scriptPath = DnhValue::toString(L, 3);
    if (auto obj = engine->getObject<ObjEnemyBossScene>(objId))
    {
        obj->add(step, scriptPath);
    }
    return 0;
}

static int ObjEnemyBossScene_LoadInThread(lua_State* L)
{
    // FUTURE : multi thread
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjEnemyBossScene>(objId))
    {
        obj->loadInThread(getSourcePos(L));
    }
    return 0;
}

static int ObjEnemyBossScene_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int info = DnhValue::toInt(L, 2);
    auto obj = engine->getObject<ObjEnemyBossScene>(objId);
    switch (info)
    {
        case INFO_IS_SPELL:
            lua_pushboolean(L, obj && obj->isSpell());
            break;
        case INFO_IS_LAST_SPELL:
            lua_pushboolean(L, obj && obj->isLastSpell());
            break;
        case INFO_IS_DURABLE_SPELL:
            lua_pushboolean(L, obj && obj->isDurableSpell());
            break;
        case INFO_IS_LAST_STEP:
            lua_pushboolean(L, obj && obj->isLastStep());
            break;
        case INFO_TIMER:
            lua_pushnumber(L, !obj ? 0 : obj->getTimer());
            break;
        case INFO_TIMERF:
            lua_pushnumber(L, !obj ? 0 : obj->getTimerF());
            break;
        case INFO_ORGTIMERF:
            lua_pushnumber(L, !obj ? 0 : obj->getOrgTimerF());
            break;
        case INFO_SPELL_SCORE:
            lua_pushnumber(L, !obj ? 0 : obj->getSpellScore());
            break;
        case INFO_REMAIN_STEP_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->getRemainStepCount());
            break;
        case INFO_ACTIVE_STEP_LIFE_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->getActiveStepLifeCount());
            break;
        case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->getActiveStepTotalMaxLife());
            break;
        case INFO_ACTIVE_STEP_TOTAL_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->getActiveStepTotalLife());
            break;
        case INFO_PLAYER_SHOOTDOWN_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->getPlayerShootDownCount());
            break;
        case INFO_PLAYER_SPELL_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->getPlayerSpellCount());
            break;
        case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
        {
            DnhArray result;
            if (obj)
            {
                for (double rate : obj->getActiveStepLifeRateList())
                {
                    result.pushBack(std::make_unique<DnhReal>(rate));
                }
            }
            result.push(L);
        }
        break;
        case INFO_CURRENT_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->getCurrentLife());
            break;
        case INFO_CURRENT_LIFE_MAX:
            lua_pushnumber(L, !obj ? 0 : obj->getCurrentLifeMax());
            break;
        default:
            return 0;
    }
    return 1;
}

static int ObjEnemyBossScene_SetSpellTimer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int sec = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjEnemyBossScene>(objId))
    {
        obj->setTimer(sec);
    }
    return 0;
}

static int ObjEnemyBossScene_StartSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjEnemyBossScene>(objId))
    {
        obj->startSpell();
    }
    return 0;
}


static int ObjShot_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    int shotType = DnhValue::toInt(L, 1);
    int objId = ID_INVALID;
    bool isPlayerShot = script->getType() == SCRIPT_TYPE_PLAYER;
    switch (shotType)
    {
        case OBJ_SHOT:
            objId = engine->createObjShot(isPlayerShot)->getID();
            break;
        case OBJ_LOOSE_LASER:
            objId = engine->createObjLooseLaser(isPlayerShot)->getID();
            break;
        case OBJ_STRAIGHT_LASER:
            objId = engine->createObjStLaser(isPlayerShot)->getID();
            break;
        case OBJ_CURVE_LASER:
            objId = engine->createObjCrLaser(isPlayerShot)->getID();
            break;
    }
    script->addAutoDeleteTargetObjectId(objId);
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjShot_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        // NOTE : IsPermitPlayerShot == falseならregistしない, ObjShot_Registで作成したときのみ
        if (auto player = engine->getPlayerObject())
        {
            if (!player->isPermitPlayerShot())
            {
                return 0;
            }
        }
        obj->regist();
    }
    return 0;
}

static int ObjShot_SetAutoDelete(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool autoDelete = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setAutoDeleteEnable(autoDelete); }
    return 0;
}

static int ObjShot_FadeDelete(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->fadeDelete(); }
    return 0;
}

static int ObjShot_SetDeleteFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int deleteFrame = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setDeleteFrame(deleteFrame); }
    return 0;
}

static int ObjShot_SetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double damage = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setDamage(damage); }
    return 0;
}

static int ObjShot_SetDelay(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int delay = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setDelay(delay); }
    return 0;
}

static int ObjShot_SetSpellResist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool spellResist = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setSpellResistEnable(spellResist); }
    return 0;
}

static int ObjShot_SetGraphic(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int shotDataId = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        if (obj->isPlayerShot())
        {
            obj->setShotData(engine->getPlayerShotData(shotDataId));
        } else
        {
            obj->setShotData(engine->getEnemyShotData(shotDataId));
        }
    }
    return 0;
}

static int ObjShot_SetSourceBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int blendType = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setSourceBlendType(blendType); }
    return 0;
}

static int ObjShot_SetPenetration(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int penetration = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setPenetration(penetration); }
    return 0;
}

static int ObjShot_SetEraseShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool eraseShot = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setEraseShotEnable(eraseShot); }
    return 0;
}

static int ObjShot_SetSpellFactor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool spellFactor = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) { obj->setSpellFactor(spellFactor); }
    return 0;
}

static int ObjShot_ToItem(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->toItem();
    return 0;
}

static int ObjShot_AddShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int addShotId = DnhValue::toInt(L, 2);
    int frame = DnhValue::toInt(L, 3);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->addShotA1(addShotId, frame);
    return 0;
}

static int ObjShot_AddShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int addShotId = DnhValue::toInt(L, 2);
    int frame = DnhValue::toInt(L, 3);
    float dist = DnhValue::toNum(L, 4);
    float angle = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->addShotA2(addShotId, frame, dist, angle);
    return 0;
}

static int ObjShot_SetIntersectionEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->setIntersectionEnable(enable);
    return 0;
}

static int ObjShot_SetIntersectionCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double r = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->addTempIntersectionCircleA1(r);
    return 0;
}

static int ObjShot_SetIntersectionCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->addTempIntersectionCircleA2(x, y, r);
    return 0;
}

static int ObjShot_SetIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x1 = DnhValue::toNum(L, 2);
    double y1 = DnhValue::toNum(L, 3);
    double x2 = DnhValue::toNum(L, 4);
    double y2 = DnhValue::toNum(L, 5);
    double width = DnhValue::toNum(L, 6);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->addTempIntersectionLine(x1, y1, x2, y2, width);
    return 0;
}

static int ObjShot_SetItemChange(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool itemChange = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjShot>(objId)) obj->setItemChange(itemChange);
    return 0;
}

static int ObjShot_GetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->getDamage() : 0);
    return 1;
}

static int ObjShot_GetPenetration(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->getPenetration() : 0);
    return 1;
}

static int ObjShot_GetDelay(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->getDelay() : 0);
    return 1;
}

static int ObjShot_IsSpellResist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjShot>(objId);
    lua_pushboolean(L, obj && obj->isSpellResistEnabled());
    return 1;
}

static int ObjShot_GetImageID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjShot>(objId))
    {
        if (const auto& shotData = obj->getShotData())
        {
            lua_pushnumber(L, shotData->id);
            return 1;
        }
    }
    lua_pushnumber(L, ID_INVALID);
    return 1;
}

static int ObjLaser_SetLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double length = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjLaser>(objId))
    {
        obj->setLength(length);
    }
    return 0;
}

static int ObjLaser_SetRenderWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double width = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjLaser>(objId))
    {
        obj->setRenderWidth(width);
    }
    return 0;
}

static int ObjLaser_SetIntersectionWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double width = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjLaser>(objId))
    {
        obj->setIntersectionWidth(width);
    }
    return 0;
}

static int ObjLaser_SetGrazeInvalidFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int frame = DnhValue::toInt(L, 2);
    if (auto obj = engine->getObject<ObjLaser>(objId))
    {
        obj->setGrazeInvalidFrame(frame);
    }
    return 0;
}

static int ObjLaser_SetInvalidLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double head = DnhValue::toNum(L, 2);
    double tail = DnhValue::toNum(L, 3);
    if (auto obj = engine->getObject<ObjLooseLaser>(objId))
    {
        obj->setInvalidLength(head, tail);
    }
    return 0;
}

static int ObjLaser_SetItemDistance(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double dist = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjLaser>(objId))
    {
        obj->setItemDistance(dist);
    }
    return 0;
}

static int ObjLaser_GetLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjLaser>(objId);
    lua_pushnumber(L, obj ? obj->getLength() : 0);
    return 1;
}

static int ObjStLaser_SetAngle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double angle = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjStLaser>(objId))
    {
        obj->setLaserAngle(angle);
    }
    return 0;
}

static int ObjStLaser_GetAngle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjStLaser>(objId);
    lua_pushnumber(L, obj ? obj->getLaserAngle() : 0);
    return 1;
}

static int ObjStLaser_SetSource(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool source = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjStLaser>(objId))
    {
        obj->setSource(source);
    }
    return 0;
}

static int ObjCrLaser_SetTipDecrement(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double decr = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjCrLaser>(objId))
    {
        obj->setTipDecrement(decr);
    }
    return 0;
}

static int ObjItem_SetItemID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int itemDataId = DnhValue::toInt(L, 2);
    if (auto item = engine->getObject<ObjItem>(objId))
    {
        item->setItemData(engine->getItemData(objId));
    }
    return 0;
}

static int ObjItem_SetRenderScoreEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto item = engine->getObject<ObjItem>(objId))
    {
        item->setRenderScoreEnable(enable);
    }
    return 0;
}

static int ObjItem_SetAutoCollectEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    if (auto item = engine->getObject<ObjItem>(objId))
    {
        item->setAutoCollectEnable(enable);
    }
    return 0;
}

static int ObjItem_SetDefinedMovePatternA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int pattern = DnhValue::toInt(L, 2);
    if (auto item = engine->getObject<ObjItem>(objId))
    {
        if (pattern == ITEM_MOVE_DOWN)
        {
            item->setMoveMode(std::make_shared<MoveModeItemDown>(2.5f));
        } else if (pattern == ITEM_MOVE_TOPLAYER)
        {
            item->setMoveMode(std::make_shared<MoveModeItemToPlayer>(8.0f, engine->getPlayerObject()));
        }
    }
    return 0;
}

static int ObjItem_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    int info = DnhValue::toInt(L, 2);
    if (auto item = engine->getObject<ObjItem>(objId))
    {
        if (info == INFO_ITEM_SCORE)
        {
            lua_pushnumber(L, item->getScore());
            return 1;
        }
    }
    return 0;
}

static int ObjPlayer_AddIntersectionCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double dx = DnhValue::toNum(L, 2);
    double dy = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    double dr = DnhValue::toNum(L, 5);
    if (auto obj = engine->getObject<ObjPlayer>(objId))
    {
        obj->addIntersectionCircleA1(dx, dy, r, dr);
    }
    return 0;
}

static int ObjPlayer_AddIntersectionCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double dx = DnhValue::toNum(L, 2);
    double dy = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjPlayer>(objId))
    {
        obj->addIntersectionCircleA2(dx, dy, r);
    }
    return 0;
}

static int ObjPlayer_ClearIntersection(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjPlayer>(objId))
    {
        obj->clearIntersection();
    }
    return 0;
}

static int ObjCol_IsIntersected(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjCol>(objId);
    lua_pushboolean(L, obj && obj->isIntersected());
    return 1;
}

static int ObjCol_GetListOfIntersectedEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    DnhArray enemyIds;
    if (auto obj = engine->getObject<ObjCol>(objId))
    {
        for (auto& wIsect : obj->getCollideIntersections())
        {
            if (auto isect = wIsect.lock())
            {
                if (auto enemyIsectToShot = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect))
                {
                    if (auto enemy = enemyIsectToShot->GetEnemy().lock())
                    {
                        enemyIds.pushBack(std::make_unique<DnhReal>(enemy->getID()));
                    }
                } else if (auto enemyIsectToPlayer = std::dynamic_pointer_cast<EnemyIntersectionToPlayer>(isect))
                {
                    if (auto enemy = enemyIsectToPlayer->GetEnemy().lock())
                    {
                        enemyIds.pushBack(std::make_unique<DnhReal>(enemy->getID()));
                    }
                }
            }
        }
    }
    enemyIds.push(L);
    return 1;
}

static int ObjCol_GetIntersectedCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    auto obj = engine->getObject<ObjCol>(objId);
    lua_pushnumber(L, obj ? obj->getIntersectedCount() : 0);
    return 1;
}

static int CallSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->getPlayerObject())
    {
        player->callSpell();
    }
    return 0;
}

static int LoadPlayerShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->loadPlayerShotData(path, getSourcePos(L));
    return 0;
}

static int ReloadPlayerShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::toString(L, 1);
    engine->reloadPlayerShotData(path, getSourcePos(L));
    return 0;
}

static int GetSpellManageObject(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto obj = engine->getSpellManageObject();
    lua_pushnumber(L, obj ? obj->getID() : ID_INVALID);
    return 1;
}

static int ObjSpell_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    if (auto obj = engine->createObjSpell())
    {
        script->addAutoDeleteTargetObjectId(obj->getID());
        lua_pushnumber(L, obj->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjSpell_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    if (auto obj = engine->getObject<ObjSpell>(objId)) { obj->regist(); }
    return 0;
}

static int ObjSpell_SetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double damage = DnhValue::toNum(L, 2);
    if (auto obj = engine->getObject<ObjSpell>(objId)) { obj->setDamage(damage); }
    return 0;
}

static int ObjSpell_SetEraseShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    bool eraseShot = DnhValue::toBool(L, 2);
    if (auto obj = engine->getObject<ObjSpell>(objId)) { obj->setEraseShotEnable(eraseShot); }
    return 0;
}

static int ObjSpell_SetIntersectionCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x = DnhValue::toNum(L, 2);
    double y = DnhValue::toNum(L, 3);
    double r = DnhValue::toNum(L, 4);
    if (auto obj = engine->getObject<ObjSpell>(objId))
    {
        obj->addTempIntersectionCircle(x, y, r);
    }
    return 0;
}

static int ObjSpell_SetIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::toInt(L, 1);
    double x1 = DnhValue::toNum(L, 2);
    double y1 = DnhValue::toNum(L, 3);
    double x2 = DnhValue::toNum(L, 4);
    double y2 = DnhValue::toNum(L, 5);
    double width = DnhValue::toNum(L, 6);
    if (auto obj = engine->getObject<ObjSpell>(objId))
    {
        obj->addTempIntersectionLine(x1, y1, x2, y2, width);
    }
    return 0;
}

static int SetPauseScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setPauseScriptPath(path);
    return 0;
}

static int SetEndSceneScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setEndSceneScriptPath(path);
    return 0;
}

static int SetReplaySaveSceneScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setReplaySaveSceneScriptPath(path);
    return 0;
}

static int CreatePlayerShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = getScript(L);
    double x = DnhValue::toNum(L, 1);
    double y = DnhValue::toNum(L, 2);
    double speed = DnhValue::toNum(L, 3);
    double angle = DnhValue::toNum(L, 4);
    double damage = DnhValue::toNum(L, 5);
    int penetration = DnhValue::toInt(L, 6);
    int shotDataId = DnhValue::toInt(L, 7);
    if (auto shot = engine->createPlayerShotA1(x, y, speed, angle, damage, penetration, shotDataId))
    {
        lua_pushnumber(L, shot->getID());
        script->addAutoDeleteTargetObjectId(shot->getID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int GetTransitionRenderTargetName(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->getTransitionRenderTargetName()).push(L);
    return 1;
}

static int SetShotDeleteEventEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int targetEvent = DnhValue::toInt(L, 1);
    bool enable = DnhValue::toBool(L, 2);
    switch (targetEvent)
    {
        case EV_DELETE_SHOT_IMMEDIATE:
            engine->setDeleteShotImmediateEventOnShotScriptEnable(enable);
            break;
        case EV_DELETE_SHOT_FADE:
            engine->setDeleteShotFadeEventOnShotScriptEnable(enable);
            break;
        case EV_DELETE_SHOT_TO_ITEM:
            engine->setDeleteShotToItemEventOnShotScriptEnable(enable);
            break;
    }
    return 0;
}

static int ClosePackage(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->closePackage();
    return 0;
}

static int InitializeStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->initializeStageScene();
    return 0;
}

static int FinalizeStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->finalizeStageScene();
    return 0;
}

static int StartStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->startStageScene(getSourcePos(L));
    return 0;
}

static int SetStageIndex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int idx = DnhValue::toInt(L, 1);
    engine->setStageIndex(idx);
    return 0;
}

static int SetStageMainScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setStageMainScript(path, getSourcePos(L));
    return 0;
}

static int SetStagePlayerScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setStagePlayerScript(path, getSourcePos(L));
    return 0;
}

static int SetStageReplayFile(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::toString(L, 1);
    engine->setStageReplayFile(path);
    return 0;
}

static int GetStageSceneState(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->isStageFinished() ? STAGE_STATE_FINISHED : -1);
    return 1;
}

static int GetStageSceneResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getStageSceneResult());
    return 1;
}

static int PauseStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool doPause = DnhValue::toBool(L, 1);
    engine->pauseStageScene(doPause);
    return 0;
}

static int TerminateStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->terminateStageScene();
    return 0;
}

static int GetLoadFreePlayerScriptList(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->getLoadFreePlayerScriptList();
    return 0;
}

static int GetFreePlayerScriptCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->getFreePlayerScriptCount());
    return 1;
}

static int GetFreePlayerScriptInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int idx = DnhValue::toInt(L, 1);
    int infoType = DnhValue::toInt(L, 2);
    if (idx >= 0 && idx < engine->getFreePlayerScriptCount())
    {
        ScriptInfo info = engine->getFreePlayerScriptInfo(idx);
        switch (infoType)
        {
            case INFO_SCRIPT_PATH:
                DnhArray(info.path).push(L);
                break;
            case INFO_SCRIPT_ID:
                DnhArray(info.id).push(L);
                break;
            case INFO_SCRIPT_TITLE:
                DnhArray(info.title).push(L);
                break;
            case INFO_SCRIPT_TEXT:
                DnhArray(info.text).push(L);
                break;
            case INFO_SCRIPT_IMAGE:
                DnhArray(info.imagePath).push(L);
                break;
            case INFO_SCRIPT_REPLAY_NAME:
                DnhArray(info.replayName).push(L);
                break;
            default:
                DnhArray(L"").push(L);
                break;
        }
        return 1;
    }
    return 0;
}

static int LoadReplayList(lua_State* L)
{
    // FUTURE : impl
    return 0;
}

static int GetValidReplayIndices(lua_State* L)
{
    // FUTURE : impl
    DnhArray(L"").push(L);
    return 1;
}

static int IsValidReplayIndex(lua_State* L)
{
    // FUTURE : impl
    lua_pushboolean(L, false);
    return 1;
}

static int GetReplayInfo(lua_State* L)
{
    // FUTURE : impl
    DnhArray(L"").push(L);
    return 1;
}

static int SetReplayInfo(lua_State* L)
{
    // FUTURE : impl
    return 0;
}

static int SaveReplay(lua_State* L)
{
    // FUTURE : impl
    return 0;
}

// helper

int getCurrentLine(lua_State* L)
{
    // コールスタックから現在の行番号を取得する
    lua_Debug deb;
    int level = 1;
    int line = -1;
    while (lua_getstack(L, level++, &deb))
    {
        deb.source = NULL;
        lua_getinfo(L, "Sl", &deb);
        // ランタイムの行ではなく、変換されたソースから行番号を取得
        if (std::string(deb.source) != DNH_RUNTIME_NAME)
        {
            line = deb.currentline;
            break;
        }
    }
    return line;
}

std::shared_ptr<SourcePos> getSourcePos(lua_State * L)
{
    int line = getCurrentLine(L);
    Script* script = getScript(L);
    return script->getSourcePos(line);
}

// MEMO :
// UnsafeFunctionCommonで例外を拾い
// __UnsafeFunctionCommonでlua_errorでlongjmpする
// UnsafeFunctionCommonから直接longjmpするとローカルに確保されたオブジェクトのデストラクタが呼ばれないため二段構成となっている

static int UnsafeFunctionCommon(lua_State* L, lua_CFunction func)
{
    try
    {
        return func(L);
    } catch (Log& log)
    {
        int line = getCurrentLine(L);
        Script* script = getScript(L);
        auto srcPos = getSourcePos(L);
        log.addSourcePos(srcPos);
        script->saveError(std::current_exception());
        lua_pushstring(L, "script_runtime_error");
    } catch (const std::exception& e)
    {
        int line = getCurrentLine(L);
        Script* script = getScript(L);
        auto srcPos = getSourcePos(L);
        try
        {
            throw Log(Log::Level::LV_ERROR)
                .setMessage("unexpected script runtime error occured.")
                .setParam(Log::Param(Log::Param::Tag::TEXT, e.what()))
                .addSourcePos(srcPos);
        } catch (...)
        {
            script->saveError(std::current_exception());
        }
        lua_pushstring(L, "unexpected_script_runtime_error");
    }
    return -1;
}

// NOTE : don't create object in this function
int __UnsafeFunctionCommon(lua_State* L, lua_CFunction func)
{
    int r = UnsafeFunctionCommon(L, func);
    if (r < 0) lua_error(L); // longjmp
    return r;
}

void setPointer(lua_State* L, const char* key, void* p)
{
    lua_pushstring(L, key);
    lua_pushlightuserdata(L, p);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void* getPointer(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_rawget(L, LUA_REGISTRYINDEX);
    void* p = lua_touserdata(L, -1);
    return p;
}

Engine* getEngine(lua_State* L)
{
    return (Engine*)getPointer(L, "Engine");
}

void setEngine(lua_State* L, Engine* p)
{
    setPointer(L, "Engine", p);
}

Script* getScript(lua_State* L)
{
    return (Script*)getPointer(L, "Script");
}

void setScript(lua_State* L, Script* p)
{
    setPointer(L, "Script", p);
}

int c_chartonum(lua_State* L)
{
    std::wstring wstr = DnhValue::toString(L, 1);
    if (wstr.empty())
    {
        lua_pushnumber(L, 0);
    } else
    {
        int n = (int)wstr[0];
        lua_pushnumber(L, n);
    }
    return 1;
}

int c_succchar(lua_State* L)
{
    std::wstring wstr = DnhValue::toString(L, 1);
    if (wstr.empty())
    {
        lua_pushstring(L, "");
    } else
    {
        wchar_t c = (int)wstr[0];
        c++;
        std::string r = toUTF8(std::wstring{ c });
        lua_pushstring(L, r.c_str());
    }
    return 1;
}

int c_predchar(lua_State* L)
{
    std::wstring wstr = DnhValue::toString(L, 1);
    if (wstr.empty())
    {
        lua_pushstring(L, "");
    } else
    {
        wchar_t c = (int)wstr[0];
        c--;
        std::string r = toUTF8(std::wstring{ c });
        lua_pushstring(L, r.c_str());
    }
    return 1;
}

int __c_raiseerror(lua_State* L)
{
    std::string msg = lua_tostring(L, 1);
    throw Log(Log::Level::LV_ERROR).setMessage(msg);
    return 0;
}

int c_raiseerror(lua_State* L)
{
    return UnsafeFunction<__c_raiseerror>(L);
}

__declspec(noinline) static void addConst(NameTable& table, const char* name, const wchar_t* value)
{
    table[name] = std::make_shared<NodeConst>(name, value);
}

__declspec(noinline) static void addConstI(NameTable& table, const char* name, int value)
{
    table[name] = std::make_shared<NodeConst>(name, std::to_wstring(value));
}

__declspec(noinline) static void addFunc(NameTable& table, const char* name, int paramc, lua_CFunction func)
{
    table[name] = std::make_shared<NodeBuiltInFunc>(name, paramc, (void*)func);
}

#define constI(name) (addConstI(table, #name, name))
#define unsafe(name, paramc) (addFunc(table, #name, (paramc), UnsafeFunction<name>))
#ifdef _DEBUG
#define safe(name, paramc) (unsafe(name, paramc))
#else
#define safe(name, paramc) (addFunc(table, #name, (paramc), name))
#endif
#define runtime(name, paramc) (addFunc(table, #name, (paramc), 0))
#define TypeIs(typeSet) ((typeSet) & type)

typedef uint8_t ScriptType;

void addStandardAPI(const std::wstring& typeName, const std::wstring& version, NameTable& table)
{
    constexpr ScriptType t_player = 1;
    constexpr ScriptType t_stage = 2;
    constexpr ScriptType t_package = 4;
    constexpr ScriptType t_shot_custom = 8;
    constexpr ScriptType t_item_custom = 16;
    constexpr ScriptType t_all = 0xff;

    ScriptType type = 0;
    if (typeName == SCRIPT_TYPE_PLAYER)
    {
        type = t_player;
    } else if (typeName == SCRIPT_TYPE_PACKAGE)
    {
        type = t_package;
    } else if (typeName == SCRIPT_TYPE_SHOT_CUSTOM)
    {
        type = t_shot_custom;
    } else if (typeName == SCRIPT_TYPE_ITEM_CUSTOM)
    {
        type = t_item_custom;
    } else
    {
        type = t_stage;
    }
    constI(OBJ_PRIMITIVE_2D);
    constI(OBJ_SPRITE_2D);
    constI(OBJ_SPRITE_LIST_2D);
    constI(OBJ_PRIMITIVE_3D);
    constI(OBJ_SPRITE_3D);
    constI(OBJ_TRAJECTORY_3D);
    constI(OBJ_SHADER);
    constI(OBJ_MESH);
    constI(OBJ_TEXT);
    constI(OBJ_SOUND);
    constI(OBJ_FILE_TEXT);
    constI(OBJ_FILE_BINARY);

    if (TypeIs(~t_package))
    {
        constI(OBJ_PLAYER);
        constI(OBJ_SPELL_MANAGE);
        constI(OBJ_SPELL);
        constI(OBJ_ENEMY);
        constI(OBJ_ENEMY_BOSS);
        constI(OBJ_ENEMY_BOSS_SCENE);
        constI(OBJ_SHOT);
        constI(OBJ_LOOSE_LASER);
        constI(OBJ_STRAIGHT_LASER);
        constI(OBJ_CURVE_LASER);
        constI(OBJ_ITEM);

        constI(ITEM_1UP);
        constI(ITEM_1UP_S);
        constI(ITEM_SPELL);
        constI(ITEM_SPELL_S);
        constI(ITEM_POWER);
        constI(ITEM_POWER_S);
        constI(ITEM_POINT);
        constI(ITEM_POINT_S);
        constI(ITEM_USER);
        constI(ITEM_MOVE_DOWN);
        constI(ITEM_MOVE_TOPLAYER);
    }

    constI(BLEND_NONE);
    constI(BLEND_ALPHA);
    constI(BLEND_ADD_RGB);
    constI(BLEND_ADD_ARGB);
    constI(BLEND_MULTIPLY);
    constI(BLEND_SUBTRACT);
    constI(BLEND_SHADOW);
    constI(BLEND_INV_DESTRGB);

    constI(PRIMITIVE_TRIANGLEFAN);
    constI(PRIMITIVE_TRIANGLESTRIP);
    constI(PRIMITIVE_TRIANGLELIST);
    constI(PRIMITIVE_LINESTRIP);
    constI(PRIMITIVE_LINELIST);
    constI(PRIMITIVE_POINT_LIST);

    constI(BORDER_NONE);
    constI(BORDER_FULL);
    constI(BORDER_SHADOW);

    constI(TYPE_SCRIPT_ALL);
    constI(TYPE_SCRIPT_PLAYER);
    constI(TYPE_SCRIPT_SINGLE);
    constI(TYPE_SCRIPT_PLURAL);
    constI(TYPE_SCRIPT_STAGE);
    constI(TYPE_SCRIPT_PACKAGE);

    constI(VK_LEFT);
    constI(VK_RIGHT);
    constI(VK_UP);
    constI(VK_DOWN);
    constI(VK_OK);
    constI(VK_CANCEL);
    constI(VK_SHOT);
    constI(VK_BOMB);
    constI(VK_SPELL);
    constI(VK_SLOWMOVE);
    constI(VK_USER1);
    constI(VK_USER2);
    constI(VK_PAUSE);
    constI(VK_USER_ID_STAGE);
    constI(VK_USER_ID_PLAYER);

    constI(KEY_INVALID);
    constI(KEY_ESCAPE);
    constI(KEY_1);
    constI(KEY_2);
    constI(KEY_3);
    constI(KEY_4);
    constI(KEY_5);
    constI(KEY_6);
    constI(KEY_7);
    constI(KEY_8);
    constI(KEY_9);
    constI(KEY_0);
    constI(KEY_MINUS);
    constI(KEY_EQUALS);
    constI(KEY_BACK);
    constI(KEY_TAB);
    constI(KEY_Q);
    constI(KEY_W);
    constI(KEY_E);
    constI(KEY_R);
    constI(KEY_T);
    constI(KEY_Y);
    constI(KEY_U);
    constI(KEY_I);
    constI(KEY_O);
    constI(KEY_P);
    constI(KEY_LBRACKET);
    constI(KEY_RBRACKET);
    constI(KEY_RETURN);
    constI(KEY_LCONTROL);
    constI(KEY_A);
    constI(KEY_S);
    constI(KEY_D);
    constI(KEY_F);
    constI(KEY_G);
    constI(KEY_H);
    constI(KEY_J);
    constI(KEY_K);
    constI(KEY_L);
    constI(KEY_SEMICOLON);
    constI(KEY_APOSTROPHE);
    constI(KEY_GRAVE);
    constI(KEY_LSHIFT);
    constI(KEY_BACKSLASH);
    constI(KEY_Z);
    constI(KEY_X);
    constI(KEY_C);
    constI(KEY_V);
    constI(KEY_B);
    constI(KEY_N);
    constI(KEY_M);
    constI(KEY_COMMA);
    constI(KEY_PERIOD);
    constI(KEY_SLASH);
    constI(KEY_RSHIFT);
    constI(KEY_MULTIPLY);
    constI(KEY_LMENU);
    constI(KEY_SPACE);
    constI(KEY_CAPITAL);
    constI(KEY_F1);
    constI(KEY_F2);
    constI(KEY_F3);
    constI(KEY_F4);
    constI(KEY_F5);
    constI(KEY_F6);
    constI(KEY_F7);
    constI(KEY_F8);
    constI(KEY_F9);
    constI(KEY_F10);
    constI(KEY_NUMLOCK);
    constI(KEY_SCROLL);
    constI(KEY_NUMPAD7);
    constI(KEY_NUMPAD8);
    constI(KEY_NUMPAD9);
    constI(KEY_SUBTRACT);
    constI(KEY_NUMPAD4);
    constI(KEY_NUMPAD5);
    constI(KEY_NUMPAD6);
    constI(KEY_ADD);
    constI(KEY_NUMPAD1);
    constI(KEY_NUMPAD2);
    constI(KEY_NUMPAD3);
    constI(KEY_NUMPAD0);
    constI(KEY_DECIMAL);
    constI(KEY_F11);
    constI(KEY_F12);
    constI(KEY_F13);
    constI(KEY_F14);
    constI(KEY_F15);
    constI(KEY_KANA);
    constI(KEY_CONVERT);
    constI(KEY_NOCONVERT);
    constI(KEY_YEN);
    constI(KEY_NUMPADEQUALS);
    constI(KEY_CIRCUMFLEX);
    constI(KEY_AT);
    constI(KEY_COLON);
    constI(KEY_UNDERLINE);
    constI(KEY_KANJI);
    constI(KEY_STOP);
    constI(KEY_AX);
    constI(KEY_UNLABELED);
    constI(KEY_NUMPADENTER);
    constI(KEY_RCONTROL);
    constI(KEY_NUMPADCOMMA);
    constI(KEY_DIVIDE);
    constI(KEY_SYSRQ);
    constI(KEY_RMENU);
    constI(KEY_PAUSE);
    constI(KEY_HOME);
    constI(KEY_UP);
    constI(KEY_PRIOR);
    constI(KEY_LEFT);
    constI(KEY_RIGHT);
    constI(KEY_END);
    constI(KEY_DOWN);
    constI(KEY_NEXT);
    constI(KEY_INSERT);
    constI(KEY_DELETE);
    constI(KEY_LWIN);
    constI(KEY_RWIN);
    constI(KEY_APPS);
    constI(KEY_POWER);
    constI(KEY_SLEEP);

    constI(MOUSE_LEFT);
    constI(MOUSE_RIGHT);
    constI(MOUSE_MIDDLE);

    constI(KEY_FREE);
    constI(KEY_PUSH);
    constI(KEY_PULL);
    constI(KEY_HOLD);

    constI(ALIGNMENT_LEFT);
    constI(ALIGNMENT_RIGHT);
    constI(ALIGNMENT_CENTER);

    constI(SOUND_BGM);
    constI(SOUND_SE);
    constI(SOUND_VOICE);

    constI(RESULT_CANCEL);
    constI(RESULT_END);
    constI(RESULT_RETRY);
    constI(RESULT_SAVE_REPLAY);

    constI(REPLAY_INDEX_ACTIVE);
    constI(REPLAY_INDEX_DIGIT_MIN);
    constI(REPLAY_FILE_PATH);
    constI(REPLAY_DATE_TIME);
    constI(REPLAY_USER_NAME);
    constI(REPLAY_TOTAL_SCORE);
    constI(REPLAY_FPS_AVERAGE);
    constI(REPLAY_PLAYER_NAME);
    constI(REPLAY_STAGE_INDEX_LIST);
    constI(REPLAY_STAGE_START_SCORE_LIST);
    constI(REPLAY_STAGE_LAST_SCORE_LIST);
    constI(REPLAY_COMMENT);
    constI(REPLAY_INDEX_DIGIT_MAX);
    constI(REPLAY_INDEX_USER);

    if (TypeIs(~t_package))
    {
        constI(EV_REQUEST_LIFE);
        constI(EV_REQUEST_TIMER);
        constI(EV_REQUEST_IS_SPELL);
        constI(EV_REQUEST_IS_LAST_SPELL);
        constI(EV_REQUEST_IS_DURABLE_SPELL);
        constI(EV_REQUEST_SPELL_SCORE);
        constI(EV_REQUEST_REPLAY_TARGET_COMMON_AREA);
        constI(EV_TIMEOUT);
        constI(EV_START_BOSS_SPELL);
        constI(EV_GAIN_SPELL);
        constI(EV_START_BOSS_STEP);
        constI(EV_END_BOSS_STEP);
        constI(EV_PLAYER_SHOOTDOWN);
        constI(EV_PLAYER_SPELL);
        constI(EV_PLAYER_REBIRTH);
        constI(EV_PAUSE_ENTER);
        constI(EV_PAUSE_LEAVE);
    }

    if (TypeIs(t_shot_custom))
    {
        constI(EV_DELETE_SHOT_IMMEDIATE);
        constI(EV_DELETE_SHOT_FADE);
    }
    if (TypeIs(t_shot_custom | t_item_custom))
    {
        constI(EV_DELETE_SHOT_TO_ITEM);
    }

    if (TypeIs(t_player))
    {
        constI(EV_REQUEST_SPELL);
        constI(EV_GRAZE);
        constI(EV_HIT);
    }

    if (TypeIs(t_player | t_item_custom))
    {
        constI(EV_GET_ITEM);
    }

    constI(EV_USER_COUNT);
    constI(EV_USER);
    constI(EV_USER_SYSTEM);
    constI(EV_USER_STAGE);
    constI(EV_USER_PLAYER);
    constI(EV_USER_PACKAGE);

    constI(INFO_SCRIPT_TYPE);
    constI(INFO_SCRIPT_PATH);
    constI(INFO_SCRIPT_ID);
    constI(INFO_SCRIPT_TITLE);
    constI(INFO_SCRIPT_TEXT);
    constI(INFO_SCRIPT_IMAGE);
    constI(INFO_SCRIPT_REPLAY_NAME);

    if (TypeIs(~t_package))
    {
        constI(INFO_LIFE);
        constI(INFO_DAMAGE_RATE_SHOT);
        constI(INFO_DAMAGE_RATE_SPELL);
        constI(INFO_SHOT_HIT_COUNT);
        constI(INFO_TIMER);
        constI(INFO_TIMERF);
        constI(INFO_ORGTIMERF);
        constI(INFO_IS_SPELL);
        constI(INFO_IS_LAST_SPELL);
        constI(INFO_IS_DURABLE_SPELL);
        constI(INFO_SPELL_SCORE);
        constI(INFO_REMAIN_STEP_COUNT);
        constI(INFO_ACTIVE_STEP_LIFE_COUNT);
        constI(INFO_ACTIVE_STEP_TOTAL_MAX_LIFE);
        constI(INFO_ACTIVE_STEP_TOTAL_LIFE);
        constI(INFO_ACTIVE_STEP_LIFE_RATE_LIST);
        constI(INFO_IS_LAST_STEP);
        constI(INFO_PLAYER_SHOOTDOWN_COUNT);
        constI(INFO_PLAYER_SPELL_COUNT);
        constI(INFO_CURRENT_LIFE);
        constI(INFO_CURRENT_LIFE_MAX);
        constI(INFO_ITEM_SCORE);
        constI(INFO_RECT);
        constI(INFO_DELAY_COLOR);
        constI(INFO_BLEND);
        constI(INFO_COLLISION);
        constI(INFO_COLLISION_LIST);

        constI(STATE_NORMAL);
        constI(STATE_HIT);
        constI(STATE_DOWN);
        constI(STATE_END);

        constI(TYPE_ITEM);
        constI(TYPE_ALL);
        constI(TYPE_SHOT);
        constI(TYPE_CHILD);
        constI(TYPE_IMMEDIATE);
        constI(TYPE_FADE);

        constI(TARGET_PLAYER);
        constI(TARGET_ENEMY);
        constI(TARGET_ALL);

        constI(NO_CHANGE);
    }

    constI(ID_INVALID);

    if (TypeIs(t_package))
    {
        constI(STAGE_STATE_FINISHED);
        constI(STAGE_RESULT_BREAK_OFF);
        constI(STAGE_RESULT_PLAYER_DOWN);
        constI(STAGE_RESULT_CLEARED);
    }

    constI(CODE_ACP);
    constI(CODE_UTF8);
    constI(CODE_UTF16LE);
    constI(CODE_UTF16BE);
    constI(ENDIAN_LITTLE);
    constI(ENDIAN_BIG);

    constI(CULL_NONE);
    constI(CULL_CW);
    constI(CULL_CCW);

    addConst(table, "pi", L"3.141592653589793");
    addConst(table, "true", L"true");
    addConst(table, "false", L"false");

    runtime(concatenate, 2);
    runtime(add, 2);
    runtime(subtract, 2);
    runtime(multiply, 2);
    runtime(divide, 2);
    runtime(remainder, 2);
    runtime(power, 2);
    runtime(index_, 2);
    runtime(slice, 3);
    runtime(not, 1);
    runtime(negative, 1);
    runtime(successor, 1);
    runtime(predecessor, 1);
    runtime(append, 2);
    runtime(erase, 2);
    runtime(compare, 2);
    runtime(length, 1);

    runtime(min, 2);
    runtime(max, 2);
    runtime(log, 1);
    runtime(log10, 1);
    runtime(cos, 1);
    runtime(sin, 1);
    runtime(tan, 1);
    runtime(acos, 1);
    runtime(asin, 1);
    runtime(atan, 1);
    runtime(atan2, 2);
    runtime(rand, 2);
    runtime(round, 1);
    runtime(truncate, 1);
    runtime(trunc, 1);
    runtime(ceil, 1);
    runtime(floor, 1);
    runtime(absolute, 1);
    runtime(modc, 2);

    safe(InstallFont, 1);
    safe(ToString, 1);
    runtime(IntToString, 1);
    runtime(itoa, 1);
    runtime(rtoa, 1);
    safe(atoi, 1);
    safe(ator, 1);
    safe(TrimString, 1);
    safe(rtos, 2);
    safe(vtos, 2);
    safe(SplitString, 2);

    safe(GetFileDirectory, 1);
    safe(GetFilePathList, 1);
    safe(GetDirectoryList, 1);
    unsafe(GetModuleDirectory, 0);

    if (TypeIs(~t_package))
    {
        safe(GetMainStgScriptPath, 0);
    }

    safe(GetMainPackageScriptPath, 0);

    if (TypeIs(~t_package))
    {
        safe(GetMainStgScriptDirectory, 0);
    }

    runtime(GetCurrentScriptDirectory, 0);
    safe(GetScriptPathList, 2);

    safe(GetCurrentDateTimeS, 0);
    safe(GetStageTime, 0);
    safe(GetPackageTime, 0);
    safe(GetCurrentFps, 0);

    if (TypeIs(~t_package))
    {
        unsafe(GetReplayFps, 0);
    }

    unsafe(WriteLog, 1);
    unsafe(RaiseError, 1);
    unsafe(assert, 2);

    safe(SetCommonData, 2);
    safe(GetCommonData, 2);
    safe(ClearCommonData, 0);
    safe(DeleteCommonData, 1);
    safe(SetAreaCommonData, 3);
    safe(GetAreaCommonData, 3);
    safe(ClearAreaCommonData, 1);
    safe(DeleteAreaCommonData, 2);
    safe(CreateCommonDataArea, 1);
    safe(IsCommonDataAreaExists, 1);
    safe(CopyCommonDataArea, 2);
    safe(GetCommonDataAreaKeyList, 0);
    safe(GetCommonDataValueKeyList, 1);
    safe(SaveCommonDataAreaA1, 1);
    safe(LoadCommonDataAreaA1, 1);
    safe(SaveCommonDataAreaA2, 2);
    safe(LoadCommonDataAreaA2, 2);

    if (TypeIs(~t_package))
    {
        unsafe(SaveCommonDataAreaToReplayFile, 1);
        unsafe(LoadCommonDataAreaFromReplayFile, 1);
    }

    unsafe(LoadSound, 1);
    safe(RemoveSound, 1);
    safe(PlayBGM, 3);
    safe(PlaySE, 1);
    safe(StopSound, 1);

    safe(GetVirtualKeyState, 1);
    safe(SetVirtualKeyState, 2);
    safe(AddVirtualKey, 3);
    unsafe(AddReplayTargetVirtualKey, 1);
    safe(GetKeyState, 1);
    safe(GetMouseState, 1);
    safe(GetMouseX, 0);
    safe(GetMouseY, 0);
    safe(GetMouseMoveZ, 0);
    safe(SetSkipModeKey, 1);
    safe(LoadTexture, 1);
    safe(LoadTextureInLoadThread, 1);
    safe(RemoveTexture, 1);
    safe(GetTextureWidth, 1);
    safe(GetTextureHeight, 1);
    safe(SetFogEnable, 1);
    safe(SetFogParam, 5);
    safe(ClearInvalidRenderPriority, 0);
    safe(SetInvalidRenderPriorityA1, 2);
    safe(GetReservedRenderTargetName, 1);
    unsafe(CreateRenderTarget, 1);
    unsafe(RenderToTextureA1, 4);
    unsafe(RenderToTextureB1, 3);
    safe(SaveRenderedTextureA1, 2);
    safe(SaveRenderedTextureA2, 6);
    unsafe(SaveSnapShotA1, 1);
    unsafe(SaveSnapShotA2, 5);
    safe(IsPixelShaderSupported, 2);
    safe(SetShader, 3);
    safe(SetShaderI, 3);
    safe(ResetShader, 2);
    safe(ResetShaderI, 2);

    safe(SetCameraFocusX, 1);
    safe(SetCameraFocusY, 1);
    safe(SetCameraFocusZ, 1);
    safe(SetCameraFocusXYZ, 3);
    safe(SetCameraRadius, 1);
    safe(SetCameraAzimuthAngle, 1);
    safe(SetCameraElevationAngle, 1);
    safe(SetCameraYaw, 1);
    safe(SetCameraPitch, 1);
    safe(SetCameraRoll, 1);

    safe(GetCameraX, 0);
    safe(GetCameraY, 0);
    safe(GetCameraZ, 0);
    safe(GetCameraFocusX, 0);
    safe(GetCameraFocusY, 0);
    safe(GetCameraFocusZ, 0);
    safe(GetCameraRadius, 0);
    safe(GetCameraAzimuthAngle, 0);
    safe(GetCameraElevationAngle, 0);
    safe(GetCameraYaw, 0);
    safe(GetCameraPitch, 0);
    safe(GetCameraRoll, 0);
    safe(SetCameraPerspectiveClip, 2);

    safe(Set2DCameraFocusX, 1);
    safe(Set2DCameraFocusY, 1);
    safe(Set2DCameraAngleZ, 1);
    safe(Set2DCameraRatio, 1);
    safe(Set2DCameraRatioX, 1);
    safe(Set2DCameraRatioY, 1);
    safe(Reset2DCamera, 0);
    safe(Get2DCameraX, 0);
    safe(Get2DCameraY, 0);
    safe(Get2DCameraAngleZ, 0);
    safe(Get2DCameraRatio, 0);
    safe(Get2DCameraRatioX, 0);
    safe(Get2DCameraRatioY, 0);

    unsafe(LoadScript, 1);
    unsafe(LoadScriptInThread, 1);
    unsafe(StartScript, 1);
    safe(CloseScript, 1);
    safe(IsCloseScript, 1);
    safe(SetScriptArgument, 3);
    safe(GetScriptArgument, 1);
    safe(GetScriptArgumentCount, 0);

    if (TypeIs(~t_package))
    {
        unsafe(CloseStgScene, 0);
    }

    safe(GetOwnScriptID, 0);
    runtime(GetEventType, 0);
    runtime(GetEventArgument, 1);
    safe(SetScriptResult, 1);
    safe(GetScriptResult, 1);
    safe(SetAutoDeleteObject, 1);
    unsafe(NotifyEvent, 3);
    unsafe(NotifyEventAll, 2);
    safe(GetScriptInfoA1, 2);

    if (TypeIs(~t_package))
    {
        safe(SetStgFrame, 6);
    }
    safe(GetScore, 0);
    safe(AddScore, 1);
    safe(GetGraze, 0);
    safe(AddGraze, 1);
    safe(GetPoint, 0);
    safe(AddPoint, 1);
    if (TypeIs(~t_package))
    {
        safe(SetItemRenderPriorityI, 1);
        safe(SetShotRenderPriorityI, 1);
        safe(GetStgFrameRenderPriorityMinI, 0);
        safe(GetStgFrameRenderPriorityMaxI, 0);
        safe(GetItemRenderPriorityI, 0);
        safe(GetShotRenderPriorityI, 0);
        safe(GetPlayerRenderPriorityI, 0);
        safe(GetCameraFocusPermitPriorityI, 0);
    }
    safe(GetStgFrameLeft, 0);
    safe(GetStgFrameTop, 0);
    safe(GetStgFrameWidth, 0);
    safe(GetStgFrameHeight, 0);
    safe(GetScreenWidth, 0);
    safe(GetScreenHeight, 0);
    unsafe(IsReplay, 0);
    unsafe(AddArchiveFile, 1);

    if (TypeIs(~t_package))
    {
        safe(SCREEN_WIDTH, 0);
        safe(SCREEN_HEIGHT, 0);
        safe(GetPlayerObjectID, 0);
        safe(GetPlayerScriptID, 0);
        safe(SetPlayerSpeed, 2);
        safe(SetPlayerClip, 4);
        safe(SetPlayerLife, 1);
        safe(SetPlayerSpell, 1);
        safe(SetPlayerPower, 1);
        safe(SetPlayerInvincibilityFrame, 1);
        safe(SetPlayerDownStateFrame, 1);
        safe(SetPlayerRebirthFrame, 1);
        safe(SetPlayerRebirthLossFrame, 1);
        safe(SetPlayerAutoItemCollectLine, 1);
        safe(SetForbidPlayerShot, 1);
        safe(SetForbidPlayerSpell, 1);
        safe(GetPlayerX, 0);
        safe(GetPlayerY, 0);
        safe(GetPlayerState, 0);
        safe(GetPlayerSpeed, 0);
        safe(GetPlayerClip, 0);
        safe(GetPlayerLife, 0);
        safe(GetPlayerSpell, 0);
        safe(GetPlayerPower, 0);
        safe(GetPlayerInvincibilityFrame, 0);
        safe(GetPlayerDownStateFrame, 0);
        safe(GetPlayerRebirthFrame, 0);
        safe(IsPermitPlayerShot, 0);
        safe(IsPermitPlayerSpell, 0);
        safe(IsPlayerLastSpellWait, 0);
        safe(IsPlayerSpellActive, 0);
        safe(GetAngleToPlayer, 1);
    }

    safe(GetPlayerID, 0);
    safe(GetPlayerReplayName, 0);

    if (TypeIs(~t_package))
    {
        safe(GetEnemyIntersectionPosition, 3);
        unsafe(GetEnemyBossSceneObjectID, 0);
        unsafe(GetEnemyBossObjectID, 0);
        safe(GetAllEnemyID, 0);
        safe(GetIntersectionRegistedEnemyID, 0);
        safe(GetAllEnemyIntersectionPosition, 0);
        safe(GetEnemyIntersectionPositionByIdA1, 1);
        safe(GetEnemyIntersectionPositionByIdA2, 3);
        unsafe(LoadEnemyShotData, 1);
        unsafe(ReloadEnemyShotData, 1);

        unsafe(DeleteShotAll, 2); // notifyEvent
        unsafe(DeleteShotInCircle, 5); // notifyEvent
        safe(CreateShotA1, 6);
        safe(CreateShotA2, 8);
        safe(CreateShotOA1, 5);
        safe(CreateShotB1, 6);
        safe(CreateShotB2, 10);
        safe(CreateShotOB1, 5);
        safe(CreateLooseLaserA1, 8);
        safe(CreateStraightLaserA1, 8);
        safe(CreateCurveLaserA1, 8);
        safe(SetShotIntersectionCircle, 3);
        safe(SetShotIntersectionLine, 5);
        safe(GetShotIdInCircleA1, 3);
        safe(GetShotIdInCircleA2, 4);
        safe(GetShotCount, 1);
        safe(SetShotAutoDeleteClip, 4);
        safe(GetShotDataInfoA1, 3);
        unsafe(StartShotScript, 1);

        safe(CreateItemA1, 4);
        safe(CreateItemA2, 6);
        safe(CreateItemU1, 4);
        safe(CreateItemU2, 6);
        safe(CollectAllItems, 0);
        safe(CollectItemsByType, 1);
        safe(CollectItemsInCircle, 3);
        safe(CancelCollectItems, 0);
        unsafe(StartItemScript, 1);
        safe(SetDefaultBonusItemEnable, 1);
        unsafe(LoadItemData, 1);
        unsafe(ReloadItemData, 1);
        safe(StartSlow, 2);
        safe(StopSlow, 1);
        safe(IsIntersected_Line_Circle, 8);
        safe(IsIntersected_Obj_Obj, 2);
    }

    safe(GetObjectDistance, 2);
    safe(GetObject2dPosition, 1);
    safe(Get2dPosition, 3);

    safe(Obj_Delete, 1);
    safe(Obj_IsDeleted, 1);
    safe(Obj_SetVisible, 2);
    safe(Obj_IsVisible, 1);
    safe(Obj_SetRenderPriority, 2);
    safe(Obj_SetRenderPriorityI, 2);
    safe(Obj_GetRenderPriority, 1);
    safe(Obj_GetRenderPriorityI, 1);
    safe(Obj_GetValue, 2);
    safe(Obj_GetValueD, 3);
    safe(Obj_SetValue, 3);
    safe(Obj_DeleteValue, 2);
    safe(Obj_IsValueExists, 2);
    safe(Obj_GetType, 1);

    safe(ObjRender_SetX, 2);
    safe(ObjRender_SetY, 2);
    safe(ObjRender_SetZ, 2);
    safe(ObjRender_SetPosition, 4);
    safe(ObjRender_SetAngleX, 2);
    safe(ObjRender_SetAngleY, 2);
    safe(ObjRender_SetAngleZ, 2);
    safe(ObjRender_SetAngleXYZ, 4);
    safe(ObjRender_SetScaleX, 2);
    safe(ObjRender_SetScaleY, 2);
    safe(ObjRender_SetScaleZ, 2);
    safe(ObjRender_SetScaleXYZ, 4);
    safe(ObjRender_SetColor, 4);
    safe(ObjRender_SetColorHSV, 4);
    safe(ObjRender_SetAlpha, 2);
    safe(ObjRender_SetBlendType, 2);

    safe(ObjRender_GetX, 1);
    safe(ObjRender_GetY, 1);
    safe(ObjRender_GetZ, 1);
    safe(ObjRender_GetAngleX, 1);
    safe(ObjRender_GetAngleY, 1);
    safe(ObjRender_GetAngleZ, 1);
    safe(ObjRender_GetScaleX, 1);
    safe(ObjRender_GetScaleY, 1);
    safe(ObjRender_GetScaleZ, 1);
    safe(ObjRender_GetBlendType, 1);

    safe(ObjRender_SetZWrite, 2);
    safe(ObjRender_SetZTest, 2);
    safe(ObjRender_SetFogEnable, 2);
    safe(ObjRender_SetPermitCamera, 2);
    safe(ObjRender_SetCullingMode, 2);

    safe(ObjPrim_Create, 1);
    safe(ObjPrim_SetPrimitiveType, 2);
    safe(ObjPrim_SetVertexCount, 2);
    safe(ObjPrim_GetVertexCount, 1);
    unsafe(ObjPrim_SetTexture, 2);
    safe(ObjPrim_SetVertexPosition, 5);
    safe(ObjPrim_GetVertexPosition, 2);
    safe(ObjPrim_SetVertexUV, 4);
    safe(ObjPrim_SetVertexUVT, 4);
    safe(ObjPrim_SetVertexColor, 5);
    safe(ObjPrim_SetVertexAlpha, 3);

    safe(ObjSprite2D_SetSourceRect, 5);
    safe(ObjSprite2D_SetDestRect, 5);
    safe(ObjSprite2D_SetDestCenter, 1);

    safe(ObjSpriteList2D_SetSourceRect, 5);
    safe(ObjSpriteList2D_SetDestRect, 5);
    safe(ObjSpriteList2D_SetDestCenter, 1);
    safe(ObjSpriteList2D_AddVertex, 1);
    safe(ObjSpriteList2D_CloseVertex, 1);
    safe(ObjSpriteList2D_ClearVertexCount, 1);

    safe(ObjSprite3D_SetSourceRect, 5);
    safe(ObjSprite3D_SetDestRect, 5);
    safe(ObjSprite3D_SetSourceDestRect, 5);
    safe(ObjSprite3D_SetBillboard, 2);

    safe(ObjTrajectory3D_SetComplementCount, 2);
    safe(ObjTrajectory3D_SetAlphaVariation, 2);
    safe(ObjTrajectory3D_SetInitialPoint, 7);

    safe(ObjMesh_Create, 0);
    unsafe(ObjMesh_Load, 2);
    safe(ObjMesh_SetColor, 4);
    safe(ObjMesh_SetAlpha, 2);
    safe(ObjMesh_SetAnimation, 3);
    unsafe(ObjMesh_SetCoordinate2D, 2);
    safe(ObjMesh_GetPath, 1);

    safe(ObjText_Create, 0);
    safe(ObjText_SetText, 2);
    safe(ObjText_SetFontType, 2);
    safe(ObjText_SetFontSize, 2);
    safe(ObjText_SetFontBold, 2);
    safe(ObjText_SetFontColorTop, 4);
    safe(ObjText_SetFontColorBottom, 4);
    safe(ObjText_SetFontBorderWidth, 2);
    safe(ObjText_SetFontBorderType, 2);
    safe(ObjText_SetFontBorderColor, 4);

    safe(ObjText_SetMaxWidth, 2);
    safe(ObjText_SetMaxHeight, 2);
    safe(ObjText_SetLinePitch, 2);
    safe(ObjText_SetSidePitch, 2);
    safe(ObjText_SetTransCenter, 3);
    safe(ObjText_SetAutoTransCenter, 2);
    safe(ObjText_SetHorizontalAlignment, 2);
    safe(ObjText_SetSyntacticAnalysis, 2);
    safe(ObjText_GetTextLength, 1);
    safe(ObjText_GetTextLengthCU, 1);
    safe(ObjText_GetTextLengthCUL, 1);
    unsafe(ObjText_GetTotalWidth, 1);
    unsafe(ObjText_GetTotalHeight, 1);

    safe(ObjShader_Create, 0);
    unsafe(ObjShader_SetShaderF, 2);
    safe(ObjShader_SetShaderO, 2);
    safe(ObjShader_ResetShader, 1);
    safe(ObjShader_SetTechnique, 2);
    safe(ObjShader_SetVector, 6);
    safe(ObjShader_SetFloat, 3);
    safe(ObjShader_SetFloatArray, 3);
    safe(ObjShader_SetTexture, 3);

    safe(ObjSound_Create, 0);
    unsafe(ObjSound_Load, 2);
    safe(ObjSound_Play, 1);
    safe(ObjSound_Stop, 1);
    safe(ObjSound_SetVolumeRate, 2);
    safe(ObjSound_SetPanRate, 2);
    safe(ObjSound_SetFade, 2);
    safe(ObjSound_SetLoopEnable, 2);
    safe(ObjSound_SetLoopTime, 3);
    safe(ObjSound_SetLoopSampleCount, 3);
    safe(ObjSound_SetRestartEnable, 2);
    safe(ObjSound_SetSoundDivision, 2);
    safe(ObjSound_IsPlaying, 1);
    safe(ObjSound_GetVolumeRate, 1);

    safe(ObjFile_Create, 1);
    unsafe(ObjFile_Open, 2);
    unsafe(ObjFile_OpenNW, 2);
    unsafe(ObjFile_Store, 1);
    safe(ObjFile_GetSize, 1);

    safe(ObjFileT_GetLineCount, 1);
    safe(ObjFileT_GetLineText, 2);
    safe(ObjFileT_SplitLineText, 3);
    safe(ObjFileT_AddLine, 2);
    safe(ObjFileT_ClearLine, 1);

    safe(ObjFileB_SetByteOrder, 2);
    safe(ObjFileB_SetCharacterCode, 2);
    safe(ObjFileB_GetPointer, 1);
    safe(ObjFileB_Seek, 2);
    safe(ObjFileB_ReadBoolean, 1);
    safe(ObjFileB_ReadByte, 1);
    safe(ObjFileB_ReadShort, 1);
    safe(ObjFileB_ReadInteger, 1);
    safe(ObjFileB_ReadLong, 1);
    safe(ObjFileB_ReadFloat, 1);
    safe(ObjFileB_ReadDouble, 1);
    safe(ObjFileB_ReadString, 2);

    if (TypeIs(~t_package))
    {
        safe(ObjMove_SetX, 2);
        safe(ObjMove_SetY, 2);
        safe(ObjMove_SetPosition, 3);
        safe(ObjMove_SetSpeed, 2);
        safe(ObjMove_SetAngle, 2);
        safe(ObjMove_SetAcceleration, 2);
        safe(ObjMove_SetMaxSpeed, 2);
        safe(ObjMove_SetAngularVelocity, 2);

        safe(ObjMove_SetDestAtSpeed, 4);
        safe(ObjMove_SetDestAtFrame, 4);
        safe(ObjMove_SetDestAtWeight, 5);

        safe(ObjMove_AddPatternA1, 4);
        safe(ObjMove_AddPatternA2, 7);
        safe(ObjMove_AddPatternA3, 8);
        safe(ObjMove_AddPatternA4, 9);
        safe(ObjMove_AddPatternB1, 4);
        safe(ObjMove_AddPatternB2, 8);
        safe(ObjMove_AddPatternB3, 9);

        safe(ObjMove_GetX, 1);
        safe(ObjMove_GetY, 1);
        safe(ObjMove_GetSpeed, 1);
        safe(ObjMove_GetAngle, 1);

        safe(ObjEnemy_Create, 1);
        safe(ObjEnemy_Regist, 1);
        safe(ObjEnemy_GetInfo, 2);
        safe(ObjEnemy_SetLife, 2);
        safe(ObjEnemy_AddLife, 2);
        safe(ObjEnemy_SetDamageRate, 3);
        safe(ObjEnemy_SetIntersectionCircleToShot, 4);
        safe(ObjEnemy_SetIntersectionCircleToPlayer, 4);

        safe(ObjEnemyBossScene_Create, 0);
        unsafe(ObjEnemyBossScene_Regist, 1);
        safe(ObjEnemyBossScene_Add, 3);
        unsafe(ObjEnemyBossScene_LoadInThread, 1);
        safe(ObjEnemyBossScene_GetInfo, 2);
        safe(ObjEnemyBossScene_SetSpellTimer, 2);
        unsafe(ObjEnemyBossScene_StartSpell, 1); // notifyEvent

        safe(ObjShot_Create, 1);
        safe(ObjShot_Regist, 1);
        safe(ObjShot_SetAutoDelete, 2);
        safe(ObjShot_FadeDelete, 1);
        safe(ObjShot_SetDeleteFrame, 2);
        safe(ObjShot_SetDamage, 2);
        safe(ObjShot_SetDelay, 2);
        safe(ObjShot_SetSpellResist, 2);
        safe(ObjShot_SetGraphic, 2);
        safe(ObjShot_SetSourceBlendType, 2);
        safe(ObjShot_SetPenetration, 2);
        safe(ObjShot_SetEraseShot, 2);
        safe(ObjShot_SetSpellFactor, 2);
        unsafe(ObjShot_ToItem, 1); // notifyEvent
        safe(ObjShot_AddShotA1, 3);
        safe(ObjShot_AddShotA2, 5);
        safe(ObjShot_SetIntersectionEnable, 2);
        safe(ObjShot_SetIntersectionCircleA1, 2);
        safe(ObjShot_SetIntersectionCircleA2, 4);
        safe(ObjShot_SetIntersectionLine, 6);
        safe(ObjShot_SetItemChange, 2);
        safe(ObjShot_GetDamage, 1);
        safe(ObjShot_GetPenetration, 1);
        safe(ObjShot_GetDelay, 1);
        safe(ObjShot_IsSpellResist, 1);
        safe(ObjShot_GetImageID, 1);

        safe(ObjLaser_SetLength, 2);
        safe(ObjLaser_SetRenderWidth, 2);
        safe(ObjLaser_SetIntersectionWidth, 2);
        safe(ObjLaser_SetGrazeInvalidFrame, 2);
        safe(ObjLaser_SetInvalidLength, 3);
        safe(ObjLaser_SetItemDistance, 2);
        safe(ObjLaser_GetLength, 1);

        safe(ObjStLaser_SetAngle, 2);
        safe(ObjStLaser_GetAngle, 1);
        safe(ObjStLaser_SetSource, 2);

        safe(ObjCrLaser_SetTipDecrement, 2);

        safe(ObjItem_SetItemID, 2);
        safe(ObjItem_SetRenderScoreEnable, 2);
        safe(ObjItem_SetAutoCollectEnable, 2);
        safe(ObjItem_SetDefinedMovePatternA1, 2);
        safe(ObjItem_GetInfo, 2);

        safe(ObjPlayer_AddIntersectionCircleA1, 5);
        safe(ObjPlayer_AddIntersectionCircleA2, 4);
        safe(ObjPlayer_ClearIntersection, 1);

        safe(ObjCol_IsIntersected, 1);
        safe(ObjCol_GetListOfIntersectedEnemyID, 1);
        safe(ObjCol_GetIntersectedCount, 1);
    }

    if (TypeIs(t_player))
    {
        safe(CreatePlayerShotA1, 7);
        unsafe(CallSpell, 0); // notifyEvent
        unsafe(LoadPlayerShotData, 1);
        unsafe(ReloadPlayerShotData, 1);
        safe(GetSpellManageObject, 0);

        safe(ObjSpell_Create, 0);
        safe(ObjSpell_Regist, 1);
        safe(ObjSpell_SetDamage, 2);
        safe(ObjSpell_SetEraseShot, 2);
        safe(ObjSpell_SetIntersectionCircle, 4);
        safe(ObjSpell_SetIntersectionLine, 6);
    }

    safe(SetPauseScriptPath, 1);
    safe(SetEndSceneScriptPath, 1);
    safe(SetReplaySaveSceneScriptPath, 1);
    safe(GetTransitionRenderTargetName, 0);

    if (TypeIs(t_shot_custom))
    {
        safe(SetShotDeleteEventEnable, 2);
    }

    if (TypeIs(t_package))
    {
        safe(ClosePackage, 0);
        unsafe(InitializeStageScene, 0);
        unsafe(FinalizeStageScene, 0);
        unsafe(StartStageScene, 0);
        safe(SetStageIndex, 1);
        safe(SetStageMainScript, 1);
        safe(SetStagePlayerScript, 1);
        safe(SetStageReplayFile, 1);
        safe(GetStageSceneState, 0);
        safe(GetStageSceneResult, 0);
        unsafe(PauseStageScene, 1); // notifyEvent
        safe(TerminateStageScene, 0);

        safe(GetLoadFreePlayerScriptList, 0);
        safe(GetFreePlayerScriptCount, 0);
        safe(GetFreePlayerScriptInfo, 2);

        unsafe(LoadReplayList, 0);
        unsafe(GetValidReplayIndices, 0);
        unsafe(IsValidReplayIndex, 1);
        unsafe(GetReplayInfo, 2);
        unsafe(SetReplayInfo, 2);
        unsafe(SaveReplay, 2);
    }
}
}