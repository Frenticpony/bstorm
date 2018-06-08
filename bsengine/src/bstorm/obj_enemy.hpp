#pragma once

#include <bstorm/point2D.hpp>
#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm
{
class Intersection;
class ObjEnemy : public ObjSprite2D, public ObjMove, public ObjCol, public std::enable_shared_from_this<ObjEnemy>
{
public:
    ObjEnemy(bool isBoss, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package);
    ~ObjEnemy();
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    bool IsBoss() const;
    bool IsRegistered() const;
    void Regist();
    double GetLife() const { return life_; }
    double GetDamageRateShot() const;
    double GetDamageRateSpell() const;
    int GetPrevFrameShotHitCount() const;
    void SetLife(double life);
    void AddLife(double life);
    void SetDamageRateShot(double rate);
    void SetDamageRateSpell(double rate);
    const std::vector<Point2D>& GetIntersectionToShotPositions() const;
    void AddTempIntersectionCircleToShot(float x, float y, float r);
    void AddTempIntersectionCircleToPlayer(float x, float y, float r);
    void AddShotDamage(double damage);
    void AddSpellDamage(double damage);
private:
    bool isRegistered_;
    const bool isBoss_;
    double life_;
    double damageRateShot_;
    double damageRateSpell_;
    int shotHitCount_;
    int prevFrameShotHitCount_;
    std::vector<Point2D> tempIsectToShotPositions_;
};
}