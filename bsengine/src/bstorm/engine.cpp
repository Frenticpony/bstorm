#include <bstorm/engine.hpp>

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
#include <bstorm/intersection.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/auto_delete_clip.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/script.hpp>
#include <bstorm/config.hpp>
#include <bstorm/package.hpp>

#include <exception>
#include <ctime>

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
Engine::Engine(HWND hWnd, int screenWidth, int screenHeight, const std::shared_ptr<conf::KeyConfig>& defaultKeyConfig) :
    hWnd(hWnd),
    graphicDevice(std::make_unique<GraphicDevice>(hWnd)),
    lostableGraphicResourceManager(std::make_unique<LostableGraphicResourceManager>()),
    defaultKeyConfig(defaultKeyConfig)
{
    Logger::WriteLog(Log::Level::LV_INFO, "boot engine.");
    Reset(screenWidth, screenHeight);
}

Engine::~Engine()
{
    Logger::WriteLog(Log::Level::LV_INFO, "shutdown engine.");
}

HWND Engine::GetWindowHandle() const
{
    return hWnd;
}

void Engine::TickFrame()
{
    if (IsPackageFinished())
    {
        Logger::WriteLog(Log::Level::LV_WARN, "package is not setted, please select package.");
        return;
    }

    if (auto packageMain = package->packageMainScript.lock())
    {
        if (packageMain->IsClosed())
        {
            Logger::WriteLog(Log::Level::LV_INFO, "finish package.");
            package->packageMainScript.reset();
            Reset(GetScreenWidth(), GetScreenHeight());
            return;
        }
    }

    if (GetElapsedFrame() % (60 / std::min(package->pseudoEnemyFps, package->pseudoPlayerFps)) == 0)
    {
        if (auto stageMain = package->stageMainScript.lock())
        {
            if (stageMain->IsClosed())
            {
                if (package->stageForceTerminated)
                {
                    package->stageSceneResult = STAGE_RESULT_BREAK_OFF;
                    package->SetPlayerLife(0);
                    package->SetPlayerSpell(3);
                    package->SetPlayerPower(1);
                    package->SetPlayerScore(0);
                    package->SetPlayerGraze(0);
                    package->SetPlayerPoint(0);
                } else if (auto player = GetPlayerObject())
                {
                    package->stageSceneResult = player->GetState() != STATE_END ? STAGE_RESULT_CLEARED : STAGE_RESULT_PLAYER_DOWN;
                } else
                {
                    package->stageSceneResult = STAGE_RESULT_PLAYER_DOWN;
                }
                RenderToTextureA1(GetTransitionRenderTargetName(), 0, MAX_RENDER_PRIORITY, true);
                package->objTable->DeleteStgSceneObject();
                package->scriptManager->CloseStgSceneScript();
                package->stageMainScript.reset();
                package->stagePlayerScript.reset();
                package->stageForceTerminated = false;
                package->stagePaused = true;
                package->pseudoPlayerFps = package->pseudoEnemyFps = 60;
            }
        }
        package->scriptManager->CleanClosedScript();

        if (!IsStagePaused())
        {
            package->colDetector->TestAllCollision();
        }

        // SetShotIntersection{Circle, Line}で設定した判定削除
        package->tempEnemyShotIsects.clear();

        package->inputDevice->UpdateInputState();

        package->scriptManager->RunAll(IsStagePaused());

        package->objTable->UpdateAll(IsStagePaused());

        package->autoItemCollectionManager->Reset();
    }
    // 使われなくなったリソース開放
    switch (GetElapsedFrame() % 1920)
    {
        case 0:
            ReleaseUnusedLostableGraphicResource();
            break;
        case 480:
            ReleaseUnusedTextureCache();
            break;
        case 960:
            ReleaseUnusedFontCache();
            break;
        case 1440:
            ReleaseUnusedMeshCache();
            break;
    }
    package->elapsedFrame += 1;
}

void Engine::Render()
{
    RenderToTexture(L"", 0, MAX_RENDER_PRIORITY, ID_INVALID, true, true, true, true);
}

void Engine::Render(const std::wstring& renderTargetName)
{
    RenderToTexture(renderTargetName, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
}

void Engine::RenderToTextureA1(const std::wstring& name, int begin, int end, bool doClear)
{
    RenderToTexture(name, begin, end, ID_INVALID, doClear, false, false, false);
}

void Engine::RenderToTextureB1(const std::wstring& name, int objId, bool doClear)
{
    RenderToTexture(name, 0, 0, objId, doClear, false, false, false);
}

void Engine::Reset(int screenWidth, int screenHeight)
{
    Logger::SetEnable(false);
    package = std::make_shared<Package>(screenWidth, screenHeight, GetWindowHandle(), GetDirect3DDevice(), defaultKeyConfig, mousePosProvider, this);
    Reset2DCamera();
    ResetCamera();

    renderTargets.clear();
    CreateRenderTarget(GetTransitionRenderTargetName(), GetScreenWidth(), GetScreenHeight(), nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(0), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(1), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(2), 1024, 512, nullptr);
    package->renderer->SetForbidCameraViewProjMatrix2D(GetScreenWidth(), GetScreenHeight());
    package->renderer->SetFogEnable(false);
    Logger::SetEnable(true);
}

int Engine::GetScreenWidth() const
{
    return package->screenWidth;
}

int Engine::GetScreenHeight() const
{
    return package->screenHeight;
}

bool Engine::IsRenderIntersectionEnabled() const
{
    return package->renderIntersectionEnable;
}

void Engine::SetRenderIntersectionEnable(bool enable)
{
    package->renderIntersectionEnable = enable;
}

bool Engine::IsForcePlayerInvincibleEnabled() const
{
    return package->forcePlayerInvincibleEnable;
}

void Engine::SetForcePlayerInvincibleEnable(bool enable)
{
    package->forcePlayerInvincibleEnable = enable;
}

IDirect3DDevice9* Engine::GetDirect3DDevice() const
{
    return graphicDevice->GetDevice();
}

void Engine::ResetGraphicDevice()
{
    graphicDevice->Reset();
}

void Engine::AddLostableGraphicResource(const std::shared_ptr<LostableGraphicResource>& resource)
{
    lostableGraphicResourceManager->AddResource(resource);
}

void Engine::ReleaseLostableGraphicResource()
{
    lostableGraphicResourceManager->OnLostDeviceAll();
}

void Engine::RestoreLostableGraphicDevice()
{
    lostableGraphicResourceManager->OnResetDeviceAll();
}

void Engine::SetBackBufferRenderTarget()
{
    graphicDevice->SetBackbufferRenderTarget();
}

void Engine::ReleaseUnusedLostableGraphicResource()
{
    lostableGraphicResourceManager->ReleaseUnusedResource();
}

KeyState Engine::GetKeyState(Key k)
{
    return package->inputDevice->GetKeyState(k);
}

KeyState Engine::GetVirtualKeyState(VirtualKey vk)
{
    return package->vKeyInputSource->GetVirtualKeyState(vk);
}

void Engine::SetVirtualKeyState(VirtualKey vk, KeyState state)
{
    package->vKeyInputSource->SetVirtualKeyState(vk, state);
}

void Engine::AddVirtualKey(VirtualKey vk, Key k, PadButton btn)
{
    package->keyAssign->AddVirtualKey(vk, k, btn);
}

KeyState Engine::GetMouseState(MouseButton btn)
{
    return package->inputDevice->GetMouseState(btn);
}

int Engine::GetMouseX()
{
    return package->inputDevice->GetMouseX(GetScreenWidth(), GetScreenHeight());
}

int Engine::GetMouseY()
{
    return package->inputDevice->GetMouseY(GetScreenWidth(), GetScreenHeight());
}

int Engine::GetMouseMoveZ()
{
    return package->inputDevice->GetMouseMoveZ();
}

void Engine::SetMousePostionProvider(const std::shared_ptr<MousePositionProvider>& provider)
{
    mousePosProvider = provider;
    package->inputDevice->SetMousePositionProvider(mousePosProvider);
}

void Engine::SetInputEnable(bool enable)
{
    package->inputDevice->SetInputEnable(enable);
}

void Engine::WriteLog(const std::string && msg, const std::shared_ptr<SourcePos>& srcPos)
{
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_USER)
        .SetMessage(std::move(msg))
        .AddSourcePos(srcPos)));
}

std::wstring Engine::GetCurrentDateTimeS()
{
    time_t now = std::time(nullptr);
    struct tm* local = std::localtime(&now);
    std::string buf(14, '\0');
    sprintf(&buf[0], "%04d%02d%02d%02d%02d%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
    return ToUnicode(buf);
}

float Engine::GetCurrentFps() const
{
    return package->fpsCounter->GetStable();
}

float Engine::GetStageTime() const
{
    if (IsStageFinished())
    {
        return 0.0;
    }
    return package->stageStartTime->GetElapsedMilliSec();
}

float Engine::GetPackageTime() const
{
    return package->packageStartTime->GetElapsedMilliSec();
}

void Engine::UpdateFpsCounter()
{
    package->fpsCounter->Update();
}

void Engine::ResetFpsCounter()
{
    package->fpsCounter = std::make_shared<FpsCounter>();
}

void Engine::StartSlow(FrameCount pseudoFps, bool byPlayer)
{
    pseudoFps = constrain(pseudoFps, (FrameCount)1, (FrameCount)60);
    if (byPlayer)
    {
        package->pseudoPlayerFps = pseudoFps;
    } else
    {
        package->pseudoEnemyFps = pseudoFps;
    }
}

void Engine::StopSlow(bool byPlayer)
{
    if (byPlayer)
    {
        package->pseudoPlayerFps = 60;
    } else
    {
        package->pseudoEnemyFps = 60;
    }
}

int64_t Engine::GetElapsedFrame() const
{
    return package->elapsedFrame;
}

std::wstring Engine::GetMainStgScriptPath() const
{
    return package->stageMainScriptInfo.path;
}

std::wstring Engine::GetMainStgScriptDirectory() const
{
    return GetParentPath(package->stageMainScriptInfo.path) + L"/";
}

std::wstring Engine::GetMainPackageScriptPath() const
{
    return package->packageMainScriptInfo.path;
}

std::shared_ptr<Texture> Engine::LoadTexture(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    return package->textureCache->Load(path, reserve, srcPos);
}

void Engine::LoadTextureInThread(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) noexcept(true)
{
    package->textureCache->LoadInThread(path, reserve, srcPos);
}

void Engine::RemoveTextureReservedFlag(const std::wstring & path)
{
    package->textureCache->RemoveReservedFlag(path);
}

void Engine::ReleaseUnusedTextureCache()
{
    package->textureCache->ReleaseUnusedTexture();
}

void Engine::ReleaseUnusedFontCache()
{
    package->ReleaseUnusedFont();
}

const std::unordered_map<FontParams, std::shared_ptr<Font>>& Engine::GetFontMap() const
{
    return package->GetFontMap();
}

bool Engine::InstallFont(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
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

std::shared_ptr<RenderTarget> Engine::CreateRenderTarget(const std::wstring & name, int width, int height, const std::shared_ptr<SourcePos>& srcPos)
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

void Engine::RemoveRenderTarget(const std::wstring & name, const std::shared_ptr<SourcePos>& srcPos)
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

NullableSharedPtr<RenderTarget> Engine::GetRenderTarget(const std::wstring & name) const
{
    auto it = renderTargets.find(name);
    if (it != renderTargets.end())
    {
        return it->second;
    }
    return nullptr;
}

std::wstring Engine::GetReservedRenderTargetName(int idx) const
{
    return RESERVED_RENDER_TARGET_PREFIX + std::to_wstring(idx);
}

std::wstring Engine::GetTransitionRenderTargetName() const
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

void Engine::SaveRenderedTextureA1(const std::wstring & name, const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto renderTarget = GetRenderTarget(name))
    {
        auto viewport = renderTarget->GetViewport();
        SaveRenderedTextureA2(name, path, viewport.X, viewport.Y, viewport.X + viewport.Width, viewport.Y + viewport.Height, srcPos);
    }
}

void Engine::SaveRenderedTextureA2(const std::wstring & name, const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos)
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

void Engine::SaveSnapShotA1(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    SaveSnapShotA2(path, 0, 0, GetScreenWidth(), GetScreenHeight(), srcPos);
}

void Engine::SaveSnapShotA2(const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos)
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

std::shared_ptr<Shader> Engine::CreateShader(const std::wstring & path, bool precompiled)
{
    auto shader = std::make_shared<Shader>(path, precompiled, GetDirect3DDevice());
    AddLostableGraphicResource(shader);
    return shader;
}

bool Engine::IsPixelShaderSupported(int major, int minor)
{
    D3DCAPS9 caps;
    GetDirect3DDevice()->GetDeviceCaps(&caps);
    return caps.PixelShaderVersion >= D3DPS_VERSION(major, minor);
}

std::shared_ptr<Mesh> Engine::LoadMesh(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return package->meshCache->Load(path, srcPos);
}

void Engine::ReleaseUnusedMeshCache()
{
    package->meshCache->ReleaseUnusedMesh();
}

std::shared_ptr<SoundBuffer> Engine::LoadSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return package->soundDevice->LoadSound(path, false, srcPos);
}

void Engine::LoadOrphanSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->orphanSounds[GetCanonicalPath(path)] = LoadSound(path, srcPos);
}

void Engine::RemoveOrphanSound(const std::wstring & path)
{
    package->orphanSounds.erase(GetCanonicalPath(path));
}

void Engine::PlayBGM(const std::wstring & path, double loopStartSec, double loopEndSec)
{
    auto it = package->orphanSounds.find(GetCanonicalPath(path));
    if (it != package->orphanSounds.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->SetLoopEnable(true);
        sound->SetLoopTime(loopStartSec, loopEndSec);
        sound->Play();
    }
}

void Engine::PlaySE(const std::wstring & path)
{
    auto it = package->orphanSounds.find(GetCanonicalPath(path));
    if (it != package->orphanSounds.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->Play();
    }
}

void Engine::StopOrphanSound(const std::wstring & path)
{
    auto it = package->orphanSounds.find(GetCanonicalPath(path));
    if (it != package->orphanSounds.end())
    {
        it->second->Stop();
    }
}

void Engine::CacheSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->soundDevice->LoadSound(path, true, srcPos);
}

void Engine::RemoveSoundCache(const std::wstring & path)
{
    package->soundDevice->RemoveSoundCache(path);
}

void Engine::ClearSoundCache()
{
    package->soundDevice->ClearSoundCache();
}

void Engine::SetObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority)
{
    package->objLayerList->SetRenderPriority(obj, priority);
}

void Engine::SetShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader)
{
    package->objLayerList->SetLayerShader(beginPriority, endPriority, shader);
}

void Engine::ResetShader(int beginPriority, int endPriority)
{
    package->objLayerList->ResetLayerShader(beginPriority, endPriority);
}

int Engine::GetStgFrameRenderPriorityMin() const
{
    return package->objLayerList->GetStgFrameRenderPriorityMin();
}

void Engine::SetStgFrameRenderPriorityMin(int p)
{
    package->objLayerList->SetStgFrameRenderPriorityMin(p);
}

int Engine::GetStgFrameRenderPriorityMax() const
{
    return package->objLayerList->GetStgFrameRenderPriorityMax();
}

void Engine::SetStgFrameRenderPriorityMax(int p)
{
    package->objLayerList->SetStgFrameRenderPriorityMax(p);
}

int Engine::GetShotRenderPriority() const
{
    return package->objLayerList->GetShotRenderPriority();
}

void Engine::SetShotRenderPriority(int p)
{
    return package->objLayerList->SetShotRenderPriority(p);
}

int Engine::GetItemRenderPriority() const
{
    return package->objLayerList->GetItemRenderPriority();
}

void Engine::SetItemRenderPriority(int p)
{
    return package->objLayerList->SetItemRenderPriority(p);
}

int Engine::GetPlayerRenderPriority() const
{
    if (auto player = GetPlayerObject())
    {
        return player->getRenderPriority();
    }
    return DEFAULT_PLAYER_RENDER_PRIORITY;
}

int Engine::GetCameraFocusPermitRenderPriority() const
{
    return package->objLayerList->GetCameraFocusPermitRenderPriority();
}

void Engine::SetInvalidRenderPriority(int min, int max)
{
    package->objLayerList->SetInvalidRenderPriority(min, max);
}

void Engine::ClearInvalidRenderPriority()
{
    package->objLayerList->ClearInvalidRenderPriority();
}

NullableSharedPtr<Shader> Engine::GetShader(int p) const
{
    return package->objLayerList->GetLayerShader(p);
}

void Engine::SetFogEnable(bool enable)
{
    package->renderer->SetFogEnable(enable);
}

void Engine::SetFogParam(float fogStart, float fogEnd, int r, int g, int b)
{
    package->renderer->SetFogParam(fogStart, fogEnd, r, g, b);
}

void Engine::SetCameraFocusX(float x)
{
    package->camera3D->SetFocusX(x);
}

void Engine::SetCameraFocusY(float y)
{
    package->camera3D->SetFocusY(y);
}

void Engine::SetCameraFocusZ(float z)
{
    package->camera3D->SetFocusZ(z);
}

void Engine::SetCameraFocusXYZ(float x, float y, float z)
{
    package->camera3D->SetFocusXYZ(x, y, z);
}

void Engine::SetCameraRadius(float r)
{
    package->camera3D->SetRadius(r);
}

void Engine::SetCameraAzimuthAngle(float angle)
{
    package->camera3D->SetAzimuthAngle(angle);
}

void Engine::SetCameraElevationAngle(float angle)
{
    package->camera3D->SetElevationAngle(angle);
}

void Engine::SetCameraYaw(float yaw)
{
    package->camera3D->SetYaw(yaw);
}

void Engine::SetCameraPitch(float pitch)
{
    package->camera3D->SetPitch(pitch);
}

void Engine::SetCameraRoll(float roll)
{
    package->camera3D->SetRoll(roll);
}

void Engine::ResetCamera()
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

float Engine::GetCameraX() const
{
    return package->camera3D->GetX();
}

float Engine::GetCameraY() const
{
    return package->camera3D->GetY();
}

float Engine::GetCameraZ() const
{
    return package->camera3D->GetZ();
}

float Engine::GetCameraFocusX() const
{
    return package->camera3D->GetFocusX();
}

float Engine::GetCameraFocusY() const
{
    return package->camera3D->GetFocusY();
}

float Engine::GetCameraFocusZ() const
{
    return package->camera3D->GetFocusZ();
}

float Engine::GetCameraRadius() const
{
    return package->camera3D->GetRadius();
}

float Engine::GetCameraAzimuthAngle() const
{
    return package->camera3D->GetAzimuthAngle();
}

float Engine::GetCameraElevationAngle() const
{
    return package->camera3D->GetElevationAngle();
}

float Engine::GetCameraYaw() const
{
    return package->camera3D->GetYaw();
}

float Engine::GetCameraPitch() const
{
    return package->camera3D->GetPitch();
}

float Engine::GetCameraRoll() const
{
    return package->camera3D->GetRoll();
}

void Engine::SetCameraPerspectiveClip(float nearClip, float farClip)
{
    return package->camera3D->SetPerspectiveClip(nearClip, farClip);
}

void Engine::Set2DCameraFocusX(float x)
{
    package->camera2D->SetFocusX(x);
}

void Engine::Set2DCameraFocusY(float y)
{
    package->camera2D->SetFocusY(y);
}

void Engine::Set2DCameraAngleZ(float z)
{
    package->camera2D->SetAngleZ(z);
}

void Engine::Set2DCameraRatio(float r)
{
    package->camera2D->SetRatio(r);
}

void Engine::Set2DCameraRatioX(float x)
{
    package->camera2D->SetRatioX(x);
}

void Engine::Set2DCameraRatioY(float y)
{
    package->camera2D->SetRatioY(y);
}

void Engine::Reset2DCamera()
{
    package->camera2D->Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
}

float Engine::Get2DCameraX() const
{
    return package->camera2D->GetX();
}

float Engine::Get2DCameraY() const
{
    return package->camera2D->GetY();
}

float Engine::Get2DCameraAngleZ() const
{
    return package->camera2D->GetAngleZ();
}

float Engine::Get2DCameraRatio() const
{
    return package->camera2D->GetRatio();
}

float Engine::Get2DCameraRatioX() const
{
    return package->camera2D->GetRatioX();
}

float Engine::Get2DCameraRatioY() const
{
    return package->camera2D->GetRatioY();
}

void Engine::SetCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    package->commonDataDB->SetCommonData(key, std::move(value));
}

const std::unique_ptr<DnhValue>& Engine::GetCommonData(const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return package->commonDataDB->GetCommonData(key, defaultValue);
}

void Engine::ClearCommonData()
{
    package->commonDataDB->ClearCommonData();
}

void Engine::DeleteCommonData(const std::wstring & key)
{
    package->commonDataDB->DeleteCommonData(key);
}

void Engine::SetAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    package->commonDataDB->SetAreaCommonData(areaName, key, std::move(value));
}

const std::unique_ptr<DnhValue>& Engine::GetAreaCommonData(const std::wstring & areaName, const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return package->commonDataDB->GetAreaCommonData(areaName, key, defaultValue);
}

void Engine::ClearAreaCommonData(const std::wstring & areaName)
{
    package->commonDataDB->ClearAreaCommonData(areaName);
}

void Engine::DeleteAreaCommonData(const std::wstring & areaName, const std::wstring & key)
{
    package->commonDataDB->DeleteAreaCommonData(areaName, key);
}

void Engine::CreateCommonDataArea(const std::wstring & areaName)
{
    package->commonDataDB->CreateCommonDataArea(areaName);
}

bool Engine::IsCommonDataAreaExists(const std::wstring & areaName) const
{
    return package->commonDataDB->IsCommonDataAreaExists(areaName);
}

void Engine::CopyCommonDataArea(const std::wstring & dest, const std::wstring & src)
{
    package->commonDataDB->CopyCommonDataArea(dest, src);
}

std::vector<std::wstring> Engine::GetCommonDataAreaKeyList() const
{
    return package->commonDataDB->GetCommonDataAreaKeyList();
}

std::vector<std::wstring> Engine::GetCommonDataValueKeyList(const std::wstring & areaName) const
{
    return package->commonDataDB->GetCommonDataValueKeyList(areaName);
}

bool Engine::SaveCommonDataAreaA1(const std::wstring & areaName) const
{
    try
    {
        package->commonDataDB->SaveCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Engine::LoadCommonDataAreaA1(const std::wstring & areaName)
{
    try
    {
        package->commonDataDB->LoadCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Engine::SaveCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path) const
{
    try
    {
        package->commonDataDB->SaveCommonDataArea(areaName, path);
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

bool Engine::LoadCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path)
{
    try
    {
        package->commonDataDB->LoadCommonDataArea(areaName, path);
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

std::wstring Engine::GetDefaultCommonDataSavePath(const std::wstring & areaName) const
{
    std::wstring basePath = GetMainPackageScriptPath() == DEFAULT_PACKAGE_PATH ? GetMainStgScriptPath() : GetMainPackageScriptPath();
    if (basePath.empty())
    {
        basePath = GetMainPackageScriptPath();
    }
    if (basePath.empty()) return false;
    return GetParentPath(basePath) + L"/data/" + GetStem(basePath) + L"_common_" + areaName + L".dat";
}

void Engine::LoadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->playerShotDataTable->Load(path, srcPos);
}

void Engine::ReloadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->playerShotDataTable->Reload(path, srcPos);
}

void Engine::LoadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->enemyShotDataTable->Load(path, srcPos);
}

void Engine::ReloadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->enemyShotDataTable->Reload(path, srcPos);
}

void Engine::LoadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->itemDataTable->Load(path, package->fileLoader, srcPos);
}

void Engine::ReloadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    package->itemDataTable->Reload(path, package->fileLoader, srcPos);
}

NullableSharedPtr<ShotData> Engine::GetPlayerShotData(int id) const
{
    return package->playerShotDataTable->Get(id);
}

NullableSharedPtr<ShotData> Engine::GetEnemyShotData(int id) const
{
    return package->enemyShotDataTable->Get(id);
}

NullableSharedPtr<ItemData> Engine::GetItemData(int id) const
{
    return package->itemDataTable->Get(id);
}

std::shared_ptr<ObjText> Engine::CreateObjText()
{
    auto obj = package->objTable->Create<ObjText>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSound> Engine::CreateObjSound()
{
    return package->objTable->Create<ObjSound>(package);
}

std::shared_ptr<ObjFileT> Engine::CreateObjFileT()
{
    return package->objTable->Create<ObjFileT>(package);
}

std::shared_ptr<ObjFileB> Engine::CreateObjFileB()
{
    return package->objTable->Create<ObjFileB>(package);
}

std::shared_ptr<ObjShader> Engine::CreateObjShader()
{
    auto obj = package->objTable->Create<ObjShader>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjShot> Engine::CreateObjShot(bool isPlayerShot)
{
    auto shot = package->objTable->Create<ObjShot>(isPlayerShot, package);
    package->objLayerList->SetRenderPriority(shot, package->objLayerList->GetShotRenderPriority());
    return shot;
}

std::shared_ptr<ObjShot> Engine::CreateShotA1(float x, float y, float speed, float angle, int shotDataId, int delay, bool isPlayerShot)
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

std::shared_ptr<ObjShot> Engine::CreateShotA2(float x, float y, float speed, float angle, float accel, float maxSpeed, int shotDataId, int delay, bool isPlayerShot)
{
    auto shot = CreateShotA1(x, y, speed, angle, shotDataId, delay, isPlayerShot);
    shot->SetAcceleration(accel);
    shot->SetMaxSpeed(maxSpeed);
    return shot;
}

NullableSharedPtr<ObjShot> Engine::CreateShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot)
{
    if (auto obj = GetObject<ObjRender>(objId))
    {
        return CreateShotA1(obj->GetX(), obj->GetY(), speed, angle, shotDataId, delay, isPlayerShot);
    } else
    {
        return nullptr;
    }
}

std::shared_ptr<ObjShot> Engine::CreateShotB1(float x, float y, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot)
{
    return CreateShotB2(x, y, speedX, speedY, 0, 0, 0, 0, shotDataId, delay, isPlayerShot);
}

std::shared_ptr<ObjShot> Engine::CreateShotB2(float x, float y, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, int shotDataId, int delay, bool isPlayerShot)
{
    auto shot = CreateObjShot(isPlayerShot);
    shot->SetMovePosition(x, y);
    shot->SetMoveMode(std::make_shared<MoveModeB>(speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY));
    shot->SetShotData(isPlayerShot ? GetPlayerShotData(shotDataId) : GetEnemyShotData(shotDataId));
    shot->SetDelay(delay);
    shot->Regist();
    return shot;
}

NullableSharedPtr<ObjShot> Engine::CreateShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot)
{
    if (auto obj = GetObject<ObjRender>(objId))
    {
        return CreateShotB1(obj->GetX(), obj->GetY(), speedX, speedY, shotDataId, delay, isPlayerShot);
    } else
    {
        return nullptr;
    }
}

NullableSharedPtr<ObjShot> Engine::CreatePlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId)
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

std::shared_ptr<ObjLooseLaser> Engine::CreateObjLooseLaser(bool isPlayerShot)
{
    auto laser = package->objTable->Create<ObjLooseLaser>(isPlayerShot, package);
    package->objLayerList->SetRenderPriority(laser, package->objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjLooseLaser> Engine::CreateLooseLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot)
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

std::shared_ptr<ObjStLaser> Engine::CreateObjStLaser(bool isPlayerShot)
{
    auto laser = package->objTable->Create<ObjStLaser>(isPlayerShot, package);
    package->objLayerList->SetRenderPriority(laser, package->objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjStLaser> Engine::CreateStraightLaserA1(float x, float y, float angle, float length, float width, int deleteFrame, int shotDataId, int delay, bool isPlayerShot)
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

std::shared_ptr<ObjCrLaser> Engine::CreateObjCrLaser(bool isPlayerShot)
{
    auto laser = package->objTable->Create<ObjCrLaser>(isPlayerShot, package);
    package->objLayerList->SetRenderPriority(laser, package->objLayerList->GetShotRenderPriority());
    return laser;
}

std::shared_ptr<ObjCrLaser> Engine::CreateCurveLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot)
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

std::shared_ptr<ObjItem> Engine::CreateItemA1(int itemType, float x, float y, PlayerScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemA2(int itemType, float x, float y, float destX, float destY, PlayerScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemU1(int itemDataId, float x, float y, PlayerScore score)
{
    auto item = CreateObjItem(ITEM_USER);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    item->SetItemData(GetItemData(itemDataId));
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemU2(int itemDataId, float x, float y, float destX, float destY, PlayerScore score)
{
    auto item = CreateObjItem(ITEM_USER);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->SetScore(score);
    item->SetItemData(GetItemData(itemDataId));
    return item;
}

std::shared_ptr<ObjEnemy> Engine::CreateObjEnemy()
{
    auto enemy = package->objTable->Create<ObjEnemy>(false, package);
    package->objLayerList->SetRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
}

std::shared_ptr<ObjEnemyBossScene> Engine::CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto bossScene = package->enemyBossSceneObj.lock())
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
    auto bossScene = package->objTable->Create<ObjEnemyBossScene>(package);
    package->enemyBossSceneObj = bossScene;
    return bossScene;
}

std::shared_ptr<ObjSpell> Engine::CreateObjSpell()
{
    auto spell = package->objTable->Create<ObjSpell>(package);
    package->objLayerList->SetRenderPriority(spell, 50);
    return spell;
}

std::shared_ptr<ObjItem> Engine::CreateObjItem(int itemType)
{
    auto obj = package->objTable->Create<ObjItem>(itemType, package);
    package->objLayerList->SetRenderPriority(obj, package->objLayerList->GetItemRenderPriority());
    obj->SetIntersection();
    return obj;
}

std::shared_ptr<Script> Engine::GetScript(int scriptId) const
{
    return package->scriptManager->Get(scriptId);
}

std::shared_ptr<Script> Engine::LoadScript(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = package->scriptManager->Compile(path, type, version, srcPos);
    script->Load();
    return script;
}

std::shared_ptr<Script> Engine::LoadScriptInThread(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = package->scriptManager->CompileInThread(path, type, version, srcPos);
    return script;
}

void Engine::CloseStgScene()
{
    if (auto stageMain = package->stageMainScript.lock())
    {
        stageMain->Close();
    }
}

void Engine::NotifyEventAll(int eventType)
{
    package->scriptManager->NotifyEventAll(eventType);
}

void Engine::NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args)
{
    package->scriptManager->NotifyEventAll(eventType, args);
}

std::wstring Engine::GetPlayerID() const
{
    return package->stagePlayerScriptInfo.id;
}

std::wstring Engine::GetPlayerReplayName() const
{
    return package->stagePlayerScriptInfo.replayName;
}

NullableSharedPtr<ObjPlayer> Engine::GetPlayerObject() const
{
    auto player = package->playerObj.lock();
    if (player && !player->IsDead()) return player;
    return nullptr;
}

NullableSharedPtr<ObjEnemy> Engine::GetEnemyBossObject() const
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

NullableSharedPtr<ObjEnemyBossScene> Engine::GetEnemyBossSceneObject() const
{
    auto bossScene = package->enemyBossSceneObj.lock();
    if (bossScene && !bossScene->IsDead()) return bossScene;
    return nullptr;
}

NullableSharedPtr<ObjSpellManage> Engine::GetSpellManageObject() const
{
    auto spellManage = package->spellManageObj.lock();
    if (spellManage && !spellManage->IsDead()) return spellManage;
    return nullptr;
}

void Engine::DeleteObject(int id)
{
    package->objTable->Delete(id);
}

bool Engine::IsObjectDeleted(int id) const
{
    return package->objTable->IsDeleted(id);
}

std::shared_ptr<ObjPrim2D> Engine::CreateObjPrim2D()
{
    auto obj = package->objTable->Create<ObjPrim2D>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite2D> Engine::CreateObjSprite2D()
{
    auto obj = package->objTable->Create<ObjSprite2D>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSpriteList2D> Engine::CreateObjSpriteList2D()
{
    auto obj = package->objTable->Create<ObjSpriteList2D>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjPrim3D> Engine::CreateObjPrim3D()
{
    auto obj = package->objTable->Create<ObjPrim3D>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite3D> Engine::CreateObjSprite3D()
{
    auto obj = package->objTable->Create<ObjSprite3D>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjMesh> Engine::CreateObjMesh()
{
    auto obj = package->objTable->Create<ObjMesh>(package);
    package->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

NullableSharedPtr<Script> Engine::GetPlayerScript() const
{
    return package->stagePlayerScript.lock();
}

const std::unique_ptr<DnhValue>& Engine::GetScriptResult(int scriptId) const
{
    return package->scriptManager->GetScriptResult(scriptId);
}

void Engine::SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value)
{
    package->scriptManager->SetScriptResult(scriptId, std::move(value));
}

std::vector<ScriptInfo> Engine::GetScriptList(const std::wstring & dirPath, int scriptType, bool doRecursive)
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
            auto info = ScanDnhScriptInfo(path, package->fileLoader);
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

void Engine::GetLoadFreePlayerScriptList()
{
    package->freePlayerScriptInfoList = GetScriptList(FREE_PLAYER_DIR, TYPE_SCRIPT_PLAYER, true);
}

int Engine::GetFreePlayerScriptCount() const
{
    return package->freePlayerScriptInfoList.size();
}

ScriptInfo Engine::GetFreePlayerScriptInfo(int idx) const
{
    return package->freePlayerScriptInfoList.at(idx);
}

ScriptInfo Engine::GetScriptInfo(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        return ScanDnhScriptInfo(path, package->fileLoader);
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

PlayerScore Engine::GetScore() const
{
    return package->GetPlayerScore();
}

void Engine::AddScore(PlayerScore score)
{
    package->SetPlayerScore(package->GetPlayerScore() + score);
}

int64_t Engine::GetGraze() const
{
    return package->GetPlayerGraze();
}

void Engine::AddGraze(int64_t graze)
{
    package->SetPlayerGraze(package->GetPlayerGraze() + graze);
}

int64_t Engine::GetPoint() const
{
    return package->GetPlayerPoint();
}

void Engine::AddPoint(int64_t point)
{
    package->SetPlayerPoint(package->GetPlayerPoint() + point);
}

void Engine::SetStgFrame(float left, float top, float right, float bottom)
{
    package->stgFrame->left = left;
    package->stgFrame->top = top;
    package->stgFrame->right = right;
    package->stgFrame->bottom = bottom;
}

float Engine::GetStgFrameLeft() const
{
    return package->stgFrame->left;
}

float Engine::GetStgFrameTop() const
{
    return package->stgFrame->top;
}

float Engine::GetStgFrameWidth() const
{
    return package->stgFrame->right - package->stgFrame->left;
}

float Engine::GetStgFrameHeight() const
{
    return package->stgFrame->bottom - package->stgFrame->top;
}

float Engine::GetStgFrameCenterWorldX() const
{
    return (package->stgFrame->right - package->stgFrame->left) / 2.0f;
}

float Engine::GetStgFrameCenterWorldY() const
{
    return (package->stgFrame->bottom - package->stgFrame->top) / 2.0f;
}

float Engine::GetStgFrameCenterScreenX() const
{
    return (package->stgFrame->right + package->stgFrame->left) / 2.0f;
}

float Engine::GetStgFrameCenterScreenY() const
{
    return (package->stgFrame->bottom + package->stgFrame->top) / 2.0f;
}

int Engine::GetAllShotCount() const
{
    return package->shotCounter->playerShotCount + package->shotCounter->enemyShotCount;
}

int Engine::GetEnemyShotCount() const
{
    return package->shotCounter->enemyShotCount;
}

int Engine::GetPlayerShotCount() const
{
    return package->shotCounter->playerShotCount;
}

void Engine::SetShotAutoDeleteClip(float left, float top, float right, float bottom)
{
    package->shotAutoDeleteClip->SetClip(left, top, right, bottom);
}

void Engine::StartShotScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
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
    auto shotScript = LoadScript(path, SCRIPT_TYPE_SHOT_CUSTOM, package->stageMainScriptInfo.version, srcPos);
    package->shotScript = shotScript;
    shotScript->Start();
    shotScript->RunInitialize();
}

void Engine::SetDeleteShotImmediateEventOnShotScriptEnable(bool enable)
{
    package->deleteShotImmediateEventOnShotScriptEnable = enable;
}

void Engine::SetDeleteShotFadeEventOnShotScriptEnable(bool enable)
{
    package->deleteShotFadeEventOnShotScriptEnable = enable;
}

void Engine::SetDeleteShotToItemEventOnShotScriptEnable(bool enable)
{
    package->deleteShotToItemEventOnShotScriptEnable = enable;
}

void Engine::DeleteShotAll(int target, int behavior) const
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

void Engine::DeleteShotInCircle(int target, int behavior, float x, float y, float r) const
{
    auto isects = package->colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), COL_GRP_ENEMY_SHOT);
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

std::vector<std::shared_ptr<ObjShot>> Engine::GetShotInCircle(float x, float y, float r, int target) const
{
    // 同じショットに紐付いている判定が複数あるので、まずIDだけを集める
    std::unordered_set<int> shotIds;

    auto isects = package->colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), -1);
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

void Engine::SetShotIntersectoinCicle(float x, float y, float r)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x, y, r);
    package->colDetector->Add(isect);
    package->tempEnemyShotIsects.push_back(isect);
}

void Engine::SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x1, y1, x2, y2, width);
    package->colDetector->Add(isect);
    package->tempEnemyShotIsects.push_back(isect);
}

void Engine::CollectAllItems()
{
    package->autoItemCollectionManager->CollectAllItems();
}

void Engine::CollectItemsByType(int itemType)
{
    package->autoItemCollectionManager->CollectItemsByType(itemType);
}

void Engine::CollectItemsInCircle(float x, float y, float r)
{
    package->autoItemCollectionManager->CollectItemsInCircle(x, y, r);
}

void Engine::CancelCollectItems()
{
    package->autoItemCollectionManager->CancelCollectItems();
}

void Engine::SetDefaultBonusItemEnable(bool enable)
{
    package->defaultBonusItemEnable = enable;
}

void Engine::StartItemScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
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
    auto itemScript = LoadScript(path, SCRIPT_TYPE_ITEM_CUSTOM, package->stageMainScriptInfo.version, srcPos);
    package->itemScript = itemScript;
    itemScript->Start();
    itemScript->RunInitialize();
}

bool Engine::IsPackageFinished() const
{
    if (package->packageMainScript.lock())
    {
        return false;
    }
    return true;
}

void Engine::ClosePackage()
{
    if (auto packageMain = package->packageMainScript.lock())
    {
        packageMain->Close();
    }
}

void Engine::InitializeStageScene()
{
    package->objTable->DeleteStgSceneObject();
    package->scriptManager->CloseStgSceneScript();
    SetStgFrame(32.0f, 16.0f, 416.0f, 464.0f);
    SetShotAutoDeleteClip(64.0f, 64.0f, 64.0f, 64.0f);
    Reset2DCamera();
    SetDefaultBonusItemEnable(true);
    package->stageSceneResult = 0;
    package->stageMainScript.reset();
    package->stagePlayerScript.reset();

    package->SetPlayerLife(2);
    package->SetPlayerSpell(3);
    package->SetPlayerPower(1);
    package->SetPlayerScore(0);
    package->SetPlayerGraze(0);
    package->SetPlayerPoint(0);

    SetStgFrameRenderPriorityMin(DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN);
    SetStgFrameRenderPriorityMax(DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX);
    SetShotRenderPriority(DEFAULT_SHOT_RENDER_PRIORITY);
    SetItemRenderPriority(DEFAULT_ITEM_RENDER_PRIORITY);
    package->stagePaused = true;
    package->stageForceTerminated = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
}

void Engine::FinalizeStageScene()
{
    // FUTURE : impl
}

void Engine::StartPackage()
{
    if (!IsPackageFinished())
    {
        Logger::WriteLog(Log::Level::LV_WARN, "package already started.");
        return;
    }
    package->packageStartTime = std::make_shared<TimePoint>();
    Logger::WriteLog(Log::Level::LV_INFO, "start package.");
    auto script = package->scriptManager->Compile(package->packageMainScriptInfo.path, SCRIPT_TYPE_PACKAGE, package->packageMainScriptInfo.version, nullptr);
    package->packageMainScript = script;
    script->Start();
    script->RunInitialize();
}

void Engine::SetPauseScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"PauseScript", std::make_unique<DnhArray>(path));
}

void Engine::SetEndSceneScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"EndSceneScript", std::make_unique<DnhArray>(path));
}

void Engine::SetReplaySaveSceneScriptPath(const std::wstring & path)
{
    CreateCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    SetAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"ReplaySaveSceneScript", std::make_unique<DnhArray>(path));
}

Point2D Engine::Get2DPosition(float x, float y, float z, bool isStgScene)
{
    D3DXMATRIX view, proj, viewport, billboard;
    package->camera3D->GenerateViewMatrix(&view, &billboard);
    if (isStgScene)
    {
        package->camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
    } else
    {
        package->camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
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
        package->camera2D->GenerateViewMatrix(&view);
        package->camera2D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        D3DXMATRIX viewProjViewport = view * proj * viewport;
        D3DXMatrixInverse(&viewProjViewport, NULL, &viewProjViewport);
        D3DXVec3TransformCoord(&pos, &pos, &viewProjViewport);
    }
    return Point2D(pos.x, pos.y);
}

void Engine::SetStageIndex(uint16_t idx)
{
    package->stageIdx = idx;
}

void Engine::SetStageMainScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        package->stageMainScriptInfo = ScanDnhScriptInfo(path, package->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        package->stageMainScriptInfo = ScriptInfo();
        package->stageMainScriptInfo.path = path;
    }
}

void Engine::SetStagePlayerScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        package->stagePlayerScriptInfo = ScanDnhScriptInfo(path, package->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        package->stageMainScriptInfo = ScriptInfo();
        package->stageMainScriptInfo.path = path;
    }
}

void Engine::SetStageMainScript(const ScriptInfo & script)
{
    package->stageMainScriptInfo = script;
}

void Engine::SetStagePlayerScript(const ScriptInfo & script)
{
    package->stagePlayerScriptInfo = script;
}

void Engine::SetStageReplayFile(const std::wstring & path)
{
    package->stageReplayFilePath = path;
}

bool Engine::IsStageFinished() const
{
    if (package->stageMainScript.lock())
    {
        return false;
    }
    return true;
}

int Engine::GetStageSceneResult() const
{
    return package->stageSceneResult;
}

bool Engine::IsStagePaused() const
{
    return package->stagePaused;
}

void Engine::PauseStageScene(bool doPause)
{
    if (doPause && !package->stagePaused)
    {
        NotifyEventAll(EV_PAUSE_ENTER);
    } else if (!doPause && package->stagePaused)
    {
        NotifyEventAll(EV_PAUSE_LEAVE);
    }
    package->stagePaused = doPause;
}

void Engine::TerminateStageScene()
{
    if (auto stageMain = package->stageMainScript.lock())
    {
        stageMain->Close();
        package->stageForceTerminated = true;
    }
}

void Engine::SetPackageMainScript(const std::wstring & path)
{
    try
    {
        package->packageMainScriptInfo = ScanDnhScriptInfo(path, package->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN);
        Logger::WriteLog(log);
        package->stageMainScriptInfo = ScriptInfo();
        package->stageMainScriptInfo.path = path;
    }
}

void Engine::SetPackageMainScript(const ScriptInfo & script)
{
    package->packageMainScriptInfo = script;
}

void Engine::StartStageScene(const std::shared_ptr<SourcePos>& srcPos)
{
    package->objTable->DeleteStgSceneObject();
    package->scriptManager->CloseStgSceneScript();
    package->inputDevice->ResetInputState();
    Reset2DCamera();
    ResetCamera();
    SetDefaultBonusItemEnable(true);
    package->playerShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, package->textureCache, package->fileLoader);
    package->enemyShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, package->textureCache, package->fileLoader);
    package->itemDataTable = std::make_shared<ItemDataTable>(package->textureCache, package->fileLoader);
    package->stageMainScript.reset();
    package->stagePlayerScript.reset();
    ReloadItemData(DEFAULT_ITEM_DATA_PATH, nullptr);
    package->stageSceneResult = 0;
    package->stageForceTerminated = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
    package->stagePaused = false;
    package->renderer->SetFogEnable(false);
    package->pseudoPlayerFps = package->pseudoEnemyFps = 60;

    if (package->stageMainScriptInfo.systemPath.empty() || package->stageMainScriptInfo.systemPath == L"DEFAULT")
    {
        package->stageMainScriptInfo.systemPath = DEFAULT_SYSTEM_PATH;
    }

    // #System
    auto systemScript = package->scriptManager->Compile(package->stageMainScriptInfo.systemPath, SCRIPT_TYPE_STAGE, package->stageMainScriptInfo.version, srcPos);
    systemScript->Start();
    systemScript->RunInitialize();

    // Create Player
    auto player = package->objTable->Create<ObjPlayer>(package);
    package->objLayerList->SetRenderPriority(player, DEFAULT_PLAYER_RENDER_PRIORITY);
    player->AddIntersectionToItem();
    package->playerObj = player;
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create player object.")
        .AddSourcePos(srcPos)));

    auto playerScript = package->scriptManager->Compile(package->stagePlayerScriptInfo.path, SCRIPT_TYPE_PLAYER, package->stagePlayerScriptInfo.version, srcPos);
    package->stagePlayerScript = playerScript;
    playerScript->Start();
    playerScript->RunInitialize();

    // Main
    auto stageMainScriptPath = package->stageMainScriptInfo.path;
    if (package->stageMainScriptInfo.type == SCRIPT_TYPE_SINGLE)
    {
        stageMainScriptPath = SYSTEM_SINGLE_STAGE_PATH;
    } else if (package->stageMainScriptInfo.type == SCRIPT_TYPE_PLURAL)
    {
        stageMainScriptPath = SYSTEM_PLURAL_STAGE_PATH;
    }
    auto stageMainScript = package->scriptManager->Compile(stageMainScriptPath, SCRIPT_TYPE_STAGE, package->stageMainScriptInfo.version, srcPos);
    package->stageMainScript = stageMainScript;
    stageMainScript->Start();
    stageMainScript->RunInitialize();
    package->stageStartTime = std::make_shared<TimePoint>();

    // #Background
    if (!package->stageMainScriptInfo.backgroundPath.empty() && package->stageMainScriptInfo.backgroundPath != L"DEFAULT")
    {
        auto backgroundScript = package->scriptManager->Compile(package->stageMainScriptInfo.backgroundPath, SCRIPT_TYPE_STAGE, package->stageMainScriptInfo.version, srcPos);
        backgroundScript->Start();
        backgroundScript->RunInitialize();
    }

    // #BGM
    if (!package->stageMainScriptInfo.bgmPath.empty() && package->stageMainScriptInfo.bgmPath != L"DEFAULT")
    {
        try
        {
            LoadOrphanSound(package->stageMainScriptInfo.bgmPath, nullptr); // TODO: #BGMヘッダのSourcePosを与える
            auto& bgm = package->orphanSounds[GetCanonicalPath(package->packageMainScriptInfo.bgmPath)];
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

void Engine::RenderToTexture(const std::wstring& name, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag)
{
    if (!package) return;

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
    package->renderer->InitRenderState();

    if (doClear)
    {
        graphicDevice->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    }

    begin = std::max(begin, 0);
    end = std::min(end, MAX_RENDER_PRIORITY);

    auto obj = package->objTable->Get<ObjRender>(objId);
    if (obj && checkVisibleFlag && !obj->IsVisible()) return;

    package->camera3D->GenerateViewMatrix(&viewMatrix3D, &billboardMatrix);

    // [0, stgFrameMin]
    {
        package->renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        package->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        package->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        package->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = 0; p < GetStgFrameRenderPriorityMin(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(package->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && package->objLayerList->IsInvalidRenderPriority(p)))
                {
                    package->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, package->renderer);
                }
            }
        }
    }
    // [stgFrameMin, stgFrameMax]
    {
        if (!IsStagePaused())
        {
            RECT scissorRect = { (LONG)package->stgFrame->left, (LONG)package->stgFrame->top, (LONG)package->stgFrame->right, (LONG)package->stgFrame->bottom };
            if (renderToBackBuffer)
            {
                scissorRect.left = package->stgFrame->left * graphicDevice->GetBackBufferWidth() / package->screenWidth;
                scissorRect.top = package->stgFrame->top * graphicDevice->GetBackBufferHeight() / package->screenHeight;
                scissorRect.right = package->stgFrame->right * graphicDevice->GetBackBufferWidth() / package->screenWidth;
                scissorRect.bottom = package->stgFrame->bottom * graphicDevice->GetBackBufferHeight() / package->screenHeight;
            }
            package->renderer->EnableScissorTest(scissorRect);
        }

        // set 2D matrix
        if (!IsStageFinished())
        {
            package->camera2D->GenerateViewMatrix(&viewMatrix2D);
            package->camera2D->GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
            package->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
        }

        // set 3D matrix
        package->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        package->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);

        for (int p = GetStgFrameRenderPriorityMin(); p <= GetStgFrameRenderPriorityMax(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(package->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && package->objLayerList->IsInvalidRenderPriority(p)))
                {
                    package->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, package->renderer);
                }
            }
            if (p == package->objLayerList->GetCameraFocusPermitRenderPriority())
            {
                // cameraFocusPermitRenderPriorityより大きい優先度では別のビュー変換行列を使う
                if (!IsStageFinished())
                {
                    Camera2D focusForbidCamera;
                    focusForbidCamera.Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
                    focusForbidCamera.GenerateViewMatrix(&viewMatrix2D);
                    package->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
                }
            }
        }
    }
    {
        // (stgFrameMax, MAX_RENDER_PRIORITY]
        package->renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        package->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        package->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        package->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = GetStgFrameRenderPriorityMax() + 1; p <= MAX_RENDER_PRIORITY; p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(package->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && package->objLayerList->IsInvalidRenderPriority(p)))
                {
                    package->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, package->renderer);
                }
            }
        }
    }
    SetBackBufferRenderTarget();
}

NullableSharedPtr<Obj> Engine::GetObj(int id) const
{
    return package->objTable->Get<Obj>(id);
}

const std::map<int, std::shared_ptr<Obj>>& Engine::GetObjAll() const
{
    return package->objTable->GetAll();
}
}