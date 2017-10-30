#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/collision_matrix.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/item_data.hpp>
#include <bstorm/texture.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_item.hpp>

namespace bstorm {
  ObjItem::ObjItem(int itemType, const std::shared_ptr<GameState>& gameState) :
    ObjRender(gameState),
    ObjMove(this),
    ObjCol(gameState),
    itemType(itemType),
    score(0),
    autoCollectEnable(true),
    autoCollectFlag(false),
    renderScoreEnable(true),
    autoDeleteClipLeft(-64),
    autoDeleteClipRight(704),
    autoDeleteClipBottom(544),
    autoCollectSpeed(8.0f),
    obtainedFlag(false),
    animationFrameCnt(0),
    animationIdx(0)
  {
    pushIntersection(gameState->colDetector->create<ItemIntersection>(getX(), getY(), 23.99999f, this));

    setType(OBJ_ITEM);
    setBlendType(BLEND_NONE);

    if (itemType != ITEM_USER) {
      itemData = gameState->itemDataTable->get(itemType);
    }
  }

  ObjItem::~ObjItem() {
  }

  void ObjItem::update() {
    auto state = getGameState();
    if (!state) return;

    if (state->autoItemCollectionManager->isAutoCollectCanceled()) {
      autoCollectFlag = false;
    }

    std::shared_ptr<ObjPlayer> player = state->playerObj.lock();
    if (autoCollectFlag) {
      // 自動回収時
      if (std::dynamic_pointer_cast<MoveModeItemDown>(getMoveMode())
          || std::dynamic_pointer_cast<MoveModeItemDest>(getMoveMode())
          || std::dynamic_pointer_cast<MoveModeItemToPlayer>(getMoveMode())) {
        // NOTE: Item用のMoveModeが設定されている場合はそれを維持する
        // (setSpeed,AngleするとModeAに変わってしまうので)
        // move前のx, yを保存
        float x = getX();
        float y = getY();
        // MoveItemDestのカウンタすすめるためのmove呼び出し
        move();
        if (player) {
          float playerX = player->getX();
          float playerY = player->getY();
          float distX = playerX - x;
          float distY = playerY - y;
          float dist = norm(distX, distY);
          float dx = autoCollectSpeed * distX / dist;
          float dy = autoCollectSpeed * distY / dist;
          // moveによる移動を消す
          setMovePosition(x + dx, y + dy);
        }
      } else {
        setSpeed(autoCollectSpeed);
        if (player) {
          float playerX = player->getX();
          float playerY = player->getY();
          setAngle(D3DXToDegree(atan2(playerY - getY(), playerX - getX())));
        }
        move();
      }
    } else {
      move();
      if (autoCollectEnable) {
        autoCollectFlag = state->autoItemCollectionManager->isAutoCollectTarget(getItemType(), getX(), getY());
        if (player && !state->autoItemCollectionManager->isAutoCollectCanceled()) {
          if (player->getY() <= player->getAutoItemCollectLineY()) {
            autoCollectFlag = true;
          }
        }
      }
    }

    if (player) {
      if (player->getState() != STATE_NORMAL) {
        obtained();
      }
    }

    if (obtainedFlag) {
      if (renderScoreEnable && isScoreItem()) {
        // 点数文字列生成
        if (auto state = getGameState()) {
          state->itemScoreTextSpawner->spawn(getX(), getY(), getScore(), state);
        }
      }
      die();
      return;
    }

    if (getX() < autoDeleteClipLeft || getX() > autoDeleteClipRight || getY() > autoDeleteClipBottom) {
      die();
    }

    if (getItemType() == ITEM_USER) {
      updateAnimationPosition();
    }
  }

  void ObjItem::render() {
    if (itemData) {
      float itemScale = 1.0;
      switch (itemType) {
        case ITEM_1UP_S:
        case ITEM_SPELL_S:
        case ITEM_POWER_S:
        case ITEM_POINT_S:
        case ITEM_DEFAULT_BONUS:
          itemScale = 0.75;
          break;
      }

      bool isOut = getY() <= 0;
      /* 配置 */
      D3DXMATRIX world = rotScaleTrans(getX(), isOut ? ((itemData->out.bottom - itemData->out.top) / 2.0f) : getY(), 0, getAngleX(), getAngleY(), getAngleZ(), getScaleX() * itemScale, getScaleY() * itemScale, 1.0f);

      const auto& rect = isOut ? itemData->out
        : (animationIdx >= 0 && animationIdx < itemData->animationData.size()) ? itemData->animationData[animationIdx].rect
        : itemData->rect;

      /* ブレンド方法の選択 */
      int itemBlend = BLEND_NONE;
      if (getBlendType() == BLEND_NONE) {
        itemBlend = itemData->render;
      } else {
        /* ObjRenderで指定されたintがある場合はそちらを使う */
        itemBlend = getBlendType();
      }

      // 色と透明度の適用
      D3DCOLOR renderColor = D3DCOLOR_ARGB(0xff, 0xff, 0xff, 0xff);
      if (itemType == ITEM_USER) {
        renderColor = getD3DCOLOR();
      } else {
        // 組み込みアイテムが画面外にあるときは決められた色を付ける // FUTURE : この仕様を削除 
        if (isOut) {
          ColorRGB outColor;
          switch (itemType) {
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

      auto vertices = rectToVertices(renderColor, itemData->texture->getWidth(), itemData->texture->getHeight(), rect);
      if (auto state = getGameState()) {
        state->renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, 4, vertices.data(), itemData->texture->getTexture(), itemBlend, world, getAppliedShader(), isPermitCamera());
      }
    }
    ObjCol::renderIntersection(isPermitCamera());
  }

  int ObjItem::getItemType() const { return itemType; }

  int64_t ObjItem::getScore() const { return score; }

  void ObjItem::setScore(int64_t score) { this->score = score; }

  bool ObjItem::isRenderScoreEnabled() const {
    return renderScoreEnable;
  }

  void ObjItem::setRenderScoreEnable(bool enable) { renderScoreEnable = enable; }

  bool ObjItem::isAutoCollectEnabled() const {
    return autoCollectEnable;
  }

  void ObjItem::setAutoCollectEnable(bool enable) { autoCollectEnable = enable; }

  bool ObjItem::isAutoCollected() const {
    return autoCollectFlag;
  }

  void ObjItem::setItemData(const std::shared_ptr<ItemData>& data) {
    if (data->texture) {
      itemData = data;
      itemType = ITEM_USER;
    }
  }

  bool ObjItem::isObtained() const {
    return obtainedFlag;
  }

  void ObjItem::obtained() {
    obtainedFlag = true;
  }

  bool ObjItem::isScoreItem() const {
    switch (getItemType()) {
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

  int ObjItem::getAnimationIndex() const {
    return animationIdx;
  }

  int ObjItem::getAnimationFrameCount() const {
    return animationFrameCnt;
  }

  const std::shared_ptr<ItemData>& ObjItem::getItemData() const {
    return itemData;
  }

  void ObjItem::transIntersection(float dx, float dy) {
    ObjCol::transIntersection(dx, dy);
  }

  void ObjItem::updateAnimationPosition() {
    if (itemData) {
      if (animationIdx < 0 || animationIdx >= itemData->animationData.size()) {
        animationFrameCnt = animationIdx = 0;
        return;
      }

      animationFrameCnt++;
      if (itemData->animationData[animationIdx].frame <= animationFrameCnt) {
        animationFrameCnt = 0;
        animationIdx++;
        if (animationIdx >= itemData->animationData.size()) animationIdx = 0;
      }
    }
  }

  ObjItemScoreText::ObjItemScoreText(int64_t score, const std::shared_ptr<Texture>& texture, const std::shared_ptr<GameState>& gameState) :
    ObjSpriteList2D(gameState),
    ObjMove(this),
    scoreTextDeleteTimer(32),
    scoreTextAlpha(0xff)
  {
    setBlendType(BLEND_ADD_ARGB);
    setTexture(texture);
    setMoveMode(std::make_shared<MoveModeHoverItemScoreText>(1.0f));

    std::string scoreText = std::to_string(score);
    int digitWidth = 8;
    int digitHeight = 14;
    for (int i = 0; i < scoreText.size(); i++) {
      int n = scoreText[i] - '0';
      setX(i * (digitWidth - 1));
      setSourceRect(n * 36, 0, (n + 1) * 36, 32);
      setDestRect(0, 0, digitWidth, digitHeight);
      addVertex();
    }
    closeVertex();
  }

  ObjItemScoreText::~ObjItemScoreText() {}

  void ObjItemScoreText::update() {
    scoreTextDeleteTimer--;
    if (scoreTextDeleteTimer <= 0) {
      die();
    } else {
      move();
      scoreTextAlpha -= 8;
      int vertexCnt = getVertexCount();
      for (int i = 0; i < vertexCnt; i++) {
        setVertexAlpha(i, scoreTextAlpha);
      }
    }
  }

  void ObjItemScoreText::render() {
    ObjSpriteList2D::render();
  }

  AutoItemCollectionManager::AutoItemCollectionManager() :
    autoItemCollectCancelFlag(false)
  {
  }

  void AutoItemCollectionManager::collectAllItems() {
    collectItemsByType(ITEM_1UP);
    collectItemsByType(ITEM_1UP_S);
    collectItemsByType(ITEM_SPELL);
    collectItemsByType(ITEM_SPELL_S);
    collectItemsByType(ITEM_POWER);
    collectItemsByType(ITEM_POWER_S);
    collectItemsByType(ITEM_POINT);
    collectItemsByType(ITEM_POINT_S);
    collectItemsByType(ITEM_USER);
  }

  void AutoItemCollectionManager::collectItemsByType(int itemType) {
    autoItemCollectTargetTypes.insert(itemType);
  }

  void AutoItemCollectionManager::collectItemsInCircle(float x, float y, float r) {
    circles.push_back(std::make_tuple(x, y, r));
  }

  void AutoItemCollectionManager::cancelCollectItems() {
    autoItemCollectCancelFlag = true;
  }

  bool AutoItemCollectionManager::isAutoCollectTarget(int itemType, float itemX, float itemY) const {
    // キャンセルは全てに優先
    if (autoItemCollectCancelFlag) {
      return false;
    }

    // 回収対象のアイテムかどうか
    if (autoItemCollectTargetTypes.count(itemType) >= 1) {
      return true;
    }

    // 回収円の中にあるか
    for (const auto& circle : circles) {
      float x = std::get<0>(circle);
      float y = std::get<1>(circle);
      float r = std::get<2>(circle);
      float dx = itemX - x;
      float dy = itemY - y;
      if (dx * dx + dy * dy <= r * r) {
        return true;
      }
    }
    return false;
  }

  bool AutoItemCollectionManager::isAutoCollectCanceled() const {
    return autoItemCollectCancelFlag;
  }

  void AutoItemCollectionManager::reset() {
    autoItemCollectCancelFlag = false;
    autoItemCollectTargetTypes.clear();
    circles.clear();
  }

  MoveModeItemDown::MoveModeItemDown(float initSpeed) :
    speed(initSpeed)
  {
  }

  void MoveModeItemDown::move(float & x, float & y) {
    speed = std::max(speed + 0.05, 2.5);
    y += speed;
  }

  MoveModeItemDest::MoveModeItemDest(float destX, float destY, ObjMove * obj) :
    obj(obj)
  {
    float distX = destX - obj->getMoveX();
    float distY = destY - obj->getMoveY();
    float dist = norm(distX, distY);
    speed = dist / 16.0;
    frame = 0;
    lastX = obj->getMoveX();
    lastY = obj->getMoveY();
    this->destX = destX;
    this->destY = destY;
    cosAngle = distX / dist;
    sinAngle = distY / dist;
  }

  void MoveModeItemDest::move(float & x, float & y) {
    if (x != lastX || y != lastY) {
      // 途中でObjMove_SetPositionなどで位置が変更されたら
      // 速さと角度をリセットする
      MoveModeItemDest mode(destX, destY, obj);
      speed = mode.speed;
      lastX = x;
      lastY = y;
      cosAngle = mode.cosAngle;
      sinAngle = mode.sinAngle;
    }
    float dx = speed * cosAngle;
    float dy = speed * sinAngle;
    x += dx;
    y += dy;
    speed *= 0.9370674; // exp(-0.065);
    frame++;
    if (frame == 60) {
      obj->setMoveMode(std::make_shared<MoveModeItemDown>(0.0f));
    }
  }
  MoveModeItemToPlayer::MoveModeItemToPlayer(float speed, const std::shared_ptr<ObjPlayer>& player) :
    speed(speed),
    targetPlayer(player)
  {
  }

  void MoveModeItemToPlayer::move(float & x, float & y) {
    if (auto player = targetPlayer.lock()) {
      float distX = player->getX() - x;
      float distY = player->getY() - y;
      float dist = norm(distX, distY);
      float dx = speed * distX / dist;
      float dy = speed * distY / dist;
      x += dx;
      y += dy;
    }
  }

  MoveModeHoverItemScoreText::MoveModeHoverItemScoreText(float speed) :
    speed(speed)
  {
  }

  void MoveModeHoverItemScoreText::move(float & x, float & y) {
    y -= speed;
  }

  ItemScoreTextSpawner::ItemScoreTextSpawner() {}
  ItemScoreTextSpawner::~ItemScoreTextSpawner() {}

  void ItemScoreTextSpawner::spawn(float x, float y, int64_t score, const std::shared_ptr<GameState>& gameState) {
    if (gameState) {
      std::shared_ptr<Texture> texture = gameState->textureCache->load(SYSTEM_STG_DIGIT_IMG_PATH, false);
      auto scoreText = gameState->objTable->create<ObjItemScoreText>(score, texture, gameState);
      gameState->objLayerList->setRenderPriority(scoreText, gameState->objLayerList->getItemRenderPriority());
      scoreText->setMovePosition(x, y);
    }
  }

  DefaultBonusItemSpawner::DefaultBonusItemSpawner() { }
  DefaultBonusItemSpawner::~DefaultBonusItemSpawner() { }

  void DefaultBonusItemSpawner::spawn(float x, float y, const std::shared_ptr<GameState>& gameState) {
    if (gameState) {
      auto bonusItem = std::make_shared<ObjItem>(ITEM_DEFAULT_BONUS, gameState);
      gameState->objTable->add(bonusItem);
      gameState->objLayerList->setRenderPriority(bonusItem, gameState->objLayerList->getItemRenderPriority());
      bonusItem->setMovePosition(x, y);
      bonusItem->setScore(300);
      bonusItem->setMoveMode(std::make_shared<MoveModeItemToPlayer>(8.0f, gameState->playerObj.lock()));
    }
  }
}