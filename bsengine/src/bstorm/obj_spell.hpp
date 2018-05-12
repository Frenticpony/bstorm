#pragma once

#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm
{
class SpellIntersection;
class ObjSpell : public ObjPrim2D, public ObjCol, public std::enable_shared_from_this<ObjSpell>
{
public:
    ObjSpell(const std::shared_ptr<Package>& package);
    ~ObjSpell();
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    bool IsRegistered() const;
    void Regist();
    double GetDamage() const;
    void SetDamage(double damage);
    bool IsEraseShotEnabled() const;
    void SetEraseShotEnable(bool enable);
    void AddTempIntersection(const std::shared_ptr<SpellIntersection>& isect);
    void AddTempIntersectionCircle(float x, float y, float r);
    void AddTempIntersectionLine(float x1, float y1, float x2, float y2, float width);
protected:
    double damage_;
    bool eraseShotEnable_;
    bool isRegistered_;
};

class ObjSpellManage : public Obj
{
public:
    ObjSpellManage(const std::shared_ptr<Package>& package);
    ~ObjSpellManage();
    void Update() override;
};
}