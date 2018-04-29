#include <bstorm/game_state.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/fps_counter.hpp>
#include <bstorm/input_device.hpp>
#include <bstorm/real_device_input_source.hpp>
#include <bstorm/file_loader.hpp>
#include <bstorm/sound_device.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/font.hpp>
#include <bstorm/mesh.hpp>
#include <bstorm/obj.hpp>
#include <bstorm/obj_render.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/camera2D.hpp>
#include <bstorm/camera3D.hpp>
#include <bstorm/common_data_db.hpp>
#include <bstorm/script.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/auto_delete_clip.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/config.hpp>

namespace bstorm
{
GameState::GameState(int screenWidth, int screenHeight, HWND hWnd, IDirect3DDevice9* d3DDevice_, const std::shared_ptr<Renderer>& renderer, const std::shared_ptr<conf::KeyConfig>& keyConfig, const std::shared_ptr<MousePositionProvider>& mousePosProvider, Engine* engine) :
    fpsCounter(std::make_shared<FpsCounter>()),
    inputDevice(std::make_shared<InputDevice>(hWnd, mousePosProvider)),
    keyAssign(std::make_shared<KeyAssign>()),
    vKeyInputSource(std::make_shared<RealDeviceInputSource>(inputDevice, keyAssign)),
    soundDevice(std::make_shared<SoundDevice>(hWnd)),
    renderer(renderer),
    objTable(std::make_shared<ObjectTable>()),
    objLayerList(std::make_shared<ObjectLayerList>()),
    colDetector(std::make_shared<CollisionDetector>(screenWidth, screenHeight, std::make_shared<CollisionMatrix>(DEFAULT_COLLISION_MATRIX_DIMENSION, DEFAULT_COLLISION_MATRIX))),
    textureCache(std::make_shared<TextureCache>(d3DDevice_)),
    fontCache(std::make_shared<FontCache>(hWnd, d3DDevice_)),
    meshCache(std::make_shared<MeshCache>()),
    camera2D(std::make_shared<Camera2D>()),
    camera3D(std::make_shared<Camera3D>()),
    commonDataDB(std::make_shared<CommonDataDB>()),
    scriptManager(std::make_shared<ScriptManager>(engine)),
    fileLoader(std::make_shared<FileLoaderFromTextFile>()),
    playerShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER)),
    enemyShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY)),
    itemDataTable(std::make_shared<ItemDataTable>()),
    globalPlayerParams(std::make_shared<GlobalPlayerParams>()),
    stgFrame(std::make_shared<Rect<float>>(32.0f, 16.0f, 416.0f, 464.0f)),
    shotCounter(std::make_shared<ShotCounter>()),
    shotAutoDeleteClip(std::make_shared<AutoDeleteClip>(stgFrame, 64.0f, 64.0f, 64.0f, 64.0f)),
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
    defaultBonusItemEnable(true)
{
    for (const auto& keyMap : keyConfig->keyMaps)
    {
        keyAssign->AddVirtualKey(keyMap.vkey, keyMap.key, keyMap.pad);
    }
}
}