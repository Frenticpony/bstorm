#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <windows.h>

#include <bstorm/type.hpp>
#include <bstorm/script_info.hpp>

struct IDirect3DDevice9;

namespace bstorm {
  class Logger;
  class FpsCounter;
  class TimePoint;
  class KeyConfig;
  class InputDevice;
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
  class MeshCache;
  class Camera2D;
  class Camera3D;
  class CommonDataDB;
  class Script;
  class ScriptManager;
  class GlobalPlayerParams;
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
  class GameState {
  public:
    GameState(int screenWidth, int screenHeight, HWND hWnd, IDirect3DDevice9* d3DDevice, const std::shared_ptr<Logger>& logger, const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<KeyConfig>& keyConfig, const std::shared_ptr<int>& screenPosX, const std::shared_ptr<int>& screenPosY, const std::shared_ptr<int>& gameViewWidth, const std::shared_ptr<int>& gameViewHeight, Engine* engine);
    std::shared_ptr<Logger> logger;
    std::shared_ptr<FpsCounter> fpsCounter;
    std::shared_ptr<InputDevice> inputDevice;
    std::shared_ptr<SoundDevice> soundDevice;
    std::unordered_map <std::wstring, std::shared_ptr<SoundBuffer>> orphanSounds;
    std::shared_ptr<Renderer> renderer;
    std::shared_ptr<ObjectTable> objTable;
    std::shared_ptr<ObjectLayerList> objLayerList;
    std::shared_ptr<CollisionDetector> colDetector;
    std::vector<std::shared_ptr<Intersection>> tempEnemyShotIsects;
    std::shared_ptr<TextureCache> textureCache;
    std::shared_ptr<FontCache> fontCache;
    std::shared_ptr<MeshCache> meshCache;
    std::shared_ptr<Camera2D> camera2D;
    std::shared_ptr<Camera3D> camera3D;
    std::shared_ptr<CommonDataDB> commonDataDB;
    std::shared_ptr<ScriptManager> scriptManager;
    std::shared_ptr<FileLoader> fileLoader;
    std::shared_ptr<ShotDataTable> playerShotDataTable;
    std::shared_ptr<ShotDataTable> enemyShotDataTable;
    std::shared_ptr<ItemDataTable> itemDataTable;
    std::shared_ptr<GlobalPlayerParams> globalPlayerParams;
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
    int64_t elapsedFrame;
    std::shared_ptr<TimePoint> packageStartTime;
    std::shared_ptr<TimePoint> stageStartTime;
    std::vector<ScriptInfo> freePlayerScriptInfoList;
    int pseudoPlayerFps;
    int pseudoEnemyFps;
    ScriptInfo packageMainScriptInfo;
    ScriptInfo stageMainScriptInfo;
    ScriptInfo stagePlayerScriptInfo;
    std::weak_ptr<Script> packageMainScript;
    std::weak_ptr<Script> stageMainScript;
    std::weak_ptr<Script> stagePlayerScript;
    std::weak_ptr<Script> shotScript;
    std::weak_ptr<Script> itemScript;
    std::wstring stageReplayFilePath;
    uint32_t stageIdx;
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
  };
}