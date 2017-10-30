#pragma once

#include <stdint.h>
#include <memory>
#include <unordered_set>
#include <tuple>
#include <vector>

#include <bstorm/obj_render.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm {
  class ItemData;
  class ItemIntersection;
  class ObjItem : public ObjRender, public ObjMove, public ObjCol {
  public:
    ObjItem(int itemType, const std::shared_ptr<GameState>& gameState);
    ~ObjItem();
    void update() override;
    void render() override;
    int getItemType() const;
    int64_t getScore() const;
    void setScore(int64_t score);
    bool isRenderScoreEnabled() const;
    void setRenderScoreEnable(bool enable);
    bool isAutoCollectEnabled() const;
    void setAutoCollectEnable(bool enable);
    bool isAutoCollected() const;
    const std::shared_ptr<ItemData>& getItemData() const;
    void setItemData(const std::shared_ptr<ItemData>& data);
    int getAnimationFrameCount() const;
    int getAnimationIndex() const;
    bool isObtained() const;
    void obtained();
    bool isScoreItem() const;
  protected:
    void transIntersection(float dx, float dy) override;
    void updateAnimationPosition();
    int itemType;
    bool autoCollectEnable;
    bool autoCollectFlag;
    bool renderScoreEnable;
    int autoDeleteClipLeft;
    int autoDeleteClipRight;
    int autoDeleteClipBottom;
    float autoCollectSpeed;
    int obtainedFlag;
  private:
    int64_t score;
    int animationFrameCnt;
    int animationIdx;
    std::shared_ptr<ItemData> itemData;
  };

  class ObjItemScoreText : public ObjSpriteList2D, public ObjMove {
  public:
    ObjItemScoreText(int64_t score, const std::shared_ptr<Texture>& texture, const std::shared_ptr<GameState>& gameState);
    ~ObjItemScoreText();
    void update() override;
    void render() override;
  protected:
    int scoreTextDeleteTimer;
    int scoreTextAlpha;
  };

  class ItemScoreTextSpawner {
  public:
    ItemScoreTextSpawner();
    virtual ~ItemScoreTextSpawner();
    virtual void spawn(float x, float y, int64_t score, const std::shared_ptr<GameState>& gameState);
  };

  class DefaultBonusItemSpawner {
  public:
    DefaultBonusItemSpawner();
    virtual ~DefaultBonusItemSpawner();
    virtual void spawn(float x, float y, const std::shared_ptr<GameState>& gameState);
  };

  class ObjItem;
  class AutoItemCollectionManager {
  public:
    AutoItemCollectionManager();
    void collectAllItems();
    void collectItemsByType(int itemType);
    void collectItemsInCircle(float x, float y, float r);
    void cancelCollectItems();
    bool isAutoCollectTarget(int itemType, float itemX, float itemY) const;
    bool isAutoCollectCanceled() const;
    void reset();
  private:
    bool autoItemCollectCancelFlag;
    std::unordered_set<int> autoItemCollectTargetTypes;
    std::vector<std::tuple<float, float, float>> circles;
  };

  class MoveModeItemDown : public MoveMode {
  public:
    MoveModeItemDown(float initSpeed);
    void move(float& x, float& y) override;
    float getAngle() const override { return 0; }
    float getSpeed() const override { return 0; }
  private:
    float speed;
  };

  class ObjMove;
  class MoveModeItemDest : public MoveMode {
  public:
    MoveModeItemDest(float destX, float destY, ObjMove* obj);
    void move(float& x, float& y) override;
    float getAngle() const override { return 0; }
    float getSpeed() const override { return 0; }
  private:
    ObjMove* obj;
    float speed;
    float frame;
    float lastX;
    float lastY;
    float destX;
    float destY;
    float cosAngle;
    float sinAngle;
  };

  class ObjPlayer;
  class MoveModeItemToPlayer : public MoveMode {
  public:
    MoveModeItemToPlayer(float speed, const std::shared_ptr<ObjPlayer>& player);
    void move(float& x, float& y) override;
    float getAngle() const override { return 0; }
    float getSpeed() const override { return 0; }
  private:
    float speed;
    std::weak_ptr<ObjPlayer> targetPlayer;
  };

  class MoveModeHoverItemScoreText : public MoveMode {
  public:
    MoveModeHoverItemScoreText(float speed);
    void move(float& x, float& y) override;
    float getAngle() const override { return 0; }
    float getSpeed() const override { return 0; }
  private:
    float speed;
  };
}