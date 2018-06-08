#include <bstorm/obj_enemy.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/package.hpp>

namespace bstorm
{
ObjEnemy::ObjEnemy(bool isBoss, const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package) :
    ObjSprite2D(package),
    ObjMove(this),
    ObjCol(colDetector, package),
    isRegistered_(false),
    life_(0),
    damageRateShot_(1.0),
    damageRateSpell_(1.0),
    shotHitCount_(0),
    prevFrameShotHitCount_(0),
    isBoss_(isBoss)
{
    SetType(isBoss ? OBJ_ENEMY_BOSS : OBJ_ENEMY);
}

ObjEnemy::~ObjEnemy() {}

void ObjEnemy::Update()
{
    if (IsRegistered())
    {
        Move();
    }
    prevFrameShotHitCount_ = 0;
    std::swap(shotHitCount_, prevFrameShotHitCount_);
    UpdateTempIntersection();
    for (const auto& isect : GetTempIntersections())
    {
        if (auto toShot = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect))
        {
            tempIsectToShotPositions_.emplace_back(toShot->GetX(), toShot->GetY());
        }
    }
}

void ObjEnemy::Render(const std::shared_ptr<Renderer>& renderer)
{
    if (IsRegistered())
    {
        ObjSprite2D::Render(renderer);
        ObjCol::RenderIntersection(renderer, IsPermitCamera());
    }
}

bool ObjEnemy::IsBoss() const { return isBoss_; }

bool ObjEnemy::IsRegistered() const
{
    return isRegistered_;
}

void ObjEnemy::Regist() { isRegistered_ = true; }

double ObjEnemy::GetDamageRateShot() const { return damageRateShot_ * 100.0; }

double ObjEnemy::GetDamageRateSpell() const { return damageRateSpell_ * 100.0; }

int ObjEnemy::GetPrevFrameShotHitCount() const { return prevFrameShotHitCount_; }

void ObjEnemy::SetLife(double life)
{
    this->life_ = life;
}

void ObjEnemy::AddLife(double life)
{
    this->life_ += life;
}

void ObjEnemy::SetDamageRateShot(double rate) { damageRateShot_ = rate / 100.0; }

void ObjEnemy::SetDamageRateSpell(double rate) { damageRateSpell_ = rate / 100.0; }

const std::vector<Point2D>& ObjEnemy::GetIntersectionToShotPositions() const
{
    return tempIsectToShotPositions_;
}

void ObjEnemy::AddTempIntersectionCircleToShot(float x, float y, float r)
{
    AddTempIntersection(std::make_shared<EnemyIntersectionToShot>(x, y, r, shared_from_this()));
}

void ObjEnemy::AddTempIntersectionCircleToPlayer(float x, float y, float r)
{
    AddTempIntersection(std::make_shared<EnemyIntersectionToPlayer>(x, y, r, shared_from_this()));
}

void ObjEnemy::AddShotDamage(double damage)
{
    damage *= damageRateShot_;
    life_ -= damage;
    shotHitCount_++;
    if (IsBoss())
    {
        if (auto package = GetPackage().lock())
        {
            if (auto bossScene = package->GetEnemyBossSceneObject())
            {
                bossScene->AddDamage(damage);
            }
        }
    }
}

void ObjEnemy::AddSpellDamage(double damage)
{
    damage *= damageRateSpell_;
    life_ -= damage;
    if (IsBoss())
    {
        if (auto package = GetPackage().lock())
        {
            if (auto bossScene = package->GetEnemyBossSceneObject())
            {
                bossScene->AddDamage(damage);
            }
        }
    }
}
}