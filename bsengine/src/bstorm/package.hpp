#pragma once

#include <bstorm/nullable_shared_ptr.hpp>
#include <bstorm/stage_types.hpp>
#include <bstorm/key_types.hpp>
#include <bstorm/stage_common_player_params.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/font_params.hpp>
#include <bstorm/point2D.hpp>
#include <bstorm/rect.hpp>

#include <memory>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <windows.h>
#include <bstorm/stage_common_player_params.hpp>
#include <d3d9.h>

#undef CreateFont
#undef GetObject

namespace bstorm
{
class AutoDeleteClip;
class AutoItemCollectionManager;
class Camera2D;
class Camera3D;
class CollisionDetector;
class CommonDataDB;
class DnhArray;
class DnhValue;
class FileLoader;
class Font;
class FontStore;
class FpsCounter;
class GraphicDevice;
class InputDevice;
class Intersection;
class ItemData;
class ItemDataTable;
class ItemScoreTextSpawner;
class VirtualKeyAssign;
class LostableGraphicResource;
class LostableGraphicResourceManager;
class Mesh;
class MeshStore;
class Obj;
class ObjCrLaser;
class ObjEnemy;
class ObjEnemyBossScene;
class ObjFileB;
class ObjFileT;
class ObjItem;
class ObjLooseLaser;
class ObjMesh;
class ObjPlayer;
class ObjPrim2D;
class ObjPrim3D;
class ObjRender;
class ObjShader;
class ObjShot;
class ObjSound;
class ObjSpell;
class ObjSpellManage;
class ObjSprite2D;
class ObjSprite3D;
class ObjSpriteList2D;
class ObjStLaser;
class ObjText;
class ObjectLayerList;
class ObjectTable;
class RandGenerator;
class RenderTarget;
class Renderer;
class ReplayData;
class Script;
class ScriptInfo;
class SerializedScriptStore;
class ScriptManager;
class Shader;
class ShotCounter;
class ShotData;
class ShotDataTable;
class SoundBuffer;
class SoundBuffer;
class SoundStreamBuffer;
class SoundDevice;
class StageCommonPlayerParams;
class Texture;
class TextureStore;
class TimePoint;
class VirtualKeyInputSource;
class EngineDevelopOptions;
struct SourcePos;

namespace conf { struct KeyConfig; }

class Package : public std::enable_shared_from_this<Package>
{
public:
    Package(HWND hWnd,
            int screenWidth,
            int screenHeight,
            const std::wstring packageMainScriptPath,
            const std::shared_ptr<conf::KeyConfig>& keyConfig,
            const std::shared_ptr<GraphicDevice>& graphicDevice,
            const std::shared_ptr<InputDevice>& inputDevice,
            const std::shared_ptr<FpsCounter>& fpsCounter,
            const std::shared_ptr<LostableGraphicResourceManager>& lostableGraphicResourceManager,
            const std::shared_ptr<EngineDevelopOptions>& engineDevelopOptions);
    ~Package();

    /* package control */
    void TickFrame();
    void Render(); // render to backbuffer
    void Render(const std::wstring& renderTargetName);
    void RenderToTextureA1(const std::wstring& renderTargetName, int begin, int end, bool doClear);
    void RenderToTextureB1(const std::wstring& renderTargetName, int objId, bool doClear);
    int GetScreenWidth() const;
    int GetScreenHeight() const;

    /* input */
    KeyState GetKeyState(Key k);
    KeyState GetVirtualKeyState(VirtualKey vk, bool isStgScene = true) const;
    void SetVirtualKeyState(VirtualKey vk, KeyState state);
    void AddVirtualKey(VirtualKey vk, Key k, PadButton btn);
    bool IsAddedVirtualKey(VirtualKey vk) const;
    KeyState GetMouseState(MouseButton btn);
    int GetMouseX();
    int GetMouseY();
    int GetMouseMoveZ();

    /* logging function */
    void WriteLog(const std::string&& msg, const std::shared_ptr<SourcePos>& srcPos);

    /* time */
    std::wstring GetCurrentDateTimeS();
    float GetCurrentFps() const;
    float GetStageTime() const;
    float GetPackageTime() const;
    void StartSlow(int pseudoFps, bool byPlayer);
    void StopSlow(bool byPlayer);
    int GetElapsedFrame() const;

    /* path */
    std::wstring GetMainStgScriptPath() const;
    std::wstring Package::GetMainStgScriptDirectory() const;
    std::wstring GetMainPackageScriptPath() const;
    // スクリプトタイプがPackageの場合はMainPackageScript、それ以外の場合はMainStgScript
    std::wstring GetMainScriptPath() const;

    /* texture */
    const std::shared_ptr<Texture>& LoadTexture(const std::wstring & path);
    void LoadTextureInThread(const std::wstring & path) noexcept(true);
    void SetTextureReserveFlag(const std::wstring & path, bool reserve);
    void RemoveUnusedTexture();

    /* font */
    const std::shared_ptr<Font>& CreateFont(const FontParams& param);
    void RemoveUnusedFont();
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
    const std::shared_ptr<Mesh>& LoadMesh(const std::wstring& path);
    void RemoveUnusedMesh();

    /* sound */
    std::shared_ptr<SoundBuffer> LoadSound(const std::wstring& path);
	void LoadOrphanSound(const std::wstring& path);
	void RemoveOrphanSound(const std::wstring& path);
	void PlayBGM(const std::wstring& path, double loopStartSec, double loopEndSec);
	void PlaySE(const std::wstring& path);
	void StopOrphanSound(const std::wstring& path);
    std::shared_ptr<SoundStreamBuffer> LoadSoundStream(const std::wstring& path);
    void LoadOrphanSoundStream(const std::wstring& path);
    void RemoveOrphanSoundStream(const std::wstring& path);
    void StreamBGM(const std::wstring& path, double loopStartSec, double loopEndSec);
    void StreamSE(const std::wstring& path);
    void StopOrphanSoundStream(const std::wstring& path);

    /* layer */
    void SetObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority);
    void SetLayerShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader);
    NullableSharedPtr<Shader> GetLayerShader(int p) const;
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
    std::wstring GetDefaultCommonDataSavePath(const std::wstring& areaName) const;

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
        std::vector<std::shared_ptr<T>> objs;
        for (const auto& entry : GetObjAll())
        {
            if (auto obj = std::dynamic_pointer_cast<T>(entry.second))
            {
                objs.push_back(obj);
            }
        }
        return objs;
    }
    void DeleteObject(int id);
    bool IsObjectDeleted(int id) const;

    NullableSharedPtr<ObjPlayer> GetPlayerObject() const;
    NullableSharedPtr<ObjEnemy> GetEnemyBossObject() const;
    NullableSharedPtr<ObjEnemyBossScene> GetEnemyBossSceneObject() const;
    NullableSharedPtr<ObjSpellManage> GetSpellManageObject() const;
    void GenerateSpellManageObject();

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
    std::shared_ptr<ObjItem> CreateObjItem(int itemType);
    void GenerateBonusItem(float x, float y);
    void GenerateItemScoreText(float x, float y, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemA1(int itemType, float x, float y, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemA2(int itemType, float x, float y, float destX, float destY, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemU1(int itemDataId, float x, float y, PlayerScore score);
    std::shared_ptr<ObjItem> CreateItemU2(int itemDataId, float x, float y, float destX, float destY, PlayerScore score);
    std::shared_ptr<ObjEnemy> CreateObjEnemy();
    std::shared_ptr<ObjEnemy> CreateObjEnemyBoss();
    std::shared_ptr<ObjEnemyBossScene> CreateObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<ObjSpell> CreateObjSpell();

    /* script */
    NullableSharedPtr<Script> GetScript(int scriptId) const;
    std::shared_ptr<Script> LoadScript(const std::wstring& path, ScriptType type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    std::shared_ptr<Script> LoadScriptInThread(const std::wstring& path, ScriptType type, const std::wstring& version, const std::shared_ptr<SourcePos>& srcPos);
    void CloseStgScene();
    void NotifyEventAll(int eventType);
    void NotifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    const std::unique_ptr<DnhValue>& GetScriptResult(int scriptId) const;
    void SetScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    std::vector<ScriptInfo> GetScriptList(const std::wstring& dirPath, ScriptType scriptType, bool doRecursive, bool getAll);
    void GetLoadFreePlayerScriptList();
    int GetFreePlayerScriptCount() const;
    ScriptInfo GetFreePlayerScriptInfo(int idx) const;
    ScriptInfo GetScriptInfo(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);

    /* common player params */
    PlayerLife GetPlayerLife() const;
    PlayerSpell GetPlayerSpell() const;
    PlayerPower GetPlayerPower() const;
    PlayerScore GetPlayerScore() const;
    PlayerGraze GetPlayerGraze() const;
    PlayerPoint GetPlayerPoint() const;
    void SetPlayerLife(PlayerLife life);
    void SetPlayerSpell(PlayerSpell spell);
    void SetPlayerPower(PlayerPower power);
    void SetPlayerScore(PlayerScore score);
    void SetPlayerGraze(PlayerGraze graze);
    void SetPlayerPoint(PlayerPoint point);

    /* player */
    std::wstring GetPlayerID() const;
    std::wstring GetPlayerReplayName() const;
    NullableSharedPtr<Script> GetPlayerScript() const;

    /* stg frame */
    void SetStgFrame(float left, float top, float right, float bottom);
    float GetStgFrameLeft() const;
    float GetStgFrameTop() const;
    float GetStgFrameRight() const;
    float GetStgFrameBottom() const;
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
    void SuccPlayerShotCount();
    void SuccEnemyShotCount();
    void PredPlayerShotCount();
    void PredEnemyShotCount();

    /* shot auto delete clip */
    void SetShotAutoDeleteClip(float left, float top, float right, float bottom);
    bool IsOutOfShotAutoDeleteClip(float x, float y) const;
    void StartShotScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    NullableSharedPtr<Script> GetShotScript() const;
    void SetDeleteShotImmediateEventOnShotScriptEnable(bool enable);
    void SetDeleteShotFadeEventOnShotScriptEnable(bool enable);
    void SetDeleteShotToItemEventOnShotScriptEnable(bool enable);
    bool IsDeleteShotImmediateEventOnShotScriptEnabled() const;
    bool IsDeleteShotFadeEventOnShotScriptEnabled() const;
    bool IsDeleteShotToItemEventOnShotScriptEnabled() const;
    void DeleteShotAll(int tarGet, int behavior);
    void DeleteShotInCircle(int tarGet, int behavior, float x, float y, float r);
    std::vector<std::shared_ptr<ObjShot>> GetShotInCircle(float x, float y, float r, int tarGet) const;
    void SetShotIntersectoinCicle(float x, float y, float r);
    void SetShotIntersectoinLine(float x1, float y1, float x2, float y2, float width);

    /* item */
    void CollectAllItems();
    void CollectItemsByType(int itemType);
    void CollectItemsInCircle(float x, float y, float r);
    void CancelCollectItems();
    bool IsAutoCollectCanceled() const;
    bool IsAutoCollectTarget(int itemType, float x, float y) const;
    void SetDefaultBonusItemEnable(bool enable);
    bool IsDefaultBonusItemEnabled() const;
    void StartItemScript(const std::wstring& path, const std::shared_ptr<SourcePos>& srcPos);
    NullableSharedPtr<Script> GetItemScript() const;

    /* package */
    bool IsClosed() const;
    void Close();
    void Finalize(); // エラー時は呼ばない
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
    void Start();

    /* replay */
    bool IsReplay() const;

    /* for default package */
    void SetPauseScriptPath(const std::wstring& path);
    void SetEndSceneScriptPath(const std::wstring& path);
    void SetReplaySaveSceneScriptPath(const std::wstring& path);

    /* etc */
    Point2D Get2DPosition(float x, float y, float z, bool isStgScene);
    double GetRandDouble(double min, double max);

    const std::shared_ptr<EngineDevelopOptions>& GetEngineDevelopOptions() const;

    /* backdoor */
    template <typename T>
    void backDoor() {}

    /* API用, API以外から取得しないこと */
    static Package* Current;
private:
    void RenderToTexture(const std::wstring& renderTargetName, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag);
    NullableSharedPtr<Obj> GetObj(int id) const;
    const std::map<int, std::shared_ptr<Obj>>& GetObjAll() const;

    const HWND hWnd_;

    const int screenWidth_;
    const int screenHeight_;

    const std::shared_ptr<GraphicDevice> graphicDevice_;
    const std::shared_ptr<InputDevice> inputDevice_;
    const std::shared_ptr<FpsCounter> fpsCounter_;
    const std::shared_ptr<LostableGraphicResourceManager> lostableGraphicResourceManager_;
    const std::shared_ptr<EngineDevelopOptions> engineDevelopOptions_;

    std::unordered_map<std::wstring, std::shared_ptr<RenderTarget>> renderTargets_;
    std::unordered_map<VirtualKey, std::pair<Key, PadButton>> virtualKeyAssign_; // AddVirtualKeyの追加先
    std::unordered_set<VirtualKey> replayTargetVirtualKeys_;
    std::shared_ptr<FileLoader> fileLoader_;
    std::shared_ptr<SoundDevice> soundDevice;
    std::unordered_map <std::wstring, std::shared_ptr<SoundBuffer>> orphanSounds_;
    std::unordered_map <std::wstring, std::shared_ptr<SoundStreamBuffer>> orphanSoundsStream_;
    std::shared_ptr<Renderer> renderer_;
    std::shared_ptr<ObjectTable> objTable_;
    std::shared_ptr<ObjectLayerList> objLayerList_;
    std::shared_ptr<CollisionDetector> colDetector_;
    std::vector<std::shared_ptr<Intersection>> tempEnemyShotIsects_;
    std::shared_ptr<TextureStore> textureStore_;
    std::shared_ptr<MeshStore> meshStore_;
    std::shared_ptr<Camera2D> camera2D_;
    std::shared_ptr<Camera3D> camera3D_;
    std::shared_ptr<CommonDataDB> commonDataDB_;
    std::shared_ptr<SerializedScriptStore> serializedScriptStore_;
    std::shared_ptr<ScriptManager> scriptManager_;

    std::shared_ptr<ShotDataTable> playerShotDataTable_;
    std::shared_ptr<ShotDataTable> enemyShotDataTable_;
    std::shared_ptr<ItemDataTable> itemDataTable_;

    std::weak_ptr<ObjPlayer> playerObj_;
    std::weak_ptr<ObjEnemyBossScene> enemyBossSceneObj_;
    std::weak_ptr<ObjSpellManage> spellManageObj_;

    std::shared_ptr<ShotCounter> shotCounter_;
    std::shared_ptr<RandGenerator> randGenerator_;
    std::shared_ptr<AutoItemCollectionManager> autoItemCollectionManager_;

    int elapsedFrame_;
    int stageElapesdFrame_;

    std::shared_ptr<TimePoint> stageStartTime_;
    std::vector<ScriptInfo> freePlayerScriptInfoList_;

    int pseudoPlayerFps_;
    int pseudoEnemyFps_;

    const ScriptInfo packageMainScriptInfo_;
    ScriptInfo stageMainScriptInfo_;
    ScriptInfo stagePlayerScriptInfo_;

    std::weak_ptr<Script> packageMainScript_;
    std::weak_ptr<Script> stageMainScript_;
    std::weak_ptr<Script> stagePlayerScript_;
    std::weak_ptr<Script> shotScript_;
    std::weak_ptr<Script> itemScript_;

    std::unordered_map<VirtualKey, KeyState> virtualKeyStates_;
    std::unordered_map<VirtualKey, KeyState> replayVirtualKeyStates_;

    std::wstring stageReplayFilePath_;
    StageIndex stageIdx_;
    int stageSceneResult_;
    bool isStagePaused_;
    bool isStageForceTerminated_;

    bool isReplay_;
    std::shared_ptr<ReplayData> replayData_;

    bool deleteShotImmediateEventOnShotScriptEnable_;
    bool deleteShotFadeEventOnShotScriptEnable_;
    bool deleteShotToItemEventOnShotScriptEnable_;
    bool defaultBonusItemEnable_;

    Rect<float> stgFrame_;
    Rect<float> shotAutoDeleteClip_;
    const std::shared_ptr<FontStore> fontStore_;
    StageCommonPlayerParams stageCommonPlayerParams_;

    std::shared_ptr<TimePoint> packageStartTime_;
};
}