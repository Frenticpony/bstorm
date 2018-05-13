#include <bstorm/package.hpp>

#include <bstorm/renderer.hpp>
#include <bstorm/dnh_const.hpp>
#include <bstorm/time_point.hpp>
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
#include <bstorm/shot_counter.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/camera2D.hpp>
#include <bstorm/camera3D.hpp>
#include <bstorm/common_data_db.hpp>
#include <bstorm/script.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/config.hpp>

#undef CreateFont

namespace bstorm
{
Package::Package(int screenWidth, int screenHeight, HWND hWnd, IDirect3DDevice9* d3DDevice, const std::shared_ptr<conf::KeyConfig>& keyConfig, const std::shared_ptr<MousePositionProvider>& mousePosProvider, Engine* engine) :
    fpsCounter(std::make_shared<FpsCounter>()),
    inputDevice(std::make_shared<InputDevice>(hWnd, mousePosProvider)),
    keyAssign(std::make_shared<KeyAssign>()),
    fileLoader(std::make_shared<FileLoaderFromTextFile>()),
    vKeyInputSource(std::make_shared<RealDeviceInputSource>(inputDevice, keyAssign)),
    soundDevice(std::make_shared<SoundDevice>(hWnd)),
    renderer(std::make_shared<Renderer>(d3DDevice)),
    objTable(std::make_shared<ObjectTable>()),
    objLayerList(std::make_shared<ObjectLayerList>()),
    colDetector(std::make_shared<CollisionDetector>(screenWidth, screenHeight, std::make_shared<CollisionMatrix>(DEFAULT_COLLISION_MATRIX_DIMENSION, DEFAULT_COLLISION_MATRIX))),
    textureCache(std::make_shared<TextureCache>(d3DDevice)),
    meshCache(std::make_shared<MeshCache>(textureCache, fileLoader)),
    camera2D(std::make_shared<Camera2D>()),
    camera3D(std::make_shared<Camera3D>()),
    commonDataDB(std::make_shared<CommonDataDB>()),
    scriptManager(std::make_shared<ScriptManager>(engine)),
    playerShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::PLAYER, textureCache, fileLoader)),
    enemyShotDataTable(std::make_shared<ShotDataTable>(ShotDataTable::Type::ENEMY, textureCache, fileLoader)),
    itemDataTable(std::make_shared<ItemDataTable>(textureCache, fileLoader)),
    shotCounter(std::make_shared<ShotCounter>()),
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
    defaultBonusItemEnable(true),
    stgFrame_(32.0f, 16.0f, 416.0f, 464.0f),
    shotAutoDeleteClip_(64.0f, 64.0f, 64.0f, 64.0f),
    fontCache_(std::make_shared<FontCache>(hWnd, d3DDevice))
{
    for (const auto& keyMap : keyConfig->keyMaps)
    {
        keyAssign->AddVirtualKey(keyMap.vkey, keyMap.key, keyMap.pad);
    }
}

std::shared_ptr<Font> Package::CreateFont(const FontParams * param)
{
    return fontCache_->Create(*param);
}

void Package::ReleaseUnusedFont()
{
    fontCache_->ReleaseUnusedFont();
}

const std::unordered_map<FontParams, std::shared_ptr<Font>>& Package::GetFontMap() const
{
    return fontCache_->GetFontMap();
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
}