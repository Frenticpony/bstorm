#include <bstorm/obj_shot.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/script.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/collision_matrix.hpp>
#include <bstorm/shot_data.hpp>
#include <bstorm/auto_delete_clip.hpp>
#include <bstorm/rand_generator.hpp>
#include <bstorm/game_state.hpp>

#include <algorithm>
#include <d3dx9.h>

namespace bstorm
{
ShotCounter::ShotCounter() :
    playerShotCount(0),
    enemyShotCount(0)
{
}

ObjShot::ObjShot(bool isPlayerShot, const std::shared_ptr<GameState>& gameState) :
    ObjRender(gameState),
    ObjMove(this),
    ObjCol(gameState),
    playerShotFlag(isPlayerShot),
    registerFlag(false),
    intersectionEnable(true),
    autoDeleteEnable(true),
    spellResistEnable(false),
    spellFactorEnable(false),
    itemChangeEnable(true),
    angularVelocity(0),
    damage(1),
    penetration(5),
    eraseShotEnable(false),
    delayTimer(0),
    deleteFrameTimer(0),
    useDeleteFrameFlag(false),
    sourceBlendType(BLEND_NONE),
    grazeInvalidFlag(false),
    useTempIntersectionFlag(false),
    addedShotFrameCnt(0),
    fadeDeleteFlag(false),
    fadeDeleteFrame(30),
    animationFrameCnt(0),
    animationIdx(0)
{
    setType(OBJ_SHOT);
    setBlendType(BLEND_NONE);
    if (playerShotFlag)
    {
        gameState->shotCounter->playerShotCount++;
    } else
    {
        gameState->shotCounter->enemyShotCount++;
    }
}

ObjShot::~ObjShot()
{
    if (auto state = getGameState())
    {
        for (auto& addedShot : addedShots)
        {
            state->objTable->del(addedShot.objId);
        }
    }
    // FUTURE :デストラクタではなくdead時にカウントする
    if (auto gameState = getGameState())
    {
        if (isPlayerShot())
        {
            gameState->shotCounter->playerShotCount--;
        } else
        {
            gameState->shotCounter->enemyShotCount--;
        }
    }
}

void ObjShot::update()
{
    if (isRegistered())
    {
        if (getPenetration() <= 0) { die(); return; }
        if (!isDelay())
        {
            move();
            checkAutoDelete(getX(), getY());
            if (shotData)
            {
                setAngleZ(getAngleZ() + getAngularVelocity());
                updateAnimationPosition();
            }
        }
        tickAddedShotFrameCount();
        tickDelayTimer();
        tickDeleteFrameTimer();
        tickFadeDeleteTimer();
    }
    clearOldTempIntersection();
}

void ObjShot::render()
{
    if (isRegistered())
    {
        if (shotData)
        {
            float delayScale = 2.0f - (23.0f - std::min(23, getDelay())) / 16.0f; // delay=23から32Fで0になるように設定

            // カーブレーザーのときは遅延光がちょっと大きい
            if (getType() == OBJ_CURVE_LASER)
            {
                delayScale *= 1.8f; // 値は適当
            }

            /* 配置 */
            // NOTE : fixedAngleじゃないなら移動方向に向ける
            D3DXMATRIX world = scaleRotTrans(getX(), getY(), 0.0f,
                                             getAngleX(), getAngleY(), getAngleZ() + (shotData->fixedAngle ? 0.0f : getAngle() + 90.0f),
                                             isDelay() ? delayScale : getScaleX(), isDelay() ? delayScale : getScaleY(), 1.0f);

            /* ブレンド方法の選択 */
            int shotBlend = BLEND_NONE;
            if (!isDelay())
            {
                if (getBlendType() == BLEND_NONE)
                {
                    shotBlend = shotData->render;
                } else
                {
                    /* ObjRenderで指定されたintがある場合はそちらを使う */
                    shotBlend = getBlendType();
                }
            } else
            {
                /* 遅延時間時 */
                if (getSourceBlendType() == BLEND_NONE)
                {
                    shotBlend = shotData->delayRender;
                } else
                {
                    shotBlend = getSourceBlendType();
                }
            }

            // NOTE : ADD_RGBはADD_ARGBとして解釈
            if (getBlendType() == BLEND_NONE && shotBlend == BLEND_ADD_RGB)
            {
                shotBlend = BLEND_ADD_ARGB;
            }

            // 色と透明度
            D3DCOLOR color;
            if (!isDelay())
            {
                color = toD3DCOLOR(getColor(), (int)(getFadeScale() * std::min(shotData->alpha, getAlpha())));
            } else
            {
                // NOTE: 遅延光は透明度反映無し
                color = toD3DCOLOR(shotData->delayColor, 0xff);
            }

            auto vertices = rectToVertices(color, shotData->texture->getWidth(), shotData->texture->getHeight(), isDelay() ? shotData->delayRect :
                (animationIdx >= 0 && animationIdx < shotData->animationData.size()) ? shotData->animationData[animationIdx].rect :
                                           shotData->rect);

            if (auto state = getGameState())
            {
                state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData->texture->getTexture(), shotBlend, world, getAppliedShader(), isPermitCamera(), true);
            }
        }
        renderIntersection();
    }
}

double ObjShot::getDamage() const
{
    return damage;
}

void ObjShot::setDamage(double damage) { this->damage = damage; }

int ObjShot::getPenetration() const
{
    return penetration;
}

void ObjShot::setPenetration(int penetration) { this->penetration = penetration; }

int ObjShot::getDelay() const
{
    return delayTimer;
}

void ObjShot::setDelay(int delay) { delayTimer = std::max(delay, 0); }

bool ObjShot::isDelay() const
{
    return delayTimer > 0;
}

int ObjShot::getSourceBlendType() const
{
    return sourceBlendType;
}

void ObjShot::setSourceBlendType(int blendType)
{
    sourceBlendType = blendType;
}

float ObjShot::getAngularVelocity() const
{
    return angularVelocity;
}

void ObjShot::setAngularVelocity(float angularVelocity)
{
    this->angularVelocity = angularVelocity;
}

bool ObjShot::isSpellResistEnabled() const { return spellResistEnable; }

void ObjShot::setSpellResist(bool enable) { spellResistEnable = enable; }

bool ObjShot::isSpellFactorEnabled() const { return spellFactorEnable; }

void ObjShot::setSpellFactor(bool enable) { spellFactorEnable = enable; }

bool ObjShot::isEraseShotEnabled() const { return eraseShotEnable; }

void ObjShot::setEraseShot(bool erase) { eraseShotEnable = erase; }

bool ObjShot::isItemChangeEnabled() const
{
    return itemChangeEnable;
}

void ObjShot::setItemChange(bool enable) { itemChangeEnable = enable; }

bool ObjShot::isAutoDeleteEnabled() const
{
    return autoDeleteEnable;
}

void ObjShot::setAutoDeleteEnable(bool enable)
{
    autoDeleteEnable = enable;
}

void ObjShot::addIntersection(const std::shared_ptr<ShotIntersection>& isect)
{
    if (!useTempIntersectionFlag)
    {
        if (auto state = getGameState())
        {
            state->colDetector->add(isect);
        }
        ObjCol::pushIntersection(isect);
    }
}

void ObjShot::addTempIntersection(const std::shared_ptr<ShotIntersection>& isect)
{
    if (!useTempIntersectionFlag)
    {
        useTempIntersectionFlag = true;
        clearIntersection();
    }
    if (auto state = getGameState())
    {
        state->colDetector->add(isect);
    }
    ObjCol::addTempIntersection(isect);
}

void ObjShot::addTempIntersectionCircleA1(float r)
{
    addTempIntersectionCircleA2(getX(), getY(), r);
}

void ObjShot::addTempIntersectionCircleA2(float x, float y, float r)
{
    addTempIntersection(std::make_shared<ShotIntersection>(x, y, r, this, true));
}

void ObjShot::addTempIntersectionLine(float x1, float y1, float x2, float y2, float width)
{
    addTempIntersection(std::make_shared<ShotIntersection>(x1, y1, x2, y2, width, this, true));
}

bool ObjShot::isIntersectionEnabled() const
{
    return intersectionEnable;
}

void ObjShot::setIntersectionEnable(bool enable) { intersectionEnable = enable; }

bool ObjShot::isTempIntersectionMode() const
{
    return useDeleteFrameFlag;
}

bool ObjShot::isRegistered() const { return registerFlag; }

void ObjShot::regist()
{
    registerFlag = true;
}

bool ObjShot::isPlayerShot() const { return playerShotFlag; }

const std::shared_ptr<ShotData>& ObjShot::getShotData() const
{
    return shotData;
}

void ObjShot::setShotData(const std::shared_ptr<ShotData>& data)
{
    if (data->texture)
    {
        shotData = data;
        clearIntersection();
        if (shotData)
        {
            angularVelocity = shotData->angularVelocity;
            if (auto state = getGameState())
            {
                if (!useTempIntersectionFlag)
                {
                    for (const auto& col : shotData->collisions)
                    {
                        auto isect = std::make_shared<ShotIntersection>(getX() + col.x, getY() + col.y, col.r, this, false);
                        state->colDetector->add(isect);
                        ObjCol::pushIntersection(isect);
                    }
                }
                if (shotData->useAngularVelocityRand)
                {
                    angularVelocity = state->randGenerator->randDouble(shotData->angularVelocityRandMin, shotData->angularVelocityRandMax);
                }
            }
        }
    }
}

int ObjShot::getAnimationFrameCount() const
{
    return animationFrameCnt;
}

int ObjShot::getAnimationIndex() const
{
    return animationIdx;
}

void ObjShot::addShotA1(int shotObjId, int frame)
{
    if (isDead()) return;
    if (auto state = getGameState())
    {
        if (auto shot = state->objTable->get<ObjShot>(shotObjId))
        {
            shot->registerFlag = false;
            addedShots.emplace_back(shotObjId, frame);
        }
    }
}

void ObjShot::addShotA2(int shotObjId, int frame, float dist, float angle)
{
    if (isDead()) return;
    if (auto state = getGameState())
    {
        if (auto shot = state->objTable->get<ObjShot>(shotObjId))
        {
            shot->registerFlag = false;
            addedShots.emplace_back(shotObjId, frame, dist, angle);
        }
    }
}

int ObjShot::getFrameCountForAddShot() const
{
    return addedShotFrameCnt;
}

const std::list<ObjShot::AddedShot>& ObjShot::getAddedShot() const
{
    return addedShots;
}

void ObjShot::generateDefaultBonusItem()
{
    if (auto state = getGameState())
    {
        state->defaultBonusItemSpawner->spawn(getX(), getY(), state);
    }
}

void ObjShot::toItem()
{
    if (isDead()) return;
    if (isItemChangeEnabled())
    {
        if (auto gameState = getGameState())
        {
            // EV_DELETE_SHOT_TO_ITEM 
            auto evArgs = std::make_unique<DnhArray>();
            evArgs->pushBack(std::make_unique<DnhReal>(getID()));
            evArgs->pushBack(std::make_unique<DnhArray>(Point2D(getX(), getY())));
            if (auto itemScript = gameState->itemScript.lock())
            {
                itemScript->notifyEvent(EV_DELETE_SHOT_TO_ITEM, evArgs);
            }
            if (gameState->deleteShotToItemEventOnShotScriptEnable)
            {
                if (auto shotScript = gameState->shotScript.lock())
                {
                    shotScript->notifyEvent(EV_DELETE_SHOT_TO_ITEM, evArgs);
                }
            }
            if (gameState->defaultBonusItemEnable)
            {
                generateDefaultBonusItem();
            }
        }
    }
    die();
}

void ObjShot::eraseWithSpell()
{
    if (!isSpellResistEnabled())
    {
        toItem();
    }
}

void ObjShot::deleteImmediate()
{
    if (isDead()) return;
    if (auto gameState = getGameState())
    {
        // EV_DELETE_SHOT_IMMEDIATE
        if (gameState->deleteShotImmediateEventOnShotScriptEnable)
        {
            if (auto shotScript = gameState->shotScript.lock())
            {
                auto evArgs = std::make_unique<DnhArray>();
                evArgs->pushBack(std::make_unique<DnhReal>(getID()));
                evArgs->pushBack(std::make_unique<DnhArray>(Point2D(getX(), getY())));
                shotScript->notifyEvent(EV_DELETE_SHOT_IMMEDIATE, evArgs);
            }
        }
    }
    die();
}

void ObjShot::fadeDelete()
{
    if (!isRegistered()) return;
    if (fadeDeleteFlag) return;
    fadeDeleteFlag = true;
    fadeDeleteTimer = fadeDeleteFrame;
}

float ObjShot::getFadeScale() const
{
    if (isFadeDeleteStarted())
    {
        return 1.0f * fadeDeleteTimer / fadeDeleteFrame;
    }
    return 1.0f;
}

bool ObjShot::isFadeDeleteStarted() const
{
    return fadeDeleteFlag;
}

bool ObjShot::isFrameDeleteStarted() const
{
    return useDeleteFrameFlag;
}

int ObjShot::getDeleteFrameTimer() const
{
    return deleteFrameTimer;
}

int ObjShot::getFadeDeleteFrameTimer() const
{
    return fadeDeleteTimer;
}

void ObjShot::setDeleteFrame(int frame)
{
    useDeleteFrameFlag = true;
    deleteFrameTimer = frame;
}

void ObjShot::transIntersection(float dx, float dy)
{
    ObjCol::transIntersection(dx, dy);
}

void ObjShot::renderIntersection()
{
    ObjCol::renderIntersection(isPermitCamera());
}

void ObjShot::checkAutoDelete(float x, float y)
{
    if (auto state = getGameState())
    {
        if (autoDeleteEnable && state->shotAutoDeleteClip->outOfClip(x, y))
        {
            die();
        }
    }
}

void ObjShot::updateAnimationPosition()
{
    if (shotData)
    {
        if (animationIdx < 0 || animationIdx >= shotData->animationData.size())
        {
            animationFrameCnt = animationIdx = 0;
            return;
        }
        animationFrameCnt++;
        if (shotData->animationData[animationIdx].frame <= animationFrameCnt)
        {
            animationFrameCnt = 0;
            animationIdx++;
            if (animationIdx >= shotData->animationData.size()) animationIdx = 0;
        }
    }
}

void ObjShot::tickDelayTimer()
{
    delayTimer = std::max(0, delayTimer - 1);
}

void ObjShot::tickDeleteFrameTimer()
{
    if (useDeleteFrameFlag)
    {
        if (!isDelay())
        {
            if (deleteFrameTimer <= 0)
            {
                deleteImmediate();
            }
            deleteFrameTimer--;
        }
    }
}

void ObjShot::tickAddedShotFrameCount()
{
    if (isDelay() || isDead()) return;
    auto it = addedShots.begin();
    while (it != addedShots.end())
    {
        if (it->frame == addedShotFrameCnt)
        {
            if (auto state = getGameState())
            {
                if (auto shot = state->objTable->get<ObjShot>(it->objId))
                {
                    float dx = 0;
                    float dy = 0;
                    if (it->type == AddedShot::Type::A1)
                    {
                        dx = shot->getX();
                        dy = shot->getX();
                    } else if (it->type == AddedShot::Type::A2)
                    {
                        float baseAngle = 0;
                        auto stLaser = dynamic_cast<ObjStLaser*>(this);
                        if (stLaser != NULL)
                        {
                            baseAngle = stLaser->getLaserAngle();
                            shot->setAngle(baseAngle + it->angle);
                        } else
                        {
                            baseAngle = getAngle();
                        }
                        float dir = D3DXToRadian(baseAngle + it->angle);
                        dx = it->dist * cos(dir);
                        dy = it->dist * sin(dir);
                    }
                    shot->setMovePosition(getX() + dx, getY() + dy);
                    shot->regist();
                }
            }
            it = addedShots.erase(it);
        } else
        {
            it++;
        }
    }
    addedShotFrameCnt++;
}

void ObjShot::tickFadeDeleteTimer()
{
    if (!isFadeDeleteStarted() || isDead()) return;
    fadeDeleteTimer--;
    if (fadeDeleteTimer <= 0)
    {
        if (auto gameState = getGameState())
        {
            //EV_DELETE_SHOT_FADE
            if (gameState->deleteShotFadeEventOnShotScriptEnable)
            {
                if (auto shotScript = gameState->shotScript.lock())
                {
                    auto evArgs = std::make_unique<DnhArray>();
                    evArgs->pushBack(std::make_unique<DnhReal>(getID()));
                    evArgs->pushBack(std::make_unique<DnhArray>(Point2D{ getX(), getY() }));
                    shotScript->notifyEvent(EV_DELETE_SHOT_FADE, evArgs);
                }
            }
        }
        die();
    }
}

void ObjShot::graze()
{
    grazeInvalidFlag = true;
}

bool ObjShot::isGrazeEnabled() const
{
    return !grazeInvalidFlag;
}

ObjShot::AddedShot::AddedShot(int objId, int frame) :
    type(Type::A1),
    objId(objId),
    frame(frame),
    dist(0),
    angle(0)
{
}

ObjShot::AddedShot::AddedShot(int objId, int frame, float dist, float angle) :
    type(Type::A2),
    objId(objId),
    frame(frame),
    dist(dist),
    angle(angle)
{
}

ObjLaser::ObjLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState) :
    ObjShot(isPlayerShot, gameState),
    length(0),
    renderWidth(0),
    intersectionWidth(0),
    hasIntersectionWidth(false),
    grazeInvalidFrame(20),
    grazeInvalidTimer(0),
    itemDistance(25)
{
    setSpellResist(true);
    setPenetration(1 << 24);
}

void ObjLaser::setShotData(const std::shared_ptr<ShotData>& shotData)
{
    this->shotData = shotData;
}

bool ObjLaser::isGrazeEnabled() const
{
    return !grazeInvalidFlag && grazeInvalidFrame > 0;
}

void ObjLaser::graze()
{
    grazeInvalidFlag = true;
    grazeInvalidTimer = grazeInvalidFrame;
}

void ObjLaser::setLength(float limit)
{
    length = limit;
}

float ObjLaser::getRenderWidth() const { return renderWidth; }

float ObjLaser::getLength() const
{
    return length;
}

void ObjLaser::setRenderWidth(float width)
{
    renderWidth = width;
    if (!hasIntersectionWidth)
    {
        setIntersectionWidth(width / 2);
    }
}

float ObjLaser::getIntersectionWidth() const
{
    return intersectionWidth;
}

void ObjLaser::setIntersectionWidth(float width)
{
    width = abs(width);
    if (intersectionWidth != width)
    {
        ObjCol::setWidthIntersection(width);
        intersectionWidth = width;
    }
    hasIntersectionWidth = true;
}

float ObjLaser::getGrazeInvalidFrame() const
{
    return grazeInvalidFrame;
}

void ObjLaser::setGrazeInvalidFrame(int frame)
{
    grazeInvalidFrame = frame;
}

float ObjLaser::getGrazeInvalidTimer() const
{
    return grazeInvalidTimer;
}

float ObjLaser::getItemDistance() const
{
    return itemDistance;
}

void ObjLaser::setItemDistance(float distance)
{
    itemDistance = abs(distance);
}

void ObjLaser::tickGrazeInvalidTimer()
{
    if (grazeInvalidFlag)
    {
        grazeInvalidTimer--;
    }
    if (grazeInvalidTimer == 0)
    {
        grazeInvalidFlag = false;
    }
}

ObjLooseLaser::ObjLooseLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState) :
    ObjLaser(isPlayerShot, gameState),
    renderLength(0),
    invalidLengthHead(0),
    invalidLengthTail(0),
    defaultInvalidLengthEnable(true)
{
    setType(OBJ_LOOSE_LASER);
}

void ObjLooseLaser::update()
{
    if (isRegistered())
    {
        if (getPenetration() <= 0) { die(); return; }
        if (!isDelay())
        {
            move();
            extend();
            Point2D tail = getTail();
            checkAutoDelete(tail.x, tail.y);
            updateAnimationPosition();
        }
        tickAddedShotFrameCount();
        tickDelayTimer();
        tickDeleteFrameTimer();
        tickFadeDeleteTimer();
        tickGrazeInvalidTimer();
    }
    clearOldTempIntersection();
}

void ObjLooseLaser::render()
{
    if (isRegistered())
    {
        if (const auto& shotData = getShotData())
        {
            if (isDelay())
            {
                ObjShot::render();
            } else
            {
                renderLaser(getRenderWidth(), renderLength, getAngle());
            }
        }
    }
}

void ObjLooseLaser::generateDefaultBonusItem()
{
    if (auto state = getGameState())
    {
        const Point2D head = getHead();
        const Point2D tail = getTail();
        const float dist = getItemDistance();
        const float dx = dist * (tail.x - head.x) / getRenderLength();
        const float dy = dist * (tail.y - head.y) / getRenderLength();
        float distSum = 0;
        float x = head.x;
        float y = head.y;
        if (dist <= 0) return; // 無限ループ防止
        while (true)
        {
            state->defaultBonusItemSpawner->spawn(x, y, state);
            x += dx;
            y += dy;
            distSum += dist;
            if (distSum > getRenderLength()) break;
        }
    }
}

float ObjLooseLaser::getInvalidLengthHead() const
{
    if (defaultInvalidLengthEnable)
    {
        return getRenderLength() * 0.1;
    }
    return invalidLengthHead;
}

float ObjLooseLaser::getInvalidLengthTail() const
{
    if (defaultInvalidLengthEnable)
    {
        return getRenderLength() * 0.1;
    }
    return invalidLengthTail;
}

bool ObjLooseLaser::isDefaultInvalidLengthEnabled() const
{
    return defaultInvalidLengthEnable;
}

void ObjLooseLaser::setDefaultInvalidLengthEnable(bool enable)
{
    defaultInvalidLengthEnable = enable;
}

void ObjLooseLaser::setInvalidLength(float head, float tail)
{
    setDefaultInvalidLengthEnable(false);
    invalidLengthHead = std::max(0.0f, head);
    invalidLengthTail = std::max(0.0f, tail);
}

Point2D ObjLooseLaser::getHead() const
{
    return Point2D(getX(), getY());
}

Point2D ObjLooseLaser::getTail() const
{
    float rdir = D3DXToRadian(getAngle() + 180.0f); // reverse dir
    if (getSpeed() < 0) rdir += D3DX_PI;
    float dx = getRenderLength() * cos(rdir);
    float dy = getRenderLength() * sin(rdir);
    return Point2D(getX() + dx, getY() + dy);
}

float ObjLooseLaser::getRenderLength() const
{
    return renderLength;
}

void ObjLooseLaser::updateIntersection()
{
    if (!useTempIntersectionFlag)
    {
        clearIntersection();
        if (getInvalidLengthHead() + getInvalidLengthTail() <= getRenderLength())
        {
            Point2D head = getHead();
            Point2D tail = getTail();
            // 新しい判定を追加
            // dir : head - tail
            float cosDir = (head.x - tail.x) / getRenderLength();
            float sinDir = (head.y - tail.y) / getRenderLength();
            float isectTailX = cosDir * getInvalidLengthTail() + tail.x;
            float isectTailY = sinDir * getInvalidLengthTail() + tail.y;
            float isectHeadX = -cosDir * getInvalidLengthHead() + head.x;
            float isectHeadY = -sinDir * getInvalidLengthHead() + head.y;
            addIntersection(std::make_shared<ShotIntersection>(isectTailX, isectTailY, isectHeadX, isectHeadY, getIntersectionWidth(), this, false));
        }
    }
}

void ObjLooseLaser::renderLaser(float width, float length, float angle)
{
    if (const auto& shotData = getShotData())
    {
        const auto head = getHead();
        const auto tail = getTail();
        float centerX = (head.x + tail.x) / 2;
        float centerY = (head.y + tail.y) / 2;

        /* ブレンド方法の選択 */
        // NOTE : shotDataのrenderは使わない
        int laserBlend = getBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : getBlendType();

        // 色と透明度を設定
        D3DCOLOR color = toD3DCOLOR(getColor(), ((int)(getFadeScale() * std::min(shotData->alpha, getAlpha()))));
        auto vertices = rectToVertices(color, shotData->texture->getWidth(), shotData->texture->getHeight(), (getAnimationIndex() >= 0 && getAnimationIndex() < shotData->animationData.size()) ? shotData->animationData[getAnimationIndex()].rect : shotData->rect);

        /* 配置 */
        float rectWidth = abs(vertices[0].x - vertices[1].x);
        float rectHeight = abs(vertices[0].y - vertices[2].y);
        D3DXMATRIX world = scaleRotTrans(centerX, centerY, 0.0f, 0.0f, 0.0f, angle + 90.0f, width / rectWidth, length / rectHeight, 1.0f);

        if (auto state = getGameState())
        {
            state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData->texture->getTexture(), laserBlend, world, getAppliedShader(), isPermitCamera(), false);
        }
    }
}

void ObjLooseLaser::extend()
{
    float dl = abs(getSpeed());
    if (renderLength + dl <= getLength())
    {
        renderLength += dl;
        updateIntersection();
    }
}

ObjStLaser::ObjStLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState) :
    ObjLooseLaser(isPlayerShot, gameState),
    laserAngle(270),
    laserSourceEnable(true),
    laserWidthScale(0)
{
    setType(OBJ_STRAIGHT_LASER);
}

void ObjStLaser::update()
{
    if (isRegistered())
    {
        if (getPenetration() <= 0) die();
        if (!isDelay())
        {
            move();
            checkAutoDelete(getX(), getY());
            updateIntersection();
            updateAnimationPosition();
        }
        tickAddedShotFrameCount();
        tickDelayTimer();
        tickDeleteFrameTimer();
        tickFadeDeleteTimer();
        tickGrazeInvalidTimer();
        // 10フレームで1倍になるようにする
        laserWidthScale = isDelay() ? 0.0f : std::min(1.0f, laserWidthScale + 0.1f);
    }
    clearOldTempIntersection();
}

void ObjStLaser::render()
{
    if (isRegistered())
    {
        if (const auto& shotData = getShotData())
        {
            if (laserSourceEnable && !isFadeDeleteStarted())
            {
                // レーザー源の描画
                auto vertices = rectToVertices(toD3DCOLOR(shotData->delayColor, 0xff), shotData->texture->getWidth(), shotData->texture->getHeight(), shotData->delayRect);

                /* 配置 */
                const Point2D head = getHead();
                float rectWidth = abs(vertices[0].x - vertices[1].x);
                float rectHeight = abs(vertices[0].y - vertices[2].y);
                float renderWidth = getRenderWidth() * 1.3125f; // レーザーの幅よりちょっと大きい
                auto world = scaleRotTrans(head.x, head.y, 0.0f,
                                           0.0f, 0.0f, getLaserAngle() - 90.0f,
                                           renderWidth / rectWidth, renderWidth / rectHeight, 1.0f);

                /* ブレンド方法の選択 */
                // NOTE :  delay_renderは使用しない
                int laserBlend = getSourceBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : getSourceBlendType();
                if (auto state = getGameState())
                {
                    state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), shotData->texture->getTexture(), laserBlend, world, getAppliedShader(), isPermitCamera(), false);
                }
            }
            // 遅延時間時は予告線
            float renderWidth = isDelay() ? getRenderWidth() / 20.0f : getRenderWidth() * laserWidthScale;
            renderWidth *= -1; // 左右反転してる
            renderLaser(renderWidth, getLength(), laserAngle);
        }
    }
    renderIntersection();
}

Point2D ObjStLaser::getTail() const
{
    float dir = D3DXToRadian(laserAngle);
    float tailX = getX() + getLength() * cos(dir);
    float tailY = getY() + getLength() * sin(dir);
    return Point2D(tailX, tailY);
}

float ObjStLaser::getLaserAngle() const
{
    return laserAngle;
}

void ObjStLaser::setLaserAngle(float angle)
{
    laserAngle = angle;
}

bool ObjStLaser::hasSource() const
{
    return laserSourceEnable;
}

void ObjStLaser::setSource(bool source)
{
    laserSourceEnable = source;
}

float ObjStLaser::getRenderLength() const
{
    return getLength();
}

ObjCrLaser::ObjCrLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState) :
    ObjLaser(isPlayerShot, gameState),
    totalLaserLength(0),
    tailPos(0),
    hasHead(false),
    shrinkThresholdOffset((size_t)(this) & 0xff),
    tipDecrement(1)
{
    setType(OBJ_CURVE_LASER);
}

void ObjCrLaser::update()
{
    if (isRegistered())
    {
        if (getPenetration() <= 0) { die(); return; }
        // 遅延時も動く
        move();
        if (tailPos != trail.size())
        {
            float tailX = (trail[tailPos].x + trail[tailPos + 1].x) / 2;
            float tailY = (trail[tailPos].y + trail[tailPos + 1].y) / 2;
            checkAutoDelete(tailX, tailY);
        }
        extend(getX(), getY());
        // NOTE : 描画にPrimitiveUpを使うので、trailはvectorである必要がある(メモリの連続性が必要)
        // 節の長さが上限を超えた時は末尾の節から削除していく必要があるが、毎フレームeraseを呼ぶのは遅いので
        // 末尾の位置(tailPos)を進めて、後でtailPosより前の部分をまとめてeraseする。
        // eraseするフレームが他のレーザーと被らないようにOffsetで閾値をずらしている
        if (tailPos >= (int)getLength() + shrinkThresholdOffset)
        {
            trail.erase(trail.begin(), trail.begin() + tailPos);
            tailPos = 0;
        }
        updateAnimationPosition();

        tickAddedShotFrameCount();
        tickDelayTimer();
        tickDeleteFrameTimer();
        tickFadeDeleteTimer();
        tickGrazeInvalidTimer();
    }
    clearOldTempIntersection();
}

void ObjCrLaser::render()
{
    if (isRegistered())
    {
        if (getLaserNodeCount() <= 0) return;
        if (const auto& shotData = getShotData())
        {
            if (isDelay())
            {
                ObjShot::render();
            } else
            {
                // uv算出用
                auto laserRect = rectToVertices(0, shotData->texture->getWidth(), shotData->texture->getHeight(), (getAnimationIndex() >= 0 && getAnimationIndex() < shotData->animationData.size()) ? shotData->animationData[getAnimationIndex()].rect : shotData->rect);

                // trailに色、UV値をセットする
                // レーザー中心の透明度
                const int baseAlpha = (int)(getFadeScale() * std::min(getAlpha(), shotData->alpha));
                // 先端~中心の減少の変化率, tipDecrementから0に変化するので負
                const float ddecr = -tipDecrement / (getLaserNodeCount() >> 1);
                float lengthSum = 0;
                float decr = tipDecrement;
                for (int i = tailPos, lengthCnt = 0; i < trail.size(); i += 2, lengthCnt++)
                {
                    Vertex& v1 = trail[i];
                    Vertex& v2 = trail[i + 1];
                    v1.u = laserRect[0].u;
                    v2.u = laserRect[1].u;
                    v1.v = v2.v = laserRect[2].v - lengthSum / totalLaserLength * (laserRect[2].v - laserRect[1].v);

                    int alpha = (int)(baseAlpha * (1 - abs(decr)));
                    decr += ddecr;

                    v1.color = v2.color = toD3DCOLOR(getColor(), alpha);
                    if (lengthCnt < laserNodeLengthList.size())
                    {
                        lengthSum += laserNodeLengthList[lengthCnt];
                    }
                }

                // 描画
                D3DXMATRIX world;
                D3DXMatrixIdentity(&world);
                // ShotDataのrenderは使わない
                if (auto state = getGameState())
                {
                    int laserBlend = getBlendType() == BLEND_NONE ? BLEND_ADD_ARGB : getBlendType();
                    state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, trail.size() - tailPos, &trail[tailPos], shotData->texture->getTexture(), laserBlend, world, getAppliedShader(), isPermitCamera(), false);
                }
            }
        }
    }
    renderIntersection();
}

void ObjCrLaser::generateDefaultBonusItem()
{
    if (auto state = getGameState())
    {
        if (getLaserNodeCount() > 0)
        {
            for (int i = tailPos; i < trail.size(); i += 2)
            {
                float x = (trail[i].x + trail[i + 1].x) / 2;
                float y = (trail[i].y + trail[i + 1].y) / 2;
                state->defaultBonusItemSpawner->spawn(x, y, state);
            }
        }
    }
}

void ObjCrLaser::setRenderWidth(float width)
{
    if (getRenderWidth() != width)
    {
        fixVertexDistance(width);
    }
    ObjLaser::setRenderWidth(width);
}

float ObjCrLaser::getTipDecrement() const
{
    return tipDecrement;
}

void ObjCrLaser::fixVertexDistance(float width)
{
    // 頂点の間隔を再設定
    float dw = width - getRenderWidth();
    float prevX = 0;
    float prevY = 0;
    for (int i = tailPos; i < trail.size(); i += 2)
    {
        Vertex& v1 = trail[i];
        Vertex& v2 = trail[i + 1];

        float centerX = (v1.x + v2.x) / 2;
        float centerY = (v1.y + v2.y) / 2;

        if (i > tailPos)
        {
            float rot = atan2(centerY - prevY, centerX - prevX) + D3DX_PI / 2;
            float dx = cos(rot) * dw / 2;
            float dy = sin(rot) * dw / 2;

            if (i == tailPos + 1)
            {
                trail[i - 2].x += dx; trail[i - 2].y += dy;
                trail[i - 1].x -= dx; trail[i - 1].y -= dy;
            }

            v1.x += dx; v1.y += dy;
            v2.x -= dx; v2.y -= dy;
        }

        prevX = centerX;
        prevY = centerY;
    }
}

void ObjCrLaser::setTipDecrement(float dec)
{
    tipDecrement = constrain(dec, 0.0f, 1.0f);
}

void ObjCrLaser::extend(float x, float y)
{
    if (hasHead)
    {
        // 進行方向+90度
        const float normalDir = atan2(y - headY, x - headX) + D3DX_PI / 2.0f;
        const float halfWidth = getRenderWidth() / 2.0f;
        const float dx = halfWidth * cos(normalDir);
        const float dy = halfWidth * sin(normalDir);

        if (tailPos == trail.size())
        {
            trail.emplace_back(headX + dx, headY + dy, 0.0f, 0, 0.0f, 0.0f);
            trail.emplace_back(headX - dx, headY - dy, 0.0f, 0, 0.0f, 0.0f);
        }

        trail.emplace_back(x + dx, y + dy, 0.0f, 0, 0.0f, 0.0f);
        trail.emplace_back(x - dx, y - dy, 0.0f, 0, 0.0f, 0.0f);

        float laserNodeLength = std::hypotf(x - headX, y - headY);
        totalLaserLength += laserNodeLength;
        laserNodeLengthList.push_back(laserNodeLength);

        if (!useTempIntersectionFlag)
        {
            addIntersection(std::make_shared<ShotIntersection>(headX, headY, x, y, getIntersectionWidth(), this, false));
        }

        // カーブレーザーの場合
        // length : レーザーを構成する頂点の組の数
        if (getLaserNodeCount() + 1 > (int)getLength())
        {
            tailPos += 2;
            totalLaserLength -= laserNodeLengthList[0];
            laserNodeLengthList.pop_front();
            if (getIntersections().size() > 0)
            {
                ObjCol::shiftIntersection();
            }
        }
    } else
    {
        hasHead = true;
    }
    headX = x;
    headY = y;
}

int ObjCrLaser::getLaserNodeCount() const
{
    return laserNodeLengthList.size();
}
}