#pragma once

#include <bstorm/type.hpp>
#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/stage_common_player_params.hpp>
#include <bstorm/key_types.hpp>

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <windows.h>
#include <d3d9.h>

namespace bstorm
{
class GraphicDevice;
class LostableGraphicResource;
class LostableGraphicResourceManager;
class MousePositionProvider;
class Renderer;
class RenderTarget;
class Shader;
class Texture;
class Font;
class Mesh;
class DnhValue;
class DnhArray;
class Obj;
class ObjRender;
class ObjShader;
class ObjPrim2D;
class ObjSprite2D;
class ObjSpriteList2D;
class ObjPrim3D;
class ObjSprite3D;
class ObjMesh;
class ObjText;
class ObjSound;
class ObjFileT;
class ObjFileB;
class ObjShot;
class ObjLooseLaser;
class ObjStLaser;
class ObjCrLaser;
class ObjEnemy;
class ObjEnemyBossScene;
class ObjSpell;
class ObjSpellManage;
class ObjItem;
class ObjPlayer;
class ShotData;
class ItemData;
class ScriptInfo;
class Script;
class SoundBuffer;
class Package;
struct SourcePos;
namespace conf { struct KeyConfig; }
class Engine
{
public:
    Engine(HWND hWnd, int screenWidth, int screenHeight, const std::shared_ptr<conf::KeyConfig>& defaultKeyConfig);
    virtual ~Engine();
    void AddLostableGraphicResource(const std::shared_ptr<LostableGraphicResource>& resource);
    HWND GetWindowHandle() const;
    /* engine control */
    void TickFrame();
    void Render(); // render to backbuffer
    void Render(const std::wstring& renderTargetName);
    void RenderToTextureA1(const std::wstring& renderTargetName, int begin, int end, bool doClear);
    void RenderToTextureB1(const std::wstring& renderTargetName, int objId, bool doClear);
    // NOTE : resetに失敗したらEngineを終了すべき
    void Reset(int screenWidth, int screenHeight);
    int GetScreenWidth() const;
    int GetScreenHeight() const;
    bool IsRenderIntersectionEnabled() const;
    void SetRenderIntersectionEnable(bool enable);
    bool IsForcePlayerInvincibleEnabled() const;
    void SetForcePlayerInvincibleEnable(bool enable);
    /* graphic device control */
    IDirect3DDevice9* GetDirect3DDevice() const;
    // should call only to reset device
    void ResetGraphicDevice();
    void ReleaseLostableGraphicResource();
    void RestoreLostableGraphicDevice();
    // set backbuffer to render-target
    void SetBackBufferRenderTarget();
    void ReleaseUnusedLostableGraphicResource();
    /* input */
    KeyState GetKeyState(Key k);
    KeyState GetVirtualKeyState(VirtualKey vk);
    void SetVirtualKeyState(VirtualKey vk, KeyState state);
    void AddVirtualKey(VirtualKey vk, Key k, PadButton btn);
    KeyState GetMouseState(MouseButton btn);
    int GetMouseX();
    int GetMouseY();
    int GetMouseMoveZ();
    void SetMousePostionProvider(const std::shared_ptr<MousePositionProvider>& provider);
    void SetInputEnable(bool enable);
    /* logging function */
    void WriteLog(const std::string&& msg, const std::shared_ptr<SourcePos>& srcPos);
    /* time */
    std::wstring GetCurrentDateTimeS();
    float GetCurrentFps() const;
    float GetStageTime() const;
    float GetPackageTime() const;
    void UpdateFpsCounter();
    void ResetFpsCounter();
    void StartSlow(FrameCount pseudoFps, bool byPlayer);
    void StopSlow(bool byPlayer);
    int64_t GetElapsedFrame() const;
    /* path */
    std::wstring GetMainStgScriptPath() const;
    std::wstring Engine::GetMainStgScriptDirectory() const;
    std::wstring GetMainPackageScriptPath() const;
    /* texture */
    std::shared_ptr<Texture> LoadTexture(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos);
    void LoadTextureInThread(const std::wstring& path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) noexcept(true);
    void RemoveTextureReservedFlag(const std::wstring& path);
    void ReleaseUnusedTextureCache();
    /* font */
    void ReleaseUnusedFontCache();
    bool InstallFont(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    /* render target */
    std::shared_ptr<RenderTarget> CreateRenderTarget(const std::wstring& name, int width, int height, const std::shared_ptr<SourcePos>& srcPos);
    void RemoveRenderTarget(const std::wstring& name, const std::shared_ptr<SourcePos>& srcPos);
    NullableSharedPtr<RenderTarget> GetRenderTarget(const std::wstring& name) const;
    std::wstring GetReservedRenderTargetName(int idx) const;
    std::wstring GetTransitionRenderTargetName() const;
    void SaveRenderedTextureA1(const std::wstring& name, const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void SaveRenderedTextureA2(const std::wstring& name, const std::wstring& path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos);
    void SaveSnapShotA1(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void SaveSnapShotA2(const std::wstring& path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos);
    /* shader */
    std::shared_ptr<Shader> CreateShader(const std::wstring& path, bool precompiled);
    bool IsPixelShaderSupported(int major, int minor);
    /* mesh */
    std::shared_ptr<Mesh> LoadMesh(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void ReleaseUnusedMeshCache();
    /* sound */
    std::shared_ptr<SoundBuffer> LoadSound(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void LoadOrphanSound(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void RemoveOrphanSound(const std::wstring& path);
    void PlayBGM(const std::wstring& path, double loopStartSec, double loopEndSec);
    void PlaySE(const std::wstring& path);
    void StopOrphanSound(const std::wstring& path);
    void CacheSound(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void RemoveSoundCache(const std::wstring& path);
    void ClearSoundCache();
    /* layer */
    void SetObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority);
    void SetShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader);
    void ResetShader(int beginPriority, int endPriority);
    int GetStgFrameRenderPriorityMin() const;
    void SetStgFrameRenderPriorityMin(int p);
    int GetStgFrameRenderPriorityMax() const;
    void SetStgFrameRenderPriorityMax(int p);
    int GetShotRenderPriority() const;
    void SetShotRenderPriority(int p);
    int GetItemRenderPriority() const;
    void SetItemRenderPriority(int p);
    int GetPlayerRenderPriority() const;
    int GetCameraFocusPermitRenderPriority() const;
    void SetInvalidRenderPriority(int min, int max);
    void ClearInvalidRenderPriority();
    NullableSharedPtr<Shader> GetShader(int p) const;
    /* renderer */
    void SetFogEnable(bool enable);
    void SetFogParam(float fogStart, float fogEnd, int r, int g, int b);
    /* 3D camera */
    void SetCameraFocusX(float x);
    void SetCameraFocusY(float y);
    void SetCameraFocusZ(float z);
    void SetCameraFocusXYZ(float x, float y, float z);
    void SetCameraRadius(float r);
    void SetCameraAzimuthAngle(float angle);
    void SetCameraElevationAngle(float angle);
    void SetCameraYaw(float yaw);
    void SetCameraPitch(float pitch);
    void SetCameraRoll(float roll);
    void ResetCamera();
    float GetCameraX() const;
    float GetCameraY() const;
    float GetCameraZ() const;
    float GetCameraFocusX() const;
    float GetCameraFocusY() const;
    float GetCameraFocusZ() const;
    float GetCameraRadius() const;
    float GetCameraAzimuthAngle() const;
    float GetCameraElevationAngle() const;
    float GetCameraYaw() const;
    float GetCameraPitch() const;
    float GetCameraRoll() const;
    void SetCameraPerspectiveClip(float nearClip, float farClip);
    /* 2D camera */
    void Set2DCameraFocusX(float x);
    void Set2DCameraFocusY(float y);
    void Set2DCameraAngleZ(float z);
    void Set2DCameraRatio(float r);
    void Set2DCameraRatioX(float x);
    void Set2DCameraRatioY(float x);
    void Reset2DCamera();
    float Get2DCameraX() const;
    float Get2DCameraY() const;
    float Get2DCameraAngleZ() const;
    float Get2DCameraRatio() const;
    float Get2DCameraRatioX() const;
    float Get2DCameraRatioY() const;
    /* common data */
    void SetCommonData(const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    const std::unique_ptr<DnhValue>& GetCommonData(const std::wstring& key, const std::unique_ptr<DnhValue>& defaultValue) const;
    void ClearCommonData();
    void DeleteCommonData(const std::wstring& key);
    void SetAreaCommonData(const std::wstring& areaName, const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    const std::unique_ptr<DnhValue>& GetAreaCommonData(const std::wstring& areaName, const std::wstring& key, const std::unique_ptr<DnhValue>& defaultValue) const;
    void ClearAreaCommonData(const std::wstring& areaName);
    void DeleteAreaCommonData(const std::wstring& areaName, const std::wstring& key);
    void CreateCommonDataArea(const std::wstring& areaName);
    bool IsCommonDataAreaExists(const std::wstring& areaName) const;
    void CopyCommonDataArea(const std::wstring& dest, const std::wstring& src);
    std::vector<std::wstring> GetCommonDataAreaKeyList() const;
    std::vector<std::wstring> GetCommonDataValueKeyList(const std::wstring& areaName) const;
    bool SaveCommonDataAreaA1(const std::wstring& areaName) const;
    bool LoadCommonDataAreaA1(const std::wstring& areaName);
    bool SaveCommonDataAreaA2(const std::wstring& areaName, const std::wstring& path) const;
    bool LoadCommonDataAreaA2(const std::wstring& areaName, const std::wstring& path);
    std::wstring Engine::GetDefaultCommonDataSavePath(const std::wstring& areaName) const;
    /* user def data */
    void LoadPlayerShotData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void ReloadPlayerShotData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void LoadEnemyShotData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void ReloadEnemyShotData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void LoadItemData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void ReloadItemData(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    NullableSharedPtr<ShotData> GetPlayerShotData(int id) const;
    NullableSharedPtr<ShotData> GetEnemyShotData(int id) const;
    NullableSharedPtr<ItemData> GetItemData(int id) const;
    /* object */
    template <class T>
    NullableSharedPtr<T> GetObject(int id) const
    {
        return std::dynamic_pointer_cast<T>(GetObj(id));
    };

    template <>
    NullableSharedPtr<Obj> GetObject(int id) const
    {
        return GetObj(id);
    };

    template <class T>
    std::vector <std::shared_ptr<T>> GetObjectAll() const
    {
        std::vector <std::shared_ptr<T>> objs;
        for (const auto& entry : GetObjAll())
        {
            if (auto obj = std::dynamic_pointer_cast<T>(entry.second))
            {
                objs.push_back(obj);
            }
        }
        return objs;
    }
    NullableSharedPtr<ObjPlayer> GetPlayerObject() const;
    NullableSharedPtr<ObjEnemy> GetEnemyBossObject() const;
    NullableSharedPtr<ObjEnemyBossScene> GetEnemyBossSceneObject() const;
    NullableSharedPtr<ObjSpellManage> GetSpellManageObject() const;
    void DeleteObject(int id);
    bool IsObjectDeleted(int id) const;
    std::shared_ptr<ObjPrim2D> CreateObjPrim2D();
    std::shared_ptr<ObjSprite2D> CreateObjSprite2D();
    std::shared_ptr<ObjSpriteList2D> CreateObjSpriteList2D();
    std::shared_ptr<ObjPrim3D> CreateObjPrim3D();
    std::shared_ptr<ObjSprite3D> CreateObjSprite3D();
    std::shared_ptr<ObjMesh> CreateObjMesh();
    std::shared_ptr<ObjText> CreateObjText();
    std::shared_ptr<ObjSound> CreateObjSound();
    std::shared_ptr<ObjFileT> CreateObjFileT();
    std::shared_ptr<ObjFileB> CreateObjFileB();
    std::shared_ptr<ObjShader> CreateObjShader();
    std::shared_ptr<ObjShot> CreateObjShot(bool isPlayerShot);
    std::shared_ptr<ObjShot> CreateShotA1(float x, float y, float speed, float angle, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> CreateShotA2(float x, float y, float speed, float angle, float accel, float maxSpeed, int shotDataId, int delay, bool isPlayerShot);
    NullableSharedPtr<ObjShot> CreateShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> CreateShotB1(float x, float y, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> CreateShotB2(float x, float y, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, int shotDataId, int delay, bool isPlayerShot);
    NullableSharedPtr<ObjShot> CreateShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot);
    NullableSharedPtr<ObjShot> CreatePlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId);
    std::shared_ptr<ObjLooseLaser> CreateObjLooseLaser(bool isPlayerShot);
    std::shared_ptr<ObjLooseLaser> CreateLooseLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjStLaser> CreateObjStLaser(bool isPlayerShot);
    std::shared_ptr<ObjStLaser> CreateStraightLaserA1(float x, float y, float angle, float length, float width, int deleteFrame, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjCrLaser> CreateObjCrLaser(bool isPlayerShot);
    std::shared_ptr<ObjCrLaser> CreateCurveLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjItem> CreateItemA1(int itemType, float x, float y, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemA2(int itemType, float x, float y, float destX, float destY, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemU1(int itemDataId, float x, float y, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemU2(int itemDataId, float x, float y, float destX, float destY, PlayerScore score);
    std::shared_ptr<ObjEnemy> CreateObjEnemy();
    std::shared_ptr<ObjEnemyBossScene> CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<ObjSpell> CreateObjSpell();
    std::shared_ptr<ObjItem> CreateObjItem(int itemType);
    /* script */
    std::shared_ptr<Script> GetScript(int scriptId) const;
    std::shared_ptr<Script> LoadScript(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<Script> LoadScriptInThread(const std::wstring& path, const std::wstring& type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    void CloseStgScene();
    void NotifyEventAll(int eventType);
    void NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    std::wstring GetPlayerID() const;
    std::wstring GetPlayerReplayName() const;
    NullableSharedPtr<Script> GetPlayerScript() const;
    const std::unique_ptr<DnhValue>& GetScriptResult(int scriptId) const;
    void SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    std::vector<ScriptInfo> GetScriptList(const std::wstring& dirPath, int scriptType, bool doRecursive);
    void GetLoadFreePlayerScriptList();
    int GetFreePlayerScriptCount() const;
    ScriptInfo GetFreePlayerScriptInfo(int idx) const;
    ScriptInfo GetScriptInfo(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    /* point */
    PlayerScore GetScore() const;
    void AddScore(PlayerScore score);
    int64_t GetGraze() const;
    void AddGraze(int64_t graze);
    int64_t GetPoint() const;
    void AddPoint(int64_t point);
    /* stg frame */
    void SetStgFrame(float left, float top, float right, float bottom);
    float GetStgFrameLeft() const;
    float GetStgFrameTop() const;
    float GetStgFrameWidth() const;
    float GetStgFrameHeight() const;
    float GetStgFrameCenterWorldX() const;
    float GetStgFrameCenterWorldY() const;
    float GetStgFrameCenterScreenX() const;
    float GetStgFrameCenterScreenY() const;
    /* shot */
    int GetAllShotCount() const;
    int GetEnemyShotCount() const;
    int GetPlayerShotCount() const;
    void SetShotAutoDeleteClip(float left, float top, float right, float bottom);
    void StartShotScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void SetDeleteShotImmediateEventOnShotScriptEnable(bool enable);
    void SetDeleteShotFadeEventOnShotScriptEnable(bool enable);
    void SetDeleteShotToItemEventOnShotScriptEnable(bool enable);
    void DeleteShotAll(int tarGet, int behavior) const;
    void DeleteShotInCircle(int tarGet, int behavior, float x, float y, float r) const;
    std::vector<std::shared_ptr<ObjShot>> GetShotInCircle(float x, float y, float r, int tarGet) const;
    void SetShotIntersectoinCicle(float x, float y, float r);
    void SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width);
    /* item */
    void CollectAllItems();
    void CollectItemsByType(int itemType);
    void CollectItemsInCircle(float x, float y, float r);
    void CancelCollectItems();
    void SetDefaultBonusItemEnable(bool enable);
    void StartItemScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    /* package */
    bool IsPackageFinished() const;
    void ClosePackage();
    void InitializeStageScene();
    void FinalizeStageScene();
    void StartStageScene(const std::shared_ptr<SourcePos>& srcPos);
    void SetStageIndex(uint16_t idx);
    void SetStageMainScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void SetStageMainScript(const ScriptInfo& script);
    void SetStagePlayerScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    void SetStagePlayerScript(const ScriptInfo& script);
    void SetStageReplayFile(const std::wstring& path);
    bool IsStageFinished() const;
    int GetStageSceneResult() const;
    bool IsStagePaused() const;
    void PauseStageScene(bool doPause);
    void TerminateStageScene();
    void SetPackageMainScript(const std::wstring& path);
    void SetPackageMainScript(const ScriptInfo& script);
    void StartPackage();
    /* for default package */
    void SetPauseScriptPath(const std::wstring& path);
    void SetEndSceneScriptPath(const std::wstring& path);
    void SetReplaySaveSceneScriptPath(const std::wstring& path);
    /* etc */
    Point2D Get2DPosition(float x, float y, float z, bool isStgScene);
    /* backdoor */
    template <typename T>
    void backDoor() {}
protected:
    void RenderToTexture(const std::wstring& renderTargetName, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag);
    NullableSharedPtr<Obj> GetObj(int id) const;
    const std::map<int, std::shared_ptr<Obj>>& GetObjAll() const;
    HWND hWnd;
    std::unique_ptr<GraphicDevice> graphicDevice;
    std::unique_ptr<LostableGraphicResourceManager> lostableGraphicResourceManager;
    std::shared_ptr<conf::KeyConfig> defaultKeyConfig;
    std::shared_ptr<MousePositionProvider> mousePosProvider;
    std::unordered_map<std::wstring, std::shared_ptr<RenderTarget>> renderTargets;
    std::shared_ptr<Package> package;
};
}