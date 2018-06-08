#pragma once

#include <deque>
#include <vector>
#include <memory>

namespace bstorm
{
class Intersection;
class Renderer;
class CollisionDetector;
class Package;
class ObjCol
{
public:
    ObjCol(const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package);
    ~ObjCol();
    const std::deque<std::shared_ptr<Intersection>>& GetIntersections() const { return isects_; }
    const std::vector<std::shared_ptr<Intersection>>& GetTempIntersections() const { return tempIsects_; }
    bool IsIntersected(const std::shared_ptr<ObjCol>& col) const;
    bool IsIntersected() const;
    int GetIntersectedCount() const;
    std::vector<std::weak_ptr<Intersection>> GetCollideIntersections() const;
protected:
    void AddIntersection(const std::shared_ptr<Intersection>& isect);
    void RemoveOldestIntersection();
    void AddTempIntersection(const std::shared_ptr<Intersection>& isect);
    void TransIntersection(float dx, float dy);
    void SetWidthIntersection(float width);
    void RenderIntersection(const std::shared_ptr<Renderer>& renderer, bool isPermitCamera) const;
    void ClearIntersection();
    // Obj::Update時に呼んで保持用を空にして追加用と保持用を入れ替える,
    void UpdateTempIntersection();
private:
    std::deque<std::shared_ptr<Intersection>> isects_;
    std::vector<std::shared_ptr<Intersection>> addedTempIsects_; // 追加用
    std::vector<std::shared_ptr<Intersection>> tempIsects_; // 保持用
    std::shared_ptr<CollisionDetector> colDetector_;
    std::weak_ptr<Package> package_;
};
}
