#include <exception>
#include <ctime>

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
#include <bstorm/collision_matrix.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/auto_delete_clip.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/script_info.hpp>
#include <bstorm/script.hpp>
#include <bstorm/config.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/engine.hpp>

#undef VK_LEFT
#undef VK_RIGHT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm {
  Engine::Engine(HWND hWnd, int screenWidth, int screenHeight, const std::shared_ptr<conf::KeyConfig>& defaultKeyConfig) :
    hWnd(hWnd),
    graphicDevice(std::make_unique<GraphicDevice>(hWnd)),
    lostableGraphicResourceManager(std::make_unique<LostableGraphicResourceManager>()),
    defaultKeyConfig(defaultKeyConfig),
    renderer(std::make_shared<Renderer>(graphicDevice->getDevice()))
  {
    Logger::WriteLog(Log::Level::LV_INFO, "boot engine.");
    reset(screenWidth, screenHeight);
  }

  Engine::~Engine() {
    Logger::WriteLog(Log::Level::LV_INFO, "shutdown engine.");
  }

  HWND Engine::getWindowHandle() const {
    return hWnd;
  }

  void Engine::tickFrame() {
    if (isPackageFinished()) {
      Logger::WriteLog(Log::Level::LV_WARN, "package is not setted, please select package.");
      return;
    }

    if (auto packageMain = gameState->packageMainScript.lock()) {
      if (packageMain->isClosed()) {
        Logger::WriteLog(Log::Level::LV_INFO, "finish package.");
        gameState->packageMainScript.reset();
        reset(getScreenWidth(), getScreenHeight());
        return;
      }
    }

    if (getElapsedFrame() % (60 / std::min(gameState->pseudoEnemyFps, gameState->pseudoPlayerFps)) == 0) {
      if (auto stageMain = gameState->stageMainScript.lock()) {
        if (stageMain->isClosed()) {
          if (gameState->stageForceTerminated) {
            gameState->stageSceneResult = STAGE_RESULT_BREAK_OFF;
            gameState->globalPlayerParams = std::make_shared<GlobalPlayerParams>();
            gameState->globalPlayerParams->life = 0.0;
          } else if (auto player = getPlayerObject()) {
            gameState->stageSceneResult = player->getState() != STATE_END ? STAGE_RESULT_CLEARED : STAGE_RESULT_PLAYER_DOWN;
          } else {
            gameState->stageSceneResult = STAGE_RESULT_PLAYER_DOWN;
          }
          renderToTextureA1(getTransitionRenderTargetName(), 0, MAX_RENDER_PRIORITY, true);
          gameState->objTable->deleteStgSceneObject();
          gameState->scriptManager->closeStgSceneScript();
          gameState->stageMainScript.reset();
          gameState->stagePlayerScript.reset();
          gameState->stageForceTerminated = false;
          gameState->stagePaused = true;
          gameState->pseudoPlayerFps = gameState->pseudoEnemyFps = 60;
        }
      }
      gameState->scriptManager->cleanClosedScript();

      if (!isStagePaused()) {
        gameState->colDetector->run();
      }

      // SetShotIntersection{Circle, Line}で設定した判定削除
      gameState->tempEnemyShotIsects.clear();

      gameState->inputDevice->updateInputState();

      gameState->scriptManager->runAll(isStagePaused());

      gameState->objTable->updateAll(isStagePaused());

      gameState->autoItemCollectionManager->reset();
    }
    // 使われなくなったリソース開放
    switch (getElapsedFrame() % 1920) {
      case 0:
        releaseUnusedLostableGraphicResource();
        break;
      case 480:
        releaseUnusedTextureCache();
        break;
      case 960:
        releaseUnusedFontCache();
        break;
      case 1440:
        releaseUnusedMeshCache();
        break;
    }
    gameState->elapsedFrame += 1;
  }

  void Engine::render() {
    renderToTexture(L"", 0, MAX_RENDER_PRIORITY, ID_INVALID, true, true, true, true);
  }

  void Engine::render(const std::wstring& renderTargetName) {
    renderToTexture(renderTargetName, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
  }

  void Engine::renderToTextureA1(const std::wstring& name, int begin, int end, bool doClear) {
    renderToTexture(name, begin, end, ID_INVALID, doClear, false, false, false);
  }

  void Engine::renderToTextureB1(const std::wstring& name, int objId, bool doClear) {
    renderToTexture(name, 0, 0, objId, doClear, false, false, false);
  }

  void Engine::reset(int screenWidth, int screenHeight) {
    Logger::SetEnable(false);
    gameState = std::make_shared<GameState>(screenWidth, screenHeight, getWindowHandle(), getGraphicDevice(), renderer, defaultKeyConfig, mousePosProvider, this);
    reset2DCamera();
    resetCamera();

    renderTargets.clear();
    createRenderTarget(getTransitionRenderTargetName(), getScreenWidth(), getScreenHeight(), nullptr);
    createRenderTarget(getReservedRenderTargetName(0), 1024, 512, nullptr);
    createRenderTarget(getReservedRenderTargetName(1), 1024, 512, nullptr);
    createRenderTarget(getReservedRenderTargetName(2), 1024, 512, nullptr);
    renderer->setForbidCameraViewProjMatrix2D(getScreenWidth(), getScreenHeight());
    renderer->setFogEnable(false);
    Logger::SetEnable(true);
  }

  int Engine::getScreenWidth() const {
    return gameState->screenWidth;
  }

  int Engine::getScreenHeight() const {
    return gameState->screenHeight;
  }

  bool Engine::isRenderIntersectionEnabled() const {
    return gameState->renderIntersectionEnable;
  }

  void Engine::setRenderIntersectionEnable(bool enable) {
    gameState->renderIntersectionEnable = enable;
  }

  bool Engine::isForcePlayerInvincibleEnabled() const {
    return gameState->forcePlayerInvincibleEnable;
  }

  void Engine::setForcePlayerInvincibleEnable(bool enable) {
    gameState->forcePlayerInvincibleEnable = enable;
  }

  IDirect3DDevice9* Engine::getGraphicDevice() const {
    return graphicDevice->getDevice();
  }

  void Engine::resetGraphicDevice() {
    graphicDevice->reset();
  }

  void Engine::addLostableGraphicResource(const std::shared_ptr<LostableGraphicResource>& resource) {
    lostableGraphicResourceManager->addResource(resource);
  }

  void Engine::releaseLostableGraphicResource() {
    lostableGraphicResourceManager->onLostDeviceAll();
  }

  void Engine::restoreLostableGraphicDevice() {
    lostableGraphicResourceManager->onResetDeviceAll();
  }

  void Engine::setBackBufferRenderTarget() {
    graphicDevice->setBackbufferRenderTarget();
  }

  void Engine::releaseUnusedLostableGraphicResource() {
    lostableGraphicResourceManager->releaseUnusedResource();
  }

  KeyState Engine::getKeyState(Key k) {
    return gameState->inputDevice->getKeyState(k);
  }

  KeyState Engine::getVirtualKeyState(VirtualKey vk) {
    return gameState->vKeyInputSource->getVirtualKeyState(vk);
  }

  void Engine::setVirtualKeyState(VirtualKey vk, KeyState state) {
    gameState->vKeyInputSource->setVirtualKeyState(vk, state);
  }

  void Engine::addVirtualKey(VirtualKey vk, Key k, PadButton btn) {
    gameState->keyAssign->addVirtualKey(vk, k, btn);
  }

  KeyState Engine::getMouseState(MouseButton btn) {
    return gameState->inputDevice->getMouseState(btn);
  }

  int Engine::getMouseX() {
    return gameState->inputDevice->getMouseX(getScreenWidth(), getScreenHeight());
  }

  int Engine::getMouseY() {
    return gameState->inputDevice->getMouseY(getScreenWidth(), getScreenHeight());
  }

  int Engine::getMouseMoveZ() {
    return gameState->inputDevice->getMouseMoveZ();
  }

  void Engine::setMousePostionProvider(const std::shared_ptr<MousePositionProvider>& provider) {
    mousePosProvider = provider;
    gameState->inputDevice->setMousePositionProvider(mousePosProvider);
  }

  void Engine::setInputEnable(bool enable) {
    gameState->inputDevice->setInputEnable(enable);
  }

  void Engine::writeLog(const std::string && msg, const std::shared_ptr<SourcePos>& srcPos) {
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_USER)
      .setMessage(std::move(msg))
      .addSourcePos(srcPos)));
  }

  std::wstring Engine::getCurrentDateTimeS() {
    time_t now = std::time(nullptr);
    struct tm* local = std::localtime(&now);
    std::string buf(14, '\0');
    sprintf(&buf[0], "%04d%02d%02d%02d%02d%02d", local->tm_year + 1900, local->tm_mon + 1, local->tm_mday, local->tm_hour, local->tm_min, local->tm_sec);
    return toUnicode(buf);
  }

  float Engine::getCurrentFps() const {
    return gameState->fpsCounter->getStable();
  }

  float Engine::getStageTime() const {
    if (isStageFinished()) {
      return 0.0;
    }
    return gameState->stageStartTime->getElapsedMilliSec();
  }

  float Engine::getPackageTime() const {
    return gameState->packageStartTime->getElapsedMilliSec();
  }

  void Engine::updateFpsCounter() {
    gameState->fpsCounter->update();
  }

  void Engine::resetFpsCounter() {
    gameState->fpsCounter = std::make_shared<FpsCounter>();
  }

  void Engine::startSlow(int pseudoFps, bool byPlayer) {
    pseudoFps = constrain(pseudoFps, 1, 60);
    if (byPlayer) {
      gameState->pseudoPlayerFps = pseudoFps;
    } else {
      gameState->pseudoEnemyFps = pseudoFps;
    }
  }

  void Engine::stopSlow(bool byPlayer) {
    if (byPlayer) {
      gameState->pseudoPlayerFps = 60;
    } else {
      gameState->pseudoEnemyFps = 60;
    }
  }

  int64_t Engine::getElapsedFrame() const {
    return gameState->elapsedFrame;
  }

  std::wstring Engine::getMainStgScriptPath() const {
    return gameState->stageMainScriptInfo.path;
  }

  std::wstring Engine::getMainStgScriptDirectory() const {
    return parentPath(gameState->stageMainScriptInfo.path) + L"/";
  }

  std::wstring Engine::getMainPackageScriptPath() const {
    return gameState->packageMainScriptInfo.path;
  }

  std::shared_ptr<Texture> Engine::loadTexture(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) {
    return gameState->textureCache->load(path, reserve, srcPos);
  }

  void Engine::loadTextureInThread(const std::wstring & path, bool reserve, const std::shared_ptr<SourcePos>& srcPos) noexcept(true) {
    gameState->textureCache->loadInThread(path, reserve, srcPos);
  }

  void Engine::removeTextureReservedFlag(const std::wstring & path) {
    gameState->textureCache->removeReservedFlag(path);
  }

  void Engine::releaseUnusedTextureCache() {
    gameState->textureCache->releaseUnusedTexture();
  }

  void Engine::releaseUnusedFontCache() {
    gameState->fontCache->releaseUnusedFont();
  }

  bool Engine::installFont(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    bool result = bstorm::installFont(path);
    if (!result) {
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("failed to install font.")
        .setParam(Log::Param(Log::Param::Tag::TEXT, path))
        .addSourcePos(srcPos)));
    }
    return result;
  }

  std::shared_ptr<RenderTarget> Engine::createRenderTarget(const std::wstring & name, int width, int height, const std::shared_ptr<SourcePos>& srcPos) {
    auto renderTarget = std::make_shared<RenderTarget>(name, width, height, getGraphicDevice());
    renderTarget->setViewport(0, 0, getScreenWidth(), getScreenHeight());
    renderTargets[name] = renderTarget;
    addLostableGraphicResource(renderTarget);
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("create render target.")
      .setParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
      .addSourcePos(srcPos)));
    return renderTarget;
  }

  void Engine::removeRenderTarget(const std::wstring & name, const std::shared_ptr<SourcePos>& srcPos) {
    auto it = renderTargets.find(name);
    if (it != renderTargets.end()) {
      renderTargets.erase(it);
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_INFO)
        .setMessage("remove render target.")
        .setParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
        .addSourcePos(srcPos)));
    }
  }

  std::shared_ptr<RenderTarget> Engine::getRenderTarget(const std::wstring & name) const {
    auto it = renderTargets.find(name);
    if (it != renderTargets.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::wstring Engine::getReservedRenderTargetName(int idx) const {
    return RESERVED_RENDER_TARGET_PREFIX + std::to_wstring(idx);
  }

  std::wstring Engine::getTransitionRenderTargetName() const {
    return TRANSITION_RENDER_TARGET_NAME;
  }

  static D3DXIMAGE_FILEFORMAT getProperFileFormat(const std::wstring& path) {
    auto ext = getLowerExt(path);
    if (ext == L".png" || ext.empty()) return D3DXIFF_PNG;
    if (ext == L".bmp") return D3DXIFF_BMP;
    if (ext == L".dds") return D3DXIFF_DDS;
    if (ext == L".jpg" || ext == L".jpeg") return D3DXIFF_JPG;
    if (ext == L".dib") return D3DXIFF_DIB;
    if (ext == L".hdr") return D3DXIFF_HDR;
    if (ext == L".pfm") return D3DXIFF_PFM;
    return D3DXIFF_PNG;
  }

  void Engine::saveRenderedTextureA1(const std::wstring & name, const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    if (auto renderTarget = getRenderTarget(name)) {
      auto viewport = renderTarget->getViewport();
      saveRenderedTextureA2(name, path, viewport.X, viewport.Y, viewport.X + viewport.Width, viewport.Y + viewport.Height, srcPos);
    }
  }

  void Engine::saveRenderedTextureA2(const std::wstring & name, const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos) {
    if (auto renderTarget = getRenderTarget(name)) {
      auto viewport = renderTarget->getViewport();
      RECT rect = { left, top, right, bottom };
      mkdir_p(parentPath(path));
      if (SUCCEEDED(D3DXSaveSurfaceToFile(path.c_str(), getProperFileFormat(path), renderTarget->getSurface(), NULL, &rect))) {
        return;
      }
    }
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_WARN)
      .setMessage("failed to save render target.")
      .setParam(Log::Param(Log::Param::Tag::RENDER_TARGET, name))
      .addSourcePos(srcPos)));
  }

  void Engine::saveSnapShotA1(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    saveSnapShotA2(path, 0, 0, getScreenWidth(), getScreenHeight(), srcPos);
  }

  void Engine::saveSnapShotA2(const std::wstring & path, int left, int top, int right, int bottom, const std::shared_ptr<SourcePos>& srcPos) {
    removeRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, srcPos);
    try {
      createRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, getScreenWidth(), getScreenHeight(), srcPos);
      renderToTexture(SNAP_SHOT_RENDER_TARGET_NAME, 0, MAX_RENDER_PRIORITY, ID_INVALID, true, false, true, true);
      saveRenderedTextureA2(SNAP_SHOT_RENDER_TARGET_NAME, path, left, top, right, bottom, srcPos);
    } catch (Log& log) {
      log.setLevel(Log::Level::LV_WARN).addSourcePos(srcPos);
      Logger::WriteLog(log);
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("failed to create snap shot.")
        .setParam(Log::Param(Log::Param::Tag::TEXT, path))
        .addSourcePos(srcPos)));
    }
    removeRenderTarget(SNAP_SHOT_RENDER_TARGET_NAME, srcPos);
  }

  std::shared_ptr<Shader> Engine::createShader(const std::wstring & path, bool precompiled) {
    auto shader = std::make_shared<Shader>(path, precompiled, getGraphicDevice());
    addLostableGraphicResource(shader);
    return shader;
  }

  bool Engine::isPixelShaderSupported(int major, int minor) {
    D3DCAPS9 caps;
    getGraphicDevice()->GetDeviceCaps(&caps);
    return caps.PixelShaderVersion >= D3DPS_VERSION(major, minor);
  }

  std::shared_ptr<Mesh> Engine::loadMesh(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    return gameState->meshCache->load(path, gameState->textureCache, srcPos);
  }

  void Engine::releaseUnusedMeshCache() {
    gameState->meshCache->releaseUnusedMesh();
  }

  std::shared_ptr<SoundBuffer> Engine::loadSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    return gameState->soundDevice->loadSound(path, false, srcPos);
  }

  void Engine::loadOrphanSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->orphanSounds[canonicalPath(path)] = loadSound(path, srcPos);
  }

  void Engine::removeOrphanSound(const std::wstring & path) {
    gameState->orphanSounds.erase(canonicalPath(path));
  }

  void Engine::playBGM(const std::wstring & path, double loopStartSec, double loopEndSec) {
    auto it = gameState->orphanSounds.find(canonicalPath(path));
    if (it != gameState->orphanSounds.end()) {
      auto& sound = it->second;
      if (sound->isPlaying()) sound->seek(0);
      sound->setLoopEnable(true);
      sound->setLoopTime(loopStartSec, loopEndSec);
      sound->play();
    }
  }

  void Engine::playSE(const std::wstring & path) {
    auto it = gameState->orphanSounds.find(canonicalPath(path));
    if (it != gameState->orphanSounds.end()) {
      auto& sound = it->second;
      if (sound->isPlaying()) sound->seek(0);
      sound->play();
    }
  }

  void Engine::stopOrphanSound(const std::wstring & path) {
    auto it = gameState->orphanSounds.find(canonicalPath(path));
    if (it != gameState->orphanSounds.end()) {
      it->second->stop();
    }
  }

  void Engine::cacheSound(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->soundDevice->loadSound(path, true, srcPos);
  }

  void Engine::removeSoundCache(const std::wstring & path) {
    gameState->soundDevice->removeSoundCache(path);
  }

  void Engine::clearSoundCache() {
    gameState->soundDevice->clearSoundCache();
  }

  void Engine::setObjectRenderPriority(const std::shared_ptr<ObjRender>& obj, int priority) {
    gameState->objLayerList->setRenderPriority(obj, priority);
  }

  void Engine::setShader(int beginPriority, int endPriority, const std::shared_ptr<Shader>& shader) {
    gameState->objLayerList->setLayerShader(beginPriority, endPriority, shader);
  }

  void Engine::resetShader(int beginPriority, int endPriority) {
    gameState->objLayerList->resetLayerShader(beginPriority, endPriority);
  }

  int Engine::getStgFrameRenderPriorityMin() const {
    return gameState->objLayerList->getStgFrameRenderPriorityMin();
  }

  void Engine::setStgFrameRenderPriorityMin(int p) {
    gameState->objLayerList->setStgFrameRenderPriorityMin(p);
  }

  int Engine::getStgFrameRenderPriorityMax() const {
    return gameState->objLayerList->getStgFrameRenderPriorityMax();
  }

  void Engine::setStgFrameRenderPriorityMax(int p) {
    gameState->objLayerList->setStgFrameRenderPriorityMax(p);
  }

  int Engine::getShotRenderPriority() const {
    return gameState->objLayerList->getShotRenderPriority();
  }

  void Engine::setShotRenderPriority(int p) {
    return gameState->objLayerList->setShotRenderPriority(p);
  }

  int Engine::getItemRenderPriority() const {
    return gameState->objLayerList->getItemRenderPriority();
  }

  void Engine::setItemRenderPriority(int p) {
    return gameState->objLayerList->setItemRenderPriority(p);
  }

  int Engine::getPlayerRenderPriority() const {
    if (auto player = getPlayerObject()) {
      return player->getRenderPriority();
    }
    return DEFAULT_PLAYER_RENDER_PRIORITY;
  }

  int Engine::getCameraFocusPermitRenderPriority() const {
    return gameState->objLayerList->getCameraFocusPermitRenderPriority();
  }

  void Engine::setInvalidRenderPriority(int min, int max) {
    gameState->objLayerList->setInvalidRenderPriority(min, max);
  }

  void Engine::clearInvalidRenderPriority() {
    gameState->objLayerList->clearInvalidRenderPriority();
  }

  std::shared_ptr<Shader> Engine::getShader(int p) const {
    return gameState->objLayerList->getLayerShader(p);
  }

  void Engine::setFogEnable(bool enable) {
    renderer->setFogEnable(enable);
  }

  void Engine::setFogParam(float fogStart, float fogEnd, int r, int g, int b) {
    renderer->setFogParam(fogStart, fogEnd, r, g, b);
  }

  void Engine::setCameraFocusX(float x) {
    gameState->camera3D->setFocusX(x);
  }

  void Engine::setCameraFocusY(float y) {
    gameState->camera3D->setFocusY(y);
  }

  void Engine::setCameraFocusZ(float z) {
    gameState->camera3D->setFocusZ(z);
  }

  void Engine::setCameraFocusXYZ(float x, float y, float z) {
    gameState->camera3D->setFocusXYZ(x, y, z);
  }

  void Engine::setCameraRadius(float r) {
    gameState->camera3D->setRadius(r);
  }

  void Engine::setCameraAzimuthAngle(float angle) {
    gameState->camera3D->setAzimuthAngle(angle);
  }

  void Engine::setCameraElevationAngle(float angle) {
    gameState->camera3D->setElevationAngle(angle);
  }

  void Engine::setCameraYaw(float yaw) {
    gameState->camera3D->setYaw(yaw);
  }

  void Engine::setCameraPitch(float pitch) {
    gameState->camera3D->setPitch(pitch);
  }

  void Engine::setCameraRoll(float roll) {
    gameState->camera3D->setRoll(roll);
  }

  void Engine::resetCamera() {
    setCameraFocusXYZ(0.0f, 0.0f, 0.0f);
    setCameraRadius(500.0f);
    setCameraAzimuthAngle(15.0f);
    setCameraElevationAngle(45.0f);
    setCameraYaw(0.0f);
    setCameraPitch(0.0f);
    setCameraRoll(0.0f);
    setCameraPerspectiveClip(10.0f, 2000.0f);
  }

  float Engine::getCameraX() const {
    return gameState->camera3D->getX();
  }

  float Engine::getCameraY() const {
    return gameState->camera3D->getY();
  }

  float Engine::getCameraZ() const {
    return gameState->camera3D->getZ();
  }

  float Engine::getCameraFocusX() const {
    return gameState->camera3D->getFocusX();
  }

  float Engine::getCameraFocusY() const {
    return gameState->camera3D->getFocusY();
  }

  float Engine::getCameraFocusZ() const {
    return gameState->camera3D->getFocusZ();
  }

  float Engine::getCameraRadius() const {
    return gameState->camera3D->getRadius();
  }

  float Engine::getCameraAzimuthAngle() const {
    return gameState->camera3D->getAzimuthAngle();
  }

  float Engine::getCameraElevationAngle() const {
    return gameState->camera3D->getElevationAngle();
  }

  float Engine::getCameraYaw() const {
    return gameState->camera3D->getYaw();
  }

  float Engine::getCameraPitch() const {
    return gameState->camera3D->getPitch();
  }

  float Engine::getCameraRoll() const {
    return gameState->camera3D->getRoll();
  }

  void Engine::setCameraPerspectiveClip(float nearClip, float farClip) {
    return gameState->camera3D->setPerspectiveClip(nearClip, farClip);
  }

  void Engine::set2DCameraFocusX(float x) {
    gameState->camera2D->setFocusX(x);
  }

  void Engine::set2DCameraFocusY(float y) {
    gameState->camera2D->setFocusY(y);
  }

  void Engine::set2DCameraAngleZ(float z) {
    gameState->camera2D->setAngleZ(z);
  }

  void Engine::set2DCameraRatio(float r) {
    gameState->camera2D->setRatio(r);
  }

  void Engine::set2DCameraRatioX(float x) {
    gameState->camera2D->setRatioX(x);
  }

  void Engine::set2DCameraRatioY(float y) {
    gameState->camera2D->setRatioY(y);
  }

  void Engine::reset2DCamera() {
    gameState->camera2D->reset(getStgFrameCenterWorldX(), getStgFrameCenterWorldY());
  }

  float Engine::get2DCameraX() const {
    return gameState->camera2D->getX();
  }

  float Engine::get2DCameraY() const {
    return gameState->camera2D->getY();
  }

  float Engine::get2DCameraAngleZ() const {
    return gameState->camera2D->getAngleZ();
  }

  float Engine::get2DCameraRatio() const {
    return gameState->camera2D->getRatio();
  }

  float Engine::get2DCameraRatioX() const {
    return gameState->camera2D->getRatioX();
  }

  float Engine::get2DCameraRatioY() const {
    return gameState->camera2D->getRatioY();
  }

  void Engine::setCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& value) {
    gameState->commonDataDB->setCommonData(key, std::move(value));
  }

  std::unique_ptr<DnhValue> Engine::getCommonData(const std::wstring & key, std::unique_ptr<DnhValue>&& defaultValue) const {
    return gameState->commonDataDB->getCommonData(key, std::move(defaultValue));
  }

  void Engine::clearCommonData() {
    gameState->commonDataDB->clearCommonData();
  }

  void Engine::deleteCommonData(const std::wstring & key) {
    gameState->commonDataDB->deleteCommonData(key);
  }

  void Engine::setAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value) {
    gameState->commonDataDB->setAreaCommonData(areaName, key, std::move(value));
  }

  std::unique_ptr<DnhValue> Engine::getAreaCommonData(const std::wstring & areaName, const std::wstring & key, std::unique_ptr<DnhValue>&& value) const {
    return gameState->commonDataDB->getAreaCommonData(areaName, key, std::move(value));
  }

  void Engine::clearAreaCommonData(const std::wstring & areaName) {
    gameState->commonDataDB->clearAreaCommonData(areaName);
  }

  void Engine::deleteAreaCommonData(const std::wstring & areaName, const std::wstring & key) {
    gameState->commonDataDB->deleteAreaCommonData(areaName, key);
  }

  void Engine::createCommonDataArea(const std::wstring & areaName) {
    gameState->commonDataDB->createCommonDataArea(areaName);
  }

  bool Engine::isCommonDataAreaExists(const std::wstring & areaName) const {
    return gameState->commonDataDB->isCommonDataAreaExists(areaName);
  }

  void Engine::copyCommonDataArea(const std::wstring & dest, const std::wstring & src) {
    gameState->commonDataDB->copyCommonDataArea(dest, src);
  }

  std::vector<std::wstring> Engine::getCommonDataAreaKeyList() const {
    return gameState->commonDataDB->getCommonDataAreaKeyList();
  }

  std::vector<std::wstring> Engine::getCommonDataValueKeyList(const std::wstring & areaName) const {
    return gameState->commonDataDB->getCommonDataValueKeyList(areaName);
  }

  bool Engine::saveCommonDataAreaA1(const std::wstring & areaName) const {
    return gameState->commonDataDB->saveCommonDataArea(areaName, getDefaultCommonDataSavePath(areaName));
  }

  bool Engine::loadCommonDataAreaA1(const std::wstring & areaName) {
    return gameState->commonDataDB->loadCommonDataArea(areaName, getDefaultCommonDataSavePath(areaName));
  }

  bool Engine::saveCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path) const {
    return gameState->commonDataDB->saveCommonDataArea(areaName, path);
  }

  bool Engine::loadCommonDataAreaA2(const std::wstring & areaName, const std::wstring & path) {
    return gameState->commonDataDB->loadCommonDataArea(areaName, path);
  }

  std::wstring Engine::getDefaultCommonDataSavePath(const std::wstring & areaName) const {
    std::wstring basePath = getMainPackageScriptPath() == DEFAULT_PACKAGE_PATH ? getMainStgScriptPath() : getMainPackageScriptPath();
    if (basePath.empty()) {
      basePath = getMainPackageScriptPath();
    }
    if (basePath.empty()) return false;
    return parentPath(basePath) + L"/data/" + getStem(basePath) + L"_common_" + areaName + L".dat";
  }

  void Engine::loadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->playerShotDataTable->load(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  void Engine::reloadPlayerShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->playerShotDataTable->reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  void Engine::loadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->enemyShotDataTable->load(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  void Engine::reloadEnemyShotData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->enemyShotDataTable->reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  void Engine::loadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->itemDataTable->load(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  void Engine::reloadItemData(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    gameState->itemDataTable->reload(path, gameState->fileLoader, gameState->textureCache, srcPos);
  }

  std::shared_ptr<ShotData> Engine::getPlayerShotData(int id) const {
    return gameState->playerShotDataTable->get(id);
  }

  std::shared_ptr<ShotData> Engine::getEnemyShotData(int id) const {
    return gameState->enemyShotDataTable->get(id);
  }

  std::shared_ptr<ItemData> Engine::getItemData(int id) const {
    return gameState->itemDataTable->get(id);
  }

  std::shared_ptr<ObjText> Engine::createObjText() {
    auto obj = gameState->objTable->create<ObjText>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjSound> Engine::createObjSound() {
    return gameState->objTable->create<ObjSound>(gameState);
  }

  std::shared_ptr<ObjFileT> Engine::createObjFileT() {
    return gameState->objTable->create<ObjFileT>(gameState);
  }

  std::shared_ptr<ObjFileB> Engine::createObjFileB() {
    return gameState->objTable->create<ObjFileB>(gameState);
  }

  std::shared_ptr<ObjShader> Engine::createObjShader() {
    auto obj = gameState->objTable->create<ObjShader>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjShot> Engine::createObjShot(bool isPlayerShot) {
    auto shot = gameState->objTable->create<ObjShot>(isPlayerShot, gameState);
    gameState->objLayerList->setRenderPriority(shot, gameState->objLayerList->getShotRenderPriority());
    return shot;
  }

  std::shared_ptr<ObjShot> Engine::createShotA1(float x, float y, float speed, float angle, int shotDataId, int delay, bool isPlayerShot) {
    auto shot = createObjShot(isPlayerShot);
    shot->setMovePosition(x, y);
    shot->setSpeed(speed);
    shot->setAngle(angle);
    shot->setShotData(isPlayerShot ? getPlayerShotData(shotDataId) : getEnemyShotData(shotDataId));
    shot->setDelay(delay);
    shot->regist();
    return shot;
  }

  std::shared_ptr<ObjShot> Engine::createShotA2(float x, float y, float speed, float angle, float accel, float maxSpeed, int shotDataId, int delay, bool isPlayerShot) {
    auto shot = createShotA1(x, y, speed, angle, shotDataId, delay, isPlayerShot);
    shot->setAcceleration(accel);
    shot->setMaxSpeed(maxSpeed);
    return shot;
  }

  // NOTE: オブジェクトが存在しなかったら空ポインタを返す
  std::shared_ptr<ObjShot> Engine::createShotOA1(int objId, float speed, float angle, int shotDataId, int delay, bool isPlayerShot) {
    if (auto obj = getObject<ObjRender>(objId)) {
      return createShotA1(obj->getX(), obj->getY(), speed, angle, shotDataId, delay, isPlayerShot);
    } else {
      return nullptr;
    }
  }

  std::shared_ptr<ObjShot> Engine::createShotB1(float x, float y, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot) {
    return createShotB2(x, y, speedX, speedY, 0, 0, 0, 0, shotDataId, delay, isPlayerShot);
  }

  std::shared_ptr<ObjShot> Engine::createShotB2(float x, float y, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, int shotDataId, int delay, bool isPlayerShot) {
    auto shot = createObjShot(isPlayerShot);
    shot->setMovePosition(x, y);
    shot->setMoveMode(std::make_shared<MoveModeB>(speedX, speedY, accelX, accelY, maxSpeedX, maxSpeedY));
    shot->setShotData(isPlayerShot ? getPlayerShotData(shotDataId) : getEnemyShotData(shotDataId));
    shot->setDelay(delay);
    shot->regist();
    return shot;
  }

  std::shared_ptr<ObjShot> Engine::createShotOB1(int objId, float speedX, float speedY, int shotDataId, int delay, bool isPlayerShot) {
    if (auto obj = getObject<ObjRender>(objId)) {
      return createShotB1(obj->getX(), obj->getY(), speedX, speedY, shotDataId, delay, isPlayerShot);
    } else {
      return nullptr;
    }
  }

  std::shared_ptr<ObjShot> Engine::createPlayerShotA1(float x, float y, float speed, float angle, double damage, int penetration, int shotDataId) {
    if (auto player = getPlayerObject()) {
      if (!player->isPermitPlayerShot()) {
        return nullptr;
      }
    }
    auto shot = createObjShot(true);
    shot->setMovePosition(x, y);
    shot->setSpeed(speed);
    shot->setAngle(angle);
    shot->setDamage(damage);
    shot->setPenetration(penetration);
    shot->setShotData(getPlayerShotData(shotDataId));
    shot->regist();
    return shot;
  }

  std::shared_ptr<ObjLooseLaser> Engine::createObjLooseLaser(bool isPlayerShot) {
    auto laser = gameState->objTable->create<ObjLooseLaser>(isPlayerShot, gameState);
    gameState->objLayerList->setRenderPriority(laser, gameState->objLayerList->getShotRenderPriority());
    return laser;
  }

  std::shared_ptr<ObjLooseLaser> Engine::createLooseLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot) {
    auto laser = createObjLooseLaser(isPlayerShot);
    laser->setMovePosition(x, y);
    laser->setSpeed(speed);
    laser->setAngle(angle);
    laser->setLength(length);
    laser->setRenderWidth(width);
    laser->setShotData(isPlayerShot ? getPlayerShotData(shotDataId) : getEnemyShotData(shotDataId));
    laser->setDelay(delay);
    laser->regist();
    return laser;
  }

  std::shared_ptr<ObjStLaser> Engine::createObjStLaser(bool isPlayerShot) {
    auto laser = gameState->objTable->create<ObjStLaser>(isPlayerShot, gameState);
    gameState->objLayerList->setRenderPriority(laser, gameState->objLayerList->getShotRenderPriority());
    return laser;
  }

  std::shared_ptr<ObjStLaser> Engine::createStraightLaserA1(float x, float y, float angle, float length, float width, int deleteFrame, int shotDataId, int delay, bool isPlayerShot) {
    auto laser = createObjStLaser(isPlayerShot);
    laser->setMovePosition(x, y);
    laser->setLaserAngle(angle);
    laser->setLength(length);
    laser->setRenderWidth(width);
    laser->setDeleteFrame(deleteFrame);
    laser->setShotData(isPlayerShot ? getPlayerShotData(shotDataId) : getEnemyShotData(shotDataId));
    laser->setDelay(delay);
    laser->regist();
    return laser;
  }

  std::shared_ptr<ObjCrLaser> Engine::createObjCrLaser(bool isPlayerShot) {
    auto laser = gameState->objTable->create<ObjCrLaser>(isPlayerShot, gameState);
    gameState->objLayerList->setRenderPriority(laser, gameState->objLayerList->getShotRenderPriority());
    return laser;
  }

  std::shared_ptr<ObjCrLaser> Engine::createCurveLaserA1(float x, float y, float speed, float angle, float length, float width, int shotDataId, int delay, bool isPlayerShot) {
    auto laser = createObjCrLaser(isPlayerShot);
    laser->setMovePosition(x, y);
    laser->setSpeed(speed);
    laser->setAngle(angle);
    laser->setLength(length);
    laser->setRenderWidth(width);
    laser->setShotData(isPlayerShot ? getPlayerShotData(shotDataId) : getEnemyShotData(shotDataId));
    laser->setDelay(delay);
    laser->regist();
    return laser;
  }

  std::shared_ptr<ObjItem> Engine::createItemA1(int itemType, float x, float y, int64_t score) {
    auto item = createObjItem(itemType);
    item->setMovePosition(x, y);
    item->setMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->setScore(score);
    return item;
  }

  std::shared_ptr<ObjItem> Engine::createItemA2(int itemType, float x, float y, float destX, float destY, int64_t score) {
    auto item = createObjItem(itemType);
    item->setMovePosition(x, y);
    item->setMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->setScore(score);
    return item;
  }

  std::shared_ptr<ObjItem> Engine::createItemU1(int itemDataId, float x, float y, int64_t score) {
    auto item = createObjItem(ITEM_USER);
    item->setMovePosition(x, y);
    item->setMoveMode(std::make_shared<MoveModeItemDest>(x, y - 128.0f, item.get()));
    item->setScore(score);
    item->setItemData(getItemData(itemDataId));
    return item;
  }

  std::shared_ptr<ObjItem> Engine::createItemU2(int itemDataId, float x, float y, float destX, float destY, int64_t score) {
    auto item = createObjItem(ITEM_USER);
    item->setMovePosition(x, y);
    item->setMoveMode(std::make_shared<MoveModeItemDest>(destX, destY, item.get()));
    item->setScore(score);
    item->setItemData(getItemData(itemDataId));
    return item;
  }

  std::shared_ptr<ObjEnemy> Engine::createObjEnemy() {
    auto enemy = gameState->objTable->create<ObjEnemy>(false, gameState);
    gameState->objLayerList->setRenderPriority(enemy, DEFAULT_ENEMY_RENDER_PRIORITY);
    return enemy;
  }

  std::shared_ptr<ObjEnemyBossScene> Engine::createObjEnemyBossScene(const std::shared_ptr<SourcePos>& srcPos) {
    if (auto bossScene = gameState->enemyBossSceneObj.lock()) {
      if (!bossScene->isDead()) {
        Logger::WriteLog(std::move(
          Log(Log::Level::LV_WARN)
          .setMessage("boss scene object already exists.")
          .addSourcePos(srcPos)));
        return bossScene;
      }
    }
    auto bossScene = gameState->objTable->create<ObjEnemyBossScene>(gameState);
    gameState->enemyBossSceneObj = bossScene;
    return bossScene;
  }

  std::shared_ptr<ObjSpell> Engine::createObjSpell() {
    auto spell = gameState->objTable->create<ObjSpell>(gameState);
    gameState->objLayerList->setRenderPriority(spell, 50);
    return spell;
  }

  std::shared_ptr<ObjItem> Engine::createObjItem(int itemType) {
    auto obj = gameState->objTable->create<ObjItem>(itemType, gameState);
    gameState->objLayerList->setRenderPriority(obj, gameState->objLayerList->getItemRenderPriority());
    return obj;
  }

  std::shared_ptr<Script> Engine::getScript(int scriptId) const {
    return gameState->scriptManager->get(scriptId);
  }

  std::shared_ptr<Script> Engine::loadScript(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos) {
    auto script = gameState->scriptManager->compile(path, type, version, srcPos);
    script->load();
    return script;
  }

  std::shared_ptr<Script> Engine::loadScriptInThread(const std::wstring & path, const std::wstring & type, const std::wstring & version, const std::shared_ptr<SourcePos>& srcPos) {
    auto script = gameState->scriptManager->compileInThread(path, type, version, srcPos);
    return script;
  }

  void Engine::closeStgScene() {
    if (auto stageMain = gameState->stageMainScript.lock()) {
      stageMain->close();
    }
  }

  void Engine::notifyEventAll(int eventType) {
    gameState->scriptManager->notifyEventAll(eventType);
  }

  void Engine::notifyEventAll(int eventType, const std::unique_ptr<DnhArray>& args) {
    gameState->scriptManager->notifyEventAll(eventType, args);
  }

  std::wstring Engine::getPlayerID() const {
    return gameState->stagePlayerScriptInfo.id;
  }

  std::wstring Engine::getPlayerReplayName() const {
    return gameState->stagePlayerScriptInfo.replayName;
  }

  std::shared_ptr<ObjPlayer> Engine::getPlayerObject() const {
    auto player = gameState->playerObj.lock();
    if (player && !player->isDead()) return player;
    return nullptr;
  }

  std::shared_ptr<ObjEnemy> Engine::getEnemyBossObject() const {
    if (auto bossScene = getEnemyBossSceneObject()) {
      if (auto boss = bossScene->getEnemyBossObject()) {
        if (!boss->isDead()) return boss;
      }
    }
    return nullptr;
  }

  std::shared_ptr<ObjEnemyBossScene> Engine::getEnemyBossSceneObject() const {
    auto bossScene = gameState->enemyBossSceneObj.lock();
    if (bossScene && !bossScene->isDead()) return bossScene;
    return nullptr;
  }

  std::shared_ptr<ObjSpellManage> Engine::getSpellManageObject() const {
    auto spellManage = gameState->spellManageObj.lock();
    if (spellManage && !spellManage->isDead()) return spellManage;
    return nullptr;
  }

  void Engine::deleteObject(int id) {
    gameState->objTable->del(id);
  }

  bool Engine::isObjectDeleted(int id) const {
    return gameState->objTable->isDeleted(id);
  }

  std::shared_ptr<ObjPrim2D> Engine::createObjPrim2D() {
    auto obj = gameState->objTable->create<ObjPrim2D>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjSprite2D> Engine::createObjSprite2D() {
    auto obj = gameState->objTable->create<ObjSprite2D>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjSpriteList2D> Engine::createObjSpriteList2D() {
    auto obj = gameState->objTable->create<ObjSpriteList2D>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjPrim3D> Engine::createObjPrim3D() {
    auto obj = gameState->objTable->create<ObjPrim3D>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjSprite3D> Engine::createObjSprite3D() {
    auto obj = gameState->objTable->create<ObjSprite3D>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<ObjMesh> Engine::createObjMesh() {
    auto obj = gameState->objTable->create<ObjMesh>(gameState);
    gameState->objLayerList->setRenderPriority(obj, 50);
    return obj;
  }

  std::shared_ptr<Script> Engine::getPlayerScript() const {
    return gameState->stagePlayerScript.lock();
  }

  std::unique_ptr<DnhValue> Engine::getScriptResult(int scriptId) const {
    return gameState->scriptManager->getScriptResult(scriptId);
  }

  void Engine::setScriptResult(int scriptId, std::unique_ptr<DnhValue>&& value) {
    gameState->scriptManager->setScriptResult(scriptId, std::move(value));
  }

  std::vector<ScriptInfo> Engine::getScriptList(const std::wstring & dirPath, int scriptType, bool doRecursive) {
    std::vector<std::wstring> pathList;
    getFilePaths(dirPath, pathList, ignoreScriptExts, doRecursive);
    std::vector<ScriptInfo> infos;
    infos.reserve(pathList.size());
    const std::wstring scriptTypeName = getScriptTypeNameFromConst(scriptType);
    for (const auto& path : pathList) {
      try {
        auto info = scanDnhScriptInfo(path, gameState->fileLoader);
        if (scriptType == TYPE_SCRIPT_ALL || scriptTypeName == info.type) {
          infos.push_back(info);
        }
      } catch (const Log& log) {
      }
    }
    return infos;
  }

  void Engine::getLoadFreePlayerScriptList() {
    gameState->freePlayerScriptInfoList = getScriptList(FREE_PLAYER_DIR, TYPE_SCRIPT_PLAYER, true);
  }

  int Engine::getFreePlayerScriptCount() const {
    return gameState->freePlayerScriptInfoList.size();
  }

  ScriptInfo Engine::getFreePlayerScriptInfo(int idx) const {
    return gameState->freePlayerScriptInfoList.at(idx);
  }

  ScriptInfo Engine::getScriptInfo(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    try {
      return scanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log) {
      log.setLevel(Log::Level::LV_WARN).addSourcePos(srcPos);
      Logger::WriteLog(log);
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("failed to load script info.")
        .setParam(Log::Param(Log::Param::Tag::SCRIPT, path))
        .addSourcePos(srcPos)));
      return ScriptInfo();
    }
  }

  int64_t Engine::getScore() const {
    return gameState->globalPlayerParams->score;
  }

  void Engine::addScore(int64_t score) {
    gameState->globalPlayerParams->score += score;
  }

  int64_t Engine::getGraze() const {
    return gameState->globalPlayerParams->graze;
  }

  void Engine::addGraze(int64_t graze) {
    gameState->globalPlayerParams->graze += graze;
  }

  int64_t Engine::getPoint() const {
    return gameState->globalPlayerParams->point;
  }

  void Engine::addPoint(int64_t point) {
    gameState->globalPlayerParams->point += point;
  }

  void Engine::setStgFrame(float left, float top, float right, float bottom) {
    gameState->stgFrame->left = left;
    gameState->stgFrame->top = top;
    gameState->stgFrame->right = right;
    gameState->stgFrame->bottom = bottom;
  }

  float Engine::getStgFrameLeft() const {
    return gameState->stgFrame->left;
  }

  float Engine::getStgFrameTop() const {
    return gameState->stgFrame->top;
  }

  float Engine::getStgFrameWidth() const {
    return gameState->stgFrame->right - gameState->stgFrame->left;
  }

  float Engine::getStgFrameHeight() const {
    return gameState->stgFrame->bottom - gameState->stgFrame->top;
  }

  float Engine::getStgFrameCenterWorldX() const {
    return (gameState->stgFrame->right - gameState->stgFrame->left) / 2.0f;
  }

  float Engine::getStgFrameCenterWorldY() const {
    return (gameState->stgFrame->bottom - gameState->stgFrame->top) / 2.0f;
  }

  float Engine::getStgFrameCenterScreenX() const {
    return (gameState->stgFrame->right + gameState->stgFrame->left) / 2.0f;
  }

  float Engine::getStgFrameCenterScreenY() const {
    return (gameState->stgFrame->bottom + gameState->stgFrame->top) / 2.0f;
  }

  int Engine::getAllShotCount() const {
    return gameState->shotCounter->playerShotCount + gameState->shotCounter->enemyShotCount;
  }

  int Engine::getEnemyShotCount() const {
    return gameState->shotCounter->enemyShotCount;
  }

  int Engine::getPlayerShotCount() const {
    return gameState->shotCounter->playerShotCount;
  }

  void Engine::setShotAutoDeleteClip(float left, float top, float right, float bottom) {
    gameState->shotAutoDeleteClip->setClip(left, top, right, bottom);
  }

  void Engine::startShotScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    if (isStageFinished()) {
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("shot script is only available in stage.")
        .setParam(Log::Param(Log::Param::Tag::SCRIPT, canonicalPath(path)))
        .addSourcePos(srcPos)));
      return;
    }
    auto shotScript = loadScript(path, SCRIPT_TYPE_SHOT_CUSTOM, gameState->stageMainScriptInfo.version, srcPos);
    gameState->shotScript = shotScript;
    shotScript->start();
    shotScript->runInitialize();
  }

  void Engine::setDeleteShotImmediateEventOnShotScriptEnable(bool enable) {
    gameState->deleteShotImmediateEventOnShotScriptEnable = enable;
  }

  void Engine::setDeleteShotFadeEventOnShotScriptEnable(bool enable) {
    gameState->deleteShotFadeEventOnShotScriptEnable = enable;
  }

  void Engine::setDeleteShotToItemEventOnShotScriptEnable(bool enable) {
    gameState->deleteShotToItemEventOnShotScriptEnable = enable;
  }

  void Engine::deleteShotAll(int target, int behavior) const {
    auto shots = getObjectAll<ObjShot>();
    if (target == TYPE_SHOT) {
      // 自機弾とスペル耐性のある弾は除外
      shots.erase(std::remove_if(shots.begin(), shots.end(), [](std::shared_ptr<ObjShot>& shot)->bool { return shot->isPlayerShot() || shot->isSpellResistEnabled(); }), shots.end());
    } else {
      // 自機弾は除外
      shots.erase(std::remove_if(shots.begin(), shots.end(), [](std::shared_ptr<ObjShot>& shot)->bool { return shot->isPlayerShot(); }), shots.end());
    }

    // delete
    for (auto& shot : shots) {
      if (behavior == TYPE_IMMEDIATE) {
        shot->deleteImmediate();
      } else if (behavior == TYPE_FADE) {
        shot->fadeDelete();
      } else if (behavior == TYPE_ITEM) {
        shot->toItem();
      }
    }
  }

  void Engine::deleteShotInCircle(int target, int behavior, float x, float y, float r) const {
    auto isects = gameState->colDetector->getIntersectionsCollideWithShape(Shape(x, y, r));
    for (auto& isectP : isects) {
      if (auto isect = isectP.lock()) {
        if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect)) {
          if (!shotIsect->isPlayerShot) {
            // スペル耐性弾は無視
            if (target == TYPE_SHOT && shotIsect->shot->isSpellResistEnabled()) continue;
            if (behavior == TYPE_IMMEDIATE) {
              shotIsect->shot->deleteImmediate();
            } else if (behavior == TYPE_FADE) {
              shotIsect->shot->fadeDelete();
            } else if (behavior == TYPE_ITEM) {
              shotIsect->shot->toItem();
            }
          }
        }
      }
    }
  }

  std::vector<std::shared_ptr<ObjShot>> Engine::getShotInCircle(float x, float y, float r, int target) const {
    auto isects = gameState->colDetector->getIntersectionsCollideWithShape(Shape(x, y, r));
    std::unordered_set<int> shotIds;
    for (auto& isectP : isects) {
      if (auto isect = isectP.lock()) {
        if (auto shotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect)) {
          if (target == TARGET_ENEMY && shotIsect->isPlayerShot) continue;
          if (target == TARGET_PLAYER && !shotIsect->isPlayerShot) continue;
          if (!shotIsect->shot->isDead()) {
            shotIds.insert(shotIsect->shot->getID());
          }
        }
      }
    }
    std::vector<std::shared_ptr<ObjShot>> shots;
    for (auto id : shotIds) {
      shots.push_back(getObject<ObjShot>(id));
    }
    return shots;
  }

  void Engine::setShotIntersectoinCicle(float x, float y, float r) {
    auto isect = std::make_shared<TempEnemyShotIntersection>(x, y, r);
    gameState->colDetector->add(isect);
    gameState->tempEnemyShotIsects.push_back(isect);
  }

  void Engine::setShotIntersectoinLine(float x1, float y1, float x2, float y2, float width) {
    auto isect = std::make_shared<TempEnemyShotIntersection>(x1, y1, x2, y2, width);
    gameState->colDetector->add(isect);
    gameState->tempEnemyShotIsects.push_back(isect);
  }

  void Engine::collectAllItems() {
    gameState->autoItemCollectionManager->collectAllItems();
  }

  void Engine::collectItemsByType(int itemType) {
    gameState->autoItemCollectionManager->collectItemsByType(itemType);
  }

  void Engine::collectItemsInCircle(float x, float y, float r) {
    gameState->autoItemCollectionManager->collectItemsInCircle(x, y, r);
  }

  void Engine::cancelCollectItems() {
    gameState->autoItemCollectionManager->cancelCollectItems();
  }

  void Engine::setDefaultBonusItemEnable(bool enable) {
    gameState->defaultBonusItemEnable = enable;
  }

  void Engine::startItemScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    if (isStageFinished()) {
      Logger::WriteLog(std::move(
        Log(Log::Level::LV_WARN)
        .setMessage("item script is only available in stage.")
        .setParam(Log::Param(Log::Param::Tag::SCRIPT, canonicalPath(path)))
        .addSourcePos(srcPos)));
      return;
    }
    auto itemScript = loadScript(path, SCRIPT_TYPE_ITEM_CUSTOM, gameState->stageMainScriptInfo.version, srcPos);
    gameState->itemScript = itemScript;
    itemScript->start();
    itemScript->runInitialize();
  }

  bool Engine::isPackageFinished() const {
    if (gameState->packageMainScript.lock()) {
      return false;
    }
    return true;
  }

  void Engine::closePackage() {
    if (auto packageMain = gameState->packageMainScript.lock()) {
      packageMain->close();
    }
  }

  void Engine::initializeStageScene() {
    gameState->objTable->deleteStgSceneObject();
    gameState->scriptManager->closeStgSceneScript();
    setStgFrame(32.0f, 16.0f, 416.0f, 464.0f);
    setShotAutoDeleteClip(64.0f, 64.0f, 64.0f, 64.0f);
    reset2DCamera();
    setDefaultBonusItemEnable(true);
    gameState->stageSceneResult = 0;
    gameState->stageMainScript.reset();
    gameState->stagePlayerScript.reset();
    gameState->globalPlayerParams = std::make_shared<GlobalPlayerParams>();
    setStgFrameRenderPriorityMin(DEFAULT_STG_FRAME_RENDER_PRIORITY_MIN);
    setStgFrameRenderPriorityMax(DEFAULT_STG_FRAME_RENDER_PRIORITY_MAX);
    setShotRenderPriority(DEFAULT_SHOT_RENDER_PRIORITY);
    setItemRenderPriority(DEFAULT_ITEM_RENDER_PRIORITY);
    gameState->stagePaused = true;
    gameState->stageForceTerminated = false;
    setDeleteShotImmediateEventOnShotScriptEnable(false);
    setDeleteShotFadeEventOnShotScriptEnable(false);
    setDeleteShotToItemEventOnShotScriptEnable(false);
  }

  void Engine::finalizeStageScene() {
    // FUTURE : impl
  }

  void Engine::startPackage() {
    if (!isPackageFinished()) {
      Logger::WriteLog(Log::Level::LV_WARN, "package already started.");
      return;
    }
    gameState->packageStartTime = std::make_shared<TimePoint>();
    Logger::WriteLog(Log::Level::LV_INFO, "start package.");
    auto script = gameState->scriptManager->compile(gameState->packageMainScriptInfo.path, SCRIPT_TYPE_PACKAGE, gameState->packageMainScriptInfo.version, nullptr);
    gameState->packageMainScript = script;
    script->start();
    script->runInitialize();
  }

  void Engine::setPauseScriptPath(const std::wstring & path) {
    createCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    setAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"PauseScript", std::make_unique<DnhArray>(path));
  }

  void Engine::setEndSceneScriptPath(const std::wstring & path) {
    createCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    setAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"EndSceneScript", std::make_unique<DnhArray>(path));
  }

  void Engine::setReplaySaveSceneScriptPath(const std::wstring & path) {
    createCommonDataArea(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME);
    setAreaCommonData(DEFAULT_PACKAGE_ARGS_COMMON_DATA_AREA_NAME, L"ReplaySaveSceneScript", std::make_unique<DnhArray>(path));
  }

  Point2D Engine::get2DPosition(float x, float y, float z, bool isStgScene) {
    D3DXMATRIX view, proj, viewport, billboard;
    gameState->camera3D->generateViewMatrix(view, billboard);
    if (isStgScene) {
      gameState->camera3D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getStgFrameCenterScreenX(), getStgFrameCenterScreenY(), proj);
    } else {
      gameState->camera3D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getScreenWidth() / 2.0f, getScreenHeight() / 2.0f, proj);
    }
    {
      D3DXMatrixScaling(&viewport, getScreenWidth() / 2.0f, -getScreenHeight() / 2.0f, 1.0f);
      viewport._41 = getScreenWidth() / 2.0f;
      viewport._42 = getScreenHeight() / 2.0f;
    }
    D3DXVECTOR3 pos = D3DXVECTOR3(x, y, z);
    D3DXVec3TransformCoord(&pos, &pos, &(view * proj * viewport));
    if (isStgScene) {
      gameState->camera2D->generateViewMatrix(view);
      gameState->camera2D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getStgFrameCenterScreenX(), getStgFrameCenterScreenY(), proj);
      D3DXMATRIX viewProjViewport = view * proj * viewport;
      D3DXMatrixInverse(&viewProjViewport, NULL, &viewProjViewport);
      D3DXVec3TransformCoord(&pos, &pos, &viewProjViewport);
    }
    return Point2D(pos.x, pos.y);
  }

  void Engine::setStageIndex(uint16_t idx) {
    gameState->stageIdx = idx;
  }

  void Engine::setStageMainScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    try {
      gameState->stageMainScriptInfo = scanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log) {
      log.setLevel(Log::Level::LV_WARN).addSourcePos(srcPos);
      Logger::WriteLog(log);
      gameState->stageMainScriptInfo = ScriptInfo();
      gameState->stageMainScriptInfo.path = path;
    }
  }

  void Engine::setStagePlayerScript(const std::wstring & path, const std::shared_ptr<SourcePos>& srcPos) {
    try {
      gameState->stagePlayerScriptInfo = scanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log) {
      log.setLevel(Log::Level::LV_WARN).addSourcePos(srcPos);
      Logger::WriteLog(log);
      gameState->stageMainScriptInfo = ScriptInfo();
      gameState->stageMainScriptInfo.path = path;
    }
  }

  void Engine::setStageMainScript(const ScriptInfo & script) {
    gameState->stageMainScriptInfo = script;
  }

  void Engine::setStagePlayerScript(const ScriptInfo & script) {
    gameState->stagePlayerScriptInfo = script;
  }

  void Engine::setStageReplayFile(const std::wstring & path) {
    gameState->stageReplayFilePath = path;
  }

  bool Engine::isStageFinished() const {
    if (gameState->stageMainScript.lock()) {
      return false;
    }
    return true;
  }

  int Engine::getStageSceneResult() const {
    return gameState->stageSceneResult;
  }

  bool Engine::isStagePaused() const {
    return gameState->stagePaused;
  }

  void Engine::pauseStageScene(bool doPause) {
    if (doPause && !gameState->stagePaused) {
      notifyEventAll(EV_PAUSE_ENTER);
    } else if (!doPause && gameState->stagePaused) {
      notifyEventAll(EV_PAUSE_LEAVE);
    }
    gameState->stagePaused = doPause;
  }

  void Engine::terminateStageScene() {
    if (auto stageMain = gameState->stageMainScript.lock()) {
      stageMain->close();
      gameState->stageForceTerminated = true;
    }
  }

  void Engine::setPackageMainScript(const std::wstring & path) {
    try {
      gameState->packageMainScriptInfo = scanDnhScriptInfo(path, gameState->fileLoader);
    } catch (Log& log) {
      log.setLevel(Log::Level::LV_WARN);
      Logger::WriteLog(log);
      gameState->stageMainScriptInfo = ScriptInfo();
      gameState->stageMainScriptInfo.path = path;
    }
  }

  void Engine::setPackageMainScript(const ScriptInfo & script) {
    gameState->packageMainScriptInfo = script;
  }

  void Engine::startStageScene(const std::shared_ptr<SourcePos>& srcPos) {
    gameState->objTable->deleteStgSceneObject();
    gameState->scriptManager->closeStgSceneScript();
    gameState->inputDevice->resetInputState();
    reset2DCamera();
    resetCamera();
    setDefaultBonusItemEnable(true);
    gameState->playerShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER);
    gameState->enemyShotDataTable = std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY);
    gameState->itemDataTable = std::make_shared<ItemDataTable>();
    gameState->stageMainScript.reset();
    gameState->stagePlayerScript.reset();
    reloadItemData(DEFAULT_ITEM_DATA_PATH, nullptr);
    gameState->stageSceneResult = 0;
    gameState->stageForceTerminated = false;
    setDeleteShotImmediateEventOnShotScriptEnable(false);
    setDeleteShotFadeEventOnShotScriptEnable(false);
    setDeleteShotToItemEventOnShotScriptEnable(false);
    gameState->stagePaused = false;
    renderer->setFogEnable(false);
    gameState->pseudoPlayerFps = gameState->pseudoEnemyFps = 60;

    if (gameState->stageMainScriptInfo.systemPath.empty() || gameState->stageMainScriptInfo.systemPath == L"DEFAULT") {
      gameState->stageMainScriptInfo.systemPath = DEFAULT_SYSTEM_PATH;
    }

    // #System
    auto systemScript = gameState->scriptManager->compile(gameState->stageMainScriptInfo.systemPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
    systemScript->start();
    systemScript->runInitialize();

    // Player
    Logger::WriteLog(std::move(
      Log(Log::Level::LV_INFO)
      .setMessage("create player object.")
      .addSourcePos(srcPos)));
    auto player = gameState->objTable->create<ObjPlayer>(gameState, gameState->globalPlayerParams);
    gameState->objLayerList->setRenderPriority(player, DEFAULT_PLAYER_RENDER_PRIORITY);
    gameState->playerObj = player;

    auto playerScript = gameState->scriptManager->compile(gameState->stagePlayerScriptInfo.path, SCRIPT_TYPE_PLAYER, gameState->stagePlayerScriptInfo.version, srcPos);
    gameState->stagePlayerScript = playerScript;
    playerScript->start();
    playerScript->runInitialize();

    // Main
    auto stageMainScriptPath = gameState->stageMainScriptInfo.path;
    if (gameState->stageMainScriptInfo.type == SCRIPT_TYPE_SINGLE) {
      stageMainScriptPath = SYSTEM_SINGLE_STAGE_PATH;
    } else if (gameState->stageMainScriptInfo.type == SCRIPT_TYPE_PLURAL) {
      stageMainScriptPath = SYSTEM_PLURAL_STAGE_PATH;
    }
    auto stageMainScript = gameState->scriptManager->compile(stageMainScriptPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
    gameState->stageMainScript = stageMainScript;
    stageMainScript->start();
    stageMainScript->runInitialize();
    gameState->stageStartTime = std::make_shared<TimePoint>();

    // #Background
    if (!gameState->stageMainScriptInfo.backgroundPath.empty() && gameState->stageMainScriptInfo.backgroundPath != L"DEFAULT") {
      auto backgroundScript = gameState->scriptManager->compile(gameState->stageMainScriptInfo.backgroundPath, SCRIPT_TYPE_STAGE, gameState->stageMainScriptInfo.version, srcPos);
      backgroundScript->start();
      backgroundScript->runInitialize();
    }

    // #BGM
    if (!gameState->stageMainScriptInfo.bgmPath.empty() && gameState->stageMainScriptInfo.bgmPath != L"DEFAULT") {
      loadOrphanSound(gameState->stageMainScriptInfo.bgmPath, nullptr);
      auto& bgm = gameState->orphanSounds[canonicalPath(gameState->packageMainScriptInfo.bgmPath)];
      bgm->setLoopEnable(true);
      bgm->play();
    }
  }

  void Engine::renderToTexture(const std::wstring& name, int begin, int end, int objId, bool doClear, bool renderToBackBuffer, bool checkInvalidRenderPriority, bool checkVisibleFlag) {
    if (!gameState) return;

    if (renderToBackBuffer) {
      graphicDevice->setBackbufferRenderTarget();
    } else {
      auto renderTarget = getRenderTarget(name);
      if (!renderTarget) return;
      renderTarget->setRenderTarget();
    }

    D3DXMATRIX viewMatrix2D, projMatrix2D, viewMatrix3D, projMatrix3D, billboardMatrix;
    Camera2D outsideStgFrameCamera2D; outsideStgFrameCamera2D.reset(0, 0);
    renderer->initRenderState();

    if (doClear) {
      graphicDevice->getDevice()->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
    }

    begin = std::max(begin, 0);
    end = std::min(end, MAX_RENDER_PRIORITY);

    auto obj = gameState->objTable->get<ObjRender>(objId);
    if (obj && checkVisibleFlag && !obj->isVisible()) return;

    gameState->camera3D->generateViewMatrix(viewMatrix3D, billboardMatrix);

    // [0, stgFrameMin]
    {
      renderer->disableScissorTest();

      // set 2D matrix
      outsideStgFrameCamera2D.generateViewMatrix(viewMatrix2D);
      outsideStgFrameCamera2D.generateProjMatrix(getScreenWidth(), getScreenHeight(), 0, 0, projMatrix2D);
      renderer->setViewProjMatrix2D(viewMatrix2D, projMatrix2D);

      // set 3D matrix
      gameState->camera3D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getScreenWidth() / 2.0f, getScreenHeight() / 2.0f, projMatrix3D);
      renderer->setViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
      for (int p = 0; p < getStgFrameRenderPriorityMin(); p++) {
        if (obj && obj->getRenderPriority() == p) {
          obj->render();
        }
        if (objId == ID_INVALID && p >= begin && p <= end) {
          if (!(checkInvalidRenderPriority && gameState->objLayerList->isInvalidRenderPriority(p))) {
            gameState->objLayerList->renderLayer(p, isStagePaused(), checkVisibleFlag);
          }
        }
      }
    }
    // [stgFrameMin, stgFrameMax]
    {
      if (!isStagePaused()) {
        RECT scissorRect = { (LONG)gameState->stgFrame->left, (LONG)gameState->stgFrame->top, (LONG)gameState->stgFrame->right, (LONG)gameState->stgFrame->bottom };
        if (renderToBackBuffer) {
          scissorRect.left = gameState->stgFrame->left * graphicDevice->getBackBufferWidth() / gameState->screenWidth;
          scissorRect.top = gameState->stgFrame->top * graphicDevice->getBackBufferHeight() / gameState->screenHeight;
          scissorRect.right = gameState->stgFrame->right * graphicDevice->getBackBufferWidth() / gameState->screenWidth;
          scissorRect.bottom = gameState->stgFrame->bottom * graphicDevice->getBackBufferHeight() / gameState->screenHeight;
        }
        renderer->enableScissorTest(scissorRect);
      }

      // set 2D matrix
      if (!isStageFinished()) {
        gameState->camera2D->generateViewMatrix(viewMatrix2D);
        gameState->camera2D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getStgFrameCenterScreenX(), getStgFrameCenterScreenY(), projMatrix2D);
        renderer->setViewProjMatrix2D(viewMatrix2D, projMatrix2D);
      }

      // set 3D matrix
      gameState->camera3D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getStgFrameCenterScreenX(), getStgFrameCenterScreenY(), projMatrix3D);
      renderer->setViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);

      for (int p = getStgFrameRenderPriorityMin(); p <= getStgFrameRenderPriorityMax(); p++) {
        if (obj && obj->getRenderPriority() == p) {
          obj->render();
        }
        if (objId == ID_INVALID && p >= begin && p <= end) {
          if (!(checkInvalidRenderPriority && gameState->objLayerList->isInvalidRenderPriority(p))) {
            gameState->objLayerList->renderLayer(p, isStagePaused(), checkVisibleFlag);
          }
        }
        if (p == gameState->objLayerList->getCameraFocusPermitRenderPriority()) {
          // cameraFocusPermitRenderPriorityより大きい優先度では別のビュー変換行列を使う
          if (!isStageFinished()) {
            Camera2D focusForbidCamera;
            focusForbidCamera.reset(getStgFrameCenterWorldX(), getStgFrameCenterWorldY());
            focusForbidCamera.generateViewMatrix(viewMatrix2D);
            renderer->setViewProjMatrix2D(viewMatrix2D, projMatrix2D);
          }
        }
      }
    }
    {
    // (stgFrameMax, MAX_RENDER_PRIORITY]
      renderer->disableScissorTest();

      // set 2D matrix
      outsideStgFrameCamera2D.generateViewMatrix(viewMatrix2D);
      outsideStgFrameCamera2D.generateProjMatrix(getScreenWidth(), getScreenHeight(), 0, 0, projMatrix2D);
      renderer->setViewProjMatrix2D(viewMatrix2D, projMatrix2D);

      // set 3D matrix
      gameState->camera3D->generateProjMatrix(getScreenWidth(), getScreenHeight(), getScreenWidth() / 2.0f, getScreenHeight() / 2.0f, projMatrix3D);
      renderer->setViewProjMatrix3D(viewMatrix3D, projMatrix3D, billboardMatrix);
      for (int p = getStgFrameRenderPriorityMax() + 1; p <= MAX_RENDER_PRIORITY; p++) {
        if (obj && obj->getRenderPriority() == p) {
          obj->render();
        }
        if (objId == ID_INVALID && p >= begin && p <= end) {
          if (!(checkInvalidRenderPriority && gameState->objLayerList->isInvalidRenderPriority(p))) {
            gameState->objLayerList->renderLayer(p, isStagePaused(), checkVisibleFlag);
          }
        }
      }
    }
    setBackBufferRenderTarget();
  }

  std::shared_ptr<Obj> Engine::getObj(int id) const {
    return gameState->objTable->get<Obj>(id);
  }

  const std::map<int, std::shared_ptr<Obj>>& Engine::getObjAll() const {
    return gameState->objTable->getAll();
  }
}