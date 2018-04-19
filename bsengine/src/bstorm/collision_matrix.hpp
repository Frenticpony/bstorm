#pragma once

#include <bstorm/intersection.hpp>

namespace bstorm
{
constexpr CollisionGroup COL_GRP_ENEMY_SHOT = 0;
constexpr CollisionGroup COL_GRP_PLAYER_SHOT = 1;
constexpr CollisionGroup COL_GRP_PLAYER = 2;
constexpr CollisionGroup COL_GRP_PLAYER_GRAZE = 3;
constexpr CollisionGroup COL_GRP_ENEMY_TO_SHOT = 4;
constexpr CollisionGroup COL_GRP_ENEMY_TO_PLAYER = 5;
constexpr CollisionGroup COL_GRP_SPELL = 6;
constexpr CollisionGroup COL_GRP_PLAYER_TO_ITEM = 7;
constexpr CollisionGroup COL_GRP_ITEM = 8;
constexpr CollisionGroup COL_GRP_TEMP_ENEMY_SHOT = 9;

class ObjShot;
class ShotIntersection : public Intersection
{
public:
    ShotIntersection(float x, float y, float r, ObjShot* shot, bool isTmpIntersection);
    ShotIntersection(float x1, float y1, float x2, float y2, float width, ObjShot* shot, bool isTmpIntersection);
    CollisionGroup getCollisionGroup() const
    {
        if (isPlayerShot)
        {
            return COL_GRP_PLAYER_SHOT;
        }
        return COL_GRP_ENEMY_SHOT;
    }
    ObjShot* shot;
    const bool isPlayerShot;
    const bool isTmpIntersection;
};

class ObjEnemy;
class EnemyIntersectionToShot : public Intersection
{
public:
    EnemyIntersectionToShot(float x, float y, float r, ObjEnemy *enemy);
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_ENEMY_TO_SHOT;
    }
    float getX() { return x; }
    float getY() { return y; }
    ObjEnemy *enemy;
private:
    const float x;
    const float y;
};

class EnemyIntersectionToPlayer : public Intersection
{
public:
    EnemyIntersectionToPlayer(float x, float y, float r, ObjEnemy *enemy);
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_ENEMY_TO_PLAYER;
    }
    ObjEnemy *enemy;
};

class ObjPlayer;
class PlayerIntersection : public Intersection
{
public:
    PlayerIntersection(float x, float y, float r, ObjPlayer *player);
    ObjPlayer *player;
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_PLAYER;
    }
};

class PlayerGrazeIntersection : public Intersection
{
public:
    PlayerGrazeIntersection(float x, float y, float r, ObjPlayer *player);
    ObjPlayer *player;
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_PLAYER_GRAZE;
    }
    virtual void render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const {};
};

class ObjSpell;
class SpellIntersection : public Intersection
{
public:
    SpellIntersection(float x, float y, float r, ObjSpell *spell);
    SpellIntersection(float x1, float y1, float x2, float y2, float width, ObjSpell *spell);
    ObjSpell *spell;
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_SPELL;
    }
};

class PlayerIntersectionToItem : public Intersection
{
public:
    PlayerIntersectionToItem(float x, float y, ObjPlayer* player);
    ObjPlayer* player;
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_PLAYER_TO_ITEM;
    }
};

class ObjItem;
class ItemIntersection : public Intersection
{
public:
    ItemIntersection(float x, float y, float r, ObjItem* item);
    ObjItem* item;
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_ITEM;
    }
};

class ObjItem;
class TempEnemyShotIntersection : public Intersection
{
public:
    TempEnemyShotIntersection(float x, float y, float r);
    TempEnemyShotIntersection(float x1, float y1, float x2, float y2, float width);
    CollisionGroup getCollisionGroup() const
    {
        return COL_GRP_TEMP_ENEMY_SHOT;
    }
};

constexpr int DEFAULT_COLLISION_MATRIX_DIMENSION = 10;
constexpr int DEFAULT_COLLISION_MATRIX_SIZE = DEFAULT_COLLISION_MATRIX_DIMENSION * DEFAULT_COLLISION_MATRIX_DIMENSION;
extern const CollisionFunction defaultCollisionMatrix[DEFAULT_COLLISION_MATRIX_SIZE];
}