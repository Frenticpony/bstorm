#include <bstorm/engine.hpp>

#include <bstorm/util.hpp>
#include <bstorm/const.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/graphic_device.hpp>
#include <bstorm/logger.hpp>
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
#include <bstorm/game_state.hpp>

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

    if (auto packageMain = gameState->packageMainScript.lock())
    {
        if (packageMain->IsClosed())
        {
            Logger::WriteLog(Log::Level::LV_INFO, "finish package.");
            gameState->packageMainScript.reset();
            Reset(GetScreenWidth(), GetScreenHeight());
            return;
        }
    }

    if (GetElapsedFrame() % (60 / std::min(gameState->pseudoEnemyFps, gameState->pseudoPlayerFps)) == 0)
    {
        if (auto stageMain = gameState->stageMainScript.lock())
        {
            if (stageMain->IsClosed())
            {
                if (gameState->stageForceTerminated)
                {
                    gameState->stageSceneResult = STAGE_RESULT_BREAK_OFF;
                    gameState->globalPlayerParams = std::make_shared<GlobalPlayerParams>();
                    gameState->globalPlayerParams->life = 0.0;
                } else if (auto player = GetPlayerObject())
                {
                    gameState->stageSceneResult = player->GetState() != STATE_END ? STAGE_RESULT_CLEARED : STAGE_RESULT_PLAYER_DOWN;
                } else
                {
                    gameState->stageSceneResult = STAGE_RESULT_PLAYER_DOWN;
                }
                RenderToTextureA1(GetTransitionRenderTargetName(), 0, MAX_RENDER_PRIORITY, true);
                gameState->objTable->DeleteStgSceneObject();
                gameState->scriptManager->CloseStgSceneScript();
                gameState->stageMainScript.reset();
                gameState->stagePlayerScript.reset();
                gameState->stageForceTerminated = false;
                gameState->stagePaused = true;
                gameState->pseudoPlayerFps = gameState->pseudoEnemyFps = 60;
            }
        }
        gameState->scriptManager->CleanClosedScript();

        if (!IsStagePaused())
        {
            gameState->colDetector->TestAllCollision();
        }

        // SetShotIntersection{Circle, Line}で設定した判定削除
        gameState->tempEnemyShotIsects.clear();

        gameState->inputDevice->UpdateInputState();

        gameState->scriptManager->RunAll(IsStagePaused());

        gameState->objTable->UpdateAll(IsStagePaused());

        gameState->autoItemCollectionManager->Reset();
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
    gameState->elapsedFrame += 1;
}

void Engine::Render()
{
    renderToTexture(L"", 0, MAX_RENDER_PRIORITY, ID_INVALID, true, true, true, true);
}

void Engine::Render(const std::wstring& renderTargetName)
{
    renderToTexture(renderTargetName, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
}

void Engine::RenderToTextureA1(const std::wstring& name, int begin, int end, bool doClear)
{
    renderToTexture(name, begin, end, ID_INVALID, doClear, false, false, false);
}

void Engine::RenderToTextureB1(const std::wstring& name, int objId, bool doClear)
{
    renderToTexture(name, 0, 0, objId, doClear, false, false, false);
}

void Engine::Reset(int screenWidth, int screenHeight)
{
    Logger::SetEnable(false);
    gameState = std::make_shared<GameState>(screenWidth, screenHeight, GetWindowHandle(), GetDirect3DDevice(), defaultKeyConfig, mousePosProvider, this);
    Reset2DCamera();
    ResetCamera();

    renderTargets.clear();
    CreateRenderTarget(GetTransitionRenderTargetName(), GetScreenWidth(), GetScreenHeight(), nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(0), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(1), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(2), 1024, 512, nullptr);
    gameState->renderer->SetForbidCameraViewProjMatrix2D(GetScreenWidth(), GetScreenHeight());
    gameState->renderer->SetFogEnable(false);
    Logger::SetEnable(true);
}

int Engine::GetScreenWidth() const
{
    return gameState->screenWidth;
}

int Engine::GetScreenHeight() const
{
    return gameState->screenHeight;
}

bool Engine::IsRenderIntersectionEnabled() const
{
    return gameState->renderIntersectionEnable;
}

void Engine::SetRenderIntersectionEnable(bool enable)
{
    gameState->renderIntersectionEnable = enable;
}

bool Engine::IsForcePlayerInvincibleEnabled() const
{
    return gameState->forcePlayerInvincibleEnable;
}

void Engine::SetForcePlayerInvincibleEnable(bool enable)
{
    gameState->forcePlayerInvincibleEnable = enable;
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
    return gameState->inputDevice->GetKeyState(k);
}

KeyState Engine::GetVirtualKeyState(VirtualKey vk)
{
    return gameState->vKeyInputSource->GetVirtualKeyState(vk);
}

void Engine::SetVirtualKeyState(VirtualKey vk, KeyState state)
{
    gameState->vKeyInputSource->SetVirtualKeyState(vk, state);
}

void Engine::AddVirtualKey(VirtualKey vk, Key k, PadButton btn)
{
    gameState->keyAssign->AddVirtualKey(vk, k, btn);
}

KeyState Engine::GetMouseState(MouseButton btn)
{
    return gameState->inputDevice->GetMouseState(btn);
}

int Engine::GetMouseX()
{
    return gameState->inputDevice->GetMouseX(GetScreenWidth(), GetScreenHeight());
}

int Engine::GetMouseY()
{
    return gameState->inputDevice->GetMouseY(GetScreenWidth(), GetScreenHeight());
}

int Engine::GetMouseMoveZ()
{
    return gameState->inputDevice->GetMouseMoveZ();
}

void Engine::SetMousePostionProvider(const std::shared_ptr<MousePositionProvider>& provider)
{
    mousePosProvider = provider;
    gameState->inputDevice->SetMousePositionProvider(mousePosProvider);
}

void Engine::SetInputEnable(bool enable)
{
    gameState->inputDevice->SetInputEnable(enable);
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
    return gameState->fpsCounter->GetStable();
}

float Engine::GetStageTime() const
{
    if (IsStageFinished())
    {
        return 0.0;
    }
    return gameState->stageStartTime->GetElapsedMilliSec();
}

float Engine::GetPackageTime() const
{
    return gameState->packageStartTime->GetElapsedMilliSec();
}

void Engine::UpdateFpsCounter()
{
    gameState->fpsCounter->Update();
}

void Engine::ResetFpsCounter()
{
    gameState->fpsCounter = std::make_shared<FpsCounter>();
}

void Engine::StartSlow(int pseudoFps, bool byPlayer)
{
    pseudoFps = constrain(pseudoFps, 1, 60);
    if (byPlayer)
    {
        gameState->pseudoPlayerFps = pseudoFps;
    } else
    {
        gameState->pseudoEnemyFps = pseudoFps;
    }
}

void Engine::StopSlow(bool byPlayer)
{
    if (byPlayer)
    {
        gameState->pseudoPlayerFps = 60;
    } else
    {
        gameState->pseudoEnemyFps = 60;
    }
}

int64_t Engine::GetElapsedFrame() const
{
    return gameState->elapsedFrame;
}

std::wstring Engine::GetMainStgScriptPath() const
{
    return gameState->stageMainScriptInfo.path;
}

std::wstring Engine::GetMainStgScriptDirectory() const
{
    return GetParentPath(gameState->stageMainScriptInfo.path) + L"/";
}

std::wstring Engine::GetMainPackageScriptPath() const
{
    return gameState->packageMainScriptInfo.path;
}

std::shared_ptr<Texture> Engine::LoadTexture(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos)
{
    return gameState->textureCache->Load(path, reserve, srcPos);
}

void Engine::LoadTextureInThread(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) noexcept(true)
{
    gameState->textureCache->LoadInThread(path, reserve, srcPos);
}

void Engine::RemoveTextureReservedFlag(const std::wstring & path)
{
    gameState->textureCache->RemoveReservedFlag(path);
}

void Engine::ReleaseUnusedTextureCache()
{
    gameState->textureCache->ReleaseUnusedTexture();
}

void Engine::ReleaseUnusedFontCache()
{
    gameState->fontCache->ReleaseUnusedFont();
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

std::shared_ptr<RenderTarget> Engine::GetRenderTarget(const std::wstring & name) const
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
        renderToTexture(SNAP_SHOT_RENDER_TARGET_NAME, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
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
    return gameState->meshCache->Load(path, gameState->textureCache, srcPos);
}

void Engine::ReleaseUnusedMeshCache()
{
    gameState->meshCache->ReleaseUnusedMesh();
}

std::shared_ptr<SoundBuffer> Engine::LoadSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return gameState->soundDevice->LoadSound(path, false, srcPos);
}

void Engine::LoadOrphanSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->orphanSounds[GetCanonicalPath(path)] = LoadSound(path, srcPos);
}

void Engine::RemoveOrphanSound(const std::wstring & path)
{
    gameState->orphanSounds.erase(GetCanonicalPath(path));
}

void Engine::PlayBGM(const std::wstring & path, double loopStartSec, double loopEndSec)
{
    auto it = gameState->orphanSounds.find(GetCanonicalPath(path));
    if (it != gameState->orphanSounds.end())
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
    auto it = gameState->orphanSounds.find(GetCanonicalPath(path));
    if (it != gameState->orphanSounds.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->Play();
    }
}

void Engine::StopOrphanSound(const std::wstring & path)
{
    auto it = gameState->orphanSounds.find(GetCanonicalPath(path));
    if (it != gameState->orphanSounds.end())
    {
        it->second->Stop();
    }
}

void Engine::CacheSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->soundDevice->LoadSound(path, true, srcPos);
}

void Engine::RemoveSoundCache(const std::wstring & path)
{
    gameState->soundDevice->RemoveSoundCache(path);
}

void Engine::ClearSoundCache()
{
    gameState->soundDevice->ClearSoundCache();
}

void Engine::SetObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority)
{
    gameState->objLayerList->SetRenderPriority(obj, priority);
}

void Engine::SetShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader)
{
    gameState->objLayerList->SetLayerShader(beginPriority, endPriority, shader);
}

void Engine::ResetShader(int beginPriority, int endPriority)
{
    gameState->objLayerList->ResetLayerShader(beginPriority, endPriority);
}

int Engine::GetStgFrameRenderPriorityMin() const
{
    return gameState->objLayerList->GetStgFrameRenderPriorityMin();
}

void Engine::SetStgFrameRenderPriorityMin(int p)
{
    gameState->objLayerList->SetStgFrameRenderPriorityMin(p);
}

int Engine::GetStgFrameRenderPriorityMax() const
{
    return gameState->objLayerList->GetStgFrameRenderPriorityMax();
}

void Engine::SetStgFrameRenderPriorityMax(int p)
{
    gameState->objLayerList->SetStgFrameRenderPriorityMax(p);
}

int Engine::GetShotRenderPriority() const
{
    return gameState->objLayerList->GetShotRenderPriority();
}

void Engine::SetShotRenderPriority(int p)
{
    return gameState->objLayerList->SetShotRenderPriority(p);
}

int Engine::GetItemRenderPriority() const
{
    return gameState->objLayerList->GetItemRenderPriority();
}

void Engine::SetItemRenderPriority(int p)
{
    return gameState->objLayerList->SetItemRenderPriority(p);
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
    return gameState->objLayerList->GetCameraFocusPermitRenderPriority();
}

void Engine::SetInvalidRenderPriority(int min, int max)
{
    gameState->objLayerList->SetInvalidRenderPriority(min, max);
}

void Engine::ClearInvalidRenderPriority()
{
    gameState->objLayerList->ClearInvalidRenderPriority();
}

std::shared_ptr<Shader> Engine::GetShader(int p) const
{
    return gameState->objLayerList->GetLayerShader(p);
}

void Engine::SetFogEnable(bool enable)
{
    gameState->renderer->SetFogEnable(enable);
}

void Engine::SetFogParam(float fogStart, float fogEnd, int r, int g, int b)
{
    gameState->renderer->SetFogParam(fogStart, fogEnd, r, g, b);
}

void Engine::SetCameraFocusX(float x)
{
    gameState->camera3D->SetFocusX(x);
}

void Engine::SetCameraFocusY(float y)
{
    gameState->camera3D->SetFocusY(y);
}

void Engine::SetCameraFocusZ(float z)
{
    gameState->camera3D->SetFocusZ(z);
}

void Engine::SetCameraFocusXYZ(float x, float y, float z)
{
    gameState->camera3D->SetFocusXYZ(x, y, z);
}

void Engine::SetCameraRadius(float r)
{
    gameState->camera3D->SetRadius(r);
}

void Engine::SetCameraAzimuthAngle(float angle)
{
    gameState->camera3D->SetAzimuthAngle(angle);
}

void Engine::SetCameraElevationAngle(float angle)
{
    gameState->camera3D->SetElevationAngle(angle);
}

void Engine::SetCameraYaw(float yaw)
{
    gameState->camera3D->SetYaw(yaw);
}

void Engine::SetCameraPitch(float pitch)
{
    gameState->camera3D->SetPitch(pitch);
}

void Engine::SetCameraRoll(float roll)
{
    gameState->camera3D->SetRoll(roll);
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
    return gameState->camera3D->GetX();
}

float Engine::GetCameraY() const
{
    return gameState->camera3D->GetY();
}

float Engine::GetCameraZ() const
{
    return gameState->camera3D->GetZ();
}

float Engine::GetCameraFocusX() const
{
    return gameState->camera3D->GetFocusX();
}

float Engine::GetCameraFocusY() const
{
    return gameState->camera3D->GetFocusY();
}

float Engine::GetCameraFocusZ() const
{
    return gameState->camera3D->GetFocusZ();
}

float Engine::GetCameraRadius() const
{
    return gameState->camera3D->GetRadius();
}

float Engine::GetCameraAzimuthAngle() const
{
    return gameState->camera3D->GetAzimuthAngle();
}

float Engine::GetCameraElevationAngle() const
{
    return gameState->camera3D->GetElevationAngle();
}

float Engine::GetCameraYaw() const
{
    return gameState->camera3D->GetYaw();
}

float Engine::GetCameraPitch() const
{
    return gameState->camera3D->GetPitch();
}

float Engine::GetCameraRoll() const
{
    return gameState->camera3D->GetRoll();
}

void Engine::SetCameraPerspectiveClip(float nearClip, float farClip)
{
    return gameState->camera3D->SetPerspectiveClip(nearClip, farClip);
}

void Engine::Set2DCameraFocusX(float x)
{
    gameState->camera2D->SetFocusX(x);
}

void Engine::Set2DCameraFocusY(float y)
{
    gameState->camera2D->SetFocusY(y);
}

void Engine::Set2DCameraAngleZ(float z)
{
    gameState->camera2D->SetAngleZ(z);
}

void Engine::Set2DCameraRatio(float r)
{
    gameState->camera2D->SetRatio(r);
}

void Engine::Set2DCameraRatioX(float x)
{
    gameState->camera2D->SetRatioX(x);
}

void Engine::Set2DCameraRatioY(float y)
{
    gameState->camera2D->SetRatioY(y);
}

void Engine::Reset2DCamera()
{
    gameState->camera2D->Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
}

float Engine::Get2DCameraX() const
{
    return gameState->camera2D->GetX();
}

float Engine::Get2DCameraY() const
{
    return gameState->camera2D->GetY();
}

float Engine::Get2DCameraAngleZ() const
{
    return gameState->camera2D->GetAngleZ();
}

float Engine::Get2DCameraRatio() const
{
    return gameState->camera2D->GetRatio();
}

float Engine::Get2DCameraRatioX() const
{
    return gameState->camera2D->GetRatioX();
}

float Engine::Get2DCameraRatioY() const
{
    return gameState->camera2D->GetRatioY();
}

void Engine::SetCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    gameState->commonDataDB->SetCommonData(key, std::move(value));
}

const std::unique_ptr<DnhValue>& Engine::GetCommonData(const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return gameState->commonDataDB->GetCommonData(key, defaultValue);
}

void Engine::ClearCommonData()
{
    gameState->commonDataDB->ClearCommonData();
}

void Engine::DeleteCommonData(const std::wstring & key)
{
    gameState->commonDataDB->DeleteCommonData(key);
}

void Engine::SetAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    gameState->commonDataDB->SetAreaCommonData(areaName, key, std::move(value));
}

const std::unique_ptr<DnhValue>& Engine::GetAreaCommonData(const std::wstring & areaName, const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return gameState->commonDataDB->GetAreaCommonData(areaName, key, defaultValue);
}

void Engine::ClearAreaCommonData(const std::wstring & areaName)
{
    gameState->commonDataDB->ClearAreaCommonData(areaName);
}

void Engine::DeleteAreaCommonData(const std::wstring & areaName, const std::wstring & key)
{
    gameState->commonDataDB->DeleteAreaCommonData(areaName, key);
}

void Engine::CreateCommonDataArea(const std::wstring & areaName)
{
    gameState->commonDataDB->CreateCommonDataArea(areaName);
}

bool Engine::IsCommonDataAreaExists(const std::wstring & areaName) const
{
    return gameState->commonDataDB->IsCommonDataAreaExists(areaName);
}

void Engine::CopyCommonDataArea(const std::wstring & dest, const std::wstring & src)
{
    gameState->commonDataDB->CopyCommonDataArea(dest, src);
}

std::vector<std::wstring> Engine::GetCommonDataAreaKeyList() const
{
    return gameState->commonDataDB->GetCommonDataAreaKeyList();
}

std::vector<std::wstring> Engine::GetCommonDataValueKeyList(const std::wstring & areaName) const
{
    return gameState->commonDataDB->GetCommonDataValueKeyList(areaName);
}

bool Engine::SaveCommonDataAreaA1(const std::wstring & areaName) const
{
    try
    {
        gameState->commonDataDB->SaveCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
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
        gameState->commonDataDB->LoadCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
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
        gameState->commonDataDB->SaveCommonDataArea(areaName, path);
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
        gameState->commonDataDB->LoadCommonDataArea(areaName, path);
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
    gameState->playerShotDataTable->Load(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

void Engine::ReloadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->playerShotDataTable->Reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

void Engine::LoadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->enemyShotDataTable->Load(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

void Engine::ReloadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->enemyShotDataTable->Reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

void Engine::LoadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->itemDataTable->Load(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

void Engine::ReloadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->itemDataTable->Reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
}

std::shared_ptr<ShotData> Engine::GetPlayerShotData(int id) const
{
    return gameState->playerShotDataTable->Get(id);
}

std::shared_ptr<ShotData> Engine::GetEnemyShotData(int id) const
{
    return gameState->enemyShotDataTable->Get(id);
}

std::shared_ptr<ItemData> Engine::GetItemData(int id) const
{
    return gameState->itemDataTable->Get(id);
}

std::shared_ptr<ObjText> Engine::CreateObjText()
{
    auto obj = gameState->objTable->Create<ObjText>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSound> Engine::CreateObjSound()
{
    return gameState->objTable->Create<ObjSound>(gameState);
}

std::shared_ptr<ObjFileT> Engine::CreateObjFileT()
{
    return gameState->objTable->Create<ObjFileT>(gameState);
}

std::shared_ptr<ObjFileB> Engine::CreateObjFileB()
{
    return gameState->objTable->Create<ObjFileB>(gameState);
}

std::shared_ptr<ObjShader> Engine::CreateObjShader()
{
    auto obj = gameState->objTable->Create<ObjShader>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjShot> Engine::CreateObjShot(bool isPlayerShot)
{
    auto shot = gameState->objTable->Create<ObjShot>(isPlayerShot, gameState);
    gameState->objLayerList->SetRenderPriority(shot, gameState->objLayerList->GetShotRenderPriority());
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

// NOTE: オブジェクトが存在しなかったら空ポインタを返す
std::shared_ptr<ObjShot> Engine::CreateShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot)
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

std::shared_ptr<ObjShot> Engine::CreateShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot)
{
    if (auto obj = GetObject<ObjRender>(objId))
    {
        return CreateShotB1(obj->GetX(), obj->GetY(), speedX, speedY, shotDataId, delay, isPlayerShot);
    } else
    {
        return nullptr;
    }
}

std::shared_ptr<ObjShot> Engine::CreatePlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId)
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
    auto laser = gameState->objTable->Create<ObjLooseLaser>(isPlayerShot, gameState);
    gameState->objLayerList->SetRenderPriority(laser, gameState->objLayerList->GetShotRenderPriority());
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
    auto laser = gameState->objTable->Create<ObjStLaser>(isPlayerShot, gameState);
    gameState->objLayerList->SetRenderPriority(laser, gameState->objLayerList->GetShotRenderPriority());
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
    auto laser = gameState->objTable->Create<ObjCrLaser>(isPlayerShot, gameState);
    gameState->objLayerList->SetRenderPriority(laser, gameState->objLayerList->GetShotRenderPriority());
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

std::shared_ptr<ObjItem> Engine::CreateItemA1(int itemType, float x, float y, GameScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemA2(int itemType, float x, float y, float destX, float destY, GameScore score)
{
    auto item = CreateObjItem(itemType);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->SetScore(score);
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemU1(int itemDataId, float x, float y, GameScore score)
{
    auto item = CreateObjItem(ITEM_USER);
    item->SetMovePosition(x, y);
    item->SetMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->SetScore(score);
    item->SetItemData(GetItemData(itemDataId));
    return item;
}

std::shared_ptr<ObjItem> Engine::CreateItemU2(int itemDataId, float x, float y, float destX, float destY, GameScore score)
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
    auto enemy = gameState->objTable->Create<ObjEnemy>(false, gameState);
    gameState->objLayerList->SetRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
}

std::shared_ptr<ObjEnemyBossScene> Engine::CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto bossScene = gameState->enemyBossSceneObj.lock())
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
    auto bossScene = gameState->objTable->Create<ObjEnemyBossScene>(gameState);
    gameState->enemyBossSceneObj = bossScene;
    return bossScene;
}

std::shared_ptr<ObjSpell> Engine::CreateObjSpell()
{
    auto spell = gameState->objTable->Create<ObjSpell>(gameState);
    gameState->objLayerList->SetRenderPriority(spell, 50);
    return spell;
}

std::shared_ptr<ObjItem> Engine::CreateObjItem(int itemType)
{
    auto obj = gameState->objTable->Create<ObjItem>(itemType, gameState);
    gameState->objLayerList->SetRenderPriority(obj, gameState->objLayerList->GetItemRenderPriority());
    obj->SetIntersection();
    return obj;
}

std::shared_ptr<Script> Engine::GetScript(int scriptId) const
{
    return gameState->scriptManager->Get(scriptId);
}

std::shared_ptr<Script> Engine::LoadScript(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = gameState->scriptManager->Compile(path, type, version, srcPos);
    script->Load();
    return script;
}

std::shared_ptr<Script> Engine::LoadScriptInThread(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = gameState->scriptManager->CompileInThread(path, type, version, srcPos);
    return script;
}

void Engine::CloseStgScene()
{
    if (auto stageMain = gameState->stageMainScript.lock())
    {
        stageMain->Close();
    }
}

void Engine::NotifyEventAll(int eventType)
{
    gameState->scriptManager->NotifyEventAll(eventType);
}

void Engine::NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args)
{
    gameState->scriptManager->NotifyEventAll(eventType, args);
}

std::wstring Engine::GetPlayerID() const
{
    return gameState->stagePlayerScriptInfo.id;
}

std::wstring Engine::GetPlayerReplayName() const
{
    return gameState->stagePlayerScriptInfo.replayName;
}

std::shared_ptr<ObjPlayer> Engine::GetPlayerObject() const
{
    auto player = gameState->playerObj.lock();
    if (player && !player->IsDead()) return player;
    return nullptr;
}

std::shared_ptr<ObjEnemy> Engine::GetEnemyBossObject() const
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

std::shared_ptr<ObjEnemyBossScene> Engine::GetEnemyBossSceneObject() const
{
    auto bossScene = gameState->enemyBossSceneObj.lock();
    if (bossScene && !bossScene->IsDead()) return bossScene;
    return nullptr;
}

std::shared_ptr<ObjSpellManage> Engine::GetSpellManageObject() const
{
    auto spellManage = gameState->spellManageObj.lock();
    if (spellManage && !spellManage->IsDead()) return spellManage;
    return nullptr;
}

void Engine::DeleteObject(int id)
{
    gameState->objTable->Delete(id);
}

bool Engine::IsObjectDeleted(int id) const
{
    return gameState->objTable->IsDeleted(id);
}

std::shared_ptr<ObjPrim2D> Engine::CreateObjPrim2D()
{
    auto obj = gameState->objTable->Create<ObjPrim2D>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite2D> Engine::CreateObjSprite2D()
{
    auto obj = gameState->objTable->Create<ObjSprite2D>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSpriteList2D> Engine::CreateObjSpriteList2D()
{
    auto obj = gameState->objTable->Create<ObjSpriteList2D>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjPrim3D> Engine::CreateObjPrim3D()
{
    auto obj = gameState->objTable->Create<ObjPrim3D>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite3D> Engine::CreateObjSprite3D()
{
    auto obj = gameState->objTable->Create<ObjSprite3D>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjMesh> Engine::CreateObjMesh()
{
    auto obj = gameState->objTable->Create<ObjMesh>(gameState);
    gameState->objLayerList->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<Script> Engine::GetPlayerScript() const
{
    return gameState->stagePlayerScript.lock();
}

const std::unique_ptr<DnhValue>& Engine::GetScriptResult(int scriptId) const
{
    return gameState->scriptManager->GetScriptResult(scriptId);
}

void Engine::SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value)
{
    gameState->scriptManager->SetScriptResult(scriptId, std::move(value));
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
            auto info = ScanDnhScriptInfo(path, gameState->fileLoader);
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
    gameState->freePlayerScriptInfoList = GetScriptList(FREE_PLAYER_DIR, TYPE_SCRIPT_PLAYER, true);
}

int Engine::GetFreePlayerScriptCount() const
{
    return gameState->freePlayerScriptInfoList.size();
}

ScriptInfo Engine::GetFreePlayerScriptInfo(int idx) const
{
    return gameState->freePlayerScriptInfoList.at(idx);
}

ScriptInfo Engine::GetScriptInfo(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        return ScanDnhScriptInfo(path, gameState->fileLoader);
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

GameScore Engine::GetScore() const
{
    return gameState->globalPlayerParams->score;
}

void Engine::AddScore(GameScore score)
{
    gameState->globalPlayerParams->score += score;
}

int64_t Engine::GetGraze() const
{
    return gameState->globalPlayerParams->graze;
}

void Engine::AddGraze(int64_t graze)
{
    gameState->globalPlayerParams->graze += graze;
}

int64_t Engine::GetPoint() const
{
    return gameState->globalPlayerParams->point;
}

void Engine::AddPoint(int64_t point)
{
    gameState->globalPlayerParams->point += point;
}

void Engine::SetStgFrame(float left, float top, float right, float bottom)
{
    gameState->stgFrame->left = left;
    gameState->stgFrame->top = top;
    gameState->stgFrame->right = right;
    gameState->stgFrame->bottom = bottom;
}

float Engine::GetStgFrameLeft() const
{
    return gameState->stgFrame->left;
}

float Engine::GetStgFrameTop() const
{
    return gameState->stgFrame->top;
}

float Engine::GetStgFrameWidth() const
{
    return gameState->stgFrame->right - gameState->stgFrame->left;
}

float Engine::GetStgFrameHeight() const
{
    return gameState->stgFrame->bottom - gameState->stgFrame->top;
}

float Engine::GetStgFrameCenterWorldX() const
{
    return (gameState->stgFrame->right - gameState->stgFrame->left) / 2.0f;
}

float Engine::GetStgFrameCenterWorldY() const
{
    return (gameState->stgFrame->bottom - gameState->stgFrame->top) / 2.0f;
}

float Engine::GetStgFrameCenterScreenX() const
{
    return (gameState->stgFrame->right + gameState->stgFrame->left) / 2.0f;
}

float Engine::GetStgFrameCenterScreenY() const
{
    return (gameState->stgFrame->bottom + gameState->stgFrame->top) / 2.0f;
}

int Engine::GetAllShotCount() const
{
    return gameState->shotCounter->playerShotCount + gameState->shotCounter->enemyShotCount;
}

int Engine::GetEnemyShotCount() const
{
    return gameState->shotCounter->enemyShotCount;
}

int Engine::GetPlayerShotCount() const
{
    return gameState->shotCounter->playerShotCount;
}

void Engine::SetShotAutoDeleteClip(float left, float top, float right, float bottom)
{
    gameState->shotAutoDeleteClip->SetClip(left, top, right, bottom);
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
    auto shotScript = LoadScript(path, SCRIPT_TYPE_SHOT_CUSTOM, gameState->stageMainScriptInfo.version, srcPos);
    gameState->shotScript = shotScript;
    shotScript->Start();
    shotScript->RunInitialize();
}

void Engine::SetDeleteShotImmediateEventOnShotScriptEnable(bool enable)
{
    gameState->deleteShotImmediateEventOnShotScriptEnable = enable;
}

void Engine::SetDeleteShotFadeEventOnShotScriptEnable(bool enable)
{
    gameState->deleteShotFadeEventOnShotScriptEnable = enable;
}

void Engine::SetDeleteShotToItemEventOnShotScriptEnable(bool enable)
{
    gameState->deleteShotToItemEventOnShotScriptEnable = enable;
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
    auto isects = gameState->colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), COL_GRP_ENEMY_SHOT);
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

    auto isects = gameState->colDetector->GetIntersectionsCollideWithShape(Shape(x, y, r), -1);
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
    gameState->colDetector->Add(isect);
    gameState->tempEnemyShotIsects.push_back(isect);
}

void Engine::SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x1, y1, x2, y2, width);
    gameState->colDetector->Add(isect);
    gameState->tempEnemyShotIsects.push_back(isect);
}

void Engine::CollectAllItems()
{
    gameState->autoItemCollectionManager->CollectAllItems();
}

void Engine::CollectItemsByType(int itemType)
{
    gameState->autoItemCollectionManager->CollectItemsByType(itemType);
}

void Engine::CollectItemsInCircle(float x, float y, float r)
{
    gameState->autoItemCollectionManager->CollectItemsInCircle(x, y, r);
}

void Engine::CancelCollectItems()
{
    gameState->autoItemCollectionManager->CancelCollectItems();
}

void Engine::SetDefaultBonusItemEnable(bool enable)
{
    gameState->defaultBonusItemEnable = enable;
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
    auto itemScript = LoadScript(path, SCRIPT_TYPE_ITEM_CUSTOM, gameState->stageMainScriptInfo.version, srcPos);
    gameState->itemScript = itemScript;
    itemScript->Start();
    itemScript->RunInitialize();
}

bool Engine::IsPackageFinished() const
{
    if (gameState->packageMainScript.lock())
    {
        return false;
    }
    return true;
}

void Engine::ClosePackage()
{
    if (auto packageMain = gameState->packageMainScript.lock())
    {
        packageMain->Close();
    }
}

void Engine::InitializeStageScene()
{
    gameState->objTable->DeleteStgSceneObject();
    gameState->scriptManager->CloseStgSceneScript();
    SetStgFrame(32.0f, 16.0f, 416.0f, 464.0f);
    SetShotAutoDeleteClip(64.0f, 64.0f, 64.0f, 64.0f);
    Reset2DCamera();
    SetDefaultBonusItemEnable(true);
    gameState->stageSceneResult = 0;
    gameState->stageMainScript.reset();
    gameState->stagePlayerScript.reset();
    gameState->globalPlayerParams = std::make_shared<GlobalPlayerParams>();
    SetStgFrameRenderPriorityMin(DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN);
    SetStgFrameRenderPriorityMax(DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX);
    SetShotRenderPriority(DEFAULT_SHOT_RENDER_PRIORITY);
    SetItemRenderPriority(DEFAULT_ITEM_RENDER_PRIORITY);
    gameState->stagePaused = true;
    gameState->stageForceTerminated = false;
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
    gameState->packageStartTime = std::make_shared<TimePoint>();
    Logger::WriteLog(Log::Level::LV_INFO, "start package.");
    auto script = gameState->scriptManager->Compile(gameState->packageMainScriptInfo.path, SCRIPT_TYPE_PACKAGE, gameState->packageMainScriptInfo.version, nullptr);
    gameState->packageMainScript = script;
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
    gameState->camera3D->GenerateViewMatrix(&view, &billboard);
    if (isStgScene)
    {
        gameState->camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
    } else
    {
        gameState->camera3D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
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
        gameState->camera2D->GenerateViewMatrix(&view);
        gameState->camera2D->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        D3DXMATRIX viewProjViewport = view * proj * viewport;
        D3DXMatrixInverse(&viewProjViewport, NULL, &viewProjViewport);
        D3DXVec3TransformCoord(&pos, &pos, &viewProjViewport);
    }
    return Point2D(pos.x, pos.y);
}

void Engine::SetStageIndex(uint16_t idx)
{
    gameState->stageIdx = idx;
}

void Engine::SetStageMainScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        gameState->stageMainScriptInfo = ScanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        gameState->stageMainScriptInfo = ScriptInfo();
        gameState->stageMainScriptInfo.path = path;
    }
}

void Engine::SetStagePlayerScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        gameState->stagePlayerScriptInfo = ScanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        gameState->stageMainScriptInfo = ScriptInfo();
        gameState->stageMainScriptInfo.path = path;
    }
}

void Engine::SetStageMainScript(const ScriptInfo & script)
{
    gameState->stageMainScriptInfo = script;
}

void Engine::SetStagePlayerScript(const ScriptInfo & script)
{
    gameState->stagePlayerScriptInfo = script;
}

void Engine::SetStageReplayFile(const std::wstring & path)
{
    gameState->stageReplayFilePath = path;
}

bool Engine::IsStageFinished() const
{
    if (gameState->stageMainScript.lock())
    {
        return false;
    }
    return true;
}

int Engine::GetStageSceneResult() const
{
    return gameState->stageSceneResult;
}

bool Engine::IsStagePaused() const
{
    return gameState->stagePaused;
}

void Engine::PauseStageScene(bool doPause)
{
    if (doPause && !gameState->stagePaused)
    {
        NotifyEventAll(EV_PAUSE_ENTER);
    } else if (!doPause && gameState->stagePaused)
    {
        NotifyEventAll(EV_PAUSE_LEAVE);
    }
    gameState->stagePaused = doPause;
}

void Engine::TerminateStageScene()
{
    if (auto stageMain = gameState->stageMainScript.lock())
    {
        stageMain->Close();
        gameState->stageForceTerminated = true;
    }
}

void Engine::SetPackageMainScript(const std::wstring & path)
{
    try
    {
        gameState->packageMainScriptInfo = ScanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN);
        Logger::WriteLog(log);
        gameState->stageMainScriptInfo = ScriptInfo();
        gameState->stageMainScriptInfo.path = path;
    }
}

void Engine::SetPackageMainScript(const ScriptInfo & script)
{
    gameState->packageMainScriptInfo = script;
}

void Engine::StartStageScene(const std::shared_ptr<SourcePos>& srcPos)
{
    gameState->objTable->DeleteStgSceneObject();
    gameState->scriptManager->CloseStgSceneScript();
    gameState->inputDevice->ResetInputState();
    Reset2DCamera();
    ResetCamera();
    SetDefaultBonusItemEnable(true);
    gameState->playerShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER);
    gameState->enemyShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY);
    gameState->itemDataTable = std::make_shared<ItemDataTable>();
    gameState->stageMainScript.reset();
    gameState->stagePlayerScript.reset();
    ReloadItemData(DEFAULT_ITEM_DATA_PATH, nullptr);
    gameState->stageSceneResult = 0;
    gameState->stageForceTerminated = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
    gameState->stagePaused = false;
    gameState->renderer->SetFogEnable(false);
    gameState->pseudoPlayerFps = gameState->pseudoEnemyFps = 60;

    if (gameState->stageMainScriptInfo.systemPath.empty() || gameState->stageMainScriptInfo.systemPath == L"DEFAULT")
    {
        gameState->stageMainScriptInfo.systemPath = DEFAULT_SYSTEM_PATH;
    }

    // #System
    auto systemScript = gameState->scriptManager->Compile(gameState->stageMainScriptInfo.systemPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
    systemScript->Start();
    systemScript->RunInitialize();

    // Create Player
    auto player = gameState->objTable->Create<ObjPlayer>(gameState, gameState->globalPlayerParams);
    gameState->objLayerList->SetRenderPriority(player, DEFAULT_PLAYER_RENDER_PRIORITY);
    player->AddIntersectionToItem();
    gameState->playerObj = player;
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create player object.")
        .AddSourcePos(srcPos)));

    auto playerScript = gameState->scriptManager->Compile(gameState->stagePlayerScriptInfo.path, SCRIPT_TYPE_PLAYER, gameState->stagePlayerScriptInfo.version, srcPos);
    gameState->stagePlayerScript = playerScript;
    playerScript->Start();
    playerScript->RunInitialize();

    // Main
    auto stageMainScriptPath = gameState->stageMainScriptInfo.path;
    if (gameState->stageMainScriptInfo.type == SCRIPT_TYPE_SINGLE)
    {
        stageMainScriptPath = SYSTEM_SINGLE_STAGE_PATH;
    } else if (gameState->stageMainScriptInfo.type == SCRIPT_TYPE_PLURAL)
    {
        stageMainScriptPath = SYSTEM_PLURAL_STAGE_PATH;
    }
    auto stageMainScript = gameState->scriptManager->Compile(stageMainScriptPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
    gameState->stageMainScript = stageMainScript;
    stageMainScript->Start();
    stageMainScript->RunInitialize();
    gameState->stageStartTime = std::make_shared<TimePoint>();

    // #Background
    if (!gameState->stageMainScriptInfo.backgroundPath.empty() && gameState->stageMainScriptInfo.backgroundPath != L"DEFAULT")
    {
        auto backgroundScript = gameState->scriptManager->Compile(gameState->stageMainScriptInfo.backgroundPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
        backgroundScript->Start();
        backgroundScript->RunInitialize();
    }

    // #BGM
    if (!gameState->stageMainScriptInfo.bgmPath.empty() && gameState->stageMainScriptInfo.bgmPath != L"DEFAULT")
    {
        try
        {
            LoadOrphanSound(gameState->stageMainScriptInfo.bgmPath, nullptr); // TODO: #BGMヘッダのSourcePosを与える
            auto& bgm = gameState->orphanSounds[GetCanonicalPath(gameState->packageMainScriptInfo.bgmPath)];
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

void Engine::renderToTexture(const std::wstring& name, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag)
{
    if (!gameState) return;

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
    gameState->renderer->InitRenderState();

    if (doClear)
    {
        graphicDevice->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    }

    begin = std::max(begin, 0);
    end = std::min(end, MAX_RENDER_PRIORITY);

    auto obj = gameState->objTable->Get<ObjRender>(objId);
    if (obj && checkVisibleFlag && !obj->IsVisible()) return;

    gameState->camera3D->GenerateViewMatrix(&viewMatrix3D, &billboardMatrix);

    // [0, stgFrameMin]
    {
        gameState->renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        gameState->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        gameState->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        gameState->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = 0; p < GetStgFrameRenderPriorityMin(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(gameState->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && gameState->objLayerList->IsInvalidRenderPriority(p)))
                {
                    gameState->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, gameState->renderer);
                }
            }
        }
    }
    // [stgFrameMin, stgFrameMax]
    {
        if (!IsStagePaused())
        {
            RECT scissorRect = { (LONG)gameState->stgFrame->left, (LONG)gameState->stgFrame->top, (LONG)gameState->stgFrame->right, (LONG)gameState->stgFrame->bottom };
            if (renderToBackBuffer)
            {
                scissorRect.left = gameState->stgFrame->left * graphicDevice->GetBackBufferWidth() / gameState->screenWidth;
                scissorRect.top = gameState->stgFrame->top * graphicDevice->GetBackBufferHeight() / gameState->screenHeight;
                scissorRect.right = gameState->stgFrame->right * graphicDevice->GetBackBufferWidth() / gameState->screenWidth;
                scissorRect.bottom = gameState->stgFrame->bottom * graphicDevice->GetBackBufferHeight() / gameState->screenHeight;
            }
            gameState->renderer->EnableScissorTest(scissorRect);
        }

        // set 2D matrix
        if (!IsStageFinished())
        {
            gameState->camera2D->GenerateViewMatrix(&viewMatrix2D);
            gameState->camera2D->GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
            gameState->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
        }

        // set 3D matrix
        gameState->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        gameState->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);

        for (int p = GetStgFrameRenderPriorityMin(); p <= GetStgFrameRenderPriorityMax(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(gameState->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && gameState->objLayerList->IsInvalidRenderPriority(p)))
                {
                    gameState->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, gameState->renderer);
                }
            }
            if (p == gameState->objLayerList->GetCameraFocusPermitRenderPriority())
            {
                // cameraFocusPermitRenderPriorityより大きい優先度では別のビュー変換行列を使う
                if (!IsStageFinished())
                {
                    Camera2D focusForbidCamera;
                    focusForbidCamera.Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
                    focusForbidCamera.GenerateViewMatrix(&viewMatrix2D);
                    gameState->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
                }
            }
        }
    }
    {
        // (stgFrameMax, MAX_RENDER_PRIORITY]
        gameState->renderer->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        gameState->renderer->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        gameState->camera3D->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        gameState->renderer->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = GetStgFrameRenderPriorityMax() + 1; p <= MAX_RENDER_PRIORITY; p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(gameState->renderer);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && gameState->objLayerList->IsInvalidRenderPriority(p)))
                {
                    gameState->objLayerList->RenderLayer(p, IsStagePaused(), checkVisibleFlag, gameState->renderer);
                }
            }
        }
    }
    SetBackBufferRenderTarget();
}

std::shared_ptr<Obj> Engine::GetObj(int id) const
{
    return gameState->objTable->Get<Obj>(id);
}

const std::map<int, std::shared_ptr<Obj>>& Engine::GetObjAll() const
{
    return gameState->objTable->GetAll();
}
}