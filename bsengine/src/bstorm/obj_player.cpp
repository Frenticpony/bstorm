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
#include <bstorm/engine_develop_options.hpp>
#include <bstorm/package.hpp>

#undef VK_RIGHT
#undef VK_LEFT
#undef VK_UP
#undef VK_DOWN
#undef VK_CANCEL
#undef VK_PAUSE

namespace bstorm
{
ObjPlayer::ObjPlayer(const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjSprite2D(package),
    ObjMove(this),
    ObjCol(colDetector, package),
    permitPlayerShot_(true),
    permitPlayerSpell_(true),
    state_(STATE_NORMAL),
    normalSpeed_(4.0f),
    slowSpeed_(1.6f),
    clipLeft_(0),
    clipTop_(0),
    clipRight_(384),
    clipBottom_(448),
    invincibilityFrame_(0),
    downStateFrame_(120),
    rebirthFrame_(15),
    rebirthLossFrame_(3),
    autoItemCollectLineY_(-1),
    hitStateTimer_(0),
    downStateTimer_(0),
    currentFrameGrazeCnt_(0)
{
    SetType(OBJ_PLAYER);
    InitPosition();
}

ObjPlayer::~ObjPlayer() {}

void ObjPlayer::Update()
{
    if (auto package = GetPackage().lock())
    {
        if (state_ == STATE_NORMAL)
        {
            MoveByKeyInput();
            ApplyClip();
            if (currentFrameGrazeCnt_ > 0)
            {
                if (auto playerScript = package->GetPlayerScript())
                {
                    // Notify EV_GRAZE
                    auto grazeInfo = std::make_unique<DnhArray>();
                    grazeInfo->PushBack(std::make_unique<DnhReal>((double)currentFrameGrazeCnt_));
                    grazeInfo->PushBack(std::make_unique<DnhArray>(currentFrameGrazeObjIds_));
                    grazeInfo->PushBack(std::make_unique<DnhArray>(currentFrameGrazeShotPoints_));
                    playerScript->NotifyEvent(EV_GRAZE, grazeInfo);
                }
            }
        }
        if (state_ == STATE_HIT)
        {
            if (hitStateTimer_ <= 0)
            {
                ShootDown();
            } else
            {
                hitStateTimer_--;
            }
        }

        // ボム入力処理
        if (state_ == STATE_NORMAL || state_ == STATE_HIT)
        {
            int spellKey = package->GetVirtualKeyState(VK_SPELL);
            if (spellKey == KEY_PUSH)
            {
                CallSpell();
            }
        }

        if (state_ == STATE_DOWN)
        {
            if (downStateTimer_ <= 0)
            {
                Rebirth();
            } else
            {
                downStateTimer_--;
            }
        }
        if (IsInvincible())
        {
            invincibilityFrame_--;
        } else
        {
            invincibilityFrame_ = 0;
        }
        currentFrameGrazeCnt_ = 0;
        currentFrameGrazeObjIds_.clear();
        currentFrameGrazeShotPoints_.clear();
    }
}

void ObjPlayer::Render(const std::shared_ptr<Renderer>& renderer)
{
    ObjSprite2D::Render(renderer);
    ObjCol::RenderIntersection(renderer, IsPermitCamera());
}

void ObjPlayer::AddIntersectionCircleA1(float dx, float dy, float r, float dr)
{
    if (auto package = GetPackage().lock())
    {
        ObjCol::AddIntersection(std::make_shared<PlayerIntersection>(GetX() + dx, GetY() + dy, r, shared_from_this()));
        ObjCol::AddIntersection(std::make_shared<PlayerGrazeIntersection>(GetX() + dx, GetY() + dy, r + dr, shared_from_this()));
    }
}

void ObjPlayer::AddIntersectionCircleA2(float dx, float dy, float r)
{
    if (auto package = GetPackage().lock())
    {
        ObjCol::AddIntersection(std::make_shared<PlayerGrazeIntersection>(GetX() + dx, GetY() + dy, r, shared_from_this()));
    }
}

void ObjPlayer::AddIntersectionToItem()
{
    if (auto package = GetPackage().lock())
    {
        ObjCol::AddIntersection(std::make_shared<PlayerIntersectionToItem>(GetX(), GetY(), shared_from_this()));
    }
}

void ObjPlayer::ClearIntersection()
{
    ObjCol::ClearIntersection();
    AddIntersectionToItem();
}

void ObjPlayer::SetNormalSpeed(double speed)
{
    normalSpeed_ = speed;
}

void ObjPlayer::SetSlowSpeed(double speed)
{
    slowSpeed_ = speed;
}

void ObjPlayer::SetClip(float left, float top, float right, float bottom)
{
    clipLeft_ = left;
    clipTop_ = top;
    clipRight_ = right;
    clipBottom_ = bottom;
}

void ObjPlayer::SetDownStateFrame(int frame)
{
    downStateFrame_ = frame;
}

void ObjPlayer::SetRebirthFrame(int frame)
{
    rebirthFrame_ = frame;
}

void ObjPlayer::SetRebirthLossFrame(int frame)
{
    rebirthLossFrame_ = frame;
}

bool ObjPlayer::IsPermitPlayerSpell() const
{
    if (auto package = GetPackage().lock())
    {
        if (auto bossScene = package->GetEnemyBossSceneObject())
        {
            if (bossScene->IsLastSpell())
            {
                return false;
            }
        }
    }
    return permitPlayerSpell_;
}

bool ObjPlayer::IsLastSpellWait() const
{
    return GetState() == STATE_HIT;
}

bool ObjPlayer::IsSpellActive() const
{
    if (auto package = GetPackage().lock())
    {
        if (package->GetSpellManageObject())
        {
            return true;
        }
    }
    return false;
}

PlayerLife ObjPlayer::GetLife() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerLife();
    }
    return 0;
}

PlayerSpell ObjPlayer::GetSpell() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerSpell();
    }
    return 0;
}

PlayerPower ObjPlayer::GetPower() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerPower();
    }
    return 0;
}


PlayerScore ObjPlayer::GetScore() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerScore();
    }
    return 0;
}

PlayerGraze ObjPlayer::GetGraze() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerGraze();
    }
    return 0;
}

PlayerPoint ObjPlayer::GetPoint() const
{
    if (auto pacakge = GetPackage().lock())
    {
        return pacakge->GetPlayerPoint();
    }
    return 0;
}

void ObjPlayer::SetLife(PlayerLife life)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerLife(life);
    }
}

void ObjPlayer::SetSpell(PlayerSpell spell)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerSpell(spell);
    }
}

void ObjPlayer::SetPower(PlayerPower power)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerPower(power);
    }
}

void ObjPlayer::SetScore(PlayerScore score)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerScore(score);
    }
}

void ObjPlayer::SetGraze(PlayerGraze graze)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerGraze(graze);
    }
}

void ObjPlayer::SetPoint(PlayerPoint point)
{
    if (auto package = GetPackage().lock())
    {
        package->SetPlayerPoint(point);
    }
}

void ObjPlayer::GrazeToShot(int shotObjId, PlayerGraze grazeCnt)
{
    if (auto package = GetPackage().lock())
    {
        if (auto shot = package->GetObject<ObjShot>(shotObjId))
        {
            SetGraze(GetGraze() + grazeCnt);
            currentFrameGrazeCnt_ += grazeCnt;
            currentFrameGrazeObjIds_.push_back((double)shot->GetID());
            currentFrameGrazeShotPoints_.emplace_back(shot->GetX(), shot->GetY());
        }
    }
}

void ObjPlayer::Hit(int collisionObjId)
{
    if (auto package = GetPackage().lock())
    {
        if (package->GetEngineDevelopOptions()->forcePlayerInvincibleEnable) return;
        if (state_ == STATE_NORMAL && !IsInvincible())
        {
            state_ = STATE_HIT;
            hitStateTimer_ = rebirthFrame_;
            // NOTE: 状態を変更してからイベントを送る
            if (auto playerScript = package->GetPlayerScript())
            {
                playerScript->NotifyEvent(EV_HIT, std::make_unique<DnhArray>(std::vector<double>{ (double)collisionObjId }));
            }
            // アイテム自動回収をキャンセル
            package->CancelCollectItems();
        }
    }
}

void ObjPlayer::OnTrans(float dx, float dy)
{
    if (auto package = GetPackage().lock())
    {
        ObjCol::TransIntersection(dx, dy);
    }
}

bool ObjPlayer::IsInvincible() const
{
    return invincibilityFrame_ > 0;
}

void ObjPlayer::ShootDown()
{
    downStateTimer_ = downStateFrame_;
    SetLife(GetLife() - 1);
    if (auto package = GetPackage().lock())
    {
        if (auto bossScene = package->GetEnemyBossSceneObject())
        {
            bossScene->AddPlayerShootDownCount(1);
        }
        package->NotifyEventAll(EV_PLAYER_SHOOTDOWN);
    }
    // Eventを送ってから状態を変更する
    if (GetLife() >= 0)
    {
        state_ = STATE_DOWN;
    } else
    {
        state_ = STATE_END;
    }
    SetVisible(false);
}

void ObjPlayer::Rebirth()
{
    state_ = STATE_NORMAL;
    SetVisible(true);
    if (auto package = GetPackage().lock())
    {
        InitPosition();
        package->NotifyEventAll(EV_PLAYER_REBIRTH);
    }
}

void ObjPlayer::MoveByKeyInput()
{
    if (auto package = GetPackage().lock())
    {
        auto r = package->GetVirtualKeyState(VK_RIGHT);
        auto l = package->GetVirtualKeyState(VK_LEFT);
        auto u = package->GetVirtualKeyState(VK_UP);
        auto d = package->GetVirtualKeyState(VK_DOWN);

        auto shift = package->GetVirtualKeyState(VK_SLOWMOVE);
        bool isSlowMode = shift == KEY_HOLD || shift == KEY_PUSH;
        float speed = (isSlowMode ? slowSpeed_ : normalSpeed_);

        int dx = 0;
        int dy = 0;

        if (r == KEY_HOLD || r == KEY_PUSH) { dx++; }
        if (l == KEY_HOLD || l == KEY_PUSH) { dx--; }
        if (u == KEY_HOLD || u == KEY_PUSH) { dy--; }
        if (d == KEY_HOLD || d == KEY_PUSH) { dy++; }

        SetMovePosition(GetX() + speed * dx, GetY() + speed * dy);
    }
}

void ObjPlayer::ApplyClip()
{
    SetMovePosition(std::min(std::max(GetX(), clipLeft_), clipRight_), std::min(std::max(GetY(), clipTop_), clipBottom_));
}

void ObjPlayer::InitPosition()
{
    if (auto package = GetPackage().lock())
    {
        SetMovePosition(package->GetStgFrameCenterWorldX(), package->GetStgFrameBottom() - 48.0f);
    }
}

void ObjPlayer::CallSpell()
{
    auto package = GetPackage().lock();
    if (!package) return;
    auto playerScript = package->GetPlayerScript();
    bool notExistSpellManageObj = !package->GetSpellManageObject();
    if (notExistSpellManageObj && IsPermitPlayerSpell() && playerScript)
    {
        package->GenerateSpellManageObject();
        playerScript->NotifyEvent(EV_REQUEST_SPELL);
        if (playerScript->GetScriptResult()->ToBool())
        {
            // スペル発動
            if (auto bossScene = package->GetEnemyBossSceneObject())
            {
                bossScene->AddPlayerSpellCount(1);
            }
            if (state_ == STATE_HIT)
            {
                // 喰らいボム
                rebirthFrame_ = std::max(0, rebirthFrame_ - rebirthLossFrame_);
                state_ = STATE_NORMAL;
            }
            package->NotifyEventAll(EV_PLAYER_SPELL);
        } else
        {
            // スペル不発
            if (auto spellManageObj = package->GetSpellManageObject())
            {
                package->DeleteObject(spellManageObj->GetID());
            }
        }
    }
}

void ObjPlayer::ObtainItem(int itemObjId)
{
    if (auto package = GetPackage().lock())
    {
        if (auto item = package->GetObject<ObjItem>(itemObjId))
        {
            if (item->IsScoreItem())
            {
                SetScore(GetScore() + item->GetScore());
            }
            // ボーナスアイテムの場合はイベントを送らない
            if (item->GetItemType() != ITEM_DEFAULT_BONUS)
            {
                int itemType = -1;
                if (item->GetItemType() == ITEM_USER)
                {
                    if (auto itemData = item->GetItemData()) itemType = itemData->type;
                } else
                {
                    itemType = item->GetItemType();
                }
                if (auto package = GetPackage().lock())
                {
                    // EV_GET_ITEM
                    auto evArgs = std::make_unique<DnhArray>(std::vector<double>{ (double)itemType, (double)item->GetID() });
                    if (auto playerScript = package->GetPlayerScript())
                    {
                        playerScript->NotifyEvent(EV_GET_ITEM, evArgs);
                    }
                    if (auto itemScript = package->GetItemScript())
                    {
                        itemScript->NotifyEvent(EV_GET_ITEM, evArgs);
                    }
                }
            }
        }
    }
}

bool ObjPlayer::IsGrazeEnabled() const
{
    return state_ == STATE_NORMAL && !IsInvincible();
}
}