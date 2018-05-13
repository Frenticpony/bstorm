#include <bstorm/package.hpp>

#include <bstorm/util.hpp>
#include <bstorm/const.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/graphic_device.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/time_point.hpp>
#include <bstorm/fps_counter.hpp>
#include <bstorm/input_device.hpp>
#include <bstorm/virtual_key_input_source.hpp>
#include <bstorm/real_device_input_source.hpp>
#include <bstorm/sound_device.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/lostable_graphic_resource.hpp>
#include <bstorm/render_target.hpp>
#include <bstorm/shader.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/mesh.hpp>
#include <bstorm/camera2D.hpp>
#include <bstorm/camera3D.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/common_data_db.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/obj_render.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_mesh.hpp>
#include <bstorm/obj_text.hpp>
#include <bstorm/obj_sound.hpp>
#include <bstorm/obj_file.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>
#include <bstorm/obj_spell.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/shot_counter.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/script.hpp>
#include <bstorm/config.hpp>

#include <exception>
#include <ctime>

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

#undef CreateFont


namespace bstorm
{
Package::Package(HWND hWnd, int screenWidth, int screenHeight, const std::shared_ptr<conf::KeyConfig>& defaultKeyConfig) :
    hWnd(hWnd),
    graphicDevice(std::make_unique<GraphicDevice>(hWnd)),
    lostableGraphicResourceManager(std::make_unique<LostableGraphicResourceManager>()),
    defaultKeyConfig(defaultKeyConfig),
    fpsCounter(std::make_shared<FpsCounter>()),
    inputDevice(std::make_shared<InputDevice>(hWnd, mousePosProvider)),
    keyAssign(std::make_shared<KeyAssign>()),
    fileLoader(std::make_shared<FileLoaderFromTextFile>()),
    vKeyInputSource(std::make_shared<RealDeviceInputSource>(inputDevice, keyAssign)),
    soundDevice(std::make_shared<SoundDevice>(hWnd)),
    renderer(std::make_shared<Renderer>(graphicDevice->GetDevice())),
    objTable(std::make_shared<ObjectTable>()),
    objLayerList(std::make_shared<ObjectLayerList>()),
    colDetector(std::make_shared<CollisionDetector>(screenWidth, screenHeight, std::make_shared<CollisionMatrix>(DEFAULT_COLLISION_MATRIX_DIMENSION, DEFAULT_COLLISION_MATRIX))),
    textureCache(std::make_shared<TextureCache>(graphicDevice->GetDevice())),
    meshCache(std::make_shared<MeshCache>(textureCache, fileLoader)),
    camera2D(std::make_shared<Camera2D>()),
    camera3D(std::make_shared<Camera3D>()),
    commonDataDB(std::make_shared<CommonDataDB>()),
#ifndef _DEBUG 
    FIXME!
#endif
    scriptManager(std::make_shared<ScriptManager>(this)),
    playerShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureCache, fileLoader)),
    enemyShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureCache, fileLoader)),
    itemDataTable(std::make_shared<ItemDataTable>(textureCache, fileLoader)),
    shotCounter(std::make_shared<ShotCounter>()),
    randGenerator(std::make_shared<RandGenerator>((uint32_t)this)), // FUTURE : from rand
    itemScoreTextSpawner(std::make_shared<ItemScoreTextSpawner>()),
    defaultBonusItemSpawner(std::make_shared<DefaultBonusItemSpawner>()),
    autoItemCollectionManager(std::make_shared<AutoItemCollectionManager>()),
    elapsedFrame(0),
    packageStartTime(std::make_shared<TimePoint>()),
    stageStartTime(std::make_shared<TimePoint>()),
    pseudoPlayerFps(60),
    pseudoEnemyFps(60),
    packageMainScriptInfo(),
    stageMainScriptInfo(),
    stagePlayerScriptInfo(),
    stageIdx(ID_INVALID),
    stageSceneResult(0),
    stagePaused(true),
    stageForceTerminated(false),
    deleteShotImmediateEventOnShotScriptEnable(false),
    deleteShotFadeEventOnShotScriptEnable(false),
    deleteShotToItemEventOnShotScriptEnable(false),
    screenWidth(screenWidth),
    screenHeight(screenHeight),
    renderIntersectionEnable(false),
    forcePlayerInvincibleEnable(false),
    defaultBonusItemEnable(true),
    stgFrame_(32.0f, 16.0f, 416.0f, 464.0f),
    shotAutoDeleteClip_(64.0f, 64.0f, 64.0f, 64.0f),
    fontCache_(std::make_shared<FontCache>(hWnd, graphicDevice->GetDevice()))
{
    Logger::SetEnable(false);
    Reset2DCamera();
    ResetCamera();
    CreateRenderTarget(GetTransitionRenderTargetName(), GetScreenWidth(), GetScreenHeight(), nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(0), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(1), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(2), 1024, 512, nullptr);
    renderer->SetForbidCameraViewProjMatrix2D(GetScreenWidth(), GetScreenHeight());
    renderer->SetFogEnable(false);
    Logger::SetEnable(true);
    Logger::WriteLog(Log::Level::LV_INFO, "boot package.");
    for (const auto& keyMap : defaultKeyConfig->keyMaps)
    {
        keyAssign->AddVirtualKey(keyMap.vkey, keyMap.key, keyMap.pad);
    }
}

Package::~Package()
{
    Logger::WriteLog(Log::Level::LV_INFO, "shutdown package.");
}

HWND Package::GetWindowHandle() const
{
    return hWnd;
}

void Package::TickFrame()
{
    if (IsPackageFinished())
    {
        Logger::WriteLog(Log::Level::LV_WARN, "package is not setted, please select package.");
        return;
    }

    if (auto packageMain = packageMainScript.lock())
    {
        if (packageMain->IsClosed())
        {
            Logger::WriteLog(Log::Level::LV_INFO, "finish package.");
            packageMainScript.reset();
            return;
        }
    }

    if (GetElapsedFrame() % (60 / std::min(pseudoEnemyFps, pseudoPlayerFps)) == 0)
    {
        if (auto stageMain = stageMainScript.lock())
        {
            if (stageMain->IsClosed())
            {
                if (stageForceTerminated)
                {
                    stageSceneResult = STAGE_RESULT_BREAK_OFF;
                    SetPlayerLife(0);
                    SetPlayerSpell(3);
                    SetPlayerPower(1);
                    SetPlayerScore(0);
                    SetPlayerGraze(0);
                    SetPlayerPoint(0);
                } else if (auto player = GetPlayerObject())
                {
                    stageSceneResult = player->GetState() != STATE_END ? STAGE_RESULT_CLEARED : STAGE_RESULT_PLAYER_DOWN;
                } else
                {
                    stageSceneResult = STAGE_RESULT_PLAYER_DOWN;
                }
                RenderToTextureA1(GetTransitionRenderTargetName(), 0, MAX_RENDER_PRIORITY, true);
                objTable->DeleteStgSceneObject();
                scriptManager->CloseStgSceneScript();
                stageMainScript.reset();
                stagePlayerScript.reset();
                stageForceTerminated = false;
                stagePaused = true;
                pseudoPlayerFps = pseudoEnemyFps = 60;
            }
        }
        scriptManager->CleanClosedScript();

        if (!IsStagePaused())
        {
            colDetector->TestAllCollision();
        }

        // SetShotIntersection{Circle, Line}で設定した判定削除
        tempEnemyShotIsects.clear();

        inputDevice->UpdateInputState();

        scriptManager->RunAll(IsStagePaused());

        objTable->UpdateAll(IsStagePaused());

        autoItemCollectionManager->Reset();
    }
    // 使われなくなったリソース開放
    switch (GetElapsedFrame() % 1920)
    {
        case 0:
            ReleaseUnusedLostableGraphicResource();
            break;
        case 480:
            ReleaseUnusedTexture();
            break;
        case 960:
            ReleaseUnusedFont();
            break;
        case 1440:
            ReleaseUnusedMesh();
            break;
    }
    elapsedFrame += 1;
}

void Package::Render()
{
    RenderToTexture(L"", 0, MAX_RENDER_PRIORITY, ID_INVALID, true, true, true, true);
}

void Package::Render(const std::wstring& renderTargetName)
{
    RenderToTexture(renderTargetName, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
}

void Package::RenderToTextureA1(const std::wstring& name, int begin, int end, bool doClear)
{
    RenderToTexture(name, begin, end, ID_INVALID, doClear, false, false, false);
}

void Package::RenderToTextureB1(const std::wstring& name, int objId, bool doClear)
{
    RenderToTexture(name, 0, 0, objId, doClear, false, false, false);
}

int Package::GetScreenWidth() const
{
    return screenWidth;
}

int Package::GetScreenHeight() const
{
    return screenHeight;
}

bool Package::IsRenderIntersectionEnabled() const
{
    return renderIntersectionEnable;
}

void Package::SetRenderIntersectionEnable(bool enable)
{
    renderIntersectionEnable = enable;
}

bool Package::IsForcePlayerInvincibleEnabled() const
{
    return forcePlayerInvincibleEnable;
}

void Package::SetForcePlayerInvincibleEnable(bool enable)
{
    forcePlayerInvincibleEnable = enable;
}

IDirect3DDevice9* Package::GetDirect3DDevice() const
{
    return graphicDevice->GetDevice();
}

void Package::ResetGraphicDevice()
{
    graphicDevice->Reset();
}

void Package::AddLostableGraphicResource(const std::shared_ptr<LostableGraphicResource>& resource)
{
    lostableGraphicResourceManager->AddResource(resource);
}

void Package::ReleaseLostableGraphicResource()
{
    lostableGraphicResourceManager->OnLostDeviceAll();
}

void Package::RestoreLostableGraphicDevice()
{
    lostableGraphicResourceManager->OnResetDeviceAll();
}

void Package::SetBackBufferRenderTarget()
{
    graphicDevice->SetBackbufferRenderTarget();
}

void Package::ReleaseUnusedLostableGraphicResource()
{
    lostableGraphicResourceManager->ReleaseUnusedResource();
}

KeyState Package::GetKeyState(Key k)
{
    return inputDevice->GetKeyState(k);
}

KeyState Package::GetVirtualKeyState(VirtualKey vk)
{
    return vKeyInputSource->GetVirtualKeyState(vk);
}

void Package::SetVirtualKeyState(VirtualKey vk, KeyState state)
{
    vKeyInputSource->SetVirtualKeyState(vk, state);
}

void Package::AddVirtualKey(VirtualKey vk, Key k, PadButton btn)
{
    keyAssign->AddVirtualKey(vk, k, btn);
}

KeyState Package::GetMouseState(MouseButton btn)
{
    return inputDevice->GetMouseState(btn);
}

int Package::GetMouseX()
{
    return inputDevice->GetMouseX(GetScreenWidth(), GetScreenHeight());
}

int Package::GetMouseY()
{
    return inputDevice->GetMouseY(GetScreenWidth(), GetScreenHeight());
}

int Package::GetMouseMoveZ()
{
    return inputDevice->GetMouseMoveZ();
}

void Package::SetMousePostionProvider(const std::shared_ptr<MousePositionProvider>& provider)
{
    mousePosProvider = provider;
    inputDevice->SetMousePositionProvider(mousePosProvider);
}

void Package::SetInputEnable(bool enable)
{
    inputDevice->SetInputEnable(enable);
}

void Package::WriteLog(const std::string && msg, const std::shared_ptr<SourcePos>& srcPos)
{
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_USER)
        .SetMessage(std::move(msg))
        .AddSourcePos(srcPos)));
}

std::wstring Package::GetCurrentDateTimeS()
{
    time_t now = std::time(nullptr);
    struct tm* local = std::localtime(&now);
    std::string buf(14, '\0');
    sprintf(&buf[0], "%04d%02d%02d%02d%02d%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
    return ToUnicode(buf);
}

float Package::GetCurrentFps() const
{
    return fpsCounter->GetStable();
}

float Package::GetStageTime() const
{
    if (IsStageFinished())
    {
        return 0.0;
    }
    return stageStartTime->GetElapsedMilliSec();
}

float Package::GetPackageTime() const
{
    return packageStartTime->GetElapsedMilliSec();
}

void Package::UpdateFpsCounter()
{
    fpsCounter->Update();
}

void Package::ResetFpsCounter()
{
    fpsCounter = std::make_shared<FpsCounter>();
}

void Package::StartSlow(int pseudoFps, bool byPlayer)
{
    pseudoFps = constrain(pseudoFps, 1, 60);
    if (byPlayer)
    {
        pseudoPlayerFps = pseudoFps;
    } else
    {
        pseudoEnemyFps = pseudoFps;
    }
}

void Package::StopSlow(bool byPlayer)
{
    if (byPlayer)
    {
        pseudoPlayerFps = 60;
    } else
    {
        pseudoEnemyFps = 60;
    }
}

int64_t Package::GetElapsedFrame() const
{
    return elapsedFrame;
}

std::wstring Package::GetMainStgScriptPath() const
{
    return stageMainScriptInfo.path;
}

std::wstring Package::GetMainStgScriptDirectory() const
{
    return GetParentPath(stageMainScriptInfo.path) + L"/";
}

std::wstring Package::GetMainPackageScriptPath() const
{
    return packageMainScriptInfo.path;
}

std::shared_ptr<Texture> Package::LoadTexture(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    return textureCache->Load(path, reserve, srcPos);
}

void Package::LoadTextureInThread(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) noexcept(true)
{
    textureCache->LoadInThread(path, reserve, srcPos);
}

void Package::RemoveTextureReservedFlag(const std::wstring & path)
{
    textureCache->RemoveReservedFlag(path);
}

void Package::ReleaseUnusedTexture()
{
    textureCache->ReleaseUnusedTexture();
}

std::shared_ptr<Font> Package::CreateFont(const FontParams& param)
{
    return fontCache_->Create(param);
}

void Package::ReleaseUnusedFont()
{
    fontCache_->ReleaseUnusedFont();
}

const std::unordered_map<FontParams, std::shared_ptr<Font>>& Package::GetFontMap() const
{
    return fontCache_->GetFontMap();
}

bool Package::InstallFont(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    bool result = bstorm::InstallFont(path);
    if (!result)
    {
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("failed to install font.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path))
            .AddSourcePos(srcPos)));
    }
    return result;
}

std::shared_ptr<RenderTarget> Package::CreateRenderTarget(const std::wstring & name, int width, int height, const std::shared_ptr<SourcePos>& srcPos)
{
    auto renderTarget = std::make_shared<RenderTarget>(name, width, height, GetDirect3DDevice());
    renderTarget->SetViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    renderTargets[name] = renderTarget;
    AddLostableGraphicResource(renderTarget);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create render target.")
        .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
        .AddSourcePos(srcPos)));
    return renderTarget;
}

void Package::RemoveRenderTarget(const std::wstring & name, const std::shared_ptr<SourcePos>& srcPos)
{
    auto it = renderTargets.find(name);
    if (it != renderTargets.end())
    {
        renderTargets.erase(it);
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_INFO)
            .SetMessage("remove render target.")
            .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
            .AddSourcePos(srcPos)));
    }
}

NullableSharedPtr<RenderTarget> Package::GetRenderTarget(const std::wstring & name) const
{
    auto it = renderTargets.find(name);
    if (it != renderTargets.end())
    {
        return it->second;
    }
    return nullptr;
}

std::wstring Package::GetReservedRenderTargetName(int idx) const
{
    return RESERVED_RENDER_TARGET_PREFIX + std::to_wstring(idx);
}

std::wstring Package::GetTransitionRenderTargetName() const
{
    return TRANSITION_RENDER_TARGET_NAME;
}

static D3DXIMAGE_FILEFORMAT getProperFileFormat(const std::wstring& path)
{
    auto ext = GetLowerExt(path);
    if (ext == L".png" || ext.empty()) return D3DXIFF_PNG;
    if (ext == L".bmp") return D3DXIFF_BMP;
    if (ext == L".dds") return D3DXIFF_DDS;
    if (ext == L".jpg" || ext == L".jpeg") return D3DXIFF_JPG;
    if (ext == L".dib") return D3DXIFF_DIB;
    if (ext == L".hdr") return D3DXIFF_HDR;
    if (ext == L".pfm") return D3DXIFF_PFM;
    return D3DXIFF_PNG;
}

void Package::SaveRenderedTextureA1(const std::wstring & name, const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto renderTarget = GetRenderTarget(name))
    {
        auto viewport = renderTarget->GetViewport();
        SaveRenderedTextureA2(name, path, viewport.X, viewport.Y, viewport.X + viewport.Width, viewport.Y + viewport.Height, srcPos);
    }
}

void Package::SaveRenderedTextureA2(const std::wstring & name, const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto renderTarget = GetRenderTarget(name))
    {
        auto viewport = renderTarget->GetViewport();
        RECT rect = { left, top, right, bottom };
        MakeDirectoryP(GetParentPath(path));
        if (SUCCEEDED(D3DXSaveSurfaceToFile(path.c_str(), getProperFileFormat(path), renderTarget->GetSurface(), NULL, &rect)))
        {
            return;
        }
    }
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .SetMessage("failed to save render target.")
        .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
        .AddSourcePos(srcPos)));
}

void Package::SaveSnapShotA1(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    SaveSnapShotA2(path, 0, 0, GetScreenWidth(), GetScreenHeight(), srcPos);
}

void Package::SaveSnapShotA2(const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos)
{
    RemoveRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, srcPos);
    try
    {
        CreateRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, GetScreenWidth(), GetScreenHeight(), srcPos);
        RenderToTexture(SNAP_SHOT_RENDER_TARGET_NAME, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
        SaveRenderedTextureA2(SNAP_SHOT_RENDER_TARGET_NAME, path, left, top, right, bottom, srcPos);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("failed to create snap shot.")
            .SetParam(Log::Param(Log::Param::Tag::TEXT, path))
            .AddSourcePos(srcPos)));
    }
    RemoveRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, srcPos);
}

std::shared_ptr<Shader> Package::CreateShader(const std::wstring & path, bool precompiled)
{
    auto shader = std::make_shared<Shader>(path, precompiled, GetDirect3DDevice());
    AddLostableGraphicResource(shader);
    return shader;
}

bool Package::IsPixelShaderSupported(int major, int minor)
{
    D3DCAPS9 caps;
    GetDirect3DDevice()->GetDeviceCaps(&caps);
    return caps.PixelShaderVersion >= D3DPS_VERSION(major, minor);
}

std::shared_ptr<Mesh> Package::LoadMesh(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return meshCache->Load(path, srcPos);
}

void Package::ReleaseUnusedMesh()
{
    meshCache->ReleaseUnusedMesh();
}

std::shared_ptr<SoundBuffer> Package::LoadSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return soundDevice->LoadSound(path, false, srcPos);
}

void Package::LoadOrphanSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    orphanSounds[GetCanonicalPath(path)] = LoadSound(path, srcPos);
}

void Package::RemoveOrphanSound(const std::wstring & path)
{
    orphanSounds.erase(GetCanonicalPath(path));
}

void Package::PlayBGM(const std::wstring & path, double loopStartSec, double loopEndSec)
{
    auto it = orphanSounds.find(GetCanonicalPath(path));
    if (it != orphanSounds.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->SetLoopEnable(true);
        sound->SetLoopTime(loopStartSec, loopEndSec);
        sound->Play();
    }
}

void Package::PlaySE(const std::wstring & path)
{
    auto it = orphanSounds.find(GetCanonicalPath(path));
    if (it != orphanSounds.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->Play();
    }
}

void Package::StopOrphanSound(const std::wstring & path)
{
    auto it = orphanSounds.find(GetCanonicalPath(path));
    if (it != orphanSounds.end())
    {
        it->second->Stop();
    }
}

void Package::CacheSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    soundDevice->LoadSound(path, true, srcPos);
}

void Package::RemoveSoundCache(const std::wstring & path)
{
    soundDevice->RemoveSoundCache(path);
}

void Package::ClearSoundCache()
{
    soundDevice->ClearSoundCache();
}

void Package::SetObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority)
{
    objLayerList->SetRenderPriority(obj, priority);
}

void Package::SetShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader)
{
    objLayerList->SetLayerShader(beginPriority, endPriority, shader);
}

void Package::ResetShader(int beginPriority, int endPriority)
{
    objLayerList->ResetLayerShader(beginPriority, endPriority);
}

int Package::GetStgFrameRenderPriorityMin() const
{
    return objLayerList->GetStgFrameRenderPriorityMin();
}

void Package::SetStgFrameRenderPriorityMin(int p)
{
    objLayerList->SetStgFrameRenderPriorityMin(p);
}

int Package::GetStgFrameRenderPriorityMax() const
{
    return objLayerList->GetStgFrameRenderPriorityMax();
}

void Package::SetStgFrameRenderPriorityMax(int p)
{
    objLayerList->SetStgFrameRenderPriorityMax(p);
}

int Package::GetShotRenderPriority() const
{
    return objLayerList->GetShotRenderPriority();
}

void Package::SetShotRenderPriority(int p)
{
    return objLayerList->SetShotRenderPriority(p);
}

int Package::GetItemRenderPriority() const
{
    return objLayerList->GetItemRenderPriority();
}

void Package::SetItemRenderPriority(int p)
{
    return objLayerList->SetItemRenderPriority(p);
}

int Package::GetPlayerRenderPriority() const
{
    if (auto player = GetPlayerObject())
    {
        return player->getRenderPriority();
    }
    return DEFAULT_PLAYER_RENDER_PRIORITY;
}

int Package::GetCameraFocusPermitRenderPriority() const
{
    return objLayerList->GetCameraFocusPermitRenderPriority();
}

void Package::SetInvalidRenderPriority(int min, int max)
{
    objLayerList->SetInvalidRenderPriority(min, max);
}

void Package::ClearInvalidRenderPriority()
{
    objLayerList->ClearInvalidRenderPriority();
}

NullableSharedPtr<Shader> Package::GetShader(int p) const
{
    return objLayerList->GetLayerShader(p);
}

void Package::SetFogEnable(bool enable)
{
    renderer->SetFogEnable(enable);
}

void Package::SetFogParam(float fogStart, float fogEnd, int r, int g, int b)
{
    renderer->SetFogParam(fogStart, fogEnd, r, g, b);
}

void Package::SetCameraFocusX(float x)
{
    camera3D->SetFocusX(x);
}

void Package::SetCameraFocusY(float y)
{
    camera3D->SetFocusY(y);
}

void Package::SetCameraFocusZ(float z)
{
    camera3D->SetFocusZ(z);
}

void Package::SetCameraFocusXYZ(float x, float y, float z)
{
    camera3D->SetFocusXYZ(x, y, z);
}

void Package::SetCameraRadius(float r)
{
    camera3D->SetRadius(r);
}

void Package::SetCameraAzimuthAngle(float angle)
{
    camera3D->SetAzimuthAngle(angle);
}

void Package::SetCameraElevationAngle(float angle)
{
    camera3D->SetElevationAngle(angle);
}

void Package::SetCameraYaw(float yaw)
{
    camera3D->SetYaw(yaw);
}

void Package::SetCameraPitch(float pitch)
{
    camera3D->SetPitch(pitch);
}

void Package::SetCameraRoll(float roll)
{
    camera3D->SetRoll(roll);
}

void Package::ResetCamera()
{
    SetCameraFocusXYZ(0.0f, 0.0f, 0.0f);
    SetCameraRadius(500.0f);
    SetCameraAzimuthAngle(15.0f);
    SetCameraElevationAngle(45.0f);
    SetCameraYaw(0.0f);
    SetCameraPitch(0.0f);
    SetCameraRoll(0.0f);
    SetCameraPerspectiveClip(10.0f, 2000.0f);
}

float Package::GetCameraX() const
{
    return camera3D->GetX();
}

float Package::GetCameraY() const
{
    return camera3D->GetY();
}

float Package::GetCameraZ() const
{
    return camera3D->GetZ();
}

float Package::GetCameraFocusX() const
{
    return camera3D->GetFocusX();
}

float Package::GetCameraFocusY() const
{
    return camera3D->GetFocusY();
}

float Package::GetCameraFocusZ() const
{
    return camera3D->GetFocusZ();
}

float Package::GetCameraRadius() const
{
    return camera3D->GetRadius();
}

float Package::GetCameraAzimuthAngle() const
{
    return camera3D->GetAzimuthAngle();
}

float Package::GetCameraElevationAngle() const
{
    return camera3D->GetElevationAngle();
}

float Package::GetCameraYaw() const
{
    return camera3D->GetYaw();
}

float Package::GetCameraPitch() const
{
    return camera3D->GetPitch();
}

float Package::GetCameraRoll() const
{
    return camera3D->GetRoll();
}

void Package::SetCameraPerspectiveClip(float nearClip, float farClip)
{
    return camera3D->SetPerspectiveClip(nearClip, farClip);
}

void Package::Set2DCameraFocusX(float x)
{
    camera2D->SetFocusX(x);
}

void Package::Set2DCameraFocusY(float y)
{
    camera2D->SetFocusY(y);
}

void Package::Set2DCameraAngleZ(float z)
{
    camera2D->SetAngleZ(z);
}

void Package::Set2DCameraRatio(float r)
{
    camera2D->SetRatio(r);
}

void Package::Set2DCameraRatioX(float x)
{
    camera2D->SetRatioX(x);
}

void Package::Set2DCameraRatioY(float y)
{
    camera2D->SetRatioY(y);
}

void Package::Reset2DCamera()
{
    camera2D->Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
}

float Package::Get2DCameraX() const
{
    return camera2D->GetX();
}

float Package::Get2DCameraY() const
{
    return camera2D->GetY();
}

float Package::Get2DCameraAngleZ() const
{
    return camera2D->GetAngleZ();
}

float Package::Get2DCameraRatio() const
{
    return camera2D->GetRatio();
}

float Package::Get2DCameraRatioX() const
{
    return camera2D->GetRatioX();
}

float Package::Get2DCameraRatioY() const
{
    return camera2D->GetRatioY();
}

void Package::SetCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    commonDataDB->SetCommonData(key, std::move(value));
}

const std::unique_ptr<DnhValue>& Package::GetCommonData(const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return commonDataDB->GetCommonData(key, defaultValue);
}

void Package::ClearCommonData()
{
    commonDataDB->ClearCommonData();
}

void Package::DeleteCommonData(const std::wstring & key)
{
    commonDataDB->DeleteCommonData(key);
}

void Package::SetAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    commonDataDB->SetAreaCommonData(areaName, key, std::move(value));
}

const std::unique_ptr<DnhValue>& Package::GetAreaCommonData(const std::wstring & areaName, const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return commonDataDB->GetAreaCommonData(areaName, key, defaultValue);
}

void Package::ClearAreaCommonData(const std::wstring & areaName)
{
    commonDataDB->ClearAreaCommonData(areaName);
}

void Package::DeleteAreaCommonData(const std::wstring & areaName, const std::wstring & key)
{
    commonDataDB->DeleteAreaCommonData(areaName, key);
}

void Package::CreateCommonDataArea(const std::wstring & areaName)
{
    commonDataDB->CreateCommonDataArea(areaName);
}

bool Package::IsCommonDataAreaExists(const std::wstring & areaName) const
{
    return commonDataDB->IsCommonDataAreaExists(areaName);
}

void Package::CopyCommonDataArea(const std::wstring & dest, const std::wstring & src)
{
    commonDataDB->CopyCommonDataArea(dest, src);
}

std::vector<std::wstring> Package::GetCommonDataAreaKeyList() const
{
    return commonDataDB->GetCommonDataAreaKeyList();
}

std::vector<std::wstring> Package::GetCommonDataValueKeyList(const std::wstring & areaName) const
{
    return commonDataDB->GetCommonDataValueKeyList(areaName);
}

bool Package::SaveCommonDataAreaA1(const std::wstring & areaName) const
{
    try
    {
        commonDataDB->SaveCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Package::LoadCommonDataAreaA1(const std::wstring & areaName)
{
    try
    {
        commonDataDB->LoadCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Package::SaveCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path) const
{
    try
    {
        commonDataDB->SaveCommonDataArea(areaName, path);
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Package::LoadCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path)
{
    try
    {
        commonDataDB->LoadCommonDataArea(areaName, path);
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

std::wstring Package::GetDefaultCommonDataSavePath(const std::wstring & areaName) const
{
    std::wstring basePath = GetMainPackageScriptPath() == DEFAULT_PACKAGE_PATH ? GetMainStgScriptPath() : GetMainPackageScriptPath();
    if (basePath.empty())
    {
        basePath = GetMainPackageScriptPath();
    }
    if (basePath.empty()) return false;
    return GetParentPath(basePath) + L"/data/" + GetStem(basePath) + L"_common_" + areaName + L".dat";
}

void Package::LoadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    playerShotDataTable->Load(path, srcPos);
}

void Package::ReloadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    playerShotDataTable->Reload(path, srcPos);
}

void Package::LoadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    enemyShotDataTable->Load(path, srcPos);
}

void Package::ReloadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    enemyShotDataTable->Reload(path, srcPos);
}

void Package::LoadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    itemDataTable->Load(path, fileLoader, srcPos);
}

void Package::ReloadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    itemDataTable->Reload(path, fileLoader, srcPos);
}

NullableSharedPtr<ShotData> Package::GetPlayerShotData(int id) const
{
    return playerShotDataTable->Get(id);
}

NullableSharedPtr<ShotData> Package::GetEnemyShotData(int id) const
{
    return enemyShotDataTable->Get(id);
}

NullableSharedPtr<ItemData> Package::GetItemData(int id) const
{
    return itemDataTable->Get(id);
}

std::shared_ptr<ObjText> Package::CreateObjText()
{
    auto obj = objTable->Create<ObjText>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSound> Package::CreateObjSound()
{
    return objTable->Create<ObjSound>(shared_from_this());
}

std::shared_ptr<ObjFileT> Package::CreateObjFileT()
{
    return objTable->Create<ObjFileT>(shared_from_this());
}

std::shared_ptr<ObjFileB> Package::CreateObjFileB()
{
    return objTable->Create<ObjFileB>(shared_from_this());
}

std::shared_ptr<ObjShader> Package::CreateObjShader()
{
    auto obj = objTable->Create<ObjShader>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjShot> Package::CreateObjShot(bool isPlayerShot)
{
    auto shot = objTable->Create<ObjShot>(isPlayerShot, shared_from_this());
    objLayerList->SetRenderPriority(shot, objLayerList->GetShotRenderPriority());
    return shot;
}

std::shared_ptr<ObjShot> Package::CreateShotA1(float x, float y, float speed, float angle, int shotDataId, int delay, bool isPlayerShot)
{
    auto shot = CreateObjShot(isPlayerShot);
    shot->SetMovePosition(x, y);
    shot->SetSpeed(speed);
    shot->SetAngle(angle);
    shot->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    shot->SetDelay(delay);
    shot->Regist();
    return shot;
}

std::shared_ptr<ObjShot> Package::CreateShotA2(float x, float y, float speed, float angle, float accel, float maxSpeed, int shotDataId, int delay, bool isPlayerShot)
{
    auto shot = CreateShotA1(x, y, speed, angle, shotDataId, delay, isPlayerShot);
    shot->SetAcceleration(accel);
    shot->SetMaxSpeed(maxSpeed);
    return shot;
}

NullableSharedPtr<ObjShot> Package::CreateShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot)
{
    if (auto obj = GetObject<ObjRender>(objId))
    {
        return CreateShotA1(obj->GetX(), obj->GetY(), speed, angle, shotDataId, delay, isPlayerShot);
    } else
    {
        return nullptr;
    }
}

std::shared_ptr<ObjShot> Package::CreateShotB1(float x, float y, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot)
{
    return CreateShotB2(x, y, speedX, speedY, 0, 0, 0, 0, shotDataId, delay, isPlayerShot);
}

std::shared_ptr<ObjShot> Package::CreateShotB2(float x, float y, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, int shotDataId, int delay, bool isPlayerShot)
{
    auto shot = CreateObjShot(isPlayerShot);
    shot->SetMovePosition(x, y);
    shot->SetMoveMode(std::make_shared<MoveModeB>(speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY));
    shot->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    shot->SetDelay(delay);
    shot->Regist();
    return shot;
}

NullableSharedPtr<ObjShot> Package::CreateShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot)
{
    if (auto obj = GetObject<ObjRender>(objId))
    {
        return CreateShotB1(obj->GetX(), obj->GetY(), speedX, speedY, shotDataId, delay, isPlayerShot);
    } else
    {
        return nullptr;
    }
}

NullableSharedPtr<ObjShot> Package::CreatePlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId)
{
    if (auto player = GetPlayerObject())
    {
        if (!player->IsPermitPlayerShot())
        {
            return nullptr;
        }
    }
    auto shot = CreateObjShot(true);
    shot->SetMovePosition(x, y);
    shot->SetSpeed(speed);
    shot->SetAngle(angle);
    shot->SetDamage(damage);
    shot->SetPenetration(penetration);
    shot->SetShotData(GetPlayerShotData(shotDataId));
    shot->Regist();
    return shot;
}

std::shared_ptr<ObjLooseLaser> Package::CreateObjLooseLaser(bool isPlayerShot)
{
    auto laser = objTable->Create<ObjLooseLaser>(isPlayerShot, shared_from_this());
    objLayerList->SetRenderPriority(laser, objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjLooseLaser> Package::CreateLooseLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot)
{
    auto laser = CreateObjLooseLaser(isPlayerShot);
    laser->SetMovePosition(x, y);
    laser->SetSpeed(speed);
    laser->SetAngle(angle);
    laser->SetLength(length);
    laser->SetRenderWidth(width);
    laser->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    laser->SetDelay(delay);
    laser->Regist();
    return laser;
}

std::shared_ptr<ObjStLaser> Package::CreateObjStLaser(bool isPlayerShot)
{
    auto laser = objTable->Create<ObjStLaser>(isPlayerShot, shared_from_this());
    objLayerList->SetRenderPriority(laser, objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjStLaser> Package::CreateStraightLaserA1(float x, float y, float angle, float length, float width, int deleteFrame, int shotDataId, int delay, bool isPlayerShot)
{
    auto laser = CreateObjStLaser(isPlayerShot);
    laser->SetMovePosition(x, y);
    laser->SetLaserAngle(angle);
    laser->SetLength(length);
    laser->SetRenderWidth(width);
    laser->SetDeleteFrame(deleteFrame);
    laser->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    laser->SetDelay(delay);
    laser->Regist();
    return laser;
}

std::shared_ptr<ObjCrLaser> Package::CreateObjCrLaser(bool isPlayerShot)
{
    auto laser = objTable->Create<ObjCrLaser>(isPlayerShot, shared_from_this());
    objLayerList->SetRenderPriority(laser, objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjCrLaser> Package::CreateCurveLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot)
{
    auto laser = CreateObjCrLaser(isPlayerShot);
    laser->SetMovePosition(x, y);
    laser->SetSpeed(speed);
    laser->SetAngle(angle);
    laser->SetLength(length);
    laser->SetRenderWidth(width);
    laser->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    laser->SetDelay(delay);
    laser->Regist();
    return laser;
}

std::shared_ptr<ObjItem> Package::CreateItemA1(int itemType, float x, float y, PlayerScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Package::CreateItemA2(int itemType, float x, float y, float destX, float destY, PlayerScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Package::CreateItemU1(int itemDataId, float x, float y, PlayerScore score)
{
    auto item = CreateObjItem(ITEM_USER);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    item->SetItemData(GetItemData(itemDataId));
    return item;
}

std::shared_ptr<ObjItem> Package::CreateItemU2(int itemDataId, float x, float y, float destX, float destY, PlayerScore score)
{
    auto item = CreateObjItem(ITEM_USER);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->SetScore(score);
    item->SetItemData(GetItemData(itemDataId));
    return item;
}

std::shared_ptr<ObjEnemy> Package::CreateObjEnemy()
{
    auto enemy = objTable->Create<ObjEnemy>(false, shared_from_this());
    objLayerList->SetRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
}

std::shared_ptr<ObjEnemyBossScene> Package::CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto bossScene = enemyBossSceneObj.lock())
    {
        if (!bossScene->IsDead())
        {
            Logger::WriteLog(std::move(
                Log(Log::Level::LV_WARN)
                .SetMessage("boss scene object already exists.")
                .AddSourcePos(srcPos)));
            return bossScene;
        }
    }
    auto bossScene = objTable->Create<ObjEnemyBossScene>(shared_from_this());
    enemyBossSceneObj = bossScene;
    return bossScene;
}

std::shared_ptr<ObjSpell> Package::CreateObjSpell()
{
    auto spell = objTable->Create<ObjSpell>(shared_from_this());
    objLayerList->SetRenderPriority(spell, 50);
    return spell;
}

std::shared_ptr<ObjItem> Package::CreateObjItem(int itemType)
{
    auto obj = objTable->Create<ObjItem>(itemType, shared_from_this());
    objLayerList->SetRenderPriority(obj, objLayerList->GetItemRenderPriority());
    obj->SetIntersection();
    return obj;
}

std::shared_ptr<Script> Package::GetScript(int scriptId) const
{
    return scriptManager->Get(scriptId);
}

std::shared_ptr<Script> Package::LoadScript(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = scriptManager->Compile(path, type, version, srcPos);
    script->Load();
    return script;
}

std::shared_ptr<Script> Package::LoadScriptInThread(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = scriptManager->CompileInThread(path, type, version, srcPos);
    return script;
}

void Package::CloseStgScene()
{
    if (auto stageMain = stageMainScript.lock())
    {
        stageMain->Close();
    }
}

void Package::NotifyEventAll(int eventType)
{
    scriptManager->NotifyEventAll(eventType);
}

void Package::NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args)
{
    scriptManager->NotifyEventAll(eventType, args);
}

std::wstring Package::GetPlayerID() const
{
    return stagePlayerScriptInfo.id;
}

std::wstring Package::GetPlayerReplayName() const
{
    return stagePlayerScriptInfo.replayName;
}

NullableSharedPtr<ObjPlayer> Package::GetPlayerObject() const
{
    auto player = playerObj.lock();
    if (player && !player->IsDead()) return player;
    return nullptr;
}

NullableSharedPtr<ObjEnemy> Package::GetEnemyBossObject() const
{
    if (auto bossScene = GetEnemyBossSceneObject())
    {
        if (auto boss = bossScene->GetEnemyBossObject())
        {
            if (!boss->IsDead()) return boss;
        }
    }
    return nullptr;
}

NullableSharedPtr<ObjEnemyBossScene> Package::GetEnemyBossSceneObject() const
{
    auto bossScene = enemyBossSceneObj.lock();
    if (bossScene && !bossScene->IsDead()) return bossScene;
    return nullptr;
}

NullableSharedPtr<ObjSpellManage> Package::GetSpellManageObject() const
{
    auto spellManage = spellManageObj.lock();
    if (spellManage && !spellManage->IsDead()) return spellManage;
    return nullptr;
}

void Package::DeleteObject(int id)
{
    objTable->Delete(id);
}

bool Package::IsObjectDeleted(int id) const
{
    return objTable->IsDeleted(id);
}

std::shared_ptr<ObjPrim2D> Package::CreateObjPrim2D()
{
    auto obj = objTable->Create<ObjPrim2D>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite2D> Package::CreateObjSprite2D()
{
    auto obj = objTable->Create<ObjSprite2D>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSpriteList2D> Package::CreateObjSpriteList2D()
{
    auto obj = objTable->Create<ObjSpriteList2D>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjPrim3D> Package::CreateObjPrim3D()
{
    auto obj = objTable->Create<ObjPrim3D>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite3D> Package::CreateObjSprite3D()
{
    auto obj = objTable->Create<ObjSprite3D>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjMesh> Package::CreateObjMesh()
{
    auto obj = objTable->Create<ObjMesh>(shared_from_this());
    objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

NullableSharedPtr<Script> Package::GetPlayerScript() const
{
    return stagePlayerScript.lock();
}

const std::unique_ptr<DnhValue>& Package::GetScriptResult(int scriptId) const
{
    return scriptManager->GetScriptResult(scriptId);
}

void Package::SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value)
{
    scriptManager->SetScriptResult(scriptId, std::move(value));
}

std::vector<ScriptInfo> Package::GetScriptList(const std::wstring & dirPath, int scriptType, bool doRecursive)
{
    std::vector<std::wstring> pathList;
    GetFilePaths(dirPath, pathList, ignoreScriptExts, doRecursive);
    std::vector<ScriptInfo> infos;
    infos.reserve(pathList.size());
    const std::wstring scriptTypeName = GetScriptTypeNameFromConst(scriptType);
    for (const auto& path : pathList)
    {
        try
        {
            auto info = ScanDnhScriptInfo(path, fileLoader);
            if (scriptType == TYPE_SCRIPT_ALL || scriptTypeName == info.type)
            {
                infos.push_back(info);
            }
        } catch (const Log& log)
        {
        }
    }
    return infos;
}

void Package::GetLoadFreePlayerScriptList()
{
    freePlayerScriptInfoList = GetScriptList(FREE_PLAYER_DIR, TYPE_SCRIPT_PLAYER, true);
}

int Package::GetFreePlayerScriptCount() const
{
    return freePlayerScriptInfoList.size();
}

ScriptInfo Package::GetFreePlayerScriptInfo(int idx) const
{
    return freePlayerScriptInfoList.at(idx);
}

ScriptInfo Package::GetScriptInfo(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        return ScanDnhScriptInfo(path, fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("failed to load script info.")
            .SetParam(Log::Param(Log::Param::Tag::SCRIPT, path))
            .AddSourcePos(srcPos)));
        return ScriptInfo();
    }
}

PlayerLife Package::GetPlayerLife() const
{
    return stageCommonPlayerParams_.life;
}
PlayerSpell Package::GetPlayerSpell() const
{
    return stageCommonPlayerParams_.spell;
}
PlayerPower Package::GetPlayerPower() const
{
    return stageCommonPlayerParams_.power;
}
PlayerScore Package::GetPlayerScore() const
{
    return stageCommonPlayerParams_.score;
}
PlayerGraze Package::GetPlayerGraze() const
{
    return stageCommonPlayerParams_.graze;
}
PlayerPoint Package::GetPlayerPoint() const
{
    return stageCommonPlayerParams_.point;
}
void Package::SetPlayerLife(PlayerLife life)
{
    stageCommonPlayerParams_.life = life;
}
void Package::SetPlayerSpell(PlayerSpell spell)
{
    stageCommonPlayerParams_.spell = spell;
}
void Package::SetPlayerPower(PlayerPower power)
{
    stageCommonPlayerParams_.power = power;
}
void Package::SetPlayerScore(PlayerScore score)
{
    stageCommonPlayerParams_.score = score;
}
void Package::SetPlayerGraze(PlayerGraze graze)
{
    stageCommonPlayerParams_.graze = graze;
}
void Package::SetPlayerPoint(PlayerPoint point)
{
    stageCommonPlayerParams_.point = point;
}

void Package::SetStgFrame(float left, float top, float right, float bottom)
{
    stgFrame_.left = left;
    stgFrame_.top = top;
    stgFrame_.right = right;
    stgFrame_.bottom = bottom;
}

float Package::GetStgFrameLeft() const
{
    return stgFrame_.left;
}

float Package::GetStgFrameTop() const
{
    return stgFrame_.top;
}

float Package::GetStgFrameRight() const
{
    return stgFrame_.right;
}

float Package::GetStgFrameBottom() const
{
    return stgFrame_.bottom;
}

float Package::GetStgFrameWidth() const
{
    return stgFrame_.right - stgFrame_.left;
}

float Package::GetStgFrameHeight() const
{
    return stgFrame_.bottom - stgFrame_.top;
}

float Package::GetStgFrameCenterWorldX() const
{
    return (stgFrame_.right - stgFrame_.left) / 2.0f;
}

float Package::GetStgFrameCenterWorldY() const
{
    return (stgFrame_.bottom - stgFrame_.top) / 2.0f;
}

float Package::GetStgFrameCenterScreenX() const
{
    return (stgFrame_.right + stgFrame_.left) / 2.0f;
}

float Package::GetStgFrameCenterScreenY() const
{
    return (stgFrame_.bottom + stgFrame_.top) / 2.0f;
}
int Package::GetAllShotCount() const
{
    return shotCounter->playerShotCount + shotCounter->enemyShotCount;
}

int Package::GetEnemyShotCount() const
{
    return shotCounter->enemyShotCount;
}

int Package::GetPlayerShotCount() const
{
    return shotCounter->playerShotCount;
}

void Package::SetShotAutoDeleteClip(float left, float top, float right, float bottom)
{
    shotAutoDeleteClip_ = Rect<float>(left, top, right, bottom);
}

bool Package::IsOutOfShotAutoDeleteClip(float x, float y) const
{
    return x <= -shotAutoDeleteClip_.left ||
        y <= -shotAutoDeleteClip_.top ||
        x >= shotAutoDeleteClip_.right + GetStgFrameWidth() ||
        y >= shotAutoDeleteClip_.bottom + GetStgFrameHeight();
}

void Package::StartShotScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (IsStageFinished())
    {
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("shot script is only available in stage.")
            .SetParam(Log::Param(Log::Param::Tag::SCRIPT, GetCanonicalPath(path)))
            .AddSourcePos(srcPos)));
        return;
    }
    auto shotScript = LoadScript(path, SCRIPT_TYPE_SHOT_CUSTOM, stageMainScriptInfo.version, srcPos);
    shotScript = shotScript;
    shotScript->Start();
    shotScript->RunInitialize();
}

void Package::SetDeleteShotImmediateEventOnShotScriptEnable(bool enable)
{
    deleteShotImmediateEventOnShotScriptEnable = enable;
}

void Package::SetDeleteShotFadeEventOnShotScriptEnable(bool enable)
{
    deleteShotFadeEventOnShotScriptEnable = enable;
}

void Package::SetDeleteShotToItemEventOnShotScriptEnable(bool enable)
{
    deleteShotToItemEventOnShotScriptEnable = enable;
}

void Package::DeleteShotAll(int target, int behavior) const
{
    auto shots = GetObjectAll<ObjShot>();
    if (target == TYPE_SHOT)
    {
        // 自機弾とスペル耐性のある弾は除外
        shots.erase(std::remove_if(shots.begin(), shots.end(), [](std::shared_ptr<ObjShot>& shot)->bool { return shot->IsPlayerShot() || shot->IsSpellResistEnabled(); }), shots.end());
    } else
    {
        // 自機弾は除外
        shots.erase(std::remove_if(shots.begin(), shots.end(), [](std::shared_ptr<ObjShot>& shot)->bool { return shot->IsPlayerShot(); }), shots.end());
    }

    // delete
    for (auto& shot : shots)
    {
        if (behavior == TYPE_IMMEDIATE)
        {
            shot->DeleteImmediate();
        } else if (behavior == TYPE_FADE)
        {
            shot->FadeDelete();
        } else if (behavior == TYPE_ITEM)
        {
            shot->ToItem();
        }
    }
}

void Package::DeleteShotInCircle(int target, int behavior, float x, float y, float r) const
{
    auto isects = colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), COL_GRP_ENEMY_SHOT);
    for (auto& isect : isects)
    {
        if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect))
        {
            if (auto shot = shotIsect->GetShot().lock())
            {
                // スペル耐性弾は無視
                if (target == TYPE_SHOT && shot->IsSpellResistEnabled()) continue;

                if (behavior == TYPE_IMMEDIATE)
                {
                    shot->DeleteImmediate();
                } else if (behavior == TYPE_FADE)
                {
                    shot->FadeDelete();
                } else if (behavior == TYPE_ITEM)
                {
                    shot->ToItem();
                }
            }
        }
    }
}

std::vector<std::shared_ptr<ObjShot>> Package::GetShotInCircle(float x, float y, float r, int target) const
{
    // 同じショットに紐付いている判定が複数あるので、まずIDだけを集める
    std::unordered_set<int> shotIds;

    auto isects = colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), -1);
    for (auto& isect : isects)
    {
        if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect))
        {
            if (target == TARGET_ENEMY && shotIsect->IsPlayerShot()) continue;
            if (target == TARGET_PLAYER && !shotIsect->IsPlayerShot()) continue;
            if (auto shot = shotIsect->GetShot().lock())
            {
                if (!shot->IsDead())
                {
                    shotIds.insert(shot->GetID());
                }
            }
        }
    }

    std::vector<std::shared_ptr<ObjShot>> shots;
    shots.reserve(shotIds.size());
    for (auto id : shotIds)
    {
        shots.push_back(GetObject<ObjShot>(id));
    }
    return shots;
}

void Package::SetShotIntersectoinCicle(float x, float y, float r)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x, y, r);
    colDetector->Add(isect);
    tempEnemyShotIsects.push_back(isect);
}

void Package::SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x1, y1, x2, y2, width);
    colDetector->Add(isect);
    tempEnemyShotIsects.push_back(isect);
}

void Package::CollectAllItems()
{
    autoItemCollectionManager->CollectAllItems();
}

void Package::CollectItemsByType(int itemType)
{
    autoItemCollectionManager->CollectItemsByType(itemType);
}

void Package::CollectItemsInCircle(float x, float y, float r)
{
    autoItemCollectionManager->CollectItemsInCircle(x, y, r);
}

void Package::CancelCollectItems()
{
    autoItemCollectionManager->CancelCollectItems();
}

void Package::SetDefaultBonusItemEnable(bool enable)
{
    defaultBonusItemEnable = enable;
}

void Package::StartItemScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (IsStageFinished())
    {
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_WARN)
            .SetMessage("item script is only available in stage.")
            .SetParam(Log::Param(Log::Param::Tag::SCRIPT, GetCanonicalPath(path)))
            .AddSourcePos(srcPos)));
        return;
    }
    auto itemScript = LoadScript(path, SCRIPT_TYPE_ITEM_CUSTOM, stageMainScriptInfo.version, srcPos);
    itemScript = itemScript;
    itemScript->Start();
    itemScript->RunInitialize();
}

bool Package::IsPackageFinished() const
{
    if (packageMainScript.lock())
    {
        return false;
    }
    return true;
}

void Package::ClosePackage()
{
    if (auto packageMain = packageMainScript.lock())
    {
        packageMain->Close();
    }
}

void Package::InitializeStageScene()
{
    objTable->DeleteStgSceneObject();
    scriptManager->CloseStgSceneScript();
    SetStgFrame(32.0f, 16.0f, 416.0f, 464.0f);
    SetShotAutoDeleteClip(64.0f, 64.0f, 64.0f, 64.0f);
    Reset2DCamera();
    SetDefaultBonusItemEnable(true);
    stageSceneResult = 0;
    stageMainScript.reset();
    stagePlayerScript.reset();

    SetPlayerLife(2);
    SetPlayerSpell(3);
    SetPlayerPower(1);
    SetPlayerScore(0);
    SetPlayerGraze(0);
    SetPlayerPoint(0);

    SetStgFrameRenderPriorityMin(DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN);
    SetStgFrameRenderPriorityMax(DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX);
    SetShotRenderPriority(DEFAULT_SHOT_RENDER_PRIORITY);
    SetItemRenderPriority(DEFAULT_ITEM_RENDER_PRIORITY);
    stagePaused = true;
    stageForceTerminated = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
}

void Package::FinalizeStageScene()
{
    // FUTURE : impl
}

void Package::StartPackage()
{
    if (!IsPackageFinished())
    {
        Logger::WriteLog(Log::Level::LV_WARN, "package already started.");
        return;
    }
    packageStartTime = std::make_shared<TimePoint>();
    Logger::WriteLog(Log::Level::LV_INFO, "start package.");
    auto script = scriptManager->Compile(packageMainScriptInfo.path, SCRIPT_TYPE_PACKAGE, packageMainScriptInfo.version, nullptr);
    packageMainScript = script;
    script->Start();
    script->RunInitialize();
}

void Package::SetPauseScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"PauseScript", std::make_unique<DnhArray>(path));
}

void Package::SetEndSceneScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"EndSceneScript", std::make_unique<DnhArray>(path));
}

void Package::SetReplaySaveSceneScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"ReplaySaveSceneScript", std::make_unique<DnhArray>(path));
}

Point2D Package::Get2DPosition(float x, float y, float z, bool isStgScene)
{
    D3DXMATRIX view, proj, viewport, billboard;
    camera3D->GenerateViewMatrix(&view, &billboard);
    if (isStgScene)
    {
        camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
    } else
    {
        camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
    }
    {
        D3DXMatrixScaling(&viewport, GetScreenWidth() / 2.0f, -GetScreenHeight() / 2.0f, 1.0f);
        viewport._41 = GetScreenWidth() / 2.0f;
        viewport._42 = GetScreenHeight() / 2.0f;
    }
    D3DXVECTOR3 pos = D3DXVECTOR3(x, y, z);
    D3DXVec3TransformCoord(&pos, &pos, &(view * proj * viewport));
    if (isStgScene)
    {
        camera2D->GenerateViewMatrix(&view);
        camera2D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        D3DXMATRIX viewProjViewport = view * proj * viewport;
        D3DXMatrixInverse(&viewProjViewport, NULL, &viewProjViewport);
        D3DXVec3TransformCoord(&pos, &pos, &viewProjViewport);
    }
    return Point2D(pos.x, pos.y);
}

void Package::SetStageIndex(uint16_t idx)
{
    stageIdx = idx;
}

void Package::SetStageMainScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        stageMainScriptInfo = ScanDnhScriptInfo(path, fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        stageMainScriptInfo = ScriptInfo();
        stageMainScriptInfo.path = path;
    }
}

void Package::SetStagePlayerScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        stagePlayerScriptInfo = ScanDnhScriptInfo(path, fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        stageMainScriptInfo = ScriptInfo();
        stageMainScriptInfo.path = path;
    }
}

void Package::SetStageMainScript(const ScriptInfo & script)
{
    stageMainScriptInfo = script;
}

void Package::SetStagePlayerScript(const ScriptInfo & script)
{
    stagePlayerScriptInfo = script;
}

void Package::SetStageReplayFile(const std::wstring & path)
{
    stageReplayFilePath = path;
}

bool Package::IsStageFinished() const
{
    if (stageMainScript.lock())
    {
        return false;
    }
    return true;
}

int Package::GetStageSceneResult() const
{
    return stageSceneResult;
}

bool Package::IsStagePaused() const
{
    return stagePaused;
}

void Package::PauseStageScene(bool doPause)
{
    if (doPause && !stagePaused)
    {
        NotifyEventAll(EV_PAUSE_ENTER);
    } else if (!doPause && stagePaused)
    {
        NotifyEventAll(EV_PAUSE_LEAVE);
    }
    stagePaused = doPause;
}

void Package::TerminateStageScene()
{
    if (auto stageMain = stageMainScript.lock())
    {
        stageMain->Close();
        stageForceTerminated = true;
    }
}

void Package::SetPackageMainScript(const std::wstring & path)
{
    try
    {
        packageMainScriptInfo = ScanDnhScriptInfo(path, fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN);
        Logger::WriteLog(log);
        stageMainScriptInfo = ScriptInfo();
        stageMainScriptInfo.path = path;
    }
}

void Package::SetPackageMainScript(const ScriptInfo & script)
{
    packageMainScriptInfo = script;
}

void Package::StartStageScene(const std::shared_ptr<SourcePos>& srcPos)
{
    objTable->DeleteStgSceneObject();
    scriptManager->CloseStgSceneScript();
    inputDevice->ResetInputState();
    Reset2DCamera();
    ResetCamera();
    SetDefaultBonusItemEnable(true);
    playerShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureCache, fileLoader);
    enemyShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureCache, fileLoader);
    itemDataTable = std::make_shared<ItemDataTable>(textureCache, fileLoader);
    stageMainScript.reset();
    stagePlayerScript.reset();
    ReloadItemData(DEFAULT_ITEM_DATA_PATH, nullptr);
    stageSceneResult = 0;
    stageForceTerminated = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
    stagePaused = false;
    renderer->SetFogEnable(false);
    pseudoPlayerFps = pseudoEnemyFps = 60;

    if (stageMainScriptInfo.systemPath.empty() || stageMainScriptInfo.systemPath == L"DEFAULT")
    {
        stageMainScriptInfo.systemPath = DEFAULT_SYSTEM_PATH;
    }

    // #System
    auto systemScript = scriptManager->Compile(stageMainScriptInfo.systemPath, SCRIPT_TYPE_STAGE, stageMainScriptInfo.version, srcPos);
    systemScript->Start();
    systemScript->RunInitialize();

    // Create Player
    auto player = objTable->Create<ObjPlayer>(shared_from_this());
    objLayerList->SetRenderPriority(player, DEFAULT_PLAYER_RENDER_PRIORITY);
    player->AddIntersectionToItem();
    playerObj = player;
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create player object.")
        .AddSourcePos(srcPos)));

    auto playerScript = scriptManager->Compile(stagePlayerScriptInfo.path, SCRIPT_TYPE_PLAYER, stagePlayerScriptInfo.version, srcPos);
    stagePlayerScript = playerScript;
    playerScript->Start();
    playerScript->RunInitialize();

    // Main
    auto stageMainScriptPath = stageMainScriptInfo.path;
    if (stageMainScriptInfo.type == SCRIPT_TYPE_SINGLE)
    {
        stageMainScriptPath = SYSTEM_SINGLE_STAGE_PATH;
    } else if (stageMainScriptInfo.type == SCRIPT_TYPE_PLURAL)
    {
        stageMainScriptPath = SYSTEM_PLURAL_STAGE_PATH;
    }
    auto stageMainScript = scriptManager->Compile(stageMainScriptPath, SCRIPT_TYPE_STAGE, stageMainScriptInfo.version, srcPos);
    stageMainScript = stageMainScript;
    stageMainScript->Start();
    stageMainScript->RunInitialize();
    stageStartTime = std::make_shared<TimePoint>();

    // #Background
    if (!stageMainScriptInfo.backgroundPath.empty() && stageMainScriptInfo.backgroundPath != L"DEFAULT")
    {
        auto backgroundScript = scriptManager->Compile(stageMainScriptInfo.backgroundPath, SCRIPT_TYPE_STAGE, stageMainScriptInfo.version, srcPos);
        backgroundScript->Start();
        backgroundScript->RunInitialize();
    }

    // #BGM
    if (!stageMainScriptInfo.bgmPath.empty() && stageMainScriptInfo.bgmPath != L"DEFAULT")
    {
        try
        {
            LoadOrphanSound(stageMainScriptInfo.bgmPath, nullptr); // TODO: #BGMヘッダのSourcePosを与える
            auto& bgm = orphanSounds[GetCanonicalPath(packageMainScriptInfo.bgmPath)];
            bgm->SetLoopEnable(true);
            bgm->Play();
        } catch (Log& log)
        {
            log.SetLevel(Log::Level::LV_WARN);
            log.AddSourcePos(srcPos);
            Logger::WriteLog(log);
        }
    }
}

void Package::RenderToTexture(const std::wstring& name, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag)
{
    if (renderToBackBuffer)
    {
        graphicDevice->SetBackbufferRenderTarget();
    } else
    {
        auto renderTarget = GetRenderTarget(name);
        if (!renderTarget) return;
        renderTarget->SetRenderTarget();
    }

    D3DXMATRIX viewMatrix2D, projMatrix2D, viewMatrix3D, projMatrix3D, billboardMatrix;
    Camera2D outsideStgFrameCamera2D; outsideStgFrameCamera2D.Reset(0, 0);
    renderer->InitRenderState();

    if (doClear)
    {
        graphicDevice->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    }

    begin = std::max(begin, 0);
    end = std::min(end, MAX_RENDER_PRIORITY);

    auto obj = objTable->Get<ObjRender>(objId);
    if (obj && checkVisibleFlag && !obj->IsVisible()) return;

    camera3D->GenerateViewMatrix(&viewMatrix3D, &billboardMatrix);

    // [0, stgFrameMin]
    {
        renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = 0; p < GetStgFrameRenderPriorityMin(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList->IsInvalidRenderPriority(p)))
                {
                    objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer);
                }
            }
        }
    }
    // [stgFrameMin, stgFrameMax]
    {
        if (!IsStagePaused())
        {
            RECT scissorRect = { (LONG)stgFrame_.left, (LONG)stgFrame_.top, (LONG)stgFrame_.right, (LONG)stgFrame_.bottom };
            if (renderToBackBuffer)
            {
                scissorRect.left = stgFrame_.left * graphicDevice->GetBackBufferWidth() / screenWidth;
                scissorRect.top = stgFrame_.top * graphicDevice->GetBackBufferHeight() / screenHeight;
                scissorRect.right = stgFrame_.right * graphicDevice->GetBackBufferWidth() / screenWidth;
                scissorRect.bottom = stgFrame_.bottom * graphicDevice->GetBackBufferHeight() / screenHeight;
            }
            renderer->EnableScissorTest(scissorRect);
        }

        // set 2D matrix
        if (!IsStageFinished())
        {
            camera2D->GenerateViewMatrix(&viewMatrix2D);
            camera2D->GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
            renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
        }

        // set 3D matrix
        camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);

        for (int p = GetStgFrameRenderPriorityMin(); p <= GetStgFrameRenderPriorityMax(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList->IsInvalidRenderPriority(p)))
                {
                    objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer);
                }
            }
            if (p == objLayerList->GetCameraFocusPermitRenderPriority())
            {
                // cameraFocusPermitRenderPriorityより大きい優先度では別のビュー変換行列を使う
                if (!IsStageFinished())
                {
                    Camera2D focusForbidCamera;
                    focusForbidCamera.Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
                    focusForbidCamera.GenerateViewMatrix(&viewMatrix2D);
                    renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
                }
            }
        }
    }
    {
        // (stgFrameMax, MAX_RENDER_PRIORITY]
        renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = GetStgFrameRenderPriorityMax() + 1; p <= MAX_RENDER_PRIORITY; p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList->IsInvalidRenderPriority(p)))
                {
                    objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer);
                }
            }
        }
    }
    SetBackBufferRenderTarget();
}

NullableSharedPtr<Obj> Package::GetObj(int id) const
{
    return objTable->Get<Obj>(id);
}

const std::map<int, std::shared_ptr<Obj>>& Package::GetObjAll() const
{
    return objTable->GetAll();
}
}