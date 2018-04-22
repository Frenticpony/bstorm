#include <bstorm/obj_player.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/virtual_key_input_source.hpp>
#include <bstorm/script.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/obj_spell.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/game_state.hpp>

#undef VK_RIGHT
#undef VK_LEFT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
GlobalPlayerParams::GlobalPlayerParams() :
    life(2),
    spell(3),
    power(1),
    score(0),
    graze(0),
    point(0)
{
}

ObjPlayer::ObjPlayer(const std::shared_ptr<GameState>& gameState, const std::shared_ptr<GlobalPlayerParams>& globalParams) :
    ObjSprite2D(gameState),
    ObjMove(this),
    ObjCol(gameState),
    permitPlayerShot(true),
    permitPlayerSpell(true),
    state(STATE_NORMAL),
    normalSpeed(4.0f),
    slowSpeed(1.6f),
    clipLeft(0),
    clipTop(0),
    clipRight(384),
    clipBottom(448),
    invincibilityFrame(0),
    downStateFrame(120),
    rebirthFrame(15),
    rebirthLossFrame(3),
    globalParams(globalParams),
    autoItemCollectLineY(-1),
    hitStateTimer(0),
    downStateTimer(0),
    currentFrameGrazeCnt(0)
{
    setType(OBJ_PLAYER);
    initPosition();
}

ObjPlayer::~ObjPlayer() {}

void ObjPlayer::update()
{
    if (auto gameState = getGameState())
    {
        if (state == STATE_NORMAL)
        {
            moveByKeyInput();
            applyClip();
            if (currentFrameGrazeCnt > 0)
            {
                if (auto playerScript = gameState->stagePlayerScript.lock())
                {
                    // notify EV_GRAZE
                    auto grazeInfo = std::make_unique<DnhArray>();
                    grazeInfo->PushBack(std::make_unique<DnhReal>((double)currentFrameGrazeCnt));
                    grazeInfo->PushBack(std::make_unique<DnhArray>(currentFrameGrazeObjIds));
                    grazeInfo->PushBack(std::make_unique<DnhArray>(currentFrameGrazeShotPoints));
                    playerScript->notifyEvent(EV_GRAZE, grazeInfo);
                }
            }
        }
        if (state == STATE_HIT)
        {
            if (hitStateTimer <= 0)
            {
                shootDown();
            } else
            {
                hitStateTimer--;
            }
        }

        // ボム入力処理
        // hitStateTimerのカウント処理の後に行う
        if (state == STATE_NORMAL || state == STATE_HIT)
        {
            int spellKey = gameState->vKeyInputSource->getVirtualKeyState(VK_SPELL);
            if (spellKey == KEY_PUSH)
            {
                callSpell();
            }
        }

        if (state == STATE_DOWN)
        {
            if (downStateTimer <= 0)
            {
                rebirth();
            } else
            {
                downStateTimer--;
            }
        }
        if (isInvincible())
        {
            invincibilityFrame--;
        } else
        {
            invincibilityFrame = 0;
        }
        currentFrameGrazeCnt = 0;
        currentFrameGrazeObjIds.clear();
        currentFrameGrazeShotPoints.clear();
    }
}

void ObjPlayer::render()
{
    ObjSprite2D::render();
    ObjCol::renderIntersection(isPermitCamera());
}

void ObjPlayer::addIntersectionCircleA1(float dx, float dy, float r, float dr)
{
    if (auto gameState = getGameState())
    {
        ObjCol::pushIntersection(gameState->colDetector->Create<PlayerIntersection>(getX() + dx, getY() + dy, r, shared_from_this()));
        ObjCol::pushIntersection(gameState->colDetector->Create<PlayerGrazeIntersection>(getX() + dx, getY() + dy, r + dr, shared_from_this()));
    }
}

void ObjPlayer::addIntersectionCircleA2(float dx, float dy, float r)
{
    if (auto gameState = getGameState())
    {
        ObjCol::pushIntersection(gameState->colDetector->Create<PlayerGrazeIntersection>(getX() + dx, getY() + dy, r, shared_from_this()));
    }
}

void ObjPlayer::addIntersectionToItem()
{
    if (auto gameState = getGameState())
    {
        isectToItem = gameState->colDetector->Create<PlayerIntersectionToItem>(getX(), getY(), shared_from_this());
    }
}

void ObjPlayer::setNormalSpeed(double speed)
{
    normalSpeed = speed;
}

void ObjPlayer::setSlowSpeed(double speed)
{
    slowSpeed = speed;
}

void ObjPlayer::setClip(float left, float top, float right, float bottom)
{
    clipLeft = left;
    clipTop = top;
    clipRight = right;
    clipBottom = bottom;
}

void ObjPlayer::setLife(double life) { globalParams->life = life; }

void ObjPlayer::setSpell(double spell) { globalParams->spell = spell; }

void ObjPlayer::setPower(double power) { globalParams->power = power; }

void ObjPlayer::setDownStateFrame(int frame)
{
    downStateFrame = frame;
}

void ObjPlayer::setRebirthFrame(int frame)
{
    rebirthFrame = frame;
}

void ObjPlayer::setRebirthLossFrame(int frame)
{
    rebirthLossFrame = frame;
}

double ObjPlayer::getLife() const { return globalParams->life; }

double ObjPlayer::getSpell() const { return globalParams->spell; }

double ObjPlayer::getPower() const { return globalParams->power; }

bool ObjPlayer::isPermitPlayerSpell() const
{
    if (auto gameState = getGameState())
    {
        auto bossScene = gameState->enemyBossSceneObj.lock();
        if (bossScene && bossScene->isLastSpell())
        {
            return false;
        }
    }
    return permitPlayerSpell;
}

bool ObjPlayer::isLastSpellWait() const
{
    return getState() == STATE_HIT;
}

bool ObjPlayer::isSpellActive() const
{
    if (auto gameState = getGameState())
    {
        if (gameState->spellManageObj.lock())
        {
            return true;
        }
    }
    return false;
}

int64_t ObjPlayer::getScore() const { return globalParams->score; }

int64_t ObjPlayer::getGraze() const { return globalParams->graze; }

int64_t ObjPlayer::getPoint() const { return globalParams->point; }

void ObjPlayer::addScore(int64_t score) { globalParams->score += score; }

void ObjPlayer::addGraze(int64_t graze) { globalParams->graze += graze; }

void ObjPlayer::addGraze(int shotObjId, int64_t graze)
{
    if (auto gameState = getGameState())
    {
        if (auto shot = gameState->objTable->Get<ObjShot>(shotObjId))
        {
            globalParams->graze += graze;
            currentFrameGrazeCnt += graze;
            currentFrameGrazeObjIds.push_back((double)shot->getID());
            currentFrameGrazeShotPoints.emplace_back(shot->getX(), shot->getY());
        }
    }
}

void ObjPlayer::addPoint(int64_t point) { globalParams->point += point; }

void ObjPlayer::hit(int collisionObjId)
{
    if (auto gameState = getGameState())
    {
        if (gameState->forcePlayerInvincibleEnable) return;
        if (state == STATE_NORMAL && !isInvincible())
        {
            state = STATE_HIT;
            hitStateTimer = rebirthFrame;
            // NOTE: 状態を変更してからイベントを送る
            if (auto playerScript = gameState->stagePlayerScript.lock())
            {
                playerScript->notifyEvent(EV_HIT, std::make_unique<DnhArray>(std::vector<double>{ (double)collisionObjId }));
            }
            // アイテム自動回収をキャンセル
            gameState->autoItemCollectionManager->cancelCollectItems();
        }
    }
}

void ObjPlayer::transIntersection(float dx, float dy)
{
    if (auto gameState = getGameState())
    {
        ObjCol::transIntersection(dx, dy);
        if (isectToItem)
        {
            gameState->colDetector->Trans(isectToItem, dx, dy);
        }
    }
}

bool ObjPlayer::isInvincible() const
{
    return invincibilityFrame > 0;
}

void ObjPlayer::shootDown()
{
    downStateTimer = downStateFrame;
    globalParams->life--;
    if (auto gameState = getGameState())
    {
        if (auto bossScene = gameState->enemyBossSceneObj.lock())
        {
            bossScene->addPlayerShootDownCount(1);
        }
        gameState->scriptManager->notifyEventAll(EV_PLAYER_SHOOTDOWN);
    }
    // Eventを送ってから状態を変更する
    if (globalParams->life >= 0)
    {
        state = STATE_DOWN;
    } else
    {
        state = STATE_END;
    }
    setVisible(false);
}

void ObjPlayer::rebirth()
{
    state = STATE_NORMAL;
    setVisible(true);
    if (auto gameState = getGameState())
    {
        initPosition();
        gameState->scriptManager->notifyEventAll(EV_PLAYER_REBIRTH);
    }
}

void ObjPlayer::moveByKeyInput()
{
    if (auto gameState = getGameState())
    {
        auto r = gameState->vKeyInputSource->getVirtualKeyState(VK_RIGHT);
        auto l = gameState->vKeyInputSource->getVirtualKeyState(VK_LEFT);
        auto u = gameState->vKeyInputSource->getVirtualKeyState(VK_UP);
        auto d = gameState->vKeyInputSource->getVirtualKeyState(VK_DOWN);

        auto shift = gameState->vKeyInputSource->getVirtualKeyState(VK_SLOWMOVE);
        bool isSlowMode = shift == KEY_HOLD || shift == KEY_PUSH;
        float speed = (isSlowMode ? slowSpeed : normalSpeed);

        int dx = 0;
        int dy = 0;

        if (r == KEY_HOLD || r == KEY_PUSH) { dx++; }
        if (l == KEY_HOLD || l == KEY_PUSH) { dx--; }
        if (u == KEY_HOLD || u == KEY_PUSH) { dy--; }
        if (d == KEY_HOLD || d == KEY_PUSH) { dy++; }

        setMovePosition(getX() + speed * dx, getY() + speed * dy);
    }
}

void ObjPlayer::applyClip()
{
    setMovePosition(std::min(std::max(getX(), clipLeft), clipRight), std::min(std::max(getY(), clipTop), clipBottom));
}

void ObjPlayer::initPosition()
{
    if (auto gameState = getGameState())
    {
        setMovePosition((gameState->stgFrame->right - gameState->stgFrame->left) / 2.0f, gameState->stgFrame->bottom - 48.0f);
    }
}

void ObjPlayer::callSpell()
{
    auto gameState = getGameState();
    if (!gameState) return;
    auto playerScript = gameState->stagePlayerScript.lock();
    auto spellManageObj = gameState->spellManageObj.lock();
    if ((!spellManageObj || spellManageObj->isDead()) && isPermitPlayerSpell() && playerScript)
    {
        gameState->spellManageObj = gameState->objTable->create<ObjSpellManage>(gameState);
        playerScript->notifyEvent(EV_REQUEST_SPELL);
        if (playerScript->getScriptResult()->ToBool())
        {
            // スペル発動
            if (auto bossScene = gameState->enemyBossSceneObj.lock())
            {
                bossScene->addPlayerSpellCount(1);
            }
            if (state == STATE_HIT)
            {
                // 喰らいボム
                rebirthFrame = std::max(0, rebirthFrame - rebirthLossFrame);
                state = STATE_NORMAL;
            }
            gameState->scriptManager->notifyEventAll(EV_PLAYER_SPELL);
        } else
        {
            // スペル不発
            if (gameState->spellManageObj.lock())
            {
                gameState->objTable->del(gameState->spellManageObj.lock()->getID());
            }
        }
    }
}

void ObjPlayer::getItem(int itemObjId)
{
    if (auto gameState = getGameState())
    {
        if (auto item = gameState->objTable->Get<ObjItem>(itemObjId))
        {
            if (item->isScoreItem())
            {
                addScore(item->getScore());
            }
            // ボーナスアイテムの場合はイベントを送らない
            if (item->getItemType() != ITEM_DEFAULT_BONUS)
            {
                int itemType = -1;
                if (item->getItemType() == ITEM_USER)
                {
                    if (auto itemData = item->getItemData()) itemType = itemData->type;
                } else
                {
                    itemType = item->getItemType();
                }
                if (auto gameState = getGameState())
                {
                    // EV_GET_ITEM
                    auto evArgs = std::make_unique<DnhArray>(std::vector<double>{ (double)itemType, (double)item->getID() });
                    if (auto playerScript = gameState->stagePlayerScript.lock())
                    {
                        playerScript->notifyEvent(EV_GET_ITEM, evArgs);
                    }
                    if (auto itemScript = gameState->itemScript.lock())
                    {
                        itemScript->notifyEvent(EV_GET_ITEM, evArgs);
                    }
                }
            }
        }
    }
}

bool ObjPlayer::isGrazeEnabled() const
{
    return state == STATE_NORMAL && !isInvincible();
}
}