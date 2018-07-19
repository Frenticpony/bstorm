#include <bstorm/package.hpp>

#include <bstorm/file_util.hpp>
#include <bstorm/math_util.hpp>
#include <bstorm/string_util.hpp>
#include <bstorm/path_const.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/parser.hpp>
#include <bstorm/graphic_device.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/time_point.hpp>
#include <bstorm/fps_counter.hpp>
#include <bstorm/input_device.hpp>
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
#include <bstorm/serialized_script.hpp>
#include <bstorm/script.hpp>
#include <bstorm/replay_data.hpp>
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
#undef GetObject

namespace bstorm
{
Package::Package(HWND hWnd,
                 int screenWidth,
                 int screenHeight,
                 const std::wstring packageMainScriptPath,
                 const std::shared_ptr<conf::KeyConfig>& keyConfig,
                 const std::shared_ptr<GraphicDevice>& graphicDevice,
                 const std::shared_ptr<InputDevice>& inputDevice,
                 const std::shared_ptr<FpsCounter>& fpsCounter,
                 const std::shared_ptr<LostableGraphicResourceManager>& lostableGraphicResourceManager,
                 const std::shared_ptr<EngineDevelopOptions>& engineDevelopOptions) :
    hWnd_(hWnd),
    screenWidth_(screenWidth),
    screenHeight_(screenHeight),
    graphicDevice_(graphicDevice),
    inputDevice_(inputDevice),
    fpsCounter_(fpsCounter),
    lostableGraphicResourceManager_(lostableGraphicResourceManager),
    engineDevelopOptions_(engineDevelopOptions),
    fileLoader_(std::make_shared<FileLoader>()),
    soundDevice(std::make_shared<SoundDevice>(hWnd)),
    renderer_(std::make_shared<Renderer>(graphicDevice_->GetDevice())),
    objTable_(std::make_shared<ObjectTable>()),
    objLayerList_(std::make_shared<ObjectLayerList>()),
    colDetector_(std::make_shared<CollisionDetector>(screenWidth, screenHeight, std::make_shared<CollisionMatrix>(DEFAULT_COLLISION_MATRIX_DIMENSION, DEFAULT_COLLISION_MATRIX))),
    textureStore_(std::make_shared<TextureStore>(graphicDevice_)),
    meshStore_(std::make_shared<MeshStore>(textureStore_, fileLoader_)),
    camera2D_(std::make_shared<Camera2D>()),
    camera3D_(std::make_shared<Camera3D>()),
    commonDataDB_(std::make_shared<CommonDataDB>()),
    serializedScriptStore_(std::make_shared<SerializedScriptStore>(fileLoader_)),
    scriptManager_(std::make_shared<ScriptManager>(fileLoader_, serializedScriptStore_)),
    playerShotDataTable_(std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureStore_, fileLoader_)),
    enemyShotDataTable_(std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureStore_, fileLoader_)),
    itemDataTable_(std::make_shared<ItemDataTable>(textureStore_, fileLoader_)),
    shotCounter_(std::make_shared<ShotCounter>()),
    randGenerator_(std::make_shared<RandGenerator>((uint32_t)this)), // FUTURE : from rand
    autoItemCollectionManager_(std::make_shared<AutoItemCollectionManager>()),
    elapsedFrame_(0),
    stageElapesdFrame_(0),
    stageStartTime_(std::make_shared<TimePoint>()),
    pseudoPlayerFps_(60),
    pseudoEnemyFps_(60),
    packageMainScriptInfo_(ScanDnhScriptInfo(packageMainScriptPath, fileLoader_)),
    stageMainScriptInfo_(),
    stagePlayerScriptInfo_(),
    stageIdx_(ID_INVALID),
    stageSceneResult_(0),
    isStagePaused_(true),
    isStageForceTerminated_(false),
    isReplay_(false),
    replayData_(std::make_shared<ReplayData>()),
    deleteShotImmediateEventOnShotScriptEnable_(false),
    deleteShotFadeEventOnShotScriptEnable_(false),
    deleteShotToItemEventOnShotScriptEnable_(false),
    defaultBonusItemEnable_(true),
    stgFrame_(32.0f, 16.0f, 416.0f, 464.0f),
    shotAutoDeleteClip_(64.0f, 64.0f, 64.0f, 64.0f),
    fontStore_(std::make_shared<FontStore>(hWnd, graphicDevice_)),
    packageStartTime_(std::make_shared<TimePoint>())
{
    Reset2DCamera();
    ResetCamera();
    CreateRenderTarget(GetTransitionRenderTargetName(), GetScreenWidth(), GetScreenHeight(), nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(0), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(1), 1024, 512, nullptr);
    CreateRenderTarget(GetReservedRenderTargetName(2), 1024, 512, nullptr);
    renderer_->SetForbidCameraViewProjMatrix2D(GetScreenWidth(), GetScreenHeight());
    renderer_->SetFogEnable(false);

    for (const auto& keyMap : keyConfig->keyMaps)
    {
        AddVirtualKey(keyMap.vkey, keyMap.key, keyMap.pad);
    }
}

Package::~Package()
{
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("close package.")
        .SetParam(Log::Param(Log::Param::Tag::SCRIPT, GetMainScriptPath()))));
}

void Package::TickFrame()
{
    if (IsClosed())
    {
        return;
    }

    if (auto stageMain = stageMainScript_.lock())
    {
        if (stageMain->IsClosed())
        {
            if (isStageForceTerminated_)
            {
                stageSceneResult_ = STAGE_RESULT_BREAK_OFF;
                SetPlayerLife(0);
                SetPlayerSpell(3);
                SetPlayerPower(1);
                SetPlayerScore(0);
                SetPlayerGraze(0);
                SetPlayerPoint(0);
            } else if (auto player = GetPlayerObject())
            {
                stageSceneResult_ = player->GetState() != STATE_END ? STAGE_RESULT_CLEARED : STAGE_RESULT_PLAYER_DOWN;
            } else
            {
                stageSceneResult_ = STAGE_RESULT_PLAYER_DOWN;
            }
            RenderToTextureA1(GetTransitionRenderTargetName(), 0, MAX_RENDER_PRIORITY, true);
            objTable_->DeleteStgSceneObject();
            scriptManager_->CloseStgSceneScript();
            stageMainScript_.reset();
            stagePlayerScript_.reset();
            isStageForceTerminated_ = false;
            isStagePaused_ = true;
            pseudoPlayerFps_ = pseudoEnemyFps_ = 60;
            playerShotDataTable_ = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureStore_, fileLoader_);
            enemyShotDataTable_ = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureStore_, fileLoader_);
            itemDataTable_ = std::make_shared<ItemDataTable>(textureStore_, fileLoader_);
        }
    }

    scriptManager_->RunFinalizeOnClosedScript();

    {
        // 入力更新
        inputDevice_->UpdateInputState();

        // VirtualKey状態更新
        virtualKeyStates_.clear();
        // キーボードの入力がパッドより優先
        for (auto& entry : virtualKeyAssign_)
        {
            auto vk = entry.first;
            auto k = entry.second.first;
            auto pad = entry.second.second;
            KeyState keyState = inputDevice_->GetKeyState(k);
            if (keyState == KEY_FREE)
            {
                keyState = inputDevice_->GetPadButtonState(pad);
            }
            virtualKeyStates_[vk] = keyState;
        }
    }

    if (IsReplay())
    {
        // リプレイVirtualKey状態更新
        replayVirtualKeyStates_.clear();
    }

    scriptManager_->RunMainLoopAllNonStgScript();

    if (IsStagePaused())
    {
        objTable_->UpdateAll(true);
    } else
    {
        if (stageElapesdFrame_ % (60 / std::min(pseudoEnemyFps_, pseudoPlayerFps_)) == 0)
        {
            colDetector_->TestAllCollision();

            // SetShotIntersection{Circle, Line}で設定した判定削除
            tempEnemyShotIsects_.clear();

            scriptManager_->RunMainLoopAllStgScript();

            objTable_->UpdateAll(false);

            autoItemCollectionManager_->Reset();
        }
        stageElapesdFrame_++;
    }

    // 使われなくなったリソース開放
    RemoveUnusedTexture();
    RemoveUnusedMesh();
    RemoveUnusedFont();
    if (elapsedFrame_ % 900 == 0)
    {
        lostableGraphicResourceManager_->RemoveUnusedResource();
    }

    elapsedFrame_++;
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
    return screenWidth_;
}

int Package::GetScreenHeight() const
{
    return screenHeight_;
}

KeyState Package::GetKeyState(Key k)
{
    return inputDevice_->GetKeyState(k);
}

KeyState Package::GetVirtualKeyState(VirtualKey vk, bool isStgScene) const
{
    // AddVirtualKeyされていない場合はKEY_FREEを返す
    if (IsAddedVirtualKey(vk))
    {
        const auto& keyStates = (isStgScene && IsReplay()) ? replayVirtualKeyStates_ : virtualKeyStates_;
        auto it = keyStates.find(vk);
        if (it != keyStates.end())
        {
            return it->second;
        }
    }
    return KEY_FREE;
}

void Package::SetVirtualKeyState(VirtualKey vk, KeyState state)
{
    // AddVirtualKeyされていない場合は何もしない
    if (virtualKeyAssign_.count(vk))
    {
        virtualKeyStates_[vk] = state;
        replayVirtualKeyStates_[vk] = state;
    }
}

void Package::AddVirtualKey(VirtualKey vk, Key k, PadButton btn)
{
    virtualKeyAssign_[vk] = std::make_pair(k, btn);
}

bool Package::IsAddedVirtualKey(VirtualKey vk) const
{
    return virtualKeyAssign_.count(vk) != 0;
}

KeyState Package::GetMouseState(MouseButton btn)
{
    return inputDevice_->GetMouseState(btn);
}

int Package::GetMouseX()
{
    return inputDevice_->GetMouseX(GetScreenWidth(), GetScreenHeight());
}

int Package::GetMouseY()
{
    return inputDevice_->GetMouseY(GetScreenWidth(), GetScreenHeight());
}

int Package::GetMouseMoveZ()
{
    return inputDevice_->GetMouseMoveZ();
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
    return fpsCounter_->GetStable();
}

float Package::GetStageTime() const
{
    if (IsStageFinished())
    {
        return 0.0;
    }
    return stageStartTime_->GetElapsedMilliSec();
}

float Package::GetPackageTime() const
{
    return packageStartTime_->GetElapsedMilliSec();
}

void Package::StartSlow(int pseudoFps, bool byPlayer)
{
    pseudoFps = constrain(pseudoFps, 1, 60);
    if (byPlayer)
    {
        pseudoPlayerFps_ = pseudoFps;
    } else
    {
        pseudoEnemyFps_ = pseudoFps;
    }
}

void Package::StopSlow(bool byPlayer)
{
    if (byPlayer)
    {
        pseudoPlayerFps_ = 60;
    } else
    {
        pseudoEnemyFps_ = 60;
    }
}

int Package::GetElapsedFrame() const
{
    return elapsedFrame_;
}

std::wstring Package::GetMainStgScriptPath() const
{
    return stageMainScriptInfo_.path;
}

std::wstring Package::GetMainStgScriptDirectory() const
{
    return GetParentPath(stageMainScriptInfo_.path) + L"/";
}

std::wstring Package::GetMainPackageScriptPath() const
{
    return packageMainScriptInfo_.path;
}

std::wstring Package::GetMainScriptPath() const
{
    return GetMainPackageScriptPath() == DEFAULT_PACKAGE_PATH ? GetMainStgScriptPath() : GetMainPackageScriptPath();
}

const std::shared_ptr<Texture>& Package::LoadTexture(const std::wstring & path)
{
    return textureStore_->Load(path);
}

void Package::LoadTextureInThread(const std::wstring & path) noexcept(true)
{
    textureStore_->LoadInThread(path);
}

void Package::SetTextureReserveFlag(const std::wstring & path, bool reserve)
{
    textureStore_->SetReserveFlag(path, reserve);
}

void Package::RemoveUnusedTexture()
{
    textureStore_->RemoveUnusedTexture();
}

const std::shared_ptr<Font>& Package::CreateFont(const FontParams& param)
{
    return fontStore_->Create(param);
}

void Package::RemoveUnusedFont()
{
    fontStore_->RemoveUnusedFont();
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
    auto renderTarget = std::make_shared<RenderTarget>(name, width, height, graphicDevice_->GetDevice());
    renderTarget->SetViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    renderTargets_[name] = renderTarget;
    lostableGraphicResourceManager_->AddResource(renderTarget);
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create render target.")
        .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
        .AddSourcePos(srcPos)));
    return renderTarget;
}

void Package::RemoveRenderTarget(const std::wstring & name, const std::shared_ptr<SourcePos>& srcPos)
{
    auto it = renderTargets_.find(name);
    if (it != renderTargets_.end())
    {
        renderTargets_.erase(it);
        Logger::WriteLog(std::move(
            Log(Log::Level::LV_INFO)
            .SetMessage("remove render target.")
            .SetParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
            .AddSourcePos(srcPos)));
    }
}

NullableSharedPtr<RenderTarget> Package::GetRenderTarget(const std::wstring & name) const
{
    auto it = renderTargets_.find(name);
    if (it != renderTargets_.end())
    {
        return it->second;
    }
    return nullptr;
}

constexpr wchar_t* TRANSITION_RENDER_TARGET_NAME = L"__RENDERTARGET_TRANSITION__";
constexpr wchar_t* RESERVED_RENDER_TARGET_PREFIX = L"__RESERVED_RENDER_TARGET__";
constexpr wchar_t* SNAP_SHOT_RENDER_TARGET_NAME = L"__SNAP_SHOT__";

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
    auto shader = std::make_shared<Shader>(path, precompiled, graphicDevice_->GetDevice());
    lostableGraphicResourceManager_->AddResource(shader);
    return shader;
}

bool Package::IsPixelShaderSupported(int major, int minor)
{
    D3DCAPS9 caps;
    graphicDevice_->GetDevice()->GetDeviceCaps(&caps);
    return caps.PixelShaderVersion >= D3DPS_VERSION(major, minor);
}

const std::shared_ptr<Mesh>& Package::LoadMesh(const std::wstring & path)
{
    return meshStore_->Load(path);
}

void Package::RemoveUnusedMesh()
{
    meshStore_->RemoveUnusedMesh();
}

std::shared_ptr<SoundBuffer> Package::LoadSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    return soundDevice->LoadSound(path, false, srcPos);
}

void Package::LoadOrphanSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    orphanSounds_[GetCanonicalPath(path)] = LoadSound(path, srcPos);
}

void Package::RemoveOrphanSound(const std::wstring & path)
{
    orphanSounds_.erase(GetCanonicalPath(path));
}

void Package::PlayBGM(const std::wstring & path, double loopStartSec, double loopEndSec)
{
    auto it = orphanSounds_.find(GetCanonicalPath(path));
    if (it != orphanSounds_.end())
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
    auto it = orphanSounds_.find(GetCanonicalPath(path));
    if (it != orphanSounds_.end())
    {
        auto& sound = it->second;
        if (sound->IsPlaying()) sound->Seek(0);
        sound->Play();
    }
}

void Package::StopOrphanSound(const std::wstring & path)
{
    auto it = orphanSounds_.find(GetCanonicalPath(path));
    if (it != orphanSounds_.end())
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
    objLayerList_->SetRenderPriority(obj, priority);
}

void Package::SetLayerShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader)
{
    objLayerList_->SetLayerShader(beginPriority, endPriority, shader);
}

void Package::ResetShader(int beginPriority, int endPriority)
{
    objLayerList_->ResetLayerShader(beginPriority, endPriority);
}

int Package::GetStgFrameRenderPriorityMin() const
{
    return objLayerList_->GetStgFrameRenderPriorityMin();
}

void Package::SetStgFrameRenderPriorityMin(int p)
{
    objLayerList_->SetStgFrameRenderPriorityMin(p);
}

int Package::GetStgFrameRenderPriorityMax() const
{
    return objLayerList_->GetStgFrameRenderPriorityMax();
}

void Package::SetStgFrameRenderPriorityMax(int p)
{
    objLayerList_->SetStgFrameRenderPriorityMax(p);
}

int Package::GetShotRenderPriority() const
{
    return objLayerList_->GetShotRenderPriority();
}

void Package::SetShotRenderPriority(int p)
{
    return objLayerList_->SetShotRenderPriority(p);
}

int Package::GetItemRenderPriority() const
{
    return objLayerList_->GetItemRenderPriority();
}

void Package::SetItemRenderPriority(int p)
{
    return objLayerList_->SetItemRenderPriority(p);
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
    return objLayerList_->GetCameraFocusPermitRenderPriority();
}

void Package::SetInvalidRenderPriority(int min, int max)
{
    objLayerList_->SetInvalidRenderPriority(min, max);
}

void Package::ClearInvalidRenderPriority()
{
    objLayerList_->ClearInvalidRenderPriority();
}

NullableSharedPtr<Shader> Package::GetLayerShader(int p) const
{
    return objLayerList_->GetLayerShader(p);
}

void Package::SetFogEnable(bool enable)
{
    renderer_->SetFogEnable(enable);
}

void Package::SetFogParam(float fogStart, float fogEnd, int r, int g, int b)
{
    renderer_->SetFogParam(fogStart, fogEnd, r, g, b);
}

void Package::SetCameraFocusX(float x)
{
    camera3D_->SetFocusX(x);
}

void Package::SetCameraFocusY(float y)
{
    camera3D_->SetFocusY(y);
}

void Package::SetCameraFocusZ(float z)
{
    camera3D_->SetFocusZ(z);
}

void Package::SetCameraFocusXYZ(float x, float y, float z)
{
    camera3D_->SetFocusXYZ(x, y, z);
}

void Package::SetCameraRadius(float r)
{
    camera3D_->SetRadius(r);
}

void Package::SetCameraAzimuthAngle(float angle)
{
    camera3D_->SetAzimuthAngle(angle);
}

void Package::SetCameraElevationAngle(float angle)
{
    camera3D_->SetElevationAngle(angle);
}

void Package::SetCameraYaw(float yaw)
{
    camera3D_->SetYaw(yaw);
}

void Package::SetCameraPitch(float pitch)
{
    camera3D_->SetPitch(pitch);
}

void Package::SetCameraRoll(float roll)
{
    camera3D_->SetRoll(roll);
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
    return camera3D_->GetX();
}

float Package::GetCameraY() const
{
    return camera3D_->GetY();
}

float Package::GetCameraZ() const
{
    return camera3D_->GetZ();
}

float Package::GetCameraFocusX() const
{
    return camera3D_->GetFocusX();
}

float Package::GetCameraFocusY() const
{
    return camera3D_->GetFocusY();
}

float Package::GetCameraFocusZ() const
{
    return camera3D_->GetFocusZ();
}

float Package::GetCameraRadius() const
{
    return camera3D_->GetRadius();
}

float Package::GetCameraAzimuthAngle() const
{
    return camera3D_->GetAzimuthAngle();
}

float Package::GetCameraElevationAngle() const
{
    return camera3D_->GetElevationAngle();
}

float Package::GetCameraYaw() const
{
    return camera3D_->GetYaw();
}

float Package::GetCameraPitch() const
{
    return camera3D_->GetPitch();
}

float Package::GetCameraRoll() const
{
    return camera3D_->GetRoll();
}

void Package::SetCameraPerspectiveClip(float nearClip, float farClip)
{
    return camera3D_->SetPerspectiveClip(nearClip, farClip);
}

void Package::Set2DCameraFocusX(float x)
{
    camera2D_->SetFocusX(x);
}

void Package::Set2DCameraFocusY(float y)
{
    camera2D_->SetFocusY(y);
}

void Package::Set2DCameraAngleZ(float z)
{
    camera2D_->SetAngleZ(z);
}

void Package::Set2DCameraRatio(float r)
{
    camera2D_->SetRatio(r);
}

void Package::Set2DCameraRatioX(float x)
{
    camera2D_->SetRatioX(x);
}

void Package::Set2DCameraRatioY(float y)
{
    camera2D_->SetRatioY(y);
}

void Package::Reset2DCamera()
{
    camera2D_->Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
}

float Package::Get2DCameraX() const
{
    return camera2D_->GetX();
}

float Package::Get2DCameraY() const
{
    return camera2D_->GetY();
}

float Package::Get2DCameraAngleZ() const
{
    return camera2D_->GetAngleZ();
}

float Package::Get2DCameraRatio() const
{
    return camera2D_->GetRatio();
}

float Package::Get2DCameraRatioX() const
{
    return camera2D_->GetRatioX();
}

float Package::Get2DCameraRatioY() const
{
    return camera2D_->GetRatioY();
}

void Package::SetCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    commonDataDB_->SetCommonData(key, std::move(value));
}

const std::unique_ptr<DnhValue>& Package::GetCommonData(const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return commonDataDB_->GetCommonData(key, defaultValue);
}

void Package::ClearCommonData()
{
    commonDataDB_->ClearCommonData();
}

void Package::DeleteCommonData(const std::wstring & key)
{
    commonDataDB_->DeleteCommonData(key);
}

void Package::SetAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value)
{
    commonDataDB_->SetAreaCommonData(areaName, key, std::move(value));
}

const std::unique_ptr<DnhValue>& Package::GetAreaCommonData(const std::wstring & areaName, const std::wstring & key, const std::unique_ptr<DnhValue>& defaultValue) const
{
    return commonDataDB_->GetAreaCommonData(areaName, key, defaultValue);
}

void Package::ClearAreaCommonData(const std::wstring & areaName)
{
    commonDataDB_->ClearAreaCommonData(areaName);
}

void Package::DeleteAreaCommonData(const std::wstring & areaName, const std::wstring & key)
{
    commonDataDB_->DeleteAreaCommonData(areaName, key);
}

void Package::CreateCommonDataArea(const std::wstring & areaName)
{
    commonDataDB_->CreateCommonDataArea(areaName);
}

bool Package::IsCommonDataAreaExists(const std::wstring & areaName) const
{
    return commonDataDB_->IsCommonDataAreaExists(areaName);
}

void Package::CopyCommonDataArea(const std::wstring & dest, const std::wstring & src)
{
    commonDataDB_->CopyCommonDataArea(dest, src);
}

std::vector<std::wstring> Package::GetCommonDataAreaKeyList() const
{
    return commonDataDB_->GetCommonDataAreaKeyList();
}

std::vector<std::wstring> Package::GetCommonDataValueKeyList(const std::wstring & areaName) const
{
    return commonDataDB_->GetCommonDataValueKeyList(areaName);
}

bool Package::SaveCommonDataAreaA1(const std::wstring & areaName) const
{
    try
    {
        commonDataDB_->SaveCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
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
        commonDataDB_->LoadCommonDataArea(areaName, GetDefaultCommonDataSavePath(areaName));
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
        commonDataDB_->SaveCommonDataArea(areaName, path);
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
        commonDataDB_->LoadCommonDataArea(areaName, path);
    } catch (Log& log)
    {
        Logger::WriteLog(std::move(log.SetLevel(Log::Level::LV_WARN)));
        return false;
    }
    return true;
}

std::wstring Package::GetDefaultCommonDataSavePath(const std::wstring & areaName) const
{
    std::wstring basePath = GetMainScriptPath();
    if (basePath.empty())
    {
        basePath = GetMainPackageScriptPath();
    }
    return GetParentPath(basePath) + L"/data/" + GetStem(basePath) + L"_common_" + areaName + L".dat";
}

void Package::LoadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    playerShotDataTable_->Load(path, srcPos);
}

void Package::ReloadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    playerShotDataTable_->Reload(path, srcPos);
}

void Package::LoadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    enemyShotDataTable_->Load(path, srcPos);
}

void Package::ReloadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    enemyShotDataTable_->Reload(path, srcPos);
}

void Package::LoadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    itemDataTable_->Load(path, srcPos);
}

void Package::ReloadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    itemDataTable_->Reload(path, srcPos);
}

NullableSharedPtr<ShotData> Package::GetPlayerShotData(int id) const
{
    return playerShotDataTable_->Get(id);
}

NullableSharedPtr<ShotData> Package::GetEnemyShotData(int id) const
{
    return enemyShotDataTable_->Get(id);
}

NullableSharedPtr<ItemData> Package::GetItemData(int id) const
{
    return itemDataTable_->Get(id);
}

std::shared_ptr<ObjText> Package::CreateObjText()
{
    auto obj = objTable_->Create<ObjText>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSound> Package::CreateObjSound()
{
    return objTable_->Create<ObjSound>(shared_from_this());
}

std::shared_ptr<ObjFileT> Package::CreateObjFileT()
{
    return objTable_->Create<ObjFileT>(shared_from_this());
}

std::shared_ptr<ObjFileB> Package::CreateObjFileB()
{
    return objTable_->Create<ObjFileB>(shared_from_this());
}

std::shared_ptr<ObjShader> Package::CreateObjShader()
{
    auto obj = objTable_->Create<ObjShader>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjShot> Package::CreateObjShot(bool isPlayerShot)
{
    auto shot = objTable_->Create<ObjShot>(isPlayerShot, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(shot, objLayerList_->GetShotRenderPriority());
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
    auto laser = objTable_->Create<ObjLooseLaser>(isPlayerShot, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(laser, objLayerList_->GetShotRenderPriority());
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
    auto laser = objTable_->Create<ObjStLaser>(isPlayerShot, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(laser, objLayerList_->GetShotRenderPriority());
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
    auto laser = objTable_->Create<ObjCrLaser>(isPlayerShot, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(laser, objLayerList_->GetShotRenderPriority());
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

std::shared_ptr<ObjItem> Package::CreateObjItem(int itemType)
{
    auto obj = objTable_->Create<ObjItem>(itemType, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(obj, objLayerList_->GetItemRenderPriority());
    obj->SetIntersection();
    return obj;
}

void Package::GenerateBonusItem(float x, float y)
{
    if (IsDefaultBonusItemEnabled())
    {
        auto bonusItem = objTable_->Create<ObjItem>(ITEM_DEFAULT_BONUS, colDetector_, shared_from_this());
        objLayerList_->SetRenderPriority(bonusItem, objLayerList_->GetItemRenderPriority());
        bonusItem->SetMovePosition(x, y);
        bonusItem->SetIntersection();
        bonusItem->SetScore(300);
        bonusItem->SetMoveMode(std::make_shared<MoveModeItemToPlayer>(8.0f, playerObj_.lock()));
    }
}

void Package::GenerateItemScoreText(float x, float y, PlayerScore score)
{
    auto& texture = textureStore_->Load(SYSTEM_STG_DIGIT_IMG_PATH);
    auto scoreText = objTable_->Create<ObjItemScoreText>(score, texture, shared_from_this());
    objLayerList_->SetRenderPriority(scoreText, objLayerList_->GetItemRenderPriority());
    scoreText->SetMovePosition(x, y);
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
    auto enemy = objTable_->Create<ObjEnemy>(false, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
}

std::shared_ptr<ObjEnemy> Package::CreateObjEnemyBoss()
{
    auto enemy = objTable_->Create<ObjEnemy>(true, colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
}

std::shared_ptr<ObjEnemyBossScene> Package::CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos)
{
    if (auto bossScene = enemyBossSceneObj_.lock())
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
    auto bossScene = objTable_->Create<ObjEnemyBossScene>(shared_from_this());
    enemyBossSceneObj_ = bossScene;
    return bossScene;
}

std::shared_ptr<ObjSpell> Package::CreateObjSpell()
{
    auto spell = objTable_->Create<ObjSpell>(colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(spell, 50);
    return spell;
}

NullableSharedPtr<Script> Package::GetScript(int scriptId) const
{
    return scriptManager_->Get(scriptId);
}

std::shared_ptr<Script> Package::LoadScript(const std::wstring & path, ScriptType type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = scriptManager_->Compile(path, type, version, shared_from_this(), srcPos);
    script->Load();
    return script;
}

std::shared_ptr<Script> Package::LoadScriptInThread(const std::wstring & path, ScriptType type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos)
{
    auto script = scriptManager_->CompileInThread(path, type, version, shared_from_this(), srcPos);
    return script;
}

void Package::CloseStgScene()
{
    if (auto stageMain = stageMainScript_.lock())
    {
        stageMain->Close();
    }
}

void Package::NotifyEventAll(int eventType)
{
    scriptManager_->NotifyEventAll(eventType);
}

void Package::NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args)
{
    scriptManager_->NotifyEventAll(eventType, args);
}

std::wstring Package::GetPlayerID() const
{
    return stagePlayerScriptInfo_.id;
}

std::wstring Package::GetPlayerReplayName() const
{
    return stagePlayerScriptInfo_.replayName;
}

NullableSharedPtr<ObjPlayer> Package::GetPlayerObject() const
{
    auto player = playerObj_.lock();
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
    auto bossScene = enemyBossSceneObj_.lock();
    if (bossScene && !bossScene->IsDead()) return bossScene;
    return nullptr;
}

NullableSharedPtr<ObjSpellManage> Package::GetSpellManageObject() const
{
    auto spellManage = spellManageObj_.lock();
    if (spellManage && !spellManage->IsDead()) return spellManage;
    return nullptr;
}

void Package::GenerateSpellManageObject()
{
    if (auto spellManageObj = spellManageObj_.lock())
    {
        // 既にある場合は削除
        DeleteObject(spellManageObj->GetID());
    }
    spellManageObj_ = objTable_->Create<ObjSpellManage>(shared_from_this());
}

void Package::DeleteObject(int id)
{
    objTable_->Delete(id);
}

bool Package::IsObjectDeleted(int id) const
{
    return objTable_->IsDeleted(id);
}

std::shared_ptr<ObjPrim2D> Package::CreateObjPrim2D()
{
    auto obj = objTable_->Create<ObjPrim2D>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite2D> Package::CreateObjSprite2D()
{
    auto obj = objTable_->Create<ObjSprite2D>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSpriteList2D> Package::CreateObjSpriteList2D()
{
    auto obj = objTable_->Create<ObjSpriteList2D>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjPrim3D> Package::CreateObjPrim3D()
{
    auto obj = objTable_->Create<ObjPrim3D>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjSprite3D> Package::CreateObjSprite3D()
{
    auto obj = objTable_->Create<ObjSprite3D>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

std::shared_ptr<ObjMesh> Package::CreateObjMesh()
{
    auto obj = objTable_->Create<ObjMesh>(shared_from_this());
    objLayerList_->SetRenderPriority(obj, 50);
    return obj;
}

NullableSharedPtr<Script> Package::GetPlayerScript() const
{
    return stagePlayerScript_.lock();
}

const std::unique_ptr<DnhValue>& Package::GetScriptResult(int scriptId) const
{
    return scriptManager_->GetScriptResult(scriptId);
}

void Package::SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value)
{
    scriptManager_->SetScriptResult(scriptId, std::move(value));
}

std::vector<ScriptInfo> Package::GetScriptList(const std::wstring & dirPath, ScriptType scriptType, bool doRecursive, bool getAll)
{
    std::vector<std::wstring> pathList;
    GetFilePaths(dirPath, pathList, ignoreScriptExts, doRecursive);
    std::vector<ScriptInfo> infos;
    infos.reserve(pathList.size());
    for (const auto& path : pathList)
    {
        try
        {
            auto info = ScanDnhScriptInfo(path, fileLoader_);
            if (getAll || scriptType == info.type)
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
    freePlayerScriptInfoList_ = GetScriptList(FREE_PLAYER_DIR, ScriptType::Value::PLAYER, true, false);
}

int Package::GetFreePlayerScriptCount() const
{
    return freePlayerScriptInfoList_.size();
}

ScriptInfo Package::GetFreePlayerScriptInfo(int idx) const
{
    return freePlayerScriptInfoList_.at(idx);
}

ScriptInfo Package::GetScriptInfo(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        return ScanDnhScriptInfo(path, fileLoader_);
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
    return shotCounter_->playerShotCount + shotCounter_->enemyShotCount;
}

int Package::GetEnemyShotCount() const
{
    return shotCounter_->enemyShotCount;
}

int Package::GetPlayerShotCount() const
{
    return shotCounter_->playerShotCount;
}

void Package::SuccPlayerShotCount()
{
    shotCounter_->playerShotCount++;
}

void Package::SuccEnemyShotCount()
{
    shotCounter_->enemyShotCount++;
}

void Package::PredPlayerShotCount()
{
    shotCounter_->playerShotCount--;
}

void Package::PredEnemyShotCount()
{
    shotCounter_->enemyShotCount--;
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
    auto shotScript = LoadScript(path, ScriptType::Value::SHOT_CUSTOM, stageMainScriptInfo_.version, srcPos);
    shotScript = shotScript;
    shotScript->RunInitialize();
}

NullableSharedPtr<Script> Package::GetShotScript() const
{
    return shotScript_.lock();
}

void Package::SetDeleteShotImmediateEventOnShotScriptEnable(bool enable)
{
    deleteShotImmediateEventOnShotScriptEnable_ = enable;
}

void Package::SetDeleteShotFadeEventOnShotScriptEnable(bool enable)
{
    deleteShotFadeEventOnShotScriptEnable_ = enable;
}

void Package::SetDeleteShotToItemEventOnShotScriptEnable(bool enable)
{
    deleteShotToItemEventOnShotScriptEnable_ = enable;
}

bool Package::IsDeleteShotImmediateEventOnShotScriptEnabled() const
{
    return deleteShotImmediateEventOnShotScriptEnable_;
}

bool Package::IsDeleteShotFadeEventOnShotScriptEnabled() const
{
    return deleteShotFadeEventOnShotScriptEnable_;
}

bool Package::IsDeleteShotToItemEventOnShotScriptEnabled() const
{
    return deleteShotToItemEventOnShotScriptEnable_;
}

void Package::DeleteShotAll(int target, int behavior)
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

void Package::DeleteShotInCircle(int target, int behavior, float x, float y, float r)
{
    auto isects = colDetector_->GetIntersectionsCollideWithShape(Shape(x, y, r), COL_GRP_ENEMY_SHOT);
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

    auto isects = colDetector_->GetIntersectionsCollideWithShape(Shape(x, y, r), -1);
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
    colDetector_->Add(isect);
    tempEnemyShotIsects_.push_back(isect);
}

void Package::SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width)
{
    auto isect = std::make_shared<TempEnemyShotIntersection>(x1, y1, x2, y2, width);
    colDetector_->Add(isect);
    tempEnemyShotIsects_.push_back(isect);
}

void Package::CollectAllItems()
{
    autoItemCollectionManager_->CollectAllItems();
}

void Package::CollectItemsByType(int itemType)
{
    autoItemCollectionManager_->CollectItemsByType(itemType);
}

void Package::CollectItemsInCircle(float x, float y, float r)
{
    autoItemCollectionManager_->CollectItemsInCircle(x, y, r);
}

void Package::CancelCollectItems()
{
    autoItemCollectionManager_->CancelCollectItems();
}

bool Package::IsAutoCollectCanceled() const
{
    return autoItemCollectionManager_->IsAutoCollectCanceled();
}

bool Package::IsAutoCollectTarget(int itemType, float x, float y) const
{
    return autoItemCollectionManager_->IsAutoCollectTarget(itemType, x, y);
}

void Package::SetDefaultBonusItemEnable(bool enable)
{
    defaultBonusItemEnable_ = enable;
}

bool Package::IsDefaultBonusItemEnabled() const
{
    return defaultBonusItemEnable_;
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
    auto itemScript = LoadScript(path, ScriptType::Value::ITEM_CUSTOM, stageMainScriptInfo_.version, srcPos);
    itemScript = itemScript;
    itemScript->RunInitialize();
}

NullableSharedPtr<Script> Package::GetItemScript() const
{
    return itemScript_.lock();
}

bool Package::IsClosed() const
{
    if (auto packageMainScript = packageMainScript_.lock())
    {
        return packageMainScript->IsClosed();
    }
    return true;
}

void Package::Close()
{
    if (auto packageMain = packageMainScript_.lock())
    {
        packageMain->Close();
        packageMainScript_.reset();
    }
}

void Package::Finalize()
{
    scriptManager_->RunFinalizeAll();
}

void Package::InitializeStageScene()
{
    objTable_->DeleteStgSceneObject();
    scriptManager_->CloseStgSceneScript();
    SetStgFrame(32.0f, 16.0f, 416.0f, 464.0f);
    SetShotAutoDeleteClip(64.0f, 64.0f, 64.0f, 64.0f);
    Reset2DCamera();
    SetDefaultBonusItemEnable(true);
    stageSceneResult_ = 0;
    stageMainScript_.reset();
    stagePlayerScript_.reset();

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
    isStagePaused_ = true;
    isStageForceTerminated_ = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
}

void Package::FinalizeStageScene()
{
    // FUTURE : impl
}

void Package::Start()
{
    packageStartTime_ = std::make_shared<TimePoint>();
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("start package.")
        .SetParam(Log::Param(Log::Param::Tag::SCRIPT, GetMainScriptPath()))));
    auto script = scriptManager_->Compile(packageMainScriptInfo_.path, ScriptType::Value::PACKAGE, packageMainScriptInfo_.version, shared_from_this(), nullptr);
    packageMainScript_ = script;
    script->RunInitialize();
}

bool Package::IsReplay() const
{
    return isReplay_;
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
    camera3D_->GenerateViewMatrix(&view, &billboard);
    if (isStgScene)
    {
        camera3D_->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
    } else
    {
        camera3D_->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
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
        camera2D_->GenerateViewMatrix(&view);
        camera2D_->GenerateProjMatrix(&proj, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        D3DXMATRIX viewProjViewport = view * proj * viewport;
        D3DXMatrixInverse(&viewProjViewport, NULL, &viewProjViewport);
        D3DXVec3TransformCoord(&pos, &pos, &viewProjViewport);
    }
    return Point2D(pos.x, pos.y);
}

double Package::GetRandDouble(double min, double max)
{
    return randGenerator_->RandDouble(min, max);
}

const std::shared_ptr<EngineDevelopOptions>& Package::GetEngineDevelopOptions() const
{
    return engineDevelopOptions_;
}

void Package::SetStageIndex(uint16_t idx)
{
    stageIdx_ = idx;
}

void Package::SetStageMainScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        stageMainScriptInfo_ = ScanDnhScriptInfo(path, fileLoader_);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        stageMainScriptInfo_ = ScriptInfo();
        stageMainScriptInfo_.path = path;
    }
}

void Package::SetStagePlayerScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos)
{
    try
    {
        stagePlayerScriptInfo_ = ScanDnhScriptInfo(path, fileLoader_);
    } catch (Log& log)
    {
        log.SetLevel(Log::Level::LV_WARN).AddSourcePos(srcPos);
        Logger::WriteLog(log);
        stageMainScriptInfo_ = ScriptInfo();
        stageMainScriptInfo_.path = path;
    }
}

void Package::SetStageMainScript(const ScriptInfo & script)
{
    stageMainScriptInfo_ = script;
}

void Package::SetStagePlayerScript(const ScriptInfo & script)
{
    stagePlayerScriptInfo_ = script;
}

void Package::SetStageReplayFile(const std::wstring & path)
{
    replayData_ = std::make_shared<ReplayData>(path);
    stageReplayFilePath_ = path;
}

bool Package::IsStageFinished() const
{
    if (stageMainScript_.lock())
    {
        return false;
    }
    return true;
}

int Package::GetStageSceneResult() const
{
    return stageSceneResult_;
}

bool Package::IsStagePaused() const
{
    return isStagePaused_;
}

void Package::PauseStageScene(bool doPause)
{
    if (doPause && !isStagePaused_)
    {
        NotifyEventAll(EV_PAUSE_ENTER);
    } else if (!doPause && isStagePaused_)
    {
        NotifyEventAll(EV_PAUSE_LEAVE);
    }
    isStagePaused_ = doPause;
}

void Package::TerminateStageScene()
{
    if (auto stageMain = stageMainScript_.lock())
    {
        stageMain->Close();
        isStageForceTerminated_ = true;
    }
}

void Package::StartStageScene(const std::shared_ptr<SourcePos>& srcPos)
{
    objTable_->DeleteStgSceneObject();
    scriptManager_->CloseStgSceneScript();
    inputDevice_->ResetInputState();
    Reset2DCamera();
    ResetCamera();
    SetDefaultBonusItemEnable(true);
    playerShotDataTable_ = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureStore_, fileLoader_);
    enemyShotDataTable_ = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureStore_, fileLoader_);
    itemDataTable_ = std::make_shared<ItemDataTable>(textureStore_, fileLoader_);
    stageMainScript_.reset();
    stagePlayerScript_.reset();
    ReloadItemData(DEFAULT_ITEM_DATA_PATH, nullptr);
    stageSceneResult_ = 0;
    isStageForceTerminated_ = false;
    SetDeleteShotImmediateEventOnShotScriptEnable(false);
    SetDeleteShotFadeEventOnShotScriptEnable(false);
    SetDeleteShotToItemEventOnShotScriptEnable(false);
    isStagePaused_ = false;
    renderer_->SetFogEnable(false);
    pseudoPlayerFps_ = pseudoEnemyFps_ = 60;
    stageElapesdFrame_ = 0;

    if (!IsReplay())
    {
        replayData_ = std::make_shared<ReplayData>();
    }

    if (stageMainScriptInfo_.systemPath.empty() || stageMainScriptInfo_.systemPath == L"DEFAULT")
    {
        stageMainScriptInfo_.systemPath = DEFAULT_SYSTEM_PATH;
    }

    // #System
    auto systemScript = scriptManager_->Compile(stageMainScriptInfo_.systemPath, ScriptType::Value::STAGE, stageMainScriptInfo_.version, shared_from_this(), srcPos);
    systemScript->RunInitialize();

    // Create Player
    auto player = objTable_->Create<ObjPlayer>(colDetector_, shared_from_this());
    objLayerList_->SetRenderPriority(player, DEFAULT_PLAYER_RENDER_PRIORITY);
    player->AddIntersectionToItem();
    playerObj_ = player;
    Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .SetMessage("create player object.")
        .AddSourcePos(srcPos)));

    auto playerScript = scriptManager_->Compile(stagePlayerScriptInfo_.path, ScriptType::Value::PLAYER, stagePlayerScriptInfo_.version, shared_from_this(), srcPos);
    stagePlayerScript_ = playerScript;
    playerScript->RunInitialize();

    // Main
    auto stageMainScriptPath = stageMainScriptInfo_.path;
    if (stageMainScriptInfo_.type == ScriptType::Value::SINGLE)
    {
        stageMainScriptPath = SYSTEM_SINGLE_STAGE_PATH;
    } else if (stageMainScriptInfo_.type == ScriptType::Value::PLURAL)
    {
        stageMainScriptPath = SYSTEM_PLURAL_STAGE_PATH;
    }
    auto stageMainScript = scriptManager_->Compile(stageMainScriptPath, ScriptType::Value::STAGE, stageMainScriptInfo_.version, shared_from_this(), srcPos);
    stageMainScript_ = stageMainScript;
    stageMainScript->RunInitialize();
    stageStartTime_ = std::make_shared<TimePoint>();

    // #Background
    if (!stageMainScriptInfo_.backgroundPath.empty() && stageMainScriptInfo_.backgroundPath != L"DEFAULT")
    {
        auto backgroundScript = scriptManager_->Compile(stageMainScriptInfo_.backgroundPath, ScriptType::Value::STAGE, stageMainScriptInfo_.version, shared_from_this(), srcPos);
        backgroundScript->RunInitialize();
    }

    // #BGM
    if (!stageMainScriptInfo_.bgmPath.empty() && stageMainScriptInfo_.bgmPath != L"DEFAULT")
    {
        try
        {
            LoadOrphanSound(stageMainScriptInfo_.bgmPath, nullptr); // TODO: #BGMヘッダのSourcePosを与える
            auto& bgm = orphanSounds_[GetCanonicalPath(packageMainScriptInfo_.bgmPath)];
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
        graphicDevice_->SwitchRenderTargetToBackBuffer();
    } else
    {
        auto renderTarget = GetRenderTarget(name);
        if (!renderTarget) return;
        renderTarget->SetRenderTarget();
    }

    D3DXMATRIX viewMatrix2D, projMatrix2D, viewMatrix3D, projMatrix3D, billboardMatrix;
    Camera2D outsideStgFrameCamera2D; outsideStgFrameCamera2D.Reset(0, 0);
    renderer_->InitRenderState();

    if (doClear)
    {
        graphicDevice_->GetDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    }

    begin = std::max(begin, 0);
    end = std::min(end, MAX_RENDER_PRIORITY);

    auto obj = objTable_->Get<ObjRender>(objId);
    if (obj && checkVisibleFlag && !obj->IsVisible()) return;

    camera3D_->GenerateViewMatrix(&viewMatrix3D, &billboardMatrix);

    // [0, stgFrameMin]
    {
        renderer_->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        renderer_->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        camera3D_->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        renderer_->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = 0; p < GetStgFrameRenderPriorityMin(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer_);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList_->IsInvalidRenderPriority(p)))
                {
                    objLayerList_->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer_);
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
                scissorRect.left = stgFrame_.left * graphicDevice_->GetBackBufferWidth() / screenWidth_;
                scissorRect.top = stgFrame_.top * graphicDevice_->GetBackBufferHeight() / screenHeight_;
                scissorRect.right = stgFrame_.right * graphicDevice_->GetBackBufferWidth() / screenWidth_;
                scissorRect.bottom = stgFrame_.bottom * graphicDevice_->GetBackBufferHeight() / screenHeight_;
            }
            renderer_->EnableScissorTest(scissorRect);
        }

        // set 2D matrix
        if (!IsStageFinished())
        {
            camera2D_->GenerateViewMatrix(&viewMatrix2D);
            camera2D_->GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
            renderer_->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
        }

        // set 3D matrix
        camera3D_->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetStgFrameCenterScreenX(), GetStgFrameCenterScreenY());
        renderer_->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);

        for (int p = GetStgFrameRenderPriorityMin(); p <= GetStgFrameRenderPriorityMax(); p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer_);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList_->IsInvalidRenderPriority(p)))
                {
                    objLayerList_->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer_);
                }
            }
            if (p == objLayerList_->GetCameraFocusPermitRenderPriority())
            {
                // cameraFocusPermitRenderPriorityより大きい優先度では別のビュー変換行列を使う
                if (!IsStageFinished())
                {
                    Camera2D focusForbidCamera;
                    focusForbidCamera.Reset(GetStgFrameCenterWorldX(), GetStgFrameCenterWorldY());
                    focusForbidCamera.GenerateViewMatrix(&viewMatrix2D);
                    renderer_->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);
                }
            }
        }
    }
    {
        // (stgFrameMax, MAX_RENDER_PRIORITY]
        renderer_->DisableScissorTest();

        // set 2D matrix
        outsideStgFrameCamera2D.GenerateViewMatrix(&viewMatrix2D);
        outsideStgFrameCamera2D.GenerateProjMatrix(&projMatrix2D, GetScreenWidth(), GetScreenHeight(), 0, 0);
        renderer_->SetViewProjMatrix2D(viewMatrix2D, projMatrix2D);

        // set 3D matrix
        camera3D_->GenerateProjMatrix(&projMatrix3D, GetScreenWidth(), GetScreenHeight(), GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f);
        renderer_->SetViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
        for (int p = GetStgFrameRenderPriorityMax() + 1; p <= MAX_RENDER_PRIORITY; p++)
        {
            if (obj && obj->getRenderPriority() == p)
            {
                obj->Render(renderer_);
            }
            if (objId == ID_INVALID && p >= begin && p <= end)
            {
                if (!(checkInvalidRenderPriority && objLayerList_->IsInvalidRenderPriority(p)))
                {
                    objLayerList_->RenderLayer(p, IsStagePaused(), checkVisibleFlag, renderer_);
                }
            }
        }
    }
    graphicDevice_->SwitchRenderTargetToBackBuffer();
}

NullableSharedPtr<Obj> Package::GetObj(int id) const
{
    return objTable_->Get<Obj>(id);
}

const std::map<int, std::shared_ptr<Obj>>& Package::GetObjAll() const
{
    return objTable_->GetAll();
}

Package* Package::Current = nullptr;
}