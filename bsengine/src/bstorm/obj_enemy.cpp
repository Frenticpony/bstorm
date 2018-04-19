#include <bstorm/obj_enemy.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>
#include <bstorm/collision_matrix.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/game_state.hpp>

namespace bstorm
{
ObjEnemy::ObjEnemy(bool isBoss, const std::shared_ptr<GameState>& gameState) :
    ObjSprite2D(gameState),
    ObjMove(this),
    ObjCol(gameState),
    registerFlag(false),
    life(0),
    damageRateShot(1.0),
    damageRateSpell(1.0),
    shotHitCount(0),
    prevFrameShotHitCount(0),
    bossFlag(isBoss)
{
    setType(isBoss ? OBJ_ENEMY_BOSS : OBJ_ENEMY);
}

ObjEnemy::~ObjEnemy() {}

void ObjEnemy::update()
{
    if (isRegistered())
    {
        move();
    }
    prevFrameShotHitCount = 0;
    std::swap(shotHitCount, prevFrameShotHitCount);
    clearOldTempIntersection();
    for (const auto& isect : getTempIntersections())
    {
        if (auto toShot = std::dynamic_pointer_cast<EnemyIntersectionToShot>(isect))
        {
            prevTempIsectToShotPositions.emplace_back(toShot->getX(), toShot->getY());
        }
    }
}

void ObjEnemy::render()
{
    if (isRegistered())
    {
        ObjSprite2D::render();
        ObjCol::renderIntersection(isPermitCamera());
    }
}

bool ObjEnemy::isBoss() const { return bossFlag; }

bool ObjEnemy::isRegistered() const
{
    return registerFlag;
}

void ObjEnemy::regist() { registerFlag = true; }

double ObjEnemy::getDamageRateShot() const { return damageRateShot * 100.0; }

double ObjEnemy::getDamageRateSpell() const { return damageRateSpell * 100.0; }

int ObjEnemy::getPrevFrameShotHitCount() const { return prevFrameShotHitCount; }

void ObjEnemy::setLife(double life)
{
    this->life = life;
}

void ObjEnemy::addLife(double life)
{
    this->life += life;
}

void ObjEnemy::setDamageRateShot(double rate) { damageRateShot = rate / 100.0; }

void ObjEnemy::setDamageRateSpell(double rate) { damageRateSpell = rate / 100.0; }

const std::vector<Point2D>& ObjEnemy::getAllIntersectionToShotPosition() const
{
    return prevTempIsectToShotPositions;
}

void ObjEnemy::addTempIntersection(const std::shared_ptr<Intersection>& isect)
{
    if (auto state = getGameState())
    {
        state->colDetector->add(isect);
    }
    ObjCol::addTempIntersection(isect);
}

void ObjEnemy::addTempIntersectionCircleToShot(float x, float y, float r)
{
    addTempIntersection(std::make_shared<EnemyIntersectionToShot>(x, y, r, this));
}

void ObjEnemy::addTempIntersectionCircleToPlayer(float x, float y, float r)
{
    addTempIntersection(std::make_shared<EnemyIntersectionToPlayer>(x, y, r, this));
}

void ObjEnemy::addShotDamage(double damage)
{
    damage *= damageRateShot;
    life -= damage;
    shotHitCount++;
    if (isBoss())
    {
        if (auto state = getGameState())
        {
            if (auto bossScene = state->enemyBossSceneObj.lock())
            {
                bossScene->addDamage(damage);
            }
        }
    }
}

void ObjEnemy::addSpellDamage(double damage)
{
    damage *= damageRateSpell;
    life -= damage;
    if (isBoss())
    {
        if (auto state = getGameState())
        {
            if (auto bossScene = state->enemyBossSceneObj.lock())
            {
                bossScene->addDamage(damage);
            }
        }
    }
}
}