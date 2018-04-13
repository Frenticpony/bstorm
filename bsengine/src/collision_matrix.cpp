#include <bstorm/collision_matrix.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/obj_shot.hpp>
#include <bstorm/obj_item.hpp>
#include <bstorm/obj_player.hpp>
#include <bstorm/obj_spell.hpp>

namespace bstorm {
  ShotIntersection::ShotIntersection(float x, float y, float r, ObjShot *shot, bool isTmpIntersection) :
    Intersection(Shape(x, y, r)),
    shot(shot),
    isPlayerShot(shot->isPlayerShot()),
    isTmpIntersection(isTmpIntersection)
  {
  }

  ShotIntersection::ShotIntersection(float x1, float y1, float x2, float y2, float width, ObjShot* shot, bool isTmpIntersection) :
    Intersection(Shape(x1, y1, x2, y2, width)),
    shot(shot),
    isPlayerShot(shot->isPlayerShot()),
    isTmpIntersection(isTmpIntersection)
  {
  }

  EnemyIntersectionToShot::EnemyIntersectionToShot(float x, float y, float r, ObjEnemy *enemy) :
    Intersection(Shape(x, y, r)),
    enemy(enemy),
    x(x),
    y(y)
  {
  }

  EnemyIntersectionToPlayer::EnemyIntersectionToPlayer(float x, float y, float r, ObjEnemy *enemy) :
    Intersection(Shape(x, y, r)),
    enemy(enemy)
  {
  }

  PlayerIntersection::PlayerIntersection(float x, float y, float r, ObjPlayer *player) :
    Intersection(Shape(x, y, r)),
    player(player)
  {
  }

  PlayerGrazeIntersection::PlayerGrazeIntersection(float x, float y, float r, ObjPlayer *player) :
    Intersection(Shape(x, y, r)),
    player(player)
  {
  }

  SpellIntersection::SpellIntersection(float x, float y, float r, ObjSpell *spell) :
    Intersection(Shape(x, y, r)),
    spell(spell)
  {
  }

  SpellIntersection::SpellIntersection(float x1, float y1, float x2, float y2, float width, ObjSpell *spell) :
    Intersection(Shape(x1, y1, x2, y2, width)),
    spell(spell)
  {
  }

  PlayerIntersectionToItem::PlayerIntersectionToItem(float x, float y, ObjPlayer* player) :
    Intersection(Shape(x, y, 0)),
    player(player)
  {
  }

  ItemIntersection::ItemIntersection(float x, float y, float r, ObjItem* item) :
    Intersection(Shape(x, y, r)),
    item(item)
  {
  }

  TempEnemyShotIntersection::TempEnemyShotIntersection(float x, float y, float r) :
    Intersection(Shape(x, y, r))
  {
  }

  TempEnemyShotIntersection::TempEnemyShotIntersection(float x1, float y1, float x2, float y2, float width) :
    Intersection(Shape(x1, y1, x2, y2, width))
  {
  }

  static inline bool isShotIntersectionEnabled(std::shared_ptr<ShotIntersection>& isect) {
    ObjShot* enemyShot = isect->shot;
    // Regist前でShotDataで元から設定されている当たり判定の時は衝突無効
    if (!enemyShot->isRegistered() && !isect->isTmpIntersection) return false;
    // 判定が無効になっているとき、または遅延時は衝突無効
    if (!enemyShot->isIntersectionEnabled() || enemyShot->isDelay()) return false;
    return true;
  }

  static void collideEnemyShotWithPlayerShot(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto playerShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect2);
    ObjShot* playerShot = playerShotIsect->shot;
    if (playerShot->isEraseShotEnabled()) {
      auto enemyShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect1);
      ObjShot* enemyShot = enemyShotIsect->shot;
      if (isShotIntersectionEnabled(enemyShotIsect) && isShotIntersectionEnabled(playerShotIsect)) {
        if (playerShot->getType() == OBJ_SHOT) {
          playerShot->setPenetration(playerShot->getPenetration() - 1);
        }
        enemyShot->eraseWithSpell();
      }
    }
  }

  static void collideEnemyShotWithPlayer(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto enemyShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect1);
    if (isShotIntersectionEnabled(enemyShotIsect)) {
      ObjShot* enemyShot = enemyShotIsect->shot;
      auto player = std::dynamic_pointer_cast<PlayerIntersection>(isect2)->player;
      player->hit(enemyShot->getID());
    }
  }

  static void collideEnemyShotWithPlayerGraze(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto enemyShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect1);
    if (isShotIntersectionEnabled(enemyShotIsect)) {
      ObjShot* enemyShot = enemyShotIsect->shot;
      ObjPlayer* player = std::dynamic_pointer_cast<PlayerGrazeIntersection>(isect2)->player;
      if (enemyShot->isGrazeEnabled() && player->isGrazeEnabled()) {
        player->addGraze(enemyShot->getID(), 1);
        enemyShot->graze();
      }
    }
  }

  static void collideEnemyShotWithSpell(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto enemyShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect1);
    ObjSpell* spell = std::dynamic_pointer_cast<SpellIntersection>(isect2)->spell;
    if (spell->isEraseShotEnabled()) {
      if (isShotIntersectionEnabled(enemyShotIsect)) {
        enemyShotIsect->shot->eraseWithSpell();
      }
    }
  }

  static void collidePlayerShotWithEnemyIntersectionToShot(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto playerShotIsect = std::dynamic_pointer_cast<ShotIntersection>(isect1);
    if (isShotIntersectionEnabled(playerShotIsect)) {
      ObjShot* shot = playerShotIsect->shot;
      ObjEnemy* enemy = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect2)->enemy;
      if (shot->getType() == OBJ_SHOT) {
        shot->setPenetration(shot->getPenetration() - 1);
      }
      if (shot->isSpellFactorEnabled()) {
        enemy->addSpellDamage(shot->getDamage());
      } else {
        enemy->addShotDamage(shot->getDamage());
      }
    }
  }

  static void collidePlayerWithEnemyIntersectionToPlayer(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    ObjPlayer* player = std::dynamic_pointer_cast<PlayerIntersection>(isect1)->player;
    ObjEnemy* enemy = std::dynamic_pointer_cast<EnemyIntersectionToPlayer>(isect2)->enemy;
    player->hit(enemy->getID());
  }

  static void collideEnemyIntersectionToShotWithSpell(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    ObjEnemy* enemy = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect1)->enemy;
    ObjSpell* spell = std::dynamic_pointer_cast<SpellIntersection>(isect2)->spell;
    enemy->addSpellDamage(spell->getDamage());
  }

  static void collideWithPlayerToItemWithItem(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    ObjPlayer* player = std::dynamic_pointer_cast<PlayerIntersectionToItem>(isect1)->player;
    ObjItem* item = std::dynamic_pointer_cast<ItemIntersection>(isect2)->item;
    if (player->getState() == STATE_NORMAL) {
      player->getItem(item->getID());
      item->obtained();
    }
  }

  static void collidePlayerWithTempEnemyShot(std::shared_ptr<Intersection>& isect1, std::shared_ptr<Intersection>& isect2) {
    auto player = std::dynamic_pointer_cast<PlayerIntersection>(isect1)->player;
    player->hit(ID_INVALID);
  }

  // col matrix
  const CollisionFunction defaultCollisionMatrix[DEFAULT_COLLISION_MATRIX_SIZE] = {
    /* COL_GRP_ENEMY_SHOT      */  NULL, collideEnemyShotWithPlayerShot, collideEnemyShotWithPlayer, collideEnemyShotWithPlayerGraze, NULL, NULL, collideEnemyShotWithSpell, NULL, NULL, NULL,
    /* COL_GRP_PLAYER_SHOT     */  NULL, NULL, NULL, NULL, collidePlayerShotWithEnemyIntersectionToShot, NULL, NULL, NULL, NULL, NULL,
    /* COL_GRP_PLAYER          */  NULL, NULL, NULL, NULL, NULL, collidePlayerWithEnemyIntersectionToPlayer, NULL, NULL, NULL, collidePlayerWithTempEnemyShot,
    /* COL_GRP_PLAYER_GRAZE    */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* COL_GRP_ENEMY_TO_SHOT   */  NULL, NULL, NULL, NULL, NULL, NULL, collideEnemyIntersectionToShotWithSpell, NULL, NULL, NULL,
    /* COL_GRP_ENEMY_TO_PLAYER */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* COL_GRP_SPELL           */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* COL_GRP_PLAYER_TO_ITEM  */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, collideWithPlayerToItemWithItem, NULL,
    /* COL_GRP_ITEM            */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    /* COL_GRP_TEMP_ENEMY_SHOT */  NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
  };
}