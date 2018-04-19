#include <bstorm/obj_spell.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/intersection.hpp>
#include <bstorm/collision_matrix.hpp>
#include <bstorm/game_state.hpp>

namespace bstorm
{
ObjSpell::ObjSpell(const std::shared_ptr<GameState>& gameState) :
    ObjPrim2D(gameState),
    ObjCol(gameState),
    damage(0),
    registerFlag(false),
    eraseShotEnable(true)
{
    setType(OBJ_SPELL);
}

ObjSpell::~ObjSpell() {}

void ObjSpell::update()
{
    clearOldTempIntersection();
}

void ObjSpell::render()
{
    if (isRegistered())
    {
        ObjPrim2D::render();
        ObjCol::renderIntersection(isPermitCamera());
    }
}

bool ObjSpell::isRegistered() const
{
    return registerFlag;
}

void ObjSpell::regist() { registerFlag = true; }

double ObjSpell::getDamage() const { return damage; }

void ObjSpell::setDamage(double damage) { this->damage = damage; }

bool ObjSpell::isEraseShotEnabled() const { return eraseShotEnable; }

void ObjSpell::setEraseShotEnable(bool enable) { eraseShotEnable = enable; }

void ObjSpell::addTempIntersection(const std::shared_ptr<SpellIntersection>& isect)
{
    if (auto state = getGameState())
    {
        state->colDetector->add(isect);
    }
    ObjCol::addTempIntersection(isect);
}

void ObjSpell::addTempIntersectionCircle(float x, float y, float r)
{
    addTempIntersection(std::make_shared<SpellIntersection>(x, y, r, this));
}

void ObjSpell::addTempIntersectionLine(float x1, float y1, float x2, float y2, float width)
{
    addTempIntersection(std::make_shared<SpellIntersection>(x1, y1, x2, y2, width, this));
}
ObjSpellManage::ObjSpellManage(const std::shared_ptr<GameState>& gameState) :
    Obj(gameState)
{
    setType(OBJ_SPELL_MANAGE);
}
ObjSpellManage::~ObjSpellManage()
{
}
void ObjSpellManage::update() {}
}