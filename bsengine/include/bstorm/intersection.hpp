#pragma once

#include <vector>
#include <list>
#include <memory>

namespace bstorm {
  class Renderer;

  struct BoundingBox {
    BoundingBox();
    BoundingBox(float left, float top, float right, float bottom);
    bool isIntersected(const BoundingBox& other) const;
    float left;
    float top;
    float right;
    float bottom;
  };

  bool isIntersectedLineCircle(float x1, float y1, float x2, float y2, float width, float cx, float cy, float r);

  class Shape {
  public:
    enum class Type {
      CIRCLE,
      RECT
    };
    Shape(float x, float y, float r);
    Shape(float x1, float y1, float x2, float y2, float width);
    bool isIntersected(const Shape& other) const;
    const BoundingBox& getBoundingBox() const;
    void trans(float dx, float dy);
    void setWidth(float width);
    void render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const;
    Type getType() const;
    void getCircle(float& x, float &y, float& r) const;
    void getRect(float& x1, float& y1, float& x2, float& y2, float& width) const;
  private:
    void updateBoundingBox();
    void transBoundingBox(float dx, float dy);
    const Type type;
    union {
      struct {
        float x;
        float y;
        float r;
      } Circle;
      struct {
        float x1;
        float y1;
        float x2;
        float y2;
        float width;
      } Rect;
    } params;
    BoundingBox boundingBox;
  };

  class Intersection;
  typedef int CollisionGroup;
  typedef void(*CollisionFunction)(std::shared_ptr<Intersection>&, std::shared_ptr<Intersection>&);
  typedef CollisionFunction* CollisionMatrix;

  class CollisionDetector;
  class Intersection {
  public:
    Intersection(const Shape& shape);
    virtual ~Intersection();
    virtual CollisionGroup getCollisionGroup() const = 0;
    bool isIntersected(const std::shared_ptr<Intersection>& isect) const;
    void render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const;
    const Shape& getShape() const;
    int getTreeIndex() const;
    const std::vector<std::weak_ptr<Intersection>>& getCollideIntersections() const;
  protected:
    Shape shape;
  private:
    int treeIdx;
    std::list<std::weak_ptr<Intersection>>::iterator posInCell;
    std::vector<std::weak_ptr<Intersection>> collideIsects; // 衝突した当たり判定
    friend CollisionDetector;
  };

  class CollisionDetector {
  public:
    CollisionDetector(int fieldWidth, int fieldHeight, int maxLevel, const CollisionMatrix mat, int matrixDim);
    ~CollisionDetector();
    void add(const std::shared_ptr<Intersection>&);
    template <class T, class... Args>
    std::shared_ptr<T> create(Args... args) {
      std::shared_ptr<T> obj = std::make_shared<T>(args...);
      add(obj);
      return obj;
    }
    void remove(const std::shared_ptr<Intersection>&);
    void update(const std::shared_ptr<Intersection>&);
    void trans(const std::shared_ptr<Intersection>&, float dx, float dy);
    void setWidth(const std::shared_ptr<Intersection>&, float width);
    void run();
    std::vector<std::weak_ptr<Intersection>> getIntersectionsCollideWithIntersection(const std::shared_ptr<Intersection>& isect) const;
    std::vector<std::weak_ptr<Intersection>> getIntersectionsCollideWithShape(const Shape& shape) const;
  private:
    void run(int idx, std::vector<std::vector<std::shared_ptr<Intersection>>>& supers);
    int calcTreeIndexFromBoundingBox(const BoundingBox& boundingBox) const;
    float fieldWidth;
    float fieldHeight;
    int maxLevel;
    float unitCellWidth;
    float unitCellHeight;
    int matrixDim;
    CollisionMatrix colMatrix;
    std::vector<std::list<std::weak_ptr<Intersection>>> quadTree;
  };
}