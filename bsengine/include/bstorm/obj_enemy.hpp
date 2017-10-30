#pragma once

#include <bstorm/type.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm {
  class Intersection;
  class ObjEnemy : public ObjSprite2D, public ObjMove, public ObjCol {
  public:
    ObjEnemy(bool isBoss, const std::shared_ptr<GameState>& gameState);
    ~ObjEnemy();
    void update() override;
    void render() override;
    bool isBoss() const;
    bool isRegistered() const;
    void regist();
    double getLife() const { return life; }
    double getDamageRateShot() const;
    double getDamageRateSpell() const;
    int getPrevFrameShotHitCount() const;
    void setLife(double life);
    void addLife(double life);
    void setDamageRateShot(double rate);
    void setDamageRateSpell(double rate);
    const std::vector<Point2D>& getAllIntersectionToShotPosition() const;
    void addTempIntersection(const std::shared_ptr<Intersection>& isect);
    void addTempIntersectionCircleToShot(float x, float y, float r);
    void addTempIntersectionCircleToPlayer(float x, float y, float r);
    void addShotDamage(double damage);
    void addSpellDamage(double damage);
  protected:
    bool registerFlag;
    const bool bossFlag;
    double life;
    double damageRateShot;
    double damageRateSpell;
    int shotHitCount;
    int prevFrameShotHitCount;
    std::vector<Point2D> prevTempIsectToShotPositions;
  };
}