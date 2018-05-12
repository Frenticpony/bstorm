#pragma once

#include <bstorm/type.hpp>
#include <bstorm/font.hpp>
#include <bstorm/animation.hpp>
#include <bstorm/stage_common_player_params.hpp>
#include <bstorm/script_info.hpp>

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <windows.h>
#include <d3d9.h>

#undef CreateFont

namespace bstorm
{
class FpsCounter;
class TimePoint;
class MousePositionProvider;
class InputDevice;
class VirtualKeyInputSource;
class KeyAssign;
class SoundDevice;
class SoundBuffer;
class Renderer;
class ObjectTable;
class ObjectLayerList;
class CollisionDetector;
class Intersection;
class FileLoader;
class TextureCache;
class FontCache;
class Font;
class FontParams;
class MeshCache;
class Camera2D;
class Camera3D;
class CommonDataDB;
class Script;
class ScriptManager;
class StageCommonPlayerParams;
class ObjPlayer;
class ObjEnemyBossScene;
class ObjSpellManage;
class ShotDataTable;
class ItemDataTable;
class ShotCounter;
class AutoDeleteClip;
class RandGenerator;
class ItemScoreTextSpawner;
class DefaultBonusItemSpawner;
class AutoItemCollectionManager;
class Engine;
namespace conf { struct KeyConfig; }
class Package
{
public:
    Package(int screenWidth, int screenHeight, HWND hWnd, IDirect3DDevice9* d3DDevice, const std::shared_ptr<conf::KeyConfig>& keyConfig, const std::shared_ptr<MousePositionProvider>& mousePosProvider, Engine* engine);
    std::shared_ptr<FpsCounter> fpsCounter;
    std::shared_ptr<InputDevice> inputDevice;
    std::shared_ptr<KeyAssign> keyAssign;
    std::shared_ptr<FileLoader> fileLoader;
    std::shared_ptr<VirtualKeyInputSource> vKeyInputSource;
    std::shared_ptr<SoundDevice> soundDevice;
    std::unordered_map <std::wstring, std::shared_ptr<SoundBuffer>> orphanSounds;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<ObjectTable> objTable;
    std::shared_ptr<ObjectLayerList> objLayerList;
    std::shared_ptr<CollisionDetector> colDetector;
    std::vector<std::shared_ptr<Intersection>> tempEnemyShotIsects;
    std::shared_ptr<TextureCache> textureCache;
    std::shared_ptr<MeshCache> meshCache;
    std::shared_ptr<Camera2D> camera2D;
    std::shared_ptr<Camera3D> camera3D;
    std::shared_ptr<CommonDataDB> commonDataDB;
    std::shared_ptr<ScriptManager> scriptManager;
    std::shared_ptr<ShotDataTable> playerShotDataTable;
    std::shared_ptr<ShotDataTable> enemyShotDataTable;
    std::shared_ptr<ItemDataTable> itemDataTable;
    std::weak_ptr<ObjPlayer> playerObj;
    std::weak_ptr<ObjEnemyBossScene> enemyBossSceneObj;
    std::weak_ptr<ObjSpellManage> spellManageObj;
    std::shared_ptr<Rect<float>> stgFrame;
    std::shared_ptr<ShotCounter> shotCounter;
    std::shared_ptr<AutoDeleteClip> shotAutoDeleteClip;
    std::shared_ptr<RandGenerator> randGenerator;
    std::shared_ptr<ItemScoreTextSpawner> itemScoreTextSpawner;
    std::shared_ptr<DefaultBonusItemSpawner> defaultBonusItemSpawner;
    std::shared_ptr<AutoItemCollectionManager> autoItemCollectionManager;
    FrameCount elapsedFrame;
    std::shared_ptr<TimePoint> packageStartTime;
    std::shared_ptr<TimePoint> stageStartTime;
    std::vector<ScriptInfo> freePlayerScriptInfoList;
    FrameCount pseudoPlayerFps;
    FrameCount pseudoEnemyFps;
    ScriptInfo packageMainScriptInfo;
    ScriptInfo stageMainScriptInfo;
    ScriptInfo stagePlayerScriptInfo;
    std::weak_ptr<Script> packageMainScript;
    std::weak_ptr<Script> stageMainScript;
    std::weak_ptr<Script> stagePlayerScript;
    std::weak_ptr<Script> shotScript;
    std::weak_ptr<Script> itemScript;
    std::wstring stageReplayFilePath;
    StageIndex stageIdx;
    int stageSceneResult;
    bool stagePaused;
    bool stageForceTerminated;
    const int screenWidth;
    const int screenHeight;
    bool deleteShotImmediateEventOnShotScriptEnable;
    bool deleteShotFadeEventOnShotScriptEnable;
    bool deleteShotToItemEventOnShotScriptEnable;
    bool renderIntersectionEnable;
    bool forcePlayerInvincibleEnable;
    bool defaultBonusItemEnable;

    /* font */
    std::shared_ptr<Font> CreateFont(const FontParams* param);
    void ReleaseUnusedFont();
    const std::unordered_map<FontParams, std::shared_ptr<Font>>& GetFontMap() const;

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
private:
    const std::shared_ptr<FontCache> fontCache_;
    StageCommonPlayerParams stageCommonPlayerParams_;
};
}