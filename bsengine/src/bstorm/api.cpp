﻿#include <bstorm/api.hpp>

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
    DnhArray(L"./").Push(L);
    return 1;
}

static int GetMainStgScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetMainStgScriptPath()).Push(L);
    return 1;
}

static int GetMainStgScriptDirectory(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetMainStgScriptDirectory()).Push(L);
    return 1;
}

static int GetMainPackageScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetMainPackageScriptPath()).Push(L);
    return 1;
}

static int GetScriptPathList(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto dirPath = DnhValue::ToString(L, 1);
    int scriptType = DnhValue::ToInt(L, 2);
    auto scriptList = engine->GetScriptList(dirPath, scriptType, false);
    DnhArray pathList(scriptList.size());
    for (const auto& info : scriptList)
    {
        pathList.PushBack(std::make_unique<DnhArray>(info.path));
    }
    pathList.Push(L);
    return 1;
}

static int GetCurrentDateTimeS(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetCurrentDateTimeS()).Push(L);
    return 1;
}

static int GetStageTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStageTime());
    return 1;
}

static int GetPackageTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetPackageTime());
    return 1;
}

static int GetCurrentFps(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetCurrentFps());
    return 1;
}

// API
static int InstallFont(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    lua_pushboolean(L, engine->InstallFont(path, GetSourcePos(L)));
    return 1;
}

static int ToString(lua_State* L)
{
    auto str = DnhValue::ToString(L, 1);
    DnhArray(str).Push(L);
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
    std::string msg = DnhValue::ToStringU8(L, 1);
    engine->WriteLog(std::move(msg), GetSourcePos(L));
    return 0;
}

static int RaiseError(lua_State* L)
{
    std::string msg = DnhValue::ToStringU8(L, 1);
    throw Log(Log::Level::LV_ERROR)
        .SetMessage("RaiseError.")
        .SetParam(Log::Param(Log::Param::Tag::TEXT, std::move(msg)));
    return 0;
}

static int assert(lua_State* L)
{
    bool cond = DnhValue::ToBool(L, 1);
    std::string msg = DnhValue::ToStringU8(L, 2);
    if (!cond)
    {
        throw Log(Log::Level::LV_ERROR)
            .SetMessage("assertion failed.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, std::move(msg)));
    }
    return 0;
}

static int SetCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::ToString(L, 1);
    auto value = DnhValue::Get(L, 2);
    engine->SetCommonData(key, std::move(value));
    return 0;
}

static int GetCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::ToString(L, 1);
    auto defaultValue = DnhValue::Get(L, 2);
    engine->GetCommonData(key, defaultValue)->Push(L);
    return 1;
}

static int ClearCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->ClearCommonData();
    return 0;
}

static int DeleteCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring key = DnhValue::ToString(L, 1);
    engine->DeleteCommonData(key);
    return 0;
}

static int SetAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    auto value = DnhValue::Get(L, 3);
    engine->SetAreaCommonData(area, key, std::move(value));
    return 0;
}

static int GetAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    auto defaultValue = DnhValue::Get(L, 3);
    engine->GetAreaCommonData(area, key, defaultValue)->Push(L);
    return 1;
}

static int ClearAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    engine->ClearAreaCommonData(area);
    return 0;
}

static int DeleteAreaCommonData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    engine->DeleteAreaCommonData(area, key);
    return 0;
}

static int CreateCommonDataArea(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    engine->CreateCommonDataArea(area);
    return 0;
}

static int IsCommonDataAreaExists(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    lua_pushboolean(L, engine->IsCommonDataAreaExists(area));
    return 1;
}

static int CopyCommonDataArea(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring dst = DnhValue::ToString(L, 1);
    std::wstring src = DnhValue::ToString(L, 2);
    engine->CopyCommonDataArea(dst, src);
    return 0;
}

static int GetCommonDataAreaKeyList(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto keyList = engine->GetCommonDataAreaKeyList();
    DnhArray ret(keyList.size());
    for (const auto& key : keyList)
    {
        ret.PushBack(std::make_unique<DnhArray>(key));
    }
    ret.Push(L);
    return 1;
}

static int GetCommonDataValueKeyList(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    DnhArray ret;
    for (const auto& key : engine->GetCommonDataValueKeyList(area))
    {
        ret.PushBack(std::make_unique<DnhArray>(key));
    }
    ret.Push(L);
    return 1;
}

static int SaveCommonDataAreaA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    lua_pushboolean(L, engine->SaveCommonDataAreaA1(area));
    return 1;
}

static int LoadCommonDataAreaA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    lua_pushboolean(L, engine->LoadCommonDataAreaA1(area));
    return 1;
}

static int SaveCommonDataAreaA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    std::wstring saveFilePath = DnhValue::ToString(L, 2);
    lua_pushboolean(L, engine->SaveCommonDataAreaA2(area, saveFilePath));
    return 1;
}

static int LoadCommonDataAreaA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring area = DnhValue::ToString(L, 1);
    std::wstring saveFilePath = DnhValue::ToString(L, 2);
    lua_pushboolean(L, engine->LoadCommonDataAreaA2(area, saveFilePath));
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
    auto path = DnhValue::ToString(L, 1);
    engine->LoadOrphanSound(path, GetSourcePos(L));
    return 0;
}

static int RemoveSound(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->RemoveOrphanSound(path);
    return 0;
}

static int PlayBGM(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    double loopStartSec = DnhValue::ToNum(L, 2);
    double loopEndSec = DnhValue::ToNum(L, 3);
    engine->PlayBGM(path, loopStartSec, loopEndSec);
    return 0;
}

static int PlaySE(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->PlaySE(path);
    return 0;
}

static int StopSound(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->StopOrphanSound(path);
    return 0;
}

static int GetVirtualKeyState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::ToInt(L, 1);
    lua_pushnumber(L, engine->GetVirtualKeyState(vk));
    return 1;
}

static int SetVirtualKeyState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::ToInt(L, 1);
    int state = DnhValue::ToInt(L, 2);
    engine->SetVirtualKeyState(vk, state);
    return 0;
}

static int AddVirtualKey(lua_State* L)
{
    Engine* engine = getEngine(L);
    int vk = DnhValue::ToInt(L, 1);
    int k = DnhValue::ToInt(L, 2);
    int btn = DnhValue::ToInt(L, 3);
    engine->AddVirtualKey(vk, k, btn);
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
    int k = DnhValue::ToInt(L, 1);
    lua_pushnumber(L, engine->GetKeyState(k));
    return 1;
}

static int GetMouseState(lua_State* L)
{
    Engine* engine = getEngine(L);
    int btn = DnhValue::ToInt(L, 1);
    lua_pushnumber(L, engine->GetMouseState(btn));
    return 1;
}

static int GetMouseX(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetMouseX());
    return 1;
}

static int GetMouseY(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetMouseY());
    return 1;
}

static int GetMouseMoveZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetMouseMoveZ());
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
    std::wstring name = DnhValue::ToString(L, 1);
    try
    {
        engine->CreateRenderTarget(name, 1024, 512, GetSourcePos(L));
        lua_pushboolean(L, true);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN)
            .AddSourcePos(GetSourcePos(L));
        Logger::WriteLog(log);
        lua_pushboolean(L, false);
    }
    return 1;
}

static int RenderToTextureA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    int begin = DnhValue::ToInt(L, 2);
    int end = DnhValue::ToInt(L, 3);
    bool doClear = DnhValue::ToBool(L, 4);
    engine->RenderToTextureA1(name, begin, end, doClear);
    return 0;
}

static int RenderToTextureB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    int objId = DnhValue::ToInt(L, 2);
    bool doClear = DnhValue::ToBool(L, 3);
    engine->RenderToTextureB1(name, objId, doClear);
    return 0;
}

static int SaveRenderedTextureA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    auto path = DnhValue::ToString(L, 2);
    engine->SaveRenderedTextureA1(name, path, GetSourcePos(L));
    return 0;
}

static int SaveRenderedTextureA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    auto path = DnhValue::ToString(L, 2);
    int l = DnhValue::ToInt(L, 3);
    int t = DnhValue::ToInt(L, 4);
    int r = DnhValue::ToInt(L, 5);
    int b = DnhValue::ToInt(L, 6);
    engine->SaveRenderedTextureA2(name, path, l, t, r, b, GetSourcePos(L));
    return 0;
}

static int SaveSnapShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SaveSnapShotA1(path, GetSourcePos(L));
    return 0;
}

static int SaveSnapShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    int l = DnhValue::ToInt(L, 2);
    int t = DnhValue::ToInt(L, 3);
    int r = DnhValue::ToInt(L, 4);
    int b = DnhValue::ToInt(L, 5);
    engine->SaveSnapShotA2(path, l, t, r, b, GetSourcePos(L));
    return 0;
}

static int IsPixelShaderSupported(lua_State* L)
{
    Engine* engine = getEngine(L);
    int major = DnhValue::ToInt(L, 1);
    int minor = DnhValue::ToInt(L, 2);
    lua_pushboolean(L, engine->IsPixelShaderSupported(major, minor));
    return 1;
}

static int SetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double begin = DnhValue::ToNum(L, 2);
    double end = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        engine->SetShader((int)(begin * MAX_RENDER_PRIORITY), (int)(end * MAX_RENDER_PRIORITY), obj->GetShader());
    }
    return 0;
}

static int SetShaderI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int begin = DnhValue::ToInt(L, 2);
    int end = DnhValue::ToInt(L, 3);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        engine->SetShader(begin, end, obj->GetShader());
    }
    return 0;
}

static int ResetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    double begin = DnhValue::ToNum(L, 1);
    double end = DnhValue::ToNum(L, 2);
    engine->ResetShader((int)(begin * MAX_RENDER_PRIORITY), (int)(end * MAX_RENDER_PRIORITY));
    return 0;
}

static int ResetShaderI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int begin = DnhValue::ToInt(L, 1);
    int end = DnhValue::ToInt(L, 2);
    engine->ResetShader(begin, end);
    return 0;
}

static int LoadTextureInLoadThread(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->LoadTextureInThread(path, true, GetSourcePos(L));
    return 0;
}

static int LoadTexture(lua_State* L)
{
    return LoadTextureInLoadThread(L);
}

static int RemoveTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->RemoveTextureReservedFlag(path);
    return 0;
}

static int GetTextureWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    if (auto target = engine->GetRenderTarget(name))
    {
        lua_pushnumber(L, target->GetWidth());
    } else
    {
        try
        {
            lua_pushnumber(L, engine->LoadTexture(name, false, GetSourcePos(L))->GetWidth());
        } catch (Log& log)
        {
            log.SetLevel(Log::Level::LV_WARN)
                .AddSourcePos(GetSourcePos(L));
            Logger::WriteLog(log);
            lua_pushnumber(L, 0);
        }
    }
    return 1;
}

static int GetTextureHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring name = DnhValue::ToString(L, 1);
    if (auto target = engine->GetRenderTarget(name))
    {
        lua_pushnumber(L, target->GetHeight());
    } else
    {
        try
        {
            lua_pushnumber(L, engine->LoadTexture(name, false, GetSourcePos(L))->GetHeight());
        } catch (Log& log)
        {
            log.SetLevel(Log::Level::LV_WARN)
                .AddSourcePos(GetSourcePos(L));
            Logger::WriteLog(log);
            lua_pushnumber(L, 0);
        }
    }
    return 1;
}

static int SetFogEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool enable = DnhValue::ToBool(L, 1);
    engine->SetFogEnable(enable);
    return 0;
}

static int SetFogParam(lua_State* L)
{
    Engine* engine = getEngine(L);
    double fogStart = DnhValue::ToNum(L, 1);
    double fogEnd = DnhValue::ToNum(L, 2);
    int r = DnhValue::ToInt(L, 3);
    int g = DnhValue::ToInt(L, 4);
    int b = DnhValue::ToInt(L, 5);
    engine->SetFogParam(fogStart, fogEnd, r, g, b);
    return 0;
}

static int ClearInvalidRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->ClearInvalidRenderPriority();
    return 0;
}

static int SetInvalidRenderPriorityA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int begin = DnhValue::ToInt(L, 1);
    int end = DnhValue::ToInt(L, 2);
    engine->SetInvalidRenderPriority(begin, end);
    return 0;
}

static int GetReservedRenderTargetName(lua_State* L)
{
    Engine* engine = getEngine(L);
    int n = DnhValue::ToInt(L, 1);
    DnhArray(engine->GetReservedRenderTargetName(n)).Push(L);
    return 1;
}

template <void (Engine::*func)(float)>
static int SetCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    double v = DnhValue::ToNum(L, 1);
    (engine->*func)(v);
    return 0;
}

static int SetCameraFocusX(lua_State* L) { return SetCamera<&Engine::SetCameraFocusX>(L); }
static int SetCameraFocusY(lua_State* L) { return SetCamera<&Engine::SetCameraFocusY>(L); }
static int SetCameraFocusZ(lua_State* L) { return SetCamera<&Engine::SetCameraFocusZ>(L); }

static int SetCameraFocusXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double z = DnhValue::ToNum(L, 3);
    engine->SetCameraFocusXYZ(x, y, z);
    return 0;
}

static int SetCameraRadius(lua_State* L) { return SetCamera<&Engine::SetCameraRadius>(L); }
static int SetCameraAzimuthAngle(lua_State* L) { return SetCamera<&Engine::SetCameraAzimuthAngle>(L); }
static int SetCameraElevationAngle(lua_State* L) { return SetCamera<&Engine::SetCameraElevationAngle>(L); }
static int SetCameraYaw(lua_State* L) { return SetCamera<&Engine::SetCameraYaw>(L); }
static int SetCameraPitch(lua_State* L) { return SetCamera<&Engine::SetCameraPitch>(L); }
static int SetCameraRoll(lua_State* L) { return SetCamera<&Engine::SetCameraRoll>(L); }

template <float (Engine::*func)() const>
static int GetCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, (engine->*func)());
    return 1;
}

static int GetCameraX(lua_State* L) { return GetCamera<&Engine::GetCameraX>(L); }
static int GetCameraY(lua_State* L) { return GetCamera<&Engine::GetCameraY>(L); }
static int GetCameraZ(lua_State* L) { return GetCamera<&Engine::GetCameraZ>(L); }
static int GetCameraFocusX(lua_State* L) { return GetCamera<&Engine::GetCameraFocusX>(L); }
static int GetCameraFocusY(lua_State* L) { return GetCamera<&Engine::GetCameraFocusY>(L); }
static int GetCameraFocusZ(lua_State* L) { return GetCamera<&Engine::GetCameraFocusZ>(L); }
static int GetCameraRadius(lua_State* L) { return GetCamera<&Engine::GetCameraRadius>(L); }
static int GetCameraAzimuthAngle(lua_State* L) { return GetCamera<&Engine::GetCameraAzimuthAngle>(L); }
static int GetCameraElevationAngle(lua_State* L) { return GetCamera<&Engine::GetCameraElevationAngle>(L); }
static int GetCameraYaw(lua_State* L) { return GetCamera<&Engine::GetCameraYaw>(L); }
static int GetCameraPitch(lua_State* L) { return GetCamera<&Engine::GetCameraPitch>(L); }
static int GetCameraRoll(lua_State* L) { return GetCamera<&Engine::GetCameraRoll>(L); }

static int SetCameraPerspectiveClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double n = DnhValue::ToNum(L, 1);
    double f = DnhValue::ToNum(L, 2);
    engine->SetCameraPerspectiveClip(n, f);
    return 0;
}

template <void (Engine::*func)(float)>
static int Set2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    float v = DnhValue::ToNum(L, 1);
    (engine->*func)(v);
    return 0;
}

static int Set2DCameraFocusX(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraFocusX>(L); }
static int Set2DCameraFocusY(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraFocusY>(L); }
static int Set2DCameraAngleZ(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraAngleZ>(L); }
static int Set2DCameraRatio(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraRatio>(L); }
static int Set2DCameraRatioX(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraRatioX>(L); }
static int Set2DCameraRatioY(lua_State* L) { return Set2DCamera<&Engine::Set2DCameraRatioY>(L); }

static int Reset2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->Reset2DCamera();
    return 0;
}

template <float (Engine::*func)() const>
static int Get2DCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, (engine->*func)());
    return 1;
}

static int Get2DCameraX(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraX>(L); }
static int Get2DCameraY(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraY>(L); }
static int Get2DCameraAngleZ(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraAngleZ>(L); }
static int Get2DCameraRatio(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraRatio>(L); }
static int Get2DCameraRatioX(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraRatioX>(L); }
static int Get2DCameraRatioY(lua_State* L) { return Get2DCamera<&Engine::Get2DCameraRatioY>(L); }

static int LoadScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    if (std::shared_ptr<Script> script = engine->LoadScript(path, GetScript(L)->GetType(), SCRIPT_VERSION_PH3, GetSourcePos(L)))
    {
        lua_pushnumber(L, (double)script->GetID());
    }
    return 1;
}

static int LoadScriptInThread(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    if (std::shared_ptr<Script> script = engine->LoadScriptInThread(path, GetScript(L)->GetType(), SCRIPT_VERSION_PH3, GetSourcePos(L)))
    {
        lua_pushnumber(L, (double)script->GetID());
    }
    return 1;
}

static int StartScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    if (auto script = engine->GetScript(scriptId))
    {
        script->Start();
        script->RunInitialize();
    }
    return 0;
}

static int CloseScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    if (auto script = engine->GetScript(scriptId))
    {
        script->Close();
    }
    return 0;
}

static int IsCloseScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    if (auto script = engine->GetScript(scriptId))
    {
        lua_pushboolean(L, script->IsClosed());
    } else
    {
        lua_pushboolean(L, true);
    }
    return 1;
}

static int SetScriptArgument(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    int idx = DnhValue::ToInt(L, 2);
    auto value = DnhValue::Get(L, 3);
    if (auto script = engine->GetScript(scriptId))
    {
        script->SetScriptArgument(idx, std::move(value));
    }
    return 0;
}

static int GetScriptArgument(lua_State* L)
{
    Script* script = GetScript(L);
    int idx = DnhValue::ToInt(L, 1);
    script->GetScriptArgument(idx)->Push(L);
    return 1;
}

static int GetScriptArgumentCount(lua_State* L)
{
    Script* script = GetScript(L);
    lua_pushnumber(L, script->GetScriptArgumentCount());
    return 1;
}

static int CloseStgScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->CloseStgScene();
    return 0;
}

static int SetStgFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int l = DnhValue::ToInt(L, 1);
    int t = DnhValue::ToInt(L, 2);
    int r = DnhValue::ToInt(L, 3);
    int b = DnhValue::ToInt(L, 4);
    int priorityMin = DnhValue::ToInt(L, 5);
    int priorityMax = DnhValue::ToInt(L, 6);
    engine->SetStgFrame(l, t, r, b);
    engine->Reset2DCamera();
    engine->Set2DCameraFocusX(engine->GetStgFrameCenterWorldX());
    engine->Set2DCameraFocusY(engine->GetStgFrameCenterWorldY());
    engine->SetStgFrameRenderPriorityMin(priorityMin);
    engine->SetStgFrameRenderPriorityMax(priorityMax);
    return 0;
}

static int GetScore(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetScore());
    return 1;
}

static int AddScore(lua_State* L)
{
    Engine* engine = getEngine(L);
    PlayerScore score = DnhValue::ToNum(L, 1);
    engine->AddScore(score);
    return 0;
}

static int GetGraze(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetGraze());
    return 1;
}

static int AddGraze(lua_State* L)
{
    Engine* engine = getEngine(L);
    int64_t graze = DnhValue::ToNum(L, 1);
    engine->AddGraze(graze);
    return 0;
}

static int GetPoint(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetPoint());
    return 1;
}

static int AddPoint(lua_State* L)
{
    Engine* engine = getEngine(L);
    int64_t point = DnhValue::ToNum(L, 1);
    engine->AddPoint(point);
    return 0;
}

static int SetItemRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    int p = DnhValue::ToInt(L, 1);
    engine->SetItemRenderPriority(p);
    return 0;
}

static int SetShotRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    int p = DnhValue::ToInt(L, 1);
    engine->SetShotRenderPriority(p);
    return 0;
}

static int GetStgFrameRenderPriorityMinI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameRenderPriorityMin());
    return 1;
}

static int GetStgFrameRenderPriorityMaxI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameRenderPriorityMax());
    return 1;
}

static int GetItemRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetItemRenderPriority());
    return 1;
}

static int GetShotRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetShotRenderPriority());
    return 1;
}

static int GetPlayerRenderPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetPlayerRenderPriority());
    return 1;
}

static int GetCameraFocusPermitPriorityI(lua_State *L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetCameraFocusPermitRenderPriority());
    return 1;
}

static int GetStgFrameLeft(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameLeft());
    return 1;
}

static int GetStgFrameTop(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameTop());
    return 1;
}

static int GetStgFrameWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameWidth());
    return 1;
}

static int GetStgFrameHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStgFrameHeight());
    return 1;
}

static int GetScreenWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetScreenWidth());
    return 1;
}

static int GetScreenHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetScreenHeight());
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
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, player ? player->GetID() : ID_INVALID);
    return 1;
}

static int GetPlayerScriptID(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto playerScript = engine->GetPlayerScript())
    {
        lua_pushnumber(L, playerScript->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int SetPlayerSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    double normalSpeed = DnhValue::ToNum(L, 1);
    double slowSpeed = DnhValue::ToNum(L, 2);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetNormalSpeed(normalSpeed);
        player->SetSlowSpeed(slowSpeed);
    }
    return 0;
}

static int SetPlayerClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double left = DnhValue::ToNum(L, 1);
    double top = DnhValue::ToNum(L, 2);
    double right = DnhValue::ToNum(L, 3);
    double bottom = DnhValue::ToNum(L, 4);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetClip(left, top, right, bottom);
    }
    return 0;
}


static int SetPlayerLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    double life = DnhValue::ToNum(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetLife(life);
    }
    return 0;
}

static int SetPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    double spell = DnhValue::ToNum(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetSpell(spell);
    }
    return 0;
}

static int SetPlayerPower(lua_State* L)
{
    Engine* engine = getEngine(L);
    double power = DnhValue::ToNum(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetPower(power);
    }
    return 0;
}

static int SetPlayerInvincibilityFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::ToInt(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetInvincibilityFrame(frame);
    }
    return 0;
}

static int SetPlayerDownStateFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::ToInt(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetDownStateFrame(frame);
    }
    return 0;
}

static int SetPlayerRebirthFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::ToInt(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetRebirthFrame(frame);
    }
    return 0;
}

static int SetPlayerRebirthLossFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int frame = DnhValue::ToInt(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetRebirthLossFrame(frame);
    }
    return 0;
}

static int SetPlayerAutoItemCollectLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    double lineY = DnhValue::ToNum(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetAutoItemCollectLineY(lineY);
    }
    return 0;
}

static int SetForbidPlayerShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool forbid = DnhValue::ToBool(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetForbidPlayerShot(forbid);
    }
    return 0;
}

static int SetForbidPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool forbid = DnhValue::ToBool(L, 1);
    if (auto player = engine->GetPlayerObject())
    {
        player->SetForbidPlayerSpell(forbid);
    }
    return 0;
}

static int GetPlayerState(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->GetPlayerObject())
    {
        lua_pushnumber(L, (double)player->GetState());
    } else
    {
        lua_pushnumber(L, (double)STATE_END);
    }
    return 1;
}

static int GetPlayerSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->GetPlayerObject())
    {
        Point2D speeds{ player->GetNormalSpeed(), player->GetSlowSpeed() };
        DnhArray(speeds).Push(L);
    } else
    {
        DnhArray(Point2D(0.0f, 0.0f)).Push(L);
    }
    return 1;
}

static int GetPlayerClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    std::vector<double> clipRect;
    clipRect.push_back(!player ? 0 : player->GetClipLeft());
    clipRect.push_back(!player ? 0 : player->GetClipTop());
    clipRect.push_back(!player ? 0 : player->GetClipRight());
    clipRect.push_back(!player ? 0 : player->GetClipBottom());
    DnhArray(clipRect).Push(L);
    return 1;
}

static int GetPlayerLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetLife());
    return 1;
}

static int GetPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetSpell());
    return 1;
}

static int GetPlayerPower(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetPower());
    return 1;
}

static int GetPlayerInvincibilityFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetInvincibilityFrame());
    return 1;
}

static int GetPlayerDownStateFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, player ? player->GetDownStateFrame() : 0);
    return 1;
}

static int GetPlayerRebirthFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, player ? player->GetRebirthFrame() : 0);
    return 1;
}

static int IsPermitPlayerShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushboolean(L, player && player->IsPermitPlayerShot());
    return 1;
}

static int IsPermitPlayerSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushboolean(L, player && player->IsPermitPlayerSpell());
    return 1;
}

static int IsPlayerLastSpellWait(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushboolean(L, player && player->IsLastSpellWait());
    return 1;
}

static int IsPlayerSpellActive(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushboolean(L, player && player->IsSpellActive());
    return 1;
}

static int GetPlayerX(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetX());
    return 1;
}
static int GetPlayerY(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto player = engine->GetPlayerObject();
    lua_pushnumber(L, !player ? 0 : player->GetY());
    return 1;
}

static int GetAngleToPlayer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        auto player = engine->GetPlayerObject();
        double angle = !player ? 0 : D3DXToDegree(atan2(player->GetMoveY() - obj->GetY(), player->GetMoveX() - obj->GetX()));
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
    DnhArray(engine->GetPlayerID()).Push(L);
    return 1;
}

static int GetPlayerReplayName(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetPlayerReplayName()).Push(L);
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
        const std::vector<Point2D>& tmp = enemy->GetAllIntersectionToShotPosition();
        // psの後ろに結合
        std::copy(tmp.begin(), tmp.end(), std::back_inserter(ps));
    }
    sortByDistanceFromPoint(ps, from);
}

static int GetEnemyIntersectionPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    int n = DnhValue::ToInt(L, 3);
    auto enemies = engine->GetObjectAll<ObjEnemy>();
    std::vector<Point2D> ps;
    getEnemyIntersectionPositionFromPoint(ps, enemies, Point2D((float)x, (float)y));
    if (ps.size() > n) { ps.resize(n); }
    DnhArray(ps).Push(L);
    return 1;
}

static int GetEnemyBossSceneObjectID(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto obj = engine->GetEnemyBossSceneObject();
    lua_pushnumber(L, obj ? obj->GetID() : ID_INVALID);
    return 1;
}

static int GetEnemyBossObjectID(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto boss = engine->GetEnemyBossObject())
    {
        DnhArray(std::vector<double>{(double)boss->GetID()}).Push(L);
    } else
    {
        DnhArray().Push(L);
    }
    return 1;
}

static int GetAllEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray ids;
    for (const auto& enemy : engine->GetObjectAll<ObjEnemy>())
    {
        ids.PushBack(std::make_unique<DnhReal>((double)enemy->GetID()));
    }
    ids.Push(L);
    return 1;
}

static int GetIntersectionRegistedEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray ids;
    for (const auto& enemy : engine->GetObjectAll<ObjEnemy>())
    {
        if (!enemy->GetAllIntersectionToShotPosition().empty())
        {
            ids.PushBack(std::make_unique<DnhReal>((double)enemy->GetID()));
        }
    }
    ids.Push(L);
    return 1;
}

static int GetAllEnemyIntersectionPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray poss;
    for (const auto& enemy : engine->GetObjectAll<ObjEnemy>())
    {
        for (const auto& pos : enemy->GetAllIntersectionToShotPosition())
        {
            poss.PushBack(std::make_unique<DnhArray>(pos));
        }
    }
    poss.Push(L);
    return 1;
}

static int GetEnemyIntersectionPositionByIdA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::vector<Point2D> ps;
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        getEnemyIntersectionPositionFromPoint(ps, { obj }, Point2D(obj->GetX(), obj->GetY()));
    }
    DnhArray(ps).Push(L);
    return 1;
}

static int GetEnemyIntersectionPositionByIdA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    float x = DnhValue::ToNum(L, 2);
    float y = DnhValue::ToNum(L, 3);
    std::vector<Point2D> ps;
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        getEnemyIntersectionPositionFromPoint(ps, { obj }, Point2D(x, y));
    }
    DnhArray(ps).Push(L);
    return 1;
}

static int LoadEnemyShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->LoadEnemyShotData(path, GetSourcePos(L));
    return 0;
}

static int ReloadEnemyShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->ReloadEnemyShotData(path, GetSourcePos(L));
    return 0;
}

static int atoi(lua_State* L)
{
    auto str = DnhValue::ToString(L, 1);
    lua_pushnumber(L, _wtoi64(str.c_str()));
    return 1;
}

static int ator(lua_State* L)
{
    double r = DnhValue::ToNum(L, 1);
    lua_pushnumber(L, r);
    return 1;
}

static int TrimString(lua_State* L)
{
    auto str = DnhValue::ToString(L, 1);
    TrimSpace(&str);
    DnhArray(str).Push(L);
    return 1;
}

static int rtos(lua_State* L)
{
    std::wstring format = DnhValue::ToString(L, 1);
    double num = DnhValue::ToNum(L, 2);
    std::vector<std::wstring> ss = Split(format, L'.');
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
    DnhArray(std::wstring(buf.c_str())).Push(L);
    return 1;
}

static int vtos(lua_State* L)
{
    auto format = DnhValue::ToString(L, 1);
    auto value = DnhValue::Get(L, 2);
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
        swprintf_s(&buf[0], buf.size(), &format[0], value->ToInt());
    } else if (format.find(L'f') != std::string::npos)
    {
        swprintf_s(&buf[0], buf.size(), &format[0], value->ToNum());
    } else if (format.find(L's') != std::string::npos)
    {
        swprintf_s(&buf[0], buf.size(), &format[0], value->ToString().c_str());
    } else
    {
        buf = L"error format";
    }
    DnhArray(std::wstring(buf.c_str())).Push(L);
    return 1;
}

static int SplitString(lua_State* L)
{
    auto str = DnhValue::ToString(L, 1);
    auto delim = DnhValue::ToString(L, 2);
    DnhArray arr;
    for (auto& s : Split(str, delim))
    {
        arr.PushBack(std::make_unique<DnhArray>(s));
    }
    arr.Push(L);
    return 1;
}

static int GetFileDirectory(lua_State* L)
{
    auto path = DnhValue::ToString(L, 1);
    DnhArray(GetParentPath(path) + L"/").Push(L);
    return 1;
}

static int GetFilePathList(lua_State* L)
{
    auto dirPath = DnhValue::ToString(L, 1);
    if (dirPath.empty())
    {
        DnhArray(L"").Push(L);
        return 1;
    }

    if (dirPath.back() != L'/' && dirPath.back() != L'\\')
    {
        dirPath = GetParentPath(dirPath);
    }

    std::vector<std::wstring> pathList;
    GetFilePaths(dirPath, pathList, {}, false);

    DnhArray ret;
    for (const auto& path : pathList)
    {
        ret.PushBack(std::make_unique<DnhArray>(path));
    }
    ret.Push(L);
    return 1;
}

static int GetDirectoryList(lua_State* L)
{
    auto dirPath = DnhValue::ToString(L, 1);
    if (dirPath.empty())
    {
        DnhArray(L"").Push(L);
        return 1;
    }

    if (dirPath.back() != L'/' && dirPath.back() != L'\\')
    {
        dirPath = GetParentPath(dirPath);
    }

    std::vector<std::wstring> dirList;
    GetDirs(dirPath, dirList, false);

    DnhArray ret;
    for (const auto& dir : dirList)
    {
        ret.PushBack(std::make_unique<DnhArray>(ConcatPath(dir, L"")));
    }
    ret.Push(L);
    return 1;
}

static int DeleteShotAll(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::ToInt(L, 1);
    int behavior = DnhValue::ToInt(L, 2);
    engine->DeleteShotAll(target, behavior);
    return 0;
}

static int DeleteShotInCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::ToInt(L, 1);
    int behavior = DnhValue::ToInt(L, 2);
    float x = DnhValue::ToNum(L, 3);
    float y = DnhValue::ToNum(L, 4);
    float r = DnhValue::ToNum(L, 5);
    engine->DeleteShotInCircle(target, behavior, x, y, r);
    return 0;
}

static int CreateShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speed = DnhValue::ToNum(L, 3);
    double angle = DnhValue::ToNum(L, 4);
    int graphic = DnhValue::ToInt(L, 5);
    int delay = DnhValue::ToInt(L, 6);
    if (auto shot = engine->CreateShotA1(x, y, speed, angle, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speed = DnhValue::ToNum(L, 3);
    double angle = DnhValue::ToNum(L, 4);
    double accel = DnhValue::ToNum(L, 5);
    double maxSpeed = DnhValue::ToNum(L, 6);
    int graphic = DnhValue::ToInt(L, 7);
    int delay = DnhValue::ToInt(L, 8);
    if (auto shot = engine->CreateShotA2(x, y, speed, angle, accel, maxSpeed, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotOA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int objId = DnhValue::ToInt(L, 1);
    double speed = DnhValue::ToNum(L, 2);
    double angle = DnhValue::ToNum(L, 3);
    int graphic = DnhValue::ToInt(L, 4);
    int delay = DnhValue::ToInt(L, 5);
    if (auto shot = engine->CreateShotOA1(objId, speed, angle, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speedX = DnhValue::ToNum(L, 3);
    double speedY = DnhValue::ToNum(L, 4);
    int graphic = DnhValue::ToInt(L, 5);
    int delay = DnhValue::ToInt(L, 6);
    if (auto shot = engine->CreateShotB1(x, y, speedX, speedY, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotB2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speedX = DnhValue::ToNum(L, 3);
    double speedY = DnhValue::ToNum(L, 4);
    double accelX = DnhValue::ToNum(L, 5);
    double accelY = DnhValue::ToNum(L, 6);
    double maxSpeedX = DnhValue::ToNum(L, 7);
    double maxSpeedY = DnhValue::ToNum(L, 8);
    int graphic = DnhValue::ToInt(L, 9);
    int delay = DnhValue::ToInt(L, 10);
    if (auto shot = engine->CreateShotB2(x, y, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateShotOB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int objId = DnhValue::ToInt(L, 1);
    double speedX = DnhValue::ToNum(L, 2);
    double speedY = DnhValue::ToNum(L, 3);
    int graphic = DnhValue::ToInt(L, 4);
    int delay = DnhValue::ToInt(L, 5);
    if (auto shot = engine->CreateShotOB1(objId, speedX, speedY, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(shot->GetID());
        lua_pushnumber(L, shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateLooseLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speed = DnhValue::ToNum(L, 3);
    double angle = DnhValue::ToNum(L, 4);
    double laserLength = DnhValue::ToNum(L, 5);
    double laserWidth = DnhValue::ToNum(L, 6);
    int graphic = DnhValue::ToInt(L, 7);
    int delay = DnhValue::ToInt(L, 8);
    if (auto laser = engine->CreateLooseLaserA1(x, y, speed, angle, laserLength, laserWidth, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(laser->GetID());
        lua_pushnumber(L, laser->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateStraightLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double angle = DnhValue::ToNum(L, 3);
    double laserLength = DnhValue::ToNum(L, 4);
    double laserWidth = DnhValue::ToNum(L, 5);
    int deleteFrame = DnhValue::ToInt(L, 6);
    int graphic = DnhValue::ToInt(L, 7);
    int delay = DnhValue::ToInt(L, 8);
    if (auto laser = engine->CreateStraightLaserA1(x, y, angle, laserLength, laserWidth, deleteFrame, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(laser->GetID());
        lua_pushnumber(L, laser->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateCurveLaserA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speed = DnhValue::ToNum(L, 3);
    double angle = DnhValue::ToNum(L, 4);
    double laserLength = DnhValue::ToNum(L, 5);
    double laserWidth = DnhValue::ToNum(L, 6);
    int graphic = DnhValue::ToInt(L, 7);
    int delay = DnhValue::ToInt(L, 8);
    if (auto laser = engine->CreateCurveLaserA1(x, y, speed, angle, laserLength, laserWidth, graphic, delay, script->GetType() == SCRIPT_TYPE_PLAYER))
    {
        script->AddAutoDeleteTargetObjectId(laser->GetID());
        lua_pushnumber(L, laser->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int SetShotIntersectionCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double r = DnhValue::ToNum(L, 3);
    engine->SetShotIntersectoinCicle(x, y, r);
    return 0;
}

static int SetShotIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x1 = DnhValue::ToNum(L, 1);
    double y1 = DnhValue::ToNum(L, 2);
    double x2 = DnhValue::ToNum(L, 3);
    double y2 = DnhValue::ToNum(L, 4);
    double width = DnhValue::ToNum(L, 5);
    engine->SetShotIntersectoinLine(x1, y1, x2, y2, width);
    return 0;
}

static int GetShotIdInCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double r = DnhValue::ToNum(L, 3);
    DnhArray ids;
    for (auto& shot : engine->GetShotInCircle(x, y, r, script->GetType() == SCRIPT_TYPE_PLAYER ? TARGET_ENEMY : TARGET_PLAYER))
    {
        if (shot)
        {
            ids.PushBack(std::make_unique<DnhReal>((double)shot->GetID()));
        }
    }
    ids.Push(L);
    return 1;
}

static int GetShotIdInCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double r = DnhValue::ToNum(L, 3);
    int target = DnhValue::ToInt(L, 4);
    DnhArray ids;
    for (auto& shot : engine->GetShotInCircle(x, y, r, target))
    {
        if (shot)
        {
            ids.PushBack(std::make_unique<DnhReal>((double)shot->GetID()));
        }
    }
    ids.Push(L);
    return 1;
}

static int GetShotCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int target = DnhValue::ToInt(L, 1);
    int cnt = 0;
    switch (target)
    {
        case TARGET_ALL:
            cnt = engine->GetAllShotCount();
            break;
        case TARGET_ENEMY:
            cnt = engine->GetEnemyShotCount();
            break;
        case TARGET_PLAYER:
            cnt = engine->GetPlayerShotCount();
            break;
    }
    lua_pushnumber(L, cnt);
    return 1;
}

static int SetShotAutoDeleteClip(lua_State* L)
{
    Engine* engine = getEngine(L);
    double l = DnhValue::ToNum(L, 1);
    double t = DnhValue::ToNum(L, 2);
    double r = DnhValue::ToNum(L, 3);
    double b = DnhValue::ToNum(L, 4);
    engine->SetShotAutoDeleteClip(l, t, r, b);
    return 0;
}

static int GetShotDataInfoA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int id = DnhValue::ToInt(L, 1);
    bool isPlayerShot = DnhValue::ToInt(L, 2) == TARGET_PLAYER;
    int infoType = DnhValue::ToInt(L, 3);
    if (auto shotData = isPlayerShot ? engine->GetPlayerShotData(id) : engine->GetEnemyShotData(id))
    {
        switch (infoType)
        {
            case INFO_RECT:
                DnhArray(std::vector<double>{(double)shotData->rect.left, (double)shotData->rect.top, (double)shotData->rect.left, (double)shotData->rect.bottom}).Push(L);
                break;
            case INFO_DELAY_COLOR:
                DnhArray(std::vector<double>{(double)shotData->delayColor.GetR(), (double)shotData->delayColor.GetG(), (double)shotData->delayColor.GetB()}).Push(L);
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
                    colList.PushBack(std::make_unique<DnhArray>(std::vector<double>{col.r, col.x, col.y}));
                }
                colList.Push(L);
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
    auto path = DnhValue::ToString(L, 1);
    engine->StartShotScript(path, GetSourcePos(L));
    return 0;
}

static int CreateItemA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int type = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    PlayerScore score = DnhValue::ToNum(L, 4);
    if (auto item = engine->CreateItemA1(type, x, y, score))
    {
        script->AddAutoDeleteTargetObjectId(item->GetID());
        lua_pushnumber(L, item->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int type = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double destX = DnhValue::ToNum(L, 4);
    double destY = DnhValue::ToNum(L, 5);
    PlayerScore score = DnhValue::ToNum(L, 6);
    if (auto item = engine->CreateItemA2(type, x, y, destX, destY, score))
    {
        script->AddAutoDeleteTargetObjectId(item->GetID());
        lua_pushnumber(L, item->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemU1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int itemDataId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    PlayerScore score = DnhValue::ToNum(L, 4);
    if (auto item = engine->CreateItemU1(itemDataId, x, y, score))
    {
        script->AddAutoDeleteTargetObjectId(item->GetID());
        lua_pushnumber(L, item->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CreateItemU2(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int itemDataId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double destX = DnhValue::ToNum(L, 4);
    double destY = DnhValue::ToNum(L, 5);
    PlayerScore score = DnhValue::ToNum(L, 6);
    if (auto item = engine->CreateItemU2(itemDataId, x, y, destX, destY, score))
    {
        script->AddAutoDeleteTargetObjectId(item->GetID());
        lua_pushnumber(L, item->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int CollectAllItems(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->CollectAllItems();
    return 0;
}

static int CollectItemsByType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int type = DnhValue::ToInt(L, 1);
    engine->CollectItemsByType(type);
    return 0;
}

static int CollectItemsInCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double r = DnhValue::ToNum(L, 3);
    engine->CollectItemsInCircle(x, y, r);
    return 0;
}

static int CancelCollectItems(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->CancelCollectItems();
    return 0;
}

static int StartItemScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->StartItemScript(path, GetSourcePos(L));
    return 0;
}

static int SetDefaultBonusItemEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool enable = DnhValue::ToBool(L, 1);
    engine->SetDefaultBonusItemEnable(enable);
    return 0;
}

static int LoadItemData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->LoadItemData(path, GetSourcePos(L));
    return 0;
}

static int ReloadItemData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->ReloadItemData(path, GetSourcePos(L));
    return 0;
}

static int StartSlow(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int fps = DnhValue::ToInt(L, 2);
    engine->StartSlow(fps, script->GetType() == SCRIPT_TYPE_PLAYER);
    return 0;
}

static int StopSlow(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    engine->StopSlow(script->GetType() == SCRIPT_TYPE_PLAYER);
    return 0;
}

static int IsIntersected_Line_Circle(lua_State* L)
{
    double x1 = DnhValue::ToNum(L, 1);
    double y1 = DnhValue::ToNum(L, 2);
    double x2 = DnhValue::ToNum(L, 3);
    double y2 = DnhValue::ToNum(L, 4);
    double width = DnhValue::ToNum(L, 5);
    double cx = DnhValue::ToNum(L, 6);
    double cy = DnhValue::ToNum(L, 7);
    double r = DnhValue::ToNum(L, 8);
    lua_pushboolean(L, IsIntersectedLineCircle(x1, y1, x2, y2, width, cx, cy, r));
    return 1;
}

static int IsIntersected_Obj_Obj(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId1 = DnhValue::ToInt(L, 1);
    int objId2 = DnhValue::ToInt(L, 2);
    auto obj1 = engine->GetObject<ObjCol>(objId1);
    auto obj2 = engine->GetObject<ObjCol>(objId2);
    lua_pushboolean(L, obj1 && obj2 && obj1->IsIntersected(obj2));
    return 1;
}

static int GetObjectDistance(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId1 = DnhValue::ToInt(L, 1);
    int objId2 = DnhValue::ToInt(L, 2);
    if (auto obj1 = engine->GetObject<ObjRender>(objId1))
    {
        if (auto obj2 = engine->GetObject<ObjRender>(objId2))
        {
            float dx = obj1->GetX() - obj2->GetX();
            float dy = obj1->GetY() - obj2->GetY();
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
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        DnhArray(engine->Get2DPosition(obj->GetX(), obj->GetY(), obj->GetZ(), obj->IsStgSceneObject())).Push(L);
    } else
    {
        DnhArray(Point2D(0, 0)).Push(L);
    }
    return 1;
}

static int Get2dPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    float x = DnhValue::ToNum(L, 1);
    float y = DnhValue::ToNum(L, 2);
    float z = DnhValue::ToNum(L, 3);
    DnhArray(engine->Get2DPosition(x, y, z, script->IsStgSceneScript())).Push(L);
    return 1;
}

static int GetOwnScriptID(lua_State* L)
{
    Script* script = GetScript(L);
    lua_pushnumber(L, script->GetID());
    return 1;
}

static int SetScriptResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    auto result = DnhValue::Get(L, 1);
    engine->SetScriptResult(script->GetID(), std::move(result));
    return 0;
}

static int GetScriptResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    engine->GetScriptResult(scriptId)->Push(L);
    return 1;
}

static int SetAutoDeleteObject(lua_State* L)
{
    bool enable = DnhValue::ToBool(L, 1);
    Script* script = GetScript(L);
    script->SetAutoDeleteObjectEnable(enable);
    return 0;
}

static int NotifyEvent(lua_State* L)
{
    Engine* engine = getEngine(L);
    int scriptId = DnhValue::ToInt(L, 1);
    int eventType = DnhValue::ToInt(L, 2);
    auto arg = DnhValue::Get(L, 3);
    if (auto script = engine->GetScript(scriptId))
    {
        auto args = std::make_unique<DnhArray>();
        args->PushBack(std::move(arg));
        script->NotifyEvent(eventType, args);
    }
    return 0;
}

static int NotifyEventAll(lua_State* L)
{
    Engine* engine = getEngine(L);
    int eventType = DnhValue::ToInt(L, 1);
    auto arg = DnhValue::Get(L, 2);
    auto args = std::make_unique<DnhArray>();
    args->PushBack(std::move(arg));
    engine->NotifyEventAll(eventType, args);
    return 0;
}

static int GetScriptInfoA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    int infoType = DnhValue::ToInt(L, 2);
    ScriptInfo info = engine->GetScriptInfo(path, GetSourcePos(L));
    switch (infoType)
    {
        case INFO_SCRIPT_TYPE:
            lua_pushnumber(L, GetScriptTypeConstFromName(info.type));
            break;
        case INFO_SCRIPT_PATH:
            DnhArray(info.path).Push(L);
            break;
        case INFO_SCRIPT_ID:
            DnhArray(info.id).Push(L);
            break;
        case INFO_SCRIPT_TITLE:
            DnhArray(info.title).Push(L);
            break;
        case INFO_SCRIPT_TEXT:
            DnhArray(info.text).Push(L);
            break;
        case INFO_SCRIPT_IMAGE:
            DnhArray(info.imagePath).Push(L);
            break;
        case INFO_SCRIPT_REPLAY_NAME:
            DnhArray(info.replayName).Push(L);
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
    int objId = DnhValue::ToInt(L, 1);
    engine->DeleteObject(objId);
    return 0;
}

static int Obj_IsDeleted(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    lua_pushboolean(L, engine->IsObjectDeleted(objId));
    return 1;
}

static int Obj_SetVisible(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool b = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetVisible(b);
    }
    return 0;
}

static int Obj_IsVisible(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjRender>(objId);
    lua_pushboolean(L, obj && obj->IsVisible());
    return 1;
}

static int Obj_SetRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double p = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        engine->SetObjectRenderPriority(obj, (int)(p * MAX_RENDER_PRIORITY));
    }
    return 0;
}

static int Obj_SetRenderPriorityI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int p = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        engine->SetObjectRenderPriority(obj, p);
    }
    return 0;
}

static int Obj_GetRenderPriority(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? 1.0 * obj->getRenderPriority() / MAX_RENDER_PRIORITY : 0);
    return 1;
}

static int Obj_GetRenderPriorityI(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? obj->getRenderPriority() : 0);
    return 1;
}

static int Obj_GetValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->GetValue(key)->Push(L);
        return 1;
    }
    return 0;
}

static int Obj_GetValueD(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    auto defaultValue = DnhValue::Get(L, 3);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->GetValueD(key, defaultValue)->Push(L);
    } else
    {
        defaultValue->Push(L);
    }
    return 1;
}

static int Obj_SetValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    auto value = DnhValue::Get(L, 3);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->SetValue(key, std::move(value));
    }
    return 0;
}

static int Obj_DeleteValue(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->DeleteValue(key);
    }
    return 0;
}

static int Obj_IsValueExists(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring key = DnhValue::ToString(L, 2);
    auto obj = engine->GetObject<Obj>(objId);
    lua_pushboolean(L, obj && obj->IsValueExists(key));
    return 1;
}

static int Obj_GetType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        lua_pushnumber(L, obj->GetType());
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
    int objId = DnhValue::ToInt(L, 1);
    float v = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        ((obj.get())->*func)(v);
    }
    return 0;
}

static int ObjRender_SetX(lua_State* L) { return ObjRender_Set<&ObjRender::SetX>(L); }
static int ObjRender_SetY(lua_State* L) { return ObjRender_Set<&ObjRender::SetY>(L); }
static int ObjRender_SetZ(lua_State* L) { return ObjRender_Set<&ObjRender::SetZ>(L); }

static int ObjRender_SetPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double z = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetPosition(x, y, z); }
    return 0;
}

static int ObjRender_SetAngleX(lua_State* L) { return ObjRender_Set<&ObjRender::SetAngleX>(L); }
static int ObjRender_SetAngleY(lua_State* L) { return ObjRender_Set<&ObjRender::SetAngleY>(L); }
static int ObjRender_SetAngleZ(lua_State* L) { return ObjRender_Set<&ObjRender::SetAngleZ>(L); }

static int ObjRender_SetAngleXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double rx = DnhValue::ToNum(L, 2);
    double ry = DnhValue::ToNum(L, 3);
    double rz = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetAngleXYZ(rx, ry, rz); }
    return 0;
}

static int ObjRender_SetScaleX(lua_State* L) { return ObjRender_Set<&ObjRender::SetScaleX>(L); }
static int ObjRender_SetScaleY(lua_State* L) { return ObjRender_Set<&ObjRender::SetScaleY>(L); }
static int ObjRender_SetScaleZ(lua_State* L) { return ObjRender_Set<&ObjRender::SetScaleZ>(L); }

static int ObjRender_SetScaleXYZ(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double sx = DnhValue::ToNum(L, 2);
    double sy = DnhValue::ToNum(L, 3);
    double sz = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetScaleXYZ(sx, sy, sz); }
    return 0;
}

static int ObjRender_SetColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int r = DnhValue::ToInt(L, 2);
    int g = DnhValue::ToInt(L, 3);
    int b = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetColor(r, g, b);
        // ObjPrimでObjSpriteList2Dでなければ全ての頂点に設定
        if (auto prim = std::dynamic_pointer_cast<ObjPrim>(obj))
        {
            if (!std::dynamic_pointer_cast<ObjSpriteList2D>(prim))
            {
                int vertexCnt = prim->GetVertexCount();
                for (int i = 0; i < vertexCnt; i++)
                {
                    prim->SetVertexColor(i, r, g, b);
                }
            }
        }
    }
    return 0;
}

static int ObjRender_SetColorHSV(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int h = DnhValue::ToInt(L, 2);
    int s = DnhValue::ToInt(L, 3);
    int v = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetColorHSV(h, s, v);
        // ObjPrimでObjSpriteList2Dでなければ全ての頂点に設定
        if (auto prim = std::dynamic_pointer_cast<ObjPrim>(obj))
        {
            if (!std::dynamic_pointer_cast<ObjSpriteList2D>(prim))
            {
                int vertexCnt = prim->GetVertexCount();
                const auto& color = prim->GetColor();
                for (int i = 0; i < vertexCnt; i++)
                {
                    prim->SetVertexColor(i, color.GetR(), color.GetG(), color.GetB());
                }
            }
        }
    }
    return 0;
}

static int ObjRender_SetAlpha(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int a = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetAlpha(a); }
    if (auto obj = engine->GetObject <ObjPrim>(objId))
    {
        int vertexCnt = obj->GetVertexCount();
        const auto& color = obj->GetColor();
        for (int i = 0; i < vertexCnt; i++)
        {
            obj->SetVertexAlpha(i, a);
        }
    }
    return 0;
}

static int ObjRender_SetBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int blendType = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetBlendType(blendType); }
    return 0;
}

template <float (ObjRender::*func)() const>
static int ObjRender_Get(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? ((obj.get())->*func)() : 0);
    return 1;
}

static int ObjRender_GetX(lua_State* L) { return ObjRender_Get<&ObjRender::GetX>(L); }
static int ObjRender_GetY(lua_State* L) { return ObjRender_Get<&ObjRender::GetY>(L); }
static int ObjRender_GetZ(lua_State* L) { return ObjRender_Get<&ObjRender::GetZ>(L); }
static int ObjRender_GetAngleX(lua_State* L) { return ObjRender_Get<&ObjRender::GetAngleX>(L); }
static int ObjRender_GetAngleY(lua_State* L) { return ObjRender_Get<&ObjRender::GetAngleY>(L); }
static int ObjRender_GetAngleZ(lua_State* L) { return ObjRender_Get<&ObjRender::GetAngleZ>(L); }
static int ObjRender_GetScaleX(lua_State* L) { return ObjRender_Get<&ObjRender::GetScaleX>(L); }
static int ObjRender_GetScaleY(lua_State* L) { return ObjRender_Get<&ObjRender::GetScaleY>(L); }
static int ObjRender_GetScaleZ(lua_State* L) { return ObjRender_Get<&ObjRender::GetScaleZ>(L); }

static int ObjRender_GetBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjRender>(objId);
    lua_pushnumber(L, obj ? obj->GetBlendType() : BLEND_NONE);
    return 1;
}

static int ObjRender_SetZWrite(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetZWrite(enable); }
    return 0;
}

static int ObjRender_SetZTest(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetZTest(enable); }
    return 0;
}

static int ObjRender_SetFogEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetFogEnable(enable); }
    return 0;
}

static int ObjRender_SetPermitCamera(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId)) { obj->SetPermitCamera(enable); }
    return 0;
}

static int ObjRender_SetCullingMode(lua_State* L)
{
    return 0;
}

static int ObjPrim_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int type = DnhValue::ToInt(L, 1);
    int objId = ID_INVALID;
    switch (type)
    {
        case OBJ_PRIMITIVE_2D:
            objId = engine->CreateObjPrim2D()->GetID();
            break;
        case OBJ_SPRITE_2D:
            objId = engine->CreateObjSprite2D()->GetID();
            break;
        case OBJ_SPRITE_LIST_2D:
            objId = engine->CreateObjSpriteList2D()->GetID();
            break;
        case OBJ_PRIMITIVE_3D:
            objId = engine->CreateObjPrim3D()->GetID();
            break;
        case OBJ_SPRITE_3D:
            objId = engine->CreateObjSprite3D()->GetID();
            break;
    }
    script->AddAutoDeleteTargetObjectId(objId);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->SetStgSceneObject(script->IsStgSceneScript());
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjPrim_SetPrimitiveType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int type = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjPrim>(objId))
    {
        obj->SetPrimitiveType(type);
    }
    return 0;
}

static int ObjPrim_SetVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int cnt = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjPrim>(objId))
    {
        obj->SetVertexCount(cnt);
    }
    return 0;
}

static int ObjPrim_GetVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjPrim>(objId);
    lua_pushnumber(L, obj ? obj->GetVertexCount() : 0);
    return 1;
}

static int ObjPrim_SetTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring name = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjPrim>(objId))
    {
        if (auto renderTarget = engine->GetRenderTarget(name))
        {
            obj->SetRenderTarget(renderTarget);
        } else
        {
            obj->SetTexture(engine->LoadTexture(name, false, GetSourcePos(L)));
        }
    }
    return 0;
}

static int ObjPrim_SetVertexPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    double x = DnhValue::ToNum(L, 3);
    double y = DnhValue::ToNum(L, 4);
    double z = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjPrim>(objId))
    {
        obj->SetVertexPosition(vIdx, x, y, z);
    }
    return 0;
}

static int ObjPrim_GetVertexPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    auto obj = engine->GetObject<ObjPrim>(objId);
    float x = obj ? obj->GetVertexPositionX(vIdx) : 0;
    float y = obj ? obj->GetVertexPositionY(vIdx) : 0;
    float z = obj ? obj->GetVertexPositionZ(vIdx) : 0;
    DnhArray ret;
    ret.PushBack(std::make_unique<DnhReal>(x));
    ret.PushBack(std::make_unique<DnhReal>(y));
    ret.PushBack(std::make_unique<DnhReal>(z));
    ret.Push(L);
    return 1;
}

static int ObjPrim_SetVertexUV(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    double u = DnhValue::ToNum(L, 3);
    double v = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjPrim>(objId)) { obj->SetVertexUV(vIdx, u, v); }
    return 0;
}

static int ObjPrim_SetVertexUVT(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    double u = DnhValue::ToNum(L, 3);
    double v = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjPrim>(objId)) { obj->SetVertexUVT(vIdx, u, v); }
    return 0;
}

static int ObjPrim_SetVertexColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    int r = DnhValue::ToInt(L, 3);
    int g = DnhValue::ToInt(L, 4);
    int b = DnhValue::ToInt(L, 5);
    if (auto obj = engine->GetObject<ObjPrim>(objId)) { obj->SetVertexColor(vIdx, r, g, b); }
    return 0;
}

static int ObjPrim_SetVertexAlpha(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int vIdx = DnhValue::ToInt(L, 2);
    int a = DnhValue::ToInt(L, 3);
    if (auto obj = engine->GetObject<ObjPrim>(objId)) { obj->SetVertexAlpha(vIdx, a); }
    return 0;
}

static int ObjSprite2D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSprite2D>(objId))
    {
        obj->SetSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite2D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSprite2D>(objId))
    {
        obj->SetDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite2D_SetDestCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSprite2D>(objId))
    {
        obj->SetDestCenter();
    }
    return 0;
}

static int ObjSpriteList2D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->SetSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSpriteList2D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->SetDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSpriteList2D_SetDestCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->SetDestCenter();
    }
    return 0;
}

static int ObjSpriteList2D_AddVertex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->AddVertex();
    }
    return 0;
}

static int ObjSpriteList2D_CloseVertex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->CloseVertex();
    }
    return 0;
}

static int ObjSpriteList2D_ClearVertexCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSpriteList2D>(objId))
    {
        obj->ClearVerexCount();
    }
    return 0;
}

static int ObjSprite3D_SetSourceRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSprite3D>(objId))
    {
        obj->SetSourceRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSprite3D>(objId))
    {
        obj->SetDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetSourceDestRect(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double l = DnhValue::ToNum(L, 2);
    double t = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double b = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjSprite3D>(objId))
    {
        obj->SetSourceDestRect(l, t, r, b);
    }
    return 0;
}

static int ObjSprite3D_SetBillboard(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjSprite3D>(objId))
    {
        obj->SetBillboard(enable);
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
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjMesh())
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        obj->SetStgSceneObject(script->IsStgSceneScript());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjMesh_Load(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto path = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjMesh>(objId))
    {
        if (auto mesh = engine->LoadMesh(path, GetSourcePos(L)))
        {
            obj->SetMesh(mesh);
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
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjText())
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        obj->SetStgSceneObject(script->IsStgSceneScript());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjText_SetText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring text = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetText(text);
    }
    return 0;
}

static int ObjText_SetFontType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring name = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontName(name);
    }
    return 0;
}

static int ObjText_SetFontSize(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int size = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontSize(size);
    }
    return 0;
}

static int ObjText_SetFontBold(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool bold = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontBold(bold);
    }
    return 0;
}

static int ObjText_SetFontColorTop(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int r = DnhValue::ToInt(L, 2);
    int g = DnhValue::ToInt(L, 3);
    int b = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontColorTop(r, g, b);
    }
    return 0;
}

static int ObjText_SetFontColorBottom(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int r = DnhValue::ToInt(L, 2);
    int g = DnhValue::ToInt(L, 3);
    int b = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontColorBottom(r, g, b);
    }
    return 0;
}

static int ObjText_SetFontBorderWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int width = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontBorderWidth(width);
    }
    return 0;
}

static int ObjText_SetFontBorderType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int t = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontBorderType(t);
    }
    return 0;
}

static int ObjText_SetFontBorderColor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int r = DnhValue::ToInt(L, 2);
    int g = DnhValue::ToInt(L, 3);
    int b = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetFontBorderColor(r, g, b);
    }
    return 0;
}

static int ObjText_SetMaxWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int width = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetMaxWidth(width);
    }
    return 0;
}

static int ObjText_SetMaxHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int height = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetMaxHeight(height);
    }
    return 0;
}

static int ObjText_SetLinePitch(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int pitch = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetLinePitch(pitch);
    }
    return 0;
}

static int ObjText_SetSidePitch(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int pitch = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetSidePitch(pitch);
    }
    return 0;
}

static int ObjText_SetTransCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetTransCenter(x, y);
    }
    return 0;
}

static int ObjText_SetAutoTransCenter(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetAutoTransCenter(enable);
    }
    return 0;
}

static int ObjText_SetHorizontalAlignment(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int alignment = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetHorizontalAlignment(alignment);
    }
    return 0;
}

static int ObjText_SetSyntacticAnalysis(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->SetSyntacticAnalysis(enable);
    }
    return 0;
}

static int ObjText_GetTextLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjText>(objId);
    lua_pushnumber(L, obj ? obj->GetTextLength() : 0);
    return 1;
}

static int ObjText_GetTextLengthCU(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjText>(objId);
    lua_pushnumber(L, obj ? obj->GetTextLengthCU() : 0);
    return 1;
}

static int ObjText_GetTextLengthCUL(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->GenerateFonts();
        auto cnts = obj->GetTextLengthCUL();
        DnhArray ret;
        for (auto cnt : cnts)
        {
            ret.PushBack(std::make_unique<DnhReal>((double)cnt));
        }
        ret.Push(L);
        return 1;
    } else
    {
        return 0;
    }
}

static int ObjText_GetTotalWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->GenerateFonts();
        lua_pushnumber(L, obj->GetTotalWidth());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjText_GetTotalHeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjText>(objId))
    {
        obj->GenerateFonts();
        lua_pushnumber(L, obj->GetTotalHeight());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjShader_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjShader())
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        obj->SetStgSceneObject(script->IsStgSceneScript());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjShader_SetShaderF(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring path = DnhValue::ToString(L, 2);

    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        auto shader = engine->CreateShader(path, false);
        if (shader)
        {
            obj->SetShader(shader);
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
    int objId = DnhValue::ToInt(L, 1);
    int shaderObjId = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        auto shaderObj = engine->GetObject<ObjRender>(shaderObjId);
        obj->SetShaderO(shaderObj);
    }
    return 0;
}

static int ObjShader_ResetShader(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->ResetShader();
    }
    return 0;
}

static int ObjShader_SetTechnique(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::string technique = DnhValue::ToStringU8(L, 2);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetShaderTechnique(technique);
    }
    return 0;
}

static int ObjShader_SetVector(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::string name = DnhValue::ToStringU8(L, 2);
    double x = DnhValue::ToNum(L, 3);
    double y = DnhValue::ToNum(L, 4);
    double z = DnhValue::ToNum(L, 5);
    double w = DnhValue::ToNum(L, 6);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetShaderVector(name, x, y, z, w);
    }
    return 0;
}

static int ObjShader_SetFloat(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::string name = DnhValue::ToStringU8(L, 2);
    double f = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        obj->SetShaderFloat(name, f);
    }
    return 0;
}

static int ObjShader_SetFloatArray(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::string name = DnhValue::ToStringU8(L, 2);
    auto value = DnhValue::Get(L, 3);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        if (DnhArray* floatArray = dynamic_cast<DnhArray*>(value.get()))
        {
            size_t size = floatArray->GetSize();
            std::vector<float> fs(size);
            for (int i = 0; i < size; i++)
            {
                fs[i] = (float)floatArray->Index(i)->ToNum();
            }
            obj->SetShaderFloatArray(name, fs);
        }
    }
    return 0;
}

static int ObjShader_SetTexture(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::string name = DnhValue::ToStringU8(L, 2);
    std::wstring path = DnhValue::ToString(L, 3);
    if (auto obj = engine->GetObject<ObjRender>(objId))
    {
        if (auto renderTarget = engine->GetRenderTarget(ToUnicode(name)))
        {
            obj->SetShaderTexture(name, renderTarget);
        } else
        {
            obj->SetShaderTexture(name, engine->LoadTexture(path, false, GetSourcePos(L)));
        }
    }
    return 0;
}

static int ObjSound_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjSound())
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        obj->SetStgSceneObject(script->IsStgSceneScript());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjSound_Load(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto path = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetSound(nullptr);
        obj->SetSound(engine->LoadSound(path, GetSourcePos(L)));
    }
    return 0;
}

static int ObjSound_Play(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->Play();
    }
    return 0;
}

static int ObjSound_Stop(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->Stop();
    }
    return 0;
}

static int ObjSound_SetVolumeRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    float vol = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetVolumeRate(vol);
    }
    return 0;
}

static int ObjSound_SetPanRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    float pan = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetPanRate(pan);
    }
    return 0;
}

static int ObjSound_SetFade(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    float fade = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetFade(fade);
    }
    return 0;
}

static int ObjSound_SetLoopEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetLoopEnable(enable);
    }
    return 0;
}

static int ObjSound_SetLoopTime(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double start = (DWORD)DnhValue::ToNum(L, 2);
    double end = (DWORD)DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetLoopTime(start, end);
    }
    return 0;
}

static int ObjSound_SetLoopSampleCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    DWORD start = (DWORD)DnhValue::ToNum(L, 2);
    DWORD end = (DWORD)DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetLoopSampleCount(start, end);
    }
    return 0;
}

static int ObjSound_SetRestartEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetRestartEnable(enable);
    }
    return 0;
}

static int ObjSound_SetSoundDivision(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int division = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjSound>(objId))
    {
        obj->SetSoundDivision(division == SOUND_SE ? ObjSound::SoundDivision::SE : ObjSound::SoundDivision::BGM);
    }
    return 0;
}

static int ObjSound_IsPlaying(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjSound>(objId);
    lua_pushboolean(L, obj && obj->IsPlaying());
    return 1;
}

static int ObjSound_GetVolumeRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjSound>(objId);
    lua_pushnumber(L, obj ? obj->GetVolumeRate() : 0);
    return 1;
}

static int ObjFile_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int type = DnhValue::ToInt(L, 1);
    int objId = ID_INVALID;
    if (type == OBJ_FILE_TEXT)
    {
        objId = engine->CreateObjFileT()->GetID();
    } else if (type == OBJ_FILE_BINARY)
    {
        objId = engine->CreateObjFileB()->GetID();
    }
    script->AddAutoDeleteTargetObjectId(objId);
    if (auto obj = engine->GetObject<Obj>(objId))
    {
        obj->SetStgSceneObject(script->IsStgSceneScript());
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjFile_Open(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto path = DnhValue::ToString(L, 2);
    auto obj = engine->GetObject<ObjFile>(objId);
    lua_pushboolean(L, obj && obj->Open(path));
    return 1;
}

static int ObjFile_OpenNW(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto path = DnhValue::ToString(L, 2);
    auto obj = engine->GetObject<ObjFile>(objId);
    lua_pushboolean(L, obj && obj->OpenNW(path));
    return 1;
}

static int ObjFile_Store(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFile>(objId))
    {
        obj->Store();
    }
    return 0;
}

static int ObjFile_GetSize(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFile>(objId))
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
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileT>(objId))
    {
        lua_pushnumber(L, obj->GetLineCount());
    } else
    {
        lua_pushnumber(L, 0);
    }
    return 1;
}

static int ObjFileT_GetLineText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int lineNum = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjFileT>(objId))
    {
        DnhArray(obj->GetLineText(lineNum)).Push(L);
    } else
    {
        DnhArray(L"").Push(L);
    }
    return 1;
}

static int ObjFileT_SplitLineText(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int lineNum = DnhValue::ToInt(L, 2);
    std::wstring delim = DnhValue::ToString(L, 3);
    if (auto obj = engine->GetObject<ObjFileT>(objId))
    {
        DnhArray ret;
        for (const auto& s : obj->SplitLineText(lineNum, delim))
        {
            ret.PushBack(std::make_unique<DnhArray>(s));
        }
        ret.Push(L);
    } else
    {
        DnhArray().Push(L);
    }
    return 1;
}

static int ObjFileT_AddLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    std::wstring line = DnhValue::ToString(L, 2);
    if (auto obj = engine->GetObject<ObjFileT>(objId))
    {
        obj->AddLine(line);
    }
    return 0;
}

static int ObjFileT_ClearLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileT>(objId))
    {
        obj->ClearLine();
    }
    return 0;
}

static int ObjFileB_SetByteOrder(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int endian = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        if (endian == ENDIAN_LITTLE)
        {
            obj->SetByteOrder(false);
        } else
        {
            obj->SetByteOrder(true);
        }
    }
    return 0;
}
static int ObjFileB_SetCharacterCode(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int code = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        switch (code)
        {
            case CODE_ACP:
                obj->SetCharacterCode(ObjFileB::Encoding::ACP);
                break;
            case CODE_UTF8:
                obj->SetCharacterCode(ObjFileB::Encoding::UTF8);
                break;
            case CODE_UTF16LE:
                obj->SetCharacterCode(ObjFileB::Encoding::UTF16LE);
                break;
            case CODE_UTF16BE:
                obj->SetCharacterCode(ObjFileB::Encoding::UTF16BE);
                break;
        }
    }
    return 0;
}

static int ObjFileB_GetPointer(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->GetPointer());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_Seek(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int pos = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        obj->Seek(pos);
    }
    return 0;
}

static int ObjFileB_ReadBoolean(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjFileB>(objId);
    lua_pushboolean(L, obj && obj->ReadBoolean());
    return 1;
}

static int ObjFileB_ReadByte(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadByte());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadShort(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadShort());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadInteger(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadInteger());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadLong(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadLong());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadFloat(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadFloat());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadDouble(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        lua_pushnumber(L, obj->ReadDouble());
    } else
    {
        lua_pushnumber(L, -1);
    }
    return 1;
}

static int ObjFileB_ReadString(lua_State*L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int size = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjFileB>(objId))
    {
        DnhArray(obj->ReadString(size)).Push(L);
    } else
    {
        DnhArray(L"").Push(L);
    }
    return 1;
}

template <void (ObjMove::*func)(float)>
static int ObjMove_Set(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    float v = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        ((obj.get())->*func)(v);
    }
    return 0;
}

static int ObjMove_SetX(lua_State* L) { return ObjMove_Set<&ObjMove::SetMoveX>(L); }
static int ObjMove_SetY(lua_State* L) { return ObjMove_Set<&ObjMove::SetMoveY>(L); }

static int ObjMove_SetPosition(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->SetMovePosition(x, y);
    }
    return 0;
}

static int ObjMove_SetSpeed(lua_State* L) { return ObjMove_Set<&ObjMove::SetSpeed>(L); }
static int ObjMove_SetAngle(lua_State* L) { return ObjMove_Set<&ObjMove::SetAngle>(L); }
static int ObjMove_SetAcceleration(lua_State* L) { return ObjMove_Set<&ObjMove::SetAcceleration>(L); }
static int ObjMove_SetMaxSpeed(lua_State* L) { return ObjMove_Set<&ObjMove::SetMaxSpeed>(L); }
static int ObjMove_SetAngularVelocity(lua_State* L) { return ObjMove_Set<&ObjMove::SetAngularVelocity>(L); }

static int ObjMove_SetDestAtSpeed(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double speed = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->SetDestAtSpeed(x, y, speed);
    }
    return 0;
}

static int ObjMove_SetDestAtFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    int frame = DnhValue::ToInt(L, 4);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->SetDestAtFrame(x, y, frame);
    }
    return 0;
}

static int ObjMove_SetDestAtWeight(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double w = DnhValue::ToNum(L, 4);
    double maxSpeed = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->SetDestAtWeight(x, y, w, maxSpeed);
    }
    return 0;
}

static int ObjMove_AddPatternA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speed = DnhValue::ToNum(L, 3);
    float angle = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, 0.0f, 0.0f, 0.0f, std::shared_ptr<ObjMove>(), std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speed = DnhValue::ToNum(L, 3);
    float angle = DnhValue::ToNum(L, 4);
    float accel = DnhValue::ToNum(L, 5);
    float angularVelocity = DnhValue::ToNum(L, 6);
    float maxSpeed = DnhValue::ToNum(L, 7);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, std::shared_ptr<ObjMove>(), std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternA3(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speed = DnhValue::ToNum(L, 3);
    float angle = DnhValue::ToNum(L, 4);
    float accel = DnhValue::ToNum(L, 5);
    float angularVelocity = DnhValue::ToNum(L, 6);
    float maxSpeed = DnhValue::ToNum(L, 7);
    int shotDataId = DnhValue::ToInt(L, 8);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        shotData = obj->IsPlayerShot() ? engine->GetPlayerShotData(shotDataId) : engine->GetEnemyShotData(shotDataId);
    }
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, std::shared_ptr<ObjMove>(), shotData));
    }
    return 0;
}

static int ObjMove_AddPatternA4(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speed = DnhValue::ToNum(L, 3);
    float angle = DnhValue::ToNum(L, 4);
    float accel = DnhValue::ToNum(L, 5);
    float angularVelocity = DnhValue::ToNum(L, 6);
    float maxSpeed = DnhValue::ToNum(L, 7);
    int baseObjId = DnhValue::ToInt(L, 8);
    int shotDataId = DnhValue::ToInt(L, 9);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        shotData = obj->IsPlayerShot() ? engine->GetPlayerShotData(shotDataId) : engine->GetEnemyShotData(shotDataId);
    }
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternA>(frame, speed, angle, accel, angularVelocity, maxSpeed, engine->GetObject<ObjMove>(baseObjId), shotData));
    }
    return 0;
}

static int ObjMove_AddPatternB1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speedX = DnhValue::ToNum(L, 3);
    float speedY = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, 0.0f, 0.0f, 0.0f, 0.0f, std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternB2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speedX = DnhValue::ToNum(L, 3);
    float speedY = DnhValue::ToNum(L, 4);
    float accelX = DnhValue::ToNum(L, 5);
    float accelY = DnhValue::ToNum(L, 6);
    float maxSpeedX = DnhValue::ToNum(L, 7);
    float maxSpeedY = DnhValue::ToNum(L, 8);
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, std::shared_ptr<ShotData>()));
    }
    return 0;
}

static int ObjMove_AddPatternB3(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    float speedX = DnhValue::ToNum(L, 3);
    float speedY = DnhValue::ToNum(L, 4);
    float accelX = DnhValue::ToNum(L, 5);
    float accelY = DnhValue::ToNum(L, 6);
    float maxSpeedX = DnhValue::ToNum(L, 7);
    float maxSpeedY = DnhValue::ToNum(L, 8);
    int shotDataId = DnhValue::ToInt(L, 9);
    std::shared_ptr<ShotData> shotData;
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        shotData = obj->IsPlayerShot() ? engine->GetPlayerShotData(shotDataId) : engine->GetEnemyShotData(shotDataId);
    }
    if (auto obj = engine->GetObject<ObjMove>(objId))
    {
        obj->AddMovePattern(std::make_shared<MovePatternB>(frame, speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY, shotData));
    }
    return 0;
}

template <float (ObjMove::*func)() const>
static int ObjMove_Get(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjMove>(objId);
    lua_pushnumber(L, obj ? ((obj.get())->*func)() : 0.0);
    return 1;
}

static int ObjMove_GetX(lua_State* L) { return ObjMove_Get<&ObjMove::GetMoveX>(L); }
static int ObjMove_GetY(lua_State* L) { return ObjMove_Get<&ObjMove::GetMoveY>(L); }
static int ObjMove_GetSpeed(lua_State* L) { return ObjMove_Get<&ObjMove::GetSpeed>(L); }
static int ObjMove_GetAngle(lua_State* L) { return ObjMove_Get<&ObjMove::GetAngle>(L); }

static int ObjEnemy_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int type = DnhValue::ToInt(L, 1);
    int objId = ID_INVALID;
    if (type == OBJ_ENEMY)
    {
        if (auto enemy = engine->CreateObjEnemy())
        {
            objId = enemy->GetID();
            script->AddAutoDeleteTargetObjectId(objId);
        }
    } else if (type == OBJ_ENEMY_BOSS)
    {
        if (auto boss = engine->GetEnemyBossObject())
        {
            objId = boss->GetID();
        }
    }
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjEnemy_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjEnemy>(objId)) { obj->Regist(); }
    return 0;
}

static int ObjEnemy_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int info = DnhValue::ToInt(L, 2);
    double ret = 0;
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        if (info == INFO_LIFE)
        {
            ret = obj->GetLife();
        } else if (info == INFO_DAMAGE_RATE_SHOT)
        {
            ret = obj->GetDamageRateShot();
        } else if (info == INFO_DAMAGE_RATE_SPELL)
        {
            ret = obj->GetDamageRateSpell();
        } else if (info == INFO_SHOT_HIT_COUNT)
        {
            ret = obj->GetPrevFrameShotHitCount();
        }
    }
    lua_pushnumber(L, ret);
    return 1;
}

static int ObjEnemy_SetLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double life = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        obj->SetLife(life);
    }
    return 0;
}

static int ObjEnemy_AddLife(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double life = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        obj->AddLife(life);
    }
    return 0;
}

static int ObjEnemy_SetDamageRate(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double damageRateShot = DnhValue::ToNum(L, 2);
    double damageRateSpell = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        obj->SetDamageRateShot(damageRateShot);
        obj->SetDamageRateSpell(damageRateSpell);
    }
    return 0;
}

static int ObjEnemy_SetIntersectionCircleToShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        obj->AddTempIntersectionCircleToShot(x, y, r);
    }
    return 0;
}

static int ObjEnemy_SetIntersectionCircleToPlayer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjEnemy>(objId))
    {
        obj->AddTempIntersectionCircleToPlayer(x, y, r);
    }
    return 0;
}

static int ObjEnemyBossScene_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjEnemyBossScene(GetSourcePos(L)))
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjEnemyBossScene_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjEnemyBossScene>(objId))
    {
        obj->Regist(GetSourcePos(L));
    }
    return 0;
}

static int ObjEnemyBossScene_Add(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int step = DnhValue::ToInt(L, 2);
    auto scriptPath = DnhValue::ToString(L, 3);
    if (auto obj = engine->GetObject<ObjEnemyBossScene>(objId))
    {
        obj->Add(step, scriptPath);
    }
    return 0;
}

static int ObjEnemyBossScene_LoadInThread(lua_State* L)
{
    // FUTURE : multi thread
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjEnemyBossScene>(objId))
    {
        obj->LoadInThread(GetSourcePos(L));
    }
    return 0;
}

static int ObjEnemyBossScene_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int info = DnhValue::ToInt(L, 2);
    auto obj = engine->GetObject<ObjEnemyBossScene>(objId);
    switch (info)
    {
        case INFO_IS_SPELL:
            lua_pushboolean(L, obj && obj->IsSpell());
            break;
        case INFO_IS_LAST_SPELL:
            lua_pushboolean(L, obj && obj->IsLastSpell());
            break;
        case INFO_IS_DURABLE_SPELL:
            lua_pushboolean(L, obj && obj->IsDurableSpell());
            break;
        case INFO_IS_LAST_STEP:
            lua_pushboolean(L, obj && obj->IsLastStep());
            break;
        case INFO_TIMER:
            lua_pushnumber(L, !obj ? 0 : obj->GetTimer());
            break;
        case INFO_TIMERF:
            lua_pushnumber(L, !obj ? 0 : obj->GetTimerF());
            break;
        case INFO_ORGTIMERF:
            lua_pushnumber(L, !obj ? 0 : obj->GetOrgTimerF());
            break;
        case INFO_SPELL_SCORE:
            lua_pushnumber(L, !obj ? 0 : obj->GetSpellScore());
            break;
        case INFO_REMAIN_STEP_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->GetRemainStepCount());
            break;
        case INFO_ACTIVE_STEP_LIFE_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->GetActiveStepLifeCount());
            break;
        case INFO_ACTIVE_STEP_TOTAL_MAX_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->GetActiveStepTotalMaxLife());
            break;
        case INFO_ACTIVE_STEP_TOTAL_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->GetActiveStepTotalLife());
            break;
        case INFO_PLAYER_SHOOTDOWN_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->GetPlayerShootDownCount());
            break;
        case INFO_PLAYER_SPELL_COUNT:
            lua_pushnumber(L, !obj ? 0 : obj->GetPlayerSpellCount());
            break;
        case INFO_ACTIVE_STEP_LIFE_RATE_LIST:
        {
            DnhArray result;
            if (obj)
            {
                for (double rate : obj->GetActiveStepLifeRateList())
                {
                    result.PushBack(std::make_unique<DnhReal>(rate));
                }
            }
            result.Push(L);
        }
        break;
        case INFO_CURRENT_LIFE:
            lua_pushnumber(L, !obj ? 0 : obj->GetCurrentLife());
            break;
        case INFO_CURRENT_LIFE_MAX:
            lua_pushnumber(L, !obj ? 0 : obj->GetCurrentLifeMax());
            break;
        default:
            return 0;
    }
    return 1;
}

static int ObjEnemyBossScene_SetSpellTimer(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int sec = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjEnemyBossScene>(objId))
    {
        obj->SetTimer(sec);
    }
    return 0;
}

static int ObjEnemyBossScene_StartSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjEnemyBossScene>(objId))
    {
        obj->StartSpell();
    }
    return 0;
}


static int ObjShot_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    int shotType = DnhValue::ToInt(L, 1);
    int objId = ID_INVALID;
    bool isPlayerShot = script->GetType() == SCRIPT_TYPE_PLAYER;
    switch (shotType)
    {
        case OBJ_SHOT:
            objId = engine->CreateObjShot(isPlayerShot)->GetID();
            break;
        case OBJ_LOOSE_LASER:
            objId = engine->CreateObjLooseLaser(isPlayerShot)->GetID();
            break;
        case OBJ_STRAIGHT_LASER:
            objId = engine->CreateObjStLaser(isPlayerShot)->GetID();
            break;
        case OBJ_CURVE_LASER:
            objId = engine->CreateObjCrLaser(isPlayerShot)->GetID();
            break;
    }
    script->AddAutoDeleteTargetObjectId(objId);
    lua_pushnumber(L, objId);
    return 1;
}

static int ObjShot_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        // NOTE : IsPermitPlayerShot == falseならregistしない, ObjShot_Registで作成したときのみ
        if (auto player = engine->GetPlayerObject())
        {
            if (!player->IsPermitPlayerShot())
            {
                return 0;
            }
        }
        obj->Regist();
    }
    return 0;
}

static int ObjShot_SetAutoDelete(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool autoDelete = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetAutoDeleteEnable(autoDelete); }
    return 0;
}

static int ObjShot_FadeDelete(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->FadeDelete(); }
    return 0;
}

static int ObjShot_SetDeleteFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int deleteFrame = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetDeleteFrame(deleteFrame); }
    return 0;
}

static int ObjShot_SetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double damage = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetDamage(damage); }
    return 0;
}

static int ObjShot_SetDelay(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int delay = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetDelay(delay); }
    return 0;
}

static int ObjShot_SetSpellResist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool spellResist = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetSpellResistEnable(spellResist); }
    return 0;
}

static int ObjShot_SetGraphic(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int shotDataId = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        if (obj->IsPlayerShot())
        {
            obj->SetShotData(engine->GetPlayerShotData(shotDataId));
        } else
        {
            obj->SetShotData(engine->GetEnemyShotData(shotDataId));
        }
    }
    return 0;
}

static int ObjShot_SetSourceBlendType(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int blendType = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetSourceBlendType(blendType); }
    return 0;
}

static int ObjShot_SetPenetration(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int penetration = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetPenetration(penetration); }
    return 0;
}

static int ObjShot_SetEraseShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool eraseShot = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetEraseShotEnable(eraseShot); }
    return 0;
}

static int ObjShot_SetSpellFactor(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool spellFactor = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) { obj->SetSpellFactor(spellFactor); }
    return 0;
}

static int ObjShot_ToItem(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->ToItem();
    return 0;
}

static int ObjShot_AddShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int addShotId = DnhValue::ToInt(L, 2);
    int frame = DnhValue::ToInt(L, 3);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->AddShotA1(addShotId, frame);
    return 0;
}

static int ObjShot_AddShotA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int addShotId = DnhValue::ToInt(L, 2);
    int frame = DnhValue::ToInt(L, 3);
    float dist = DnhValue::ToNum(L, 4);
    float angle = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->AddShotA2(addShotId, frame, dist, angle);
    return 0;
}

static int ObjShot_SetIntersectionEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->SetIntersectionEnable(enable);
    return 0;
}

static int ObjShot_SetIntersectionCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double r = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->AddTempIntersectionCircleA1(r);
    return 0;
}

static int ObjShot_SetIntersectionCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->AddTempIntersectionCircleA2(x, y, r);
    return 0;
}

static int ObjShot_SetIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x1 = DnhValue::ToNum(L, 2);
    double y1 = DnhValue::ToNum(L, 3);
    double x2 = DnhValue::ToNum(L, 4);
    double y2 = DnhValue::ToNum(L, 5);
    double width = DnhValue::ToNum(L, 6);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->AddTempIntersectionLine(x1, y1, x2, y2, width);
    return 0;
}

static int ObjShot_SetItemChange(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool itemChange = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjShot>(objId)) obj->SetItemChangeEnable(itemChange);
    return 0;
}

static int ObjShot_GetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->GetDamage() : 0);
    return 1;
}

static int ObjShot_GetPenetration(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->GetPenetration() : 0);
    return 1;
}

static int ObjShot_GetDelay(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjShot>(objId);
    lua_pushnumber(L, obj ? obj->GetDelay() : 0);
    return 1;
}

static int ObjShot_IsSpellResist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjShot>(objId);
    lua_pushboolean(L, obj && obj->IsSpellResistEnabled());
    return 1;
}

static int ObjShot_GetImageID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjShot>(objId))
    {
        if (const auto& shotData = obj->GetShotData())
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
    int objId = DnhValue::ToInt(L, 1);
    double length = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjLaser>(objId))
    {
        obj->SetLength(length);
    }
    return 0;
}

static int ObjLaser_SetRenderWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double width = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjLaser>(objId))
    {
        obj->SetRenderWidth(width);
    }
    return 0;
}

static int ObjLaser_SetIntersectionWidth(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double width = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjLaser>(objId))
    {
        obj->SetIntersectionWidth(width);
    }
    return 0;
}

static int ObjLaser_SetGrazeInvalidFrame(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int frame = DnhValue::ToInt(L, 2);
    if (auto obj = engine->GetObject<ObjLaser>(objId))
    {
        obj->SetGrazeInvalidFrame(frame);
    }
    return 0;
}

static int ObjLaser_SetInvalidLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double head = DnhValue::ToNum(L, 2);
    double tail = DnhValue::ToNum(L, 3);
    if (auto obj = engine->GetObject<ObjLooseLaser>(objId))
    {
        obj->SetInvalidLength(head, tail);
    }
    return 0;
}

static int ObjLaser_SetItemDistance(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double dist = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjLaser>(objId))
    {
        obj->SetItemDistance(dist);
    }
    return 0;
}

static int ObjLaser_GetLength(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjLaser>(objId);
    lua_pushnumber(L, obj ? obj->GetLength() : 0);
    return 1;
}

static int ObjStLaser_SetAngle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double angle = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjStLaser>(objId))
    {
        obj->SetLaserAngle(angle);
    }
    return 0;
}

static int ObjStLaser_GetAngle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjStLaser>(objId);
    lua_pushnumber(L, obj ? obj->GetLaserAngle() : 0);
    return 1;
}

static int ObjStLaser_SetSource(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool source = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjStLaser>(objId))
    {
        obj->SetSource(source);
    }
    return 0;
}

static int ObjCrLaser_SetTipDecrement(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double decr = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjCrLaser>(objId))
    {
        obj->SetTipDecrement(decr);
    }
    return 0;
}

static int ObjItem_SetItemID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int itemDataId = DnhValue::ToInt(L, 2);
    if (auto item = engine->GetObject<ObjItem>(objId))
    {
        item->SetItemData(engine->GetItemData(objId));
    }
    return 0;
}

static int ObjItem_SetRenderScoreEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto item = engine->GetObject<ObjItem>(objId))
    {
        item->SetRenderScoreEnable(enable);
    }
    return 0;
}

static int ObjItem_SetAutoCollectEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    if (auto item = engine->GetObject<ObjItem>(objId))
    {
        item->SetAutoCollectEnable(enable);
    }
    return 0;
}

static int ObjItem_SetDefinedMovePatternA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int pattern = DnhValue::ToInt(L, 2);
    if (auto item = engine->GetObject<ObjItem>(objId))
    {
        if (pattern == ITEM_MOVE_DOWN)
        {
            item->SetMoveMode(std::make_shared<MoveModeItemDown>(2.5f));
        } else if (pattern == ITEM_MOVE_TOPLAYER)
        {
            item->SetMoveMode(std::make_shared<MoveModeItemToPlayer>(8.0f, engine->GetPlayerObject()));
        }
    }
    return 0;
}

static int ObjItem_GetInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    int info = DnhValue::ToInt(L, 2);
    if (auto item = engine->GetObject<ObjItem>(objId))
    {
        if (info == INFO_ITEM_SCORE)
        {
            lua_pushnumber(L, item->GetScore());
            return 1;
        }
    }
    return 0;
}

static int ObjPlayer_AddIntersectionCircleA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double dx = DnhValue::ToNum(L, 2);
    double dy = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    double dr = DnhValue::ToNum(L, 5);
    if (auto obj = engine->GetObject<ObjPlayer>(objId))
    {
        obj->AddIntersectionCircleA1(dx, dy, r, dr);
    }
    return 0;
}

static int ObjPlayer_AddIntersectionCircleA2(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double dx = DnhValue::ToNum(L, 2);
    double dy = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjPlayer>(objId))
    {
        obj->AddIntersectionCircleA2(dx, dy, r);
    }
    return 0;
}

static int ObjPlayer_ClearIntersection(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjPlayer>(objId))
    {
        obj->ClearIntersection();
    }
    return 0;
}

static int ObjCol_IsIntersected(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjCol>(objId);
    lua_pushboolean(L, obj && obj->IsIntersected());
    return 1;
}

static int ObjCol_GetListOfIntersectedEnemyID(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    DnhArray enemyIds;
    if (auto obj = engine->GetObject<ObjCol>(objId))
    {
        for (auto& wIsect : obj->GetCollideIntersections())
        {
            if (auto isect = wIsect.lock())
            {
                if (auto enemyIsectToShot = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect))
                {
                    if (auto enemy = enemyIsectToShot->GetEnemy().lock())
                    {
                        enemyIds.PushBack(std::make_unique<DnhReal>(enemy->GetID()));
                    }
                } else if (auto enemyIsectToPlayer = std::dynamic_pointer_cast<EnemyIntersectionToPlayer>(isect))
                {
                    if (auto enemy = enemyIsectToPlayer->GetEnemy().lock())
                    {
                        enemyIds.PushBack(std::make_unique<DnhReal>(enemy->GetID()));
                    }
                }
            }
        }
    }
    enemyIds.Push(L);
    return 1;
}

static int ObjCol_GetIntersectedCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    auto obj = engine->GetObject<ObjCol>(objId);
    lua_pushnumber(L, obj ? obj->GetIntersectedCount() : 0);
    return 1;
}

static int CallSpell(lua_State* L)
{
    Engine* engine = getEngine(L);
    if (auto player = engine->GetPlayerObject())
    {
        player->CallSpell();
    }
    return 0;
}

static int LoadPlayerShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->LoadPlayerShotData(path, GetSourcePos(L));
    return 0;
}

static int ReloadPlayerShotData(lua_State* L)
{
    Engine* engine = getEngine(L);
    std::wstring path = DnhValue::ToString(L, 1);
    engine->ReloadPlayerShotData(path, GetSourcePos(L));
    return 0;
}

static int GetSpellManageObject(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto obj = engine->GetSpellManageObject();
    lua_pushnumber(L, obj ? obj->GetID() : ID_INVALID);
    return 1;
}

static int ObjSpell_Create(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    if (auto obj = engine->CreateObjSpell())
    {
        script->AddAutoDeleteTargetObjectId(obj->GetID());
        lua_pushnumber(L, obj->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int ObjSpell_Regist(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    if (auto obj = engine->GetObject<ObjSpell>(objId)) { obj->Regist(); }
    return 0;
}

static int ObjSpell_SetDamage(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double damage = DnhValue::ToNum(L, 2);
    if (auto obj = engine->GetObject<ObjSpell>(objId)) { obj->SetDamage(damage); }
    return 0;
}

static int ObjSpell_SetEraseShot(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    bool eraseShot = DnhValue::ToBool(L, 2);
    if (auto obj = engine->GetObject<ObjSpell>(objId)) { obj->SetEraseShotEnable(eraseShot); }
    return 0;
}

static int ObjSpell_SetIntersectionCircle(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x = DnhValue::ToNum(L, 2);
    double y = DnhValue::ToNum(L, 3);
    double r = DnhValue::ToNum(L, 4);
    if (auto obj = engine->GetObject<ObjSpell>(objId))
    {
        obj->AddTempIntersectionCircle(x, y, r);
    }
    return 0;
}

static int ObjSpell_SetIntersectionLine(lua_State* L)
{
    Engine* engine = getEngine(L);
    int objId = DnhValue::ToInt(L, 1);
    double x1 = DnhValue::ToNum(L, 2);
    double y1 = DnhValue::ToNum(L, 3);
    double x2 = DnhValue::ToNum(L, 4);
    double y2 = DnhValue::ToNum(L, 5);
    double width = DnhValue::ToNum(L, 6);
    if (auto obj = engine->GetObject<ObjSpell>(objId))
    {
        obj->AddTempIntersectionLine(x1, y1, x2, y2, width);
    }
    return 0;
}

static int SetPauseScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetPauseScriptPath(path);
    return 0;
}

static int SetEndSceneScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetEndSceneScriptPath(path);
    return 0;
}

static int SetReplaySaveSceneScriptPath(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetReplaySaveSceneScriptPath(path);
    return 0;
}

static int CreatePlayerShotA1(lua_State* L)
{
    Engine* engine = getEngine(L);
    Script* script = GetScript(L);
    double x = DnhValue::ToNum(L, 1);
    double y = DnhValue::ToNum(L, 2);
    double speed = DnhValue::ToNum(L, 3);
    double angle = DnhValue::ToNum(L, 4);
    double damage = DnhValue::ToNum(L, 5);
    int penetration = DnhValue::ToInt(L, 6);
    int shotDataId = DnhValue::ToInt(L, 7);
    if (auto shot = engine->CreatePlayerShotA1(x, y, speed, angle, damage, penetration, shotDataId))
    {
        lua_pushnumber(L, shot->GetID());
        script->AddAutoDeleteTargetObjectId(shot->GetID());
    } else
    {
        lua_pushnumber(L, ID_INVALID);
    }
    return 1;
}

static int GetTransitionRenderTargetName(lua_State* L)
{
    Engine* engine = getEngine(L);
    DnhArray(engine->GetTransitionRenderTargetName()).Push(L);
    return 1;
}

static int SetShotDeleteEventEnable(lua_State* L)
{
    Engine* engine = getEngine(L);
    int targetEvent = DnhValue::ToInt(L, 1);
    bool enable = DnhValue::ToBool(L, 2);
    switch (targetEvent)
    {
        case EV_DELETE_SHOT_IMMEDIATE:
            engine->SetDeleteShotImmediateEventOnShotScriptEnable(enable);
            break;
        case EV_DELETE_SHOT_FADE:
            engine->SetDeleteShotFadeEventOnShotScriptEnable(enable);
            break;
        case EV_DELETE_SHOT_TO_ITEM:
            engine->SetDeleteShotToItemEventOnShotScriptEnable(enable);
            break;
    }
    return 0;
}

static int ClosePackage(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->ClosePackage();
    return 0;
}

static int InitializeStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->InitializeStageScene();
    return 0;
}

static int FinalizeStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->FinalizeStageScene();
    return 0;
}

static int StartStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->StartStageScene(GetSourcePos(L));
    return 0;
}

static int SetStageIndex(lua_State* L)
{
    Engine* engine = getEngine(L);
    int idx = DnhValue::ToInt(L, 1);
    engine->SetStageIndex(idx);
    return 0;
}

static int SetStageMainScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetStageMainScript(path, GetSourcePos(L));
    return 0;
}

static int SetStagePlayerScript(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetStagePlayerScript(path, GetSourcePos(L));
    return 0;
}

static int SetStageReplayFile(lua_State* L)
{
    Engine* engine = getEngine(L);
    auto path = DnhValue::ToString(L, 1);
    engine->SetStageReplayFile(path);
    return 0;
}

static int GetStageSceneState(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->IsStageFinished() ? STAGE_STATE_FINISHED : -1);
    return 1;
}

static int GetStageSceneResult(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetStageSceneResult());
    return 1;
}

static int PauseStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    bool doPause = DnhValue::ToBool(L, 1);
    engine->PauseStageScene(doPause);
    return 0;
}

static int TerminateStageScene(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->TerminateStageScene();
    return 0;
}

static int GetLoadFreePlayerScriptList(lua_State* L)
{
    Engine* engine = getEngine(L);
    engine->GetLoadFreePlayerScriptList();
    return 0;
}

static int GetFreePlayerScriptCount(lua_State* L)
{
    Engine* engine = getEngine(L);
    lua_pushnumber(L, engine->GetFreePlayerScriptCount());
    return 1;
}

static int GetFreePlayerScriptInfo(lua_State* L)
{
    Engine* engine = getEngine(L);
    int idx = DnhValue::ToInt(L, 1);
    int infoType = DnhValue::ToInt(L, 2);
    if (idx >= 0 && idx < engine->GetFreePlayerScriptCount())
    {
        ScriptInfo info = engine->GetFreePlayerScriptInfo(idx);
        switch (infoType)
        {
            case INFO_SCRIPT_PATH:
                DnhArray(info.path).Push(L);
                break;
            case INFO_SCRIPT_ID:
                DnhArray(info.id).Push(L);
                break;
            case INFO_SCRIPT_TITLE:
                DnhArray(info.title).Push(L);
                break;
            case INFO_SCRIPT_TEXT:
                DnhArray(info.text).Push(L);
                break;
            case INFO_SCRIPT_IMAGE:
                DnhArray(info.imagePath).Push(L);
                break;
            case INFO_SCRIPT_REPLAY_NAME:
                DnhArray(info.replayName).Push(L);
                break;
            default:
                DnhArray(L"").Push(L);
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
    DnhArray(L"").Push(L);
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
    DnhArray(L"").Push(L);
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

int GetCurrentLine(lua_State* L)
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

std::shared_ptr<SourcePos> GetSourcePos(lua_State * L)
{
    int line = GetCurrentLine(L);
    Script* script = GetScript(L);
    return script->GetSourcePos(line);
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
        int line = GetCurrentLine(L);
        Script* script = GetScript(L);
        auto srcPos = GetSourcePos(L);
        log.AddSourcePos(srcPos);
        script->SaveError(std::current_exception());
        lua_pushstring(L, "script_runtime_error");
    } catch (const std::exception& e)
    {
        int line = GetCurrentLine(L);
        Script* script = GetScript(L);
        auto srcPos = GetSourcePos(L);
        try
        {
            throw Log(Log::Level::LV_ERROR)
                .SetMessage("unexpected script runtime error occured.")
                .SetParam(Log::Param(Log::Param::Tag::TEXT, e.what()))
                .AddSourcePos(srcPos);
        } catch (...)
        {
            script->SaveError(std::current_exception());
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

void SetPointer(lua_State* L, const char* key, void* p)
{
    lua_pushstring(L, key);
    lua_pushlightuserdata(L, p);
    lua_rawset(L, LUA_REGISTRYINDEX);
}

void* GetPointer(lua_State* L, const char* key)
{
    lua_pushstring(L, key);
    lua_rawget(L, LUA_REGISTRYINDEX);
    void* p = lua_touserdata(L, -1);
    return p;
}

Engine* getEngine(lua_State* L)
{
    return (Engine*)GetPointer(L, "Engine");
}

void setEngine(lua_State* L, Engine* p)
{
    SetPointer(L, "Engine", p);
}

Script* GetScript(lua_State* L)
{
    return (Script*)GetPointer(L, "Script");
}

void SetScript(lua_State* L, Script* p)
{
    SetPointer(L, "Script", p);
}

int c_chartonum(lua_State* L)
{
    std::wstring wstr = DnhValue::ToString(L, 1);
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
    std::wstring wstr = DnhValue::ToString(L, 1);
    if (wstr.empty())
    {
        lua_pushstring(L, "");
    } else
    {
        wchar_t c = (int)wstr[0];
        c++;
        std::string r = ToUTF8(std::wstring{ c });
        lua_pushstring(L, r.c_str());
    }
    return 1;
}

int c_predchar(lua_State* L)
{
    std::wstring wstr = DnhValue::ToString(L, 1);
    if (wstr.empty())
    {
        lua_pushstring(L, "");
    } else
    {
        wchar_t c = (int)wstr[0];
        c--;
        std::string r = ToUTF8(std::wstring{ c });
        lua_pushstring(L, r.c_str());
    }
    return 1;
}

int __c_raiseerror(lua_State* L)
{
    std::string msg = lua_tostring(L, 1);
    throw Log(Log::Level::LV_ERROR).SetMessage(msg);
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

        unsafe(DeleteShotAll, 2); // NotifyEvent
        unsafe(DeleteShotInCircle, 5); // NotifyEvent
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
        unsafe(ObjEnemyBossScene_StartSpell, 1); // NotifyEvent

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
        unsafe(ObjShot_ToItem, 1); // NotifyEvent
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
        unsafe(CallSpell, 0); // NotifyEvent
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
        unsafe(PauseStageScene, 1); // NotifyEvent
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