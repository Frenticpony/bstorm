#pragma once

#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm
{
class SpellIntersection;
class ObjSpell : public ObjPrim2D, public ObjCol, public std::enable_shared_from_this<ObjSpell>
{
public:
    ObjSpell(const std::shared_ptr<GameState>& gameState);
    ~ObjSpell();
    void update() override;
    void render() override;
    bool isRegistered() const;
    void regist();
    double getDamage() const;
    void setDamage(double damage);
    bool isEraseShotEnabled() const;
    void setEraseShotEnable(bool enable);
    void addTempIntersection(const std::shared_ptr<SpellIntersection>& isect);
    void addTempIntersectionCircle(float x, float y, float r);
    void addTempIntersectionLine(float x1, float y1, float x2, float y2, float width);
protected:
    double damage;
    bool eraseShotEnable;
    bool registerFlag;
};

class ObjSpellManage : public Obj
{
public:
    ObjSpellManage(const std::shared_ptr<GameState>& gameState);
    ~ObjSpellManage();
    void update() override;
};
}