#pragma once

#include <bstorm/obj_render.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>
#include <bstorm/stage_common_player_params.hpp>

#include <stdint.h>
#include <memory>
#include <unordered_set>
#include <tuple>
#include <vector>

namespace bstorm
{
class ItemData;
class ItemIntersection;
class ObjItem : public ObjRender, public ObjMove, public ObjCol, public std::enable_shared_from_this<ObjItem>
{
public:
    ObjItem(int itemType, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package);
    ~ObjItem();
    void SetIntersection();
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    int GetItemType() const;
    PlayerScore GetScore() const;
    void SetScore(PlayerScore score);
    bool IsRenderScoreEnabled() const;
    void SetRenderScoreEnable(bool enable);
    bool IsAutoCollectEnabled() const;
    void SetAutoCollectEnable(bool enable);
    bool IsAutoCollected() const;
    const NullableSharedPtr<ItemData>& GetItemData() const;
    void SetItemData(const std::shared_ptr<ItemData>& data);
    int GetAnimationFrameCount() const;
    int GetAnimationIndex() const;
    bool IsObtained() const;
    void Obtained();
    bool IsScoreItem() const;
private:
    void OnTrans(float dx, float dy) override;
    void UpdateAnimationPosition();
    int itemType_;
	int itemDelay_;
    bool autoCollectEnable_;
    bool isAutoCollected_;
    bool renderScoreEnable_;
    int autoDeleteClipLeft_;
    int autoDeleteClipRight_;
    int autoDeleteClipBottom_;
    float autoCollectSpeed_;
    bool isObtained_;
    PlayerScore score_;
    int animationFrameCnt_;
    int animationIdx_;
    std::shared_ptr<ItemData> itemData_;
};

class ObjItemScoreText : public ObjSpriteList2D, public ObjMove
{
public:
    ObjItemScoreText(PlayerScore score, const std::shared_ptr<Texture>& texture, const std::shared_ptr<Package>& package);
    ~ObjItemScoreText();
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
protected:
    int scoreTextDeleteTimer_;
    int scoreTextAlpha_;
};

class ObjItem;
class AutoItemCollectionManager
{
public:
    AutoItemCollectionManager();
    void CollectAllItems();
    void CollectItemsByType(int itemType);
    void CollectItemsInCircle(float x, float y, float r);
    void CancelCollectItems();
    bool IsAutoCollectTarget(int itemType, float itemX, float itemY) const;
    bool IsAutoCollectCanceled() const;
    void Reset();
private:
    bool isAutoItemCollectCanceled_;
    std::unordered_set<int> autoItemCollectTargetTypes_;
    std::vector<std::tuple<float, float, float>> circles_;
};

class MoveModeItemDown : public MoveMode
{
public:
    MoveModeItemDown(float initSpeed);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return 0; }
    float GetSpeed() const override { return 0; }
private:
    float speed_;
};

class ObjMove;
class MoveModeItemDest : public MoveMode
{
public:
    MoveModeItemDest(float destX, float destY, ObjMove* obj);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return 0; }
    float GetSpeed() const override { return 0; }
private:
    ObjMove * obj_;
    float speed_;
    float frame_;
    float lastX_;
    float lastY_;
    float destX_;
    float destY_;
    float cosAngle_;
    float sinAngle_;
};

class ObjPlayer;
class MoveModeItemToPlayer : public MoveMode
{
public:
    MoveModeItemToPlayer(float speed, const std::shared_ptr<ObjPlayer>& player);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return 0; }
    float GetSpeed() const override { return 0; }
private:
    const float speed_;
    std::weak_ptr<ObjPlayer> targetPlayer_;
};

class MoveModeHoverItemScoreText : public MoveMode
{
public:
    MoveModeHoverItemScoreText(float speed);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return 0; }
    float GetSpeed() const override { return 0; }
private:
    float speed_;
};
}