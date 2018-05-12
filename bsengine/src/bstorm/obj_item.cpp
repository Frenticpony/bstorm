#include <bstorm/obj_item.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/package.hpp>

namespace bstorm
{
ObjItem::ObjItem(int itemType, const std::shared_ptr<Package>& package) :
    ObjRender(package),
    ObjMove(this),
    ObjCol(package),
    itemType_(itemType),
    score_(0),
    autoCollectEnable_(true),
    isAutoCollected_(false),
    renderScoreEnable_(true),
    autoDeleteClipLeft_(-64),
    autoDeleteClipRight_(704),
    autoDeleteClipBottom_(544),
    autoCollectSpeed_(8.0f),
    isObtained_(false),
    animationFrameCnt_(0),
    animationIdx_(0)
{
    SetType(OBJ_ITEM);
    SetBlendType(BLEND_NONE);

    if (itemType != ITEM_USER)
    {
        itemData_ = package->itemDataTable->Get(itemType);
    }
}

ObjItem::~ObjItem()
{
}

void ObjItem::SetIntersection()
{
    if (auto package = GetPackage().lock())
    {
        if (GetIntersections().size() == 0)
        {
            PushBackIntersection(package->colDetector->Create<ItemIntersection>(GetX(), GetY(), 23.99999f, shared_from_this()));
        }
    }

}

void ObjItem::Update()
{
    auto package = GetPackage().lock();
    if (!package) return;

    if (package->autoItemCollectionManager->IsAutoCollectCanceled())
    {
        isAutoCollected_ = false;
    }

    std::shared_ptr<ObjPlayer> player = package->playerObj.lock();
    if (isAutoCollected_)
    {
        // 自動回収時
        if (std::dynamic_pointer_cast<MoveModeItemDown>(GetMoveMode())
            || std::dynamic_pointer_cast<MoveModeItemDest>(GetMoveMode())
            || std::dynamic_pointer_cast<MoveModeItemToPlayer>(GetMoveMode()))
        {
            // NOTE: Item用のMoveModeが設定されている場合はそれを維持する
            // (SetSpeed,AngleするとModeAに変わってしまうので)
            // move前のx, yを保存
            float x = GetX();
            float y = GetY();
            // MoveItemDestのカウンタすすめるためのmove呼び出し
            Move();
            if (player)
            {
                float playerX = player->GetX();
                float playerY = player->GetY();
                float distX = playerX - x;
                float distY = playerY - y;
                float dist = std::hypotf(distX, distY);
                float dx = autoCollectSpeed_ * distX / dist;
                float dy = autoCollectSpeed_ * distY / dist;
                // moveによる移動を消す
                SetMovePosition(x + dx, y + dy);
            }
        } else
        {
            SetSpeed(autoCollectSpeed_);
            if (player)
            {
                float playerX = player->GetX();
                float playerY = player->GetY();
                SetAngle(D3DXToDegree(atan2(playerY - GetY(), playerX - GetX())));
            }
            Move();
        }
    } else
    {
        Move();
        if (autoCollectEnable_)
        {
            isAutoCollected_ = package->autoItemCollectionManager->IsAutoCollectTarget(GetItemType(), GetX(), GetY());
            if (player && !package->autoItemCollectionManager->IsAutoCollectCanceled())
            {
                if (player->GetY() <= player->GetAutoItemCollectLineY())
                {
                    isAutoCollected_ = true;
                }
            }
        }
    }

    if (player)
    {
        if (player->GetState() != STATE_NORMAL)
        {
            Obtained();
        }
    }

    if (isObtained_)
    {
        if (renderScoreEnable_ && IsScoreItem())
        {
            // 点数文字列生成
            if (auto package = GetPackage().lock())
            {
                package->itemScoreTextSpawner->Spawn(GetX(), GetY(), GetScore(), package);
            }
        }
        Die();
        return;
    }

    if (GetX() < autoDeleteClipLeft_ || GetX() > autoDeleteClipRight_ || GetY() > autoDeleteClipBottom_)
    {
        Die();
    }

    if (GetItemType() == ITEM_USER)
    {
        UpdateAnimationPosition();
    }
}

void ObjItem::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (itemData_)
    {
        float itemScale = 1.0;
        switch (itemType_)
        {
            case ITEM_1UP_S:
            case ITEM_SPELL_S:
            case ITEM_POWER_S:
            case ITEM_POINT_S:
            case ITEM_DEFAULT_BONUS:
                itemScale = 0.75;
                break;
        }

        bool isOut = GetY() <= 0;
        /* 配置 */
        D3DXMATRIX world = CreateScaleRotTransMatrix(GetX(), isOut ? ((itemData_->out.bottom - itemData_->out.top) / 2.0f) : GetY(), 0, GetAngleX(), GetAngleY(), GetAngleZ(), GetScaleX() * itemScale, GetScaleY() * itemScale, 1.0f);

        const auto& rect = isOut ? itemData_->out
            : (animationIdx_ >= 0 && animationIdx_ < itemData_->animationData.size()) ? itemData_->animationData[animationIdx_].rect
            : itemData_->rect;

        /* ブレンド方法の選択 */
        int itemBlend = BLEND_NONE;
        if (GetBlendType() == BLEND_NONE)
        {
            itemBlend = itemData_->render;
        } else
        {
            /* ObjRenderで指定されたintがある場合はそちらを使う */
            itemBlend = GetBlendType();
        }

        // 色と透明度の適用
        D3DCOLOR renderColor = D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff);
        if (itemType_ == ITEM_USER)
        {
            renderColor = GetD3DCOLOR();
        } else
        {
            // 組み込みアイテムが画面外にあるときは決められた色を付ける // FUTURE : この仕様を削除 
            if (isOut)
            {
                ColorRGB outColor;
                switch (itemType_)
                {
                    case ITEM_1UP:
                    case ITEM_1UP_S:
                        renderColor = D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0);
                        break;
                    case ITEM_SPELL:
                    case ITEM_SPELL_S:
                        renderColor = D3DCOLOR_ARGB(0xff, 0, 0xff, 0);
                        break;
                    case ITEM_POWER:
                    case ITEM_POWER_S:
                        renderColor = D3DCOLOR_ARGB(0xff, 0xff, 0, 0);
                        break;
                    case ITEM_POINT:
                    case ITEM_POINT_S:
                        renderColor = D3DCOLOR_ARGB(0xff, 0, 0, 0xff);
                        break;
                    default:
                        renderColor = D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff);
                }
            }
        }

        auto vertices = RectToVertices(renderColor, itemData_->texture->GetWidth(), itemData_->texture->GetHeight(), rect);
        renderer->RenderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), itemData_->texture->GetTexture(), itemBlend, world, GetAppliedShader(), IsPermitCamera(), true);
    }
    ObjCol::RenderIntersection(renderer, IsPermitCamera());
}

int ObjItem::GetItemType() const { return itemType_; }

PlayerScore ObjItem::GetScore() const { return score_; }

void ObjItem::SetScore(PlayerScore score)
{
    score_ = score;
}

bool ObjItem::IsRenderScoreEnabled() const
{
    return renderScoreEnable_;
}

void ObjItem::SetRenderScoreEnable(bool enable) { renderScoreEnable_ = enable; }

bool ObjItem::IsAutoCollectEnabled() const
{
    return autoCollectEnable_;
}

void ObjItem::SetAutoCollectEnable(bool enable) { autoCollectEnable_ = enable; }

bool ObjItem::IsAutoCollected() const
{
    return isAutoCollected_;
}

void ObjItem::SetItemData(const std::shared_ptr<ItemData>& data)
{
    if (data->texture)
    {
        itemData_ = data;
        itemType_ = ITEM_USER;
    }
}

bool ObjItem::IsObtained() const
{
    return isObtained_;
}

void ObjItem::Obtained()
{
    isObtained_ = true;
}

bool ObjItem::IsScoreItem() const
{
    switch (GetItemType())
    {
        case ITEM_POINT:
        case ITEM_POINT_S:
        case ITEM_POWER:
        case ITEM_POWER_S:
        case ITEM_DEFAULT_BONUS:
        case ITEM_USER:
            return true;
    }
    return false;
}

int ObjItem::GetAnimationIndex() const
{
    return animationIdx_;
}

int ObjItem::GetAnimationFrameCount() const
{
    return animationFrameCnt_;
}

const std::shared_ptr<ItemData>& ObjItem::GetItemData() const
{
    return itemData_;
}

void ObjItem::TransIntersection(float dx, float dy)
{
    ObjCol::TransIntersection(dx, dy);
}

void ObjItem::UpdateAnimationPosition()
{
    if (itemData_)
    {
        if (animationIdx_ < 0 || animationIdx_ >= itemData_->animationData.size())
        {
            animationFrameCnt_ = animationIdx_ = 0;
            return;
        }

        animationFrameCnt_++;
        if (itemData_->animationData[animationIdx_].frame <= animationFrameCnt_)
        {
            animationFrameCnt_ = 0;
            animationIdx_++;
            if (animationIdx_ >= itemData_->animationData.size()) animationIdx_ = 0;
        }
    }
}

ObjItemScoreText::ObjItemScoreText(PlayerScore score, const std::shared_ptr<Texture>& texture, const std::shared_ptr<Package>& package) :
    ObjSpriteList2D(package),
    ObjMove(this),
    scoreTextDeleteTimer_(32),
    scoreTextAlpha_(0xff)
{
    SetBlendType(BLEND_ADD_ARGB);
    SetTexture(texture);
    SetMoveMode(std::make_shared<MoveModeHoverItemScoreText>(1.0f));

    std::string scoreText = std::to_string(score);
    int digitWidth = 8;
    int digitHeight = 14;
    for (int i = 0; i < scoreText.size(); i++)
    {
        int n = scoreText[i] - '0';
        SetX(i * (digitWidth - 1));
        SetSourceRect(n * 36, 0, (n + 1) * 36, 32);
        SetDestRect(0, 0, digitWidth, digitHeight);
        AddVertex();
    }
    CloseVertex();
}

ObjItemScoreText::~ObjItemScoreText() {}

void ObjItemScoreText::Update()
{
    scoreTextDeleteTimer_--;
    if (scoreTextDeleteTimer_ <= 0)
    {
        Die();
    } else
    {
        Move();
        scoreTextAlpha_ -= 8;
        int vertexCnt = GetVertexCount();
        for (int i = 0; i < vertexCnt; i++)
        {
            SetVertexAlpha(i, scoreTextAlpha_);
        }
    }
}

void ObjItemScoreText::Render(const std::shared_ptr<Renderer>& renderer)
{
    ObjSpriteList2D::Render(renderer);
}

AutoItemCollectionManager::AutoItemCollectionManager() :
    isAutoItemCollectCanceled_(false)
{
}

void AutoItemCollectionManager::CollectAllItems()
{
    CollectItemsByType(ITEM_1UP);
    CollectItemsByType(ITEM_1UP_S);
    CollectItemsByType(ITEM_SPELL);
    CollectItemsByType(ITEM_SPELL_S);
    CollectItemsByType(ITEM_POWER);
    CollectItemsByType(ITEM_POWER_S);
    CollectItemsByType(ITEM_POINT);
    CollectItemsByType(ITEM_POINT_S);
    CollectItemsByType(ITEM_USER);
}

void AutoItemCollectionManager::CollectItemsByType(int itemType)
{
    autoItemCollectTargetTypes_.insert(itemType);
}

void AutoItemCollectionManager::CollectItemsInCircle(float x, float y, float r)
{
    circles_.emplace_back(x, y, r);
}

void AutoItemCollectionManager::CancelCollectItems()
{
    isAutoItemCollectCanceled_ = true;
}

bool AutoItemCollectionManager::IsAutoCollectTarget(int itemType, float itemX, float itemY) const
{
    // キャンセルは全てに優先
    if (isAutoItemCollectCanceled_)
    {
        return false;
    }

    // 回収対象のアイテムかどうか
    if (autoItemCollectTargetTypes_.count(itemType) >= 1)
    {
        return true;
    }

    // 回収円の中にあるか
    for (const auto& circle : circles_)
    {
        float x = std::get<0>(circle);
        float y = std::get<1>(circle);
        float r = std::get<2>(circle);
        float dx = itemX - x;
        float dy = itemY - y;
        if (dx * dx + dy * dy <= r * r)
        {
            return true;
        }
    }
    return false;
}

bool AutoItemCollectionManager::IsAutoCollectCanceled() const
{
    return isAutoItemCollectCanceled_;
}

void AutoItemCollectionManager::Reset()
{
    isAutoItemCollectCanceled_ = false;
    autoItemCollectTargetTypes_.clear();
    circles_.clear();
}

MoveModeItemDown::MoveModeItemDown(float initSpeed) :
    speed_(initSpeed)
{
}

void MoveModeItemDown::Move(float & x, float & y)
{
    speed_ = std::max(speed_ + 0.05, 2.5);
    y += speed_;
}

MoveModeItemDest::MoveModeItemDest(float destX, float destY, ObjMove * obj) :
    obj_(obj)
{
    float distX = destX - obj->GetMoveX();
    float distY = destY - obj->GetMoveY();
    float dist = std::hypotf(distX, distY);
    speed_ = dist / 16.0;
    frame_ = 0;
    lastX_ = obj->GetMoveX();
    lastY_ = obj->GetMoveY();
    this->destX_ = destX;
    this->destY_ = destY;
    cosAngle_ = distX / dist;
    sinAngle_ = distY / dist;
}

void MoveModeItemDest::Move(float & x, float & y)
{
    if (x != lastX_ || y != lastY_)
    {
        // 途中でObjMove_SetPositionなどで位置が変更されたら
        // 速さと角度をリセットする
        MoveModeItemDest mode(destX_, destY_, obj_);
        speed_ = mode.speed_;
        lastX_ = x;
        lastY_ = y;
        cosAngle_ = mode.cosAngle_;
        sinAngle_ = mode.sinAngle_;
    }
    float dx = speed_ * cosAngle_;
    float dy = speed_ * sinAngle_;
    x += dx;
    y += dy;
    speed_ *= 0.9370674; // exp(-0.065);
    frame_++;
    if (frame_ == 60)
    {
        obj_->SetMoveMode(std::make_shared<MoveModeItemDown>(0.0f));
    }
}
MoveModeItemToPlayer::MoveModeItemToPlayer(float speed, const std::shared_ptr<ObjPlayer>& player) :
    speed_(speed),
    targetPlayer_(player)
{
}

void MoveModeItemToPlayer::Move(float & x, float & y)
{
    if (auto player = targetPlayer_.lock())
    {
        float distX = player->GetX() - x;
        float distY = player->GetY() - y;
        float dist = std::hypotf(distX, distY);
        float dx = speed_ * distX / dist;
        float dy = speed_ * distY / dist;
        x += dx;
        y += dy;
    }
}

MoveModeHoverItemScoreText::MoveModeHoverItemScoreText(float speed) :
    speed_(speed)
{
}

void MoveModeHoverItemScoreText::Move(float & x, float & y)
{
    y -= speed_;
}

ItemScoreTextSpawner::ItemScoreTextSpawner() {}
ItemScoreTextSpawner::~ItemScoreTextSpawner() {}

void ItemScoreTextSpawner::Spawn(float x, float y, PlayerScore score, const std::shared_ptr<Package>& package)
{
    if (package)
    {
        std::shared_ptr<Texture> texture = package->textureCache->Load(SYSTEM_STG_DIGIT_IMG_PATH, false, nullptr);
        auto scoreText = package->objTable->Create<ObjItemScoreText>(score, texture, package);
        package->objLayerList->SetRenderPriority(scoreText, package->objLayerList->GetItemRenderPriority());
        scoreText->SetMovePosition(x, y);
    }
}

DefaultBonusItemSpawner::DefaultBonusItemSpawner() {}
DefaultBonusItemSpawner::~DefaultBonusItemSpawner() {}

void DefaultBonusItemSpawner::Spawn(float x, float y, const std::shared_ptr<Package>& package)
{
    if (package)
    {
        auto bonusItem = std::make_shared<ObjItem>(ITEM_DEFAULT_BONUS, package);
        package->objTable->Add(bonusItem);
        package->objLayerList->SetRenderPriority(bonusItem, package->objLayerList->GetItemRenderPriority());
        bonusItem->SetMovePosition(x, y);
        bonusItem->SetIntersection();
        bonusItem->SetScore(300);
        bonusItem->SetMoveMode(std::make_shared<MoveModeItemToPlayer>(8.0f, package->playerObj.lock()));
    }
}
}