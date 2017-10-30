#pragma once

#include <memory>
#include <string>
#include <map>
#include <unordered_map>
#include <windows.h>
#include <d3d9.h>

#include <bstorm/type.hpp>

namespace bstorm {
  class GraphicDevice;
  class LostableGraphicResource;
  class LostableGraphicResourceManager;
  class Logger;
  class KeyConfig;
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
  class GameState;
  class Engine {
  public:
    Engine(HWND hWnd, int screenWidth, int screenHeight, const std::shared_ptr<Logger>& logger, const std::shared_ptr<KeyConfig>& masterKeyConfig);
    virtual ~Engine();
    void addLostableGraphicResource(const std::shared_ptr<LostableGraphicResource>& resource);
    template <class T, class... Args>
    std::shared_ptr<LostableGraphicResource> createLostableGraphicResource(Args... args) {
      auto resource = std::make_shared<T>(args...);
      addLostableGraphicResource(resource);
      return resource;
    }
    HWND getWindowHandle() const;
    /* engine control */
    bool tickFrame();
    void render(); // render to backbuffer
    void render(const std::wstring& renderTargetName);
    void renderToTextureA1(const std::wstring& renderTargetName, int begin, int end, bool doClear);
    void renderToTextureB1(const std::wstring& renderTargetName, int objId, bool doClear);
    // NOTE : resetに失敗したらEngineを終了すべき
    void reset(int screenWidth, int screenHeight);
    int getScreenWidth() const;
    int getScreenHeight() const;
    void setScreenPos(const std::shared_ptr<int>& posX, const std::shared_ptr<int>& posY);
    void setGameViewSize(const std::shared_ptr<int>& width, const std::shared_ptr<int>& height);
    /* graphic device control */
    IDirect3DDevice9* getGraphicDevice() const;
    // should call only to reset device
    void resetGraphicDevice();
    void releaseLostableGraphicResource();
    void restoreLostableGraphicDevice();
    // set backbuffer to render-target
    void setBackBufferRenderTarget();
    void releaseUnusedLostableGraphicResource();
    /* input */
    KeyState getKeyState(Key k);
    KeyState getVirtualKeyState(VirtualKey vk);
    void setVirtualKeyState(VirtualKey vk, KeyState state);
    void addVirtualKey(VirtualKey vk, Key k, PadButton btn);
    KeyState getMouseState(MouseButton btn);
    int getMouseX();
    int getMouseY();
    int getMouseMoveZ();
    /* logging function */
    void logInfo(const std::string& msg);
    void logInfo(const std::wstring& msg);
    void logWarn(const std::string& msg);
    void logWarn(const std::wstring& msg);
    void logError(const std::string& msg);
    void logError(const std::wstring& msg);
    void logDebug(const std::string& msg);
    void logDebug(const std::wstring& msg);
    void writeLog(const std::wstring& msg);
    /* time */
    std::wstring getCurrentDateTimeS();
    double getCurrentFps() const;
    double getStageTime() const;
    double getPackageTime() const;
    void updateFpsCounter();
    void resetFpsCounter();
    void startSlow(int pseudoFps, bool byPlayer);
    void stopSlow(bool byPlayer);
    int64_t getElapsedFrame() const;
    /* path */
    std::wstring getMainStgScriptPath() const;
    std::wstring Engine::getMainStgScriptDirectory() const;
    std::wstring getMainPackageScriptPath() const;
    /* texture */
    std::shared_ptr<Texture> loadTexture(const std::wstring& path, bool reserve);
    void removeTextureReservedFlag(const std::wstring& path);
    void releaseUnusedTextureCache();
    /* font */
    void releaseUnusedFontCache();
    /* render target */
    std::shared_ptr<RenderTarget> createRenderTarget(const std::wstring& name, int width, int height);
    void removeRenderTarget(const std::wstring& name);
    std::shared_ptr<RenderTarget> getRenderTarget(const std::wstring& name) const;
    std::wstring getReservedRenderTargetName(int idx) const;
    std::wstring getTransitionRenderTargetName() const;
    void saveRenderedTextureA1(const std::wstring& name, const std::wstring& path);
    void saveRenderedTextureA2(const std::wstring& name, const std::wstring& path, int left, int top, int right, int bottom);
    void saveSnapShotA1(const std::wstring& path);
    void saveSnapShotA2(const std::wstring& path, int left, int top, int right, int bottom);
    /* shader */
    std::shared_ptr<Shader> createShader(const std::wstring& path, bool precompiled);
    bool isPixelShaderSupported(int major, int minor);
    /* mesh */
    std::shared_ptr<Mesh> loadMesh(const std::wstring& path);
    void releaseUnusedMeshCache();
    /* sound */
    void loadSound(const std::wstring& path);
    void removeSound(const std::wstring& path);
    void playBGM(const std::wstring& path, double loopStartSec, double loopEndSec);
    void playSE(const std::wstring& path);
    void stopSound(const std::wstring& path);
    void cacheSound(const std::wstring& path);
    void removeSoundCache(const std::wstring& path);
    void clearSoundCache();
    /* layer */
    void setObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority);
    void setShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader);
    void resetShader(int beginPriority, int endPriority);
    int getStgFrameRenderPriorityMin() const;
    void setStgFrameRenderPriorityMin(int p);
    int getStgFrameRenderPriorityMax() const;
    void setStgFrameRenderPriorityMax(int p);
    int getShotRenderPriority() const;
    void setShotRenderPriority(int p);
    int getItemRenderPriority() const;
    void setItemRenderPriority(int p);
    int getPlayerRenderPriority() const;
    int getCameraFocusPermitRenderPriority() const;
    void setInvalidRenderPriority(int min, int max);
    void clearInvalidRenderPriority();
    std::shared_ptr<Shader> getShader(int p) const;
    /* renderer */
    void setFogEnable(bool enable);
    void setFogParam(float fogStart, float fogEnd, int r, int g, int b);
    /* 3D camera */
    void setCameraFocusX(float x);
    void setCameraFocusY(float y);
    void setCameraFocusZ(float z);
    void setCameraFocusXYZ(float x, float y, float z);
    void setCameraRadius(float r);
    void setCameraAzimuthAngle(float angle);
    void setCameraElevationAngle(float angle);
    void setCameraYaw(float yaw);
    void setCameraPitch(float pitch);
    void setCameraRoll(float roll);
    void resetCamera();
    float getCameraX() const;
    float getCameraY() const;
    float getCameraZ() const;
    float getCameraFocusX() const;
    float getCameraFocusY() const;
    float getCameraFocusZ() const;
    float getCameraRadius() const;
    float getCameraAzimuthAngle() const;
    float getCameraElevationAngle() const;
    float getCameraYaw() const;
    float getCameraPitch() const;
    float getCameraRoll() const;
    void setCameraPerspectiveClip(float nearClip, float farClip);
    /* 2D camera */
    void set2DCameraFocusX(float x);
    void set2DCameraFocusY(float y);
    void set2DCameraAngleZ(float z);
    void set2DCameraRatio(float r);
    void set2DCameraRatioX(float x);
    void set2DCameraRatioY(float x);
    void reset2DCamera();
    float get2DCameraX() const;
    float get2DCameraY() const;
    float get2DCameraAngleZ() const;
    float get2DCameraRatio() const;
    float get2DCameraRatioX() const;
    float get2DCameraRatioY() const;
    /* common data */
    void setCommonData(const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> getCommonData(const std::wstring& key, std::unique_ptr<DnhValue>&& defaultValue) const;
    void clearCommonData();
    void deleteCommonData(const std::wstring& key);
    void setAreaCommonData(const std::wstring& areaName, const std::wstring& key, std::unique_ptr<DnhValue>&& value);
    std::unique_ptr<DnhValue> getAreaCommonData(const std::wstring& areaName, const std::wstring& key, std::unique_ptr<DnhValue>&& value) const;
    void clearAreaCommonData(const std::wstring& areaName);
    void deleteAreaCommonData(const std::wstring& areaName, const std::wstring& key);
    void createCommonDataArea(const std::wstring& areaName);
    bool isCommonDataAreaExists(const std::wstring& areaName) const;
    void copyCommonDataArea(const std::wstring& dest, const std::wstring& src);
    std::vector<std::wstring> getCommonDataAreaKeyList() const;
    std::vector<std::wstring> getCommonDataValueKeyList(const std::wstring& areaName) const;
    bool saveCommonDataAreaA1(const std::wstring& areaName) const;
    bool loadCommonDataAreaA1(const std::wstring& areaName);
    bool saveCommonDataAreaA2(const std::wstring& areaName, const std::wstring& path) const;
    bool loadCommonDataAreaA2(const std::wstring& areaName, const std::wstring& path);
    std::wstring Engine::getDefaultCommonDataSavePath(const std::wstring& areaName) const;
    /* user def data */
    void loadPlayerShotData(const std::wstring& path);
    void reloadPlayerShotData(const std::wstring& path);
    void loadEnemyShotData(const std::wstring& path);
    void reloadEnemyShotData(const std::wstring& path);
    void loadItemData(const std::wstring& path);
    void reloadItemData(const std::wstring& path);
    std::shared_ptr<ShotData> getPlayerShotData(int id) const;
    std::shared_ptr<ShotData> getEnemyShotData(int id) const;
    std::shared_ptr<ItemData> getItemData(int id) const;
    /* object */
    template <class T>
    std::shared_ptr<T> getObject(int id) const {
      return std::dynamic_pointer_cast<T>(getObj(id));
    };

    template <class T>
    std::vector <std::shared_ptr<T>> getObjectAll() const {
      std::vector <std::shared_ptr<T>> objs;
      for (const auto& entry : getObjAll()) {
        if (auto obj = std::dynamic_pointer_cast<T>(entry.second)) {
          objs.push_back(obj);
        }
      }
      return objs;
    }
    std::shared_ptr<ObjPlayer> getPlayerObject() const;
    std::shared_ptr<ObjEnemy> getEnemyBossObject() const;
    std::shared_ptr<ObjEnemyBossScene> getEnemyBossSceneObject() const;
    std::shared_ptr<ObjSpellManage> getSpellManageObject() const;
    void deleteObject(int id);
    bool isObjectDeleted(int id) const;
    std::shared_ptr<ObjPrim2D> createObjPrim2D();
    std::shared_ptr<ObjSprite2D> createObjSprite2D();
    std::shared_ptr<ObjSpriteList2D> createObjSpriteList2D();
    std::shared_ptr<ObjPrim3D> createObjPrim3D();
    std::shared_ptr<ObjSprite3D> createObjSprite3D();
    std::shared_ptr<ObjMesh> createObjMesh();
    std::shared_ptr<ObjText> createObjText();
    std::shared_ptr<ObjSound> createObjSound();
    std::shared_ptr<ObjFileT> createObjFileT();
    std::shared_ptr<ObjFileB> createObjFileB();
    std::shared_ptr<ObjShader> createObjShader();
    std::shared_ptr<ObjShot> createObjShot(bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotA1(float x, float y, float speed, float angle, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotA2(float x, float y, float speed, float angle, float accel, float maxSpeed, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotB1(float x, float y, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotB2(float x, float y, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjShot> createPlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId);
    std::shared_ptr<ObjLooseLaser> createObjLooseLaser(bool isPlayerShot);
    std::shared_ptr<ObjLooseLaser> createLooseLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjStLaser> createObjStLaser(bool isPlayerShot);
    std::shared_ptr<ObjStLaser> createStraightLaserA1(float x, float y, float angle, float length, float width, int deleteFrame, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjCrLaser> createObjCrLaser(bool isPlayerShot);
    std::shared_ptr<ObjCrLaser> createCurveLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot);
    std::shared_ptr<ObjItem> createItemA1(int itemType, float x, float y, int64_t score);
    std::shared_ptr<ObjItem> createItemA2(int itemType, float x, float y, float destX, float destY, int64_t score);
    std::shared_ptr<ObjItem> createItemU1(int itemDataId, float x, float y, int64_t score);
    std::shared_ptr<ObjItem> createItemU2(int itemDataId, float x, float y, float destX, float destY, int64_t score);
    std::shared_ptr<ObjEnemy> createObjEnemy();
    std::shared_ptr<ObjEnemyBossScene> createObjEnemyBossScene();
    std::shared_ptr<ObjSpell> createObjSpell();
    std::shared_ptr<ObjItem> createObjItem(int itemType);
    /* script */
    std::shared_ptr<Script> getScript(int scriptId) const;
    std::shared_ptr<Script> loadScript(const std::wstring& path, const std::wstring& type, const std::wstring& version);
    void closeStgScene();
    void notifyEventAll(int eventType);
    void notifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args);
    std::wstring getPlayerID() const;
    std::wstring getPlayerReplayName() const;
    std::shared_ptr<Script> getPlayerScript() const;
    std::unique_ptr<DnhValue> getScriptResult(int scriptId) const;
    void setScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value);
    std::vector<ScriptInfo> getScriptList(const std::wstring& dirPath, int scriptType, bool doRecursive);
    void getLoadFreePlayerScriptList();
    int getFreePlayerScriptCount() const;
    ScriptInfo getFreePlayerScriptInfo(int idx) const;
    ScriptInfo getScriptInfo(const std::wstring& path);
    /* point */
    int64_t getScore() const;
    void addScore(int64_t score);
    int64_t getGraze() const;
    void addGraze(int64_t graze);
    int64_t getPoint() const;
    void addPoint(int64_t point);
    /* stg frame */
    void setStgFrame(float left, float top, float right, float bottom);
    float getStgFrameLeft() const;
    float getStgFrameTop() const;
    float getStgFrameWidth() const;
    float getStgFrameHeight() const;
    float getStgFrameCenterWorldX() const;
    float getStgFrameCenterWorldY() const;
    float getStgFrameCenterScreenX() const;
    float getStgFrameCenterScreenY() const;
    /* shot */
    int getAllShotCount() const;
    int getEnemyShotCount() const;
    int getPlayerShotCount() const;
    void setShotAutoDeleteClip(float left, float top, float right, float bottom);
    void startShotScript(const std::wstring& path);
    void setDeleteShotImmediateEventOnShotScriptEnable(bool enable);
    void setDeleteShotFadeEventOnShotScriptEnable(bool enable);
    void setDeleteShotToItemEventOnShotScriptEnable(bool enable);
    void deleteShotAll(int target, int behavior) const;
    void deleteShotInCircle(int target, int behavior, float x, float y, float r) const;
    std::vector<std::shared_ptr<ObjShot>> getShotInCircle(float x, float y, float r, int target) const;
    void setShotIntersectoinCicle(float x, float y, float r);
    void setShotIntersectoinLine(float x1, float y1, float x2, float y2, float width);
    /* item */
    void collectAllItems();
    void collectItemsByType(int itemType);
    void collectItemsInCircle(float x, float y, float r);
    void cancelCollectItems();
    void setDefaultBonusItemEnable(bool enable);
    void startItemScript(const std::wstring& path);
    /* package */
    bool isPackageFinished() const;
    void closePackage();
    void initializeStageScene();
    void finalizeStageScene();
    void startStageScene();
    void setStageIndex(uint16_t idx);
    void setStageMainScript(const std::wstring& path);
    void setStageMainScript(const ScriptInfo& script);
    void setStagePlayerScript(const std::wstring& path);
    void setStagePlayerScript(const ScriptInfo& script);
    void setStageReplayFile(const std::wstring& path);
    bool isStageFinished() const;
    int getStageSceneResult() const;
    bool isStagePaused() const;
    void pauseStageScene(bool doPause);
    void terminateStageScene();
    void setPackageMainScript(const std::wstring& path);
    void setPackageMainScript(const ScriptInfo& script);
    void startPackage();
    /* for default package */
    void setPauseScriptPath(const std::wstring& path);
    void setEndSceneScriptPath(const std::wstring& path);
    void setReplaySaveSceneScriptPath(const std::wstring& path);
    /* etc */
    Point2D get2DPosition(float x, float y, float z, bool isStgScene);
    /* backdoor */
    template <typename T>
    void backDoor() {}
  protected:
    void renderToTexture(const std::wstring& renderTargetName, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag);
    std::shared_ptr<Obj> getObj(int id) const;
    const std::map<int, std::shared_ptr<Obj>>& getObjAll() const;
    HWND hWnd;
    std::unique_ptr<GraphicDevice> graphicDevice;
    std::unique_ptr<LostableGraphicResourceManager> lostableGraphicResourceManager;
    std::shared_ptr<int> screenPosX;
    std::shared_ptr<int> screenPosY;
    std::shared_ptr<int> gameViewWidth;
    std::shared_ptr<int> gameViewHeight;
    std::shared_ptr<Logger> logger;
    std::shared_ptr<KeyConfig> masterKeyConfig;
    std::shared_ptr<Renderer> renderer;
    std::unordered_map<std::wstring, std::shared_ptr<RenderTarget>> renderTargets;
    std::shared_ptr<GameState> gameState;
  };
}