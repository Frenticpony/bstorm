#include <array>
#include <deque>
#include <algorithm>
#include <d3dx9.h>

#include <bstorm/dnh_const.hpp>
#include <bstorm/type.hpp>
#include <bstorm/util.hpp>
#include <bstorm/renderer.hpp>
#include <bstorm/intersection.hpp>

namespace bstorm {

// 頂点の順番
// 必ず時計回りになる
// [0] (x1, y1) [1]
//  |            |
// [3] (x2, y2) [2]
  static std::array<Point2D, 4> lineToRect(float x1, float y1, float x2, float y2, float width) {
    const float halfWidth = width / 2;
    const float normalDir = atan2(y2 - y1, x2 - x1) + D3DX_PI / 2;
    const float dx = halfWidth * cos(normalDir);
    const float dy = halfWidth * sin(normalDir);
    return std::array<Point2D, 4>{Point2D{ x1 + dx, y1 + dy },
      Point2D{ x1 - dx, y1 - dy },
      Point2D{ x2 - dx, y2 - dy },
      Point2D{ x2 + dx, y2 + dy }};
  }

  BoundingBox::BoundingBox() :
    left(0),
    top(0),
    right(0),
    bottom(0)
  {
  }

  BoundingBox::BoundingBox(float l, float t, float r, float b) :
    left(l),
    top(t),
    right(r),
    bottom(b)
  {
  }

  bool BoundingBox::isIntersected(const BoundingBox& other) const {
    return left <= other.right && top <= other.bottom && other.left <= right && other.top <= bottom;
  }


  // 外積
  static inline float cross2(float x1, float y1, float x2, float y2) {
    return x1 * y2 - x2 * y1;
  }

  // 内積
  static inline float dot2(float x1, float y1, float x2, float y2) {
    return x1 * x2 + y1 * y2;
  }

  // 線分と線分の交差判定
  static bool isIntersectedSegmentSegment(const Point2D& a, const Point2D& b, const Point2D& c, const Point2D& d) {
    // x : cross
    // * : multiply
    // check
    // AC x DC * BC x DC <= 0 && AC x AB * AD x AB <= 0

    float acX = c.x - a.x;
    float acY = c.y - a.y;
    float abX = b.x - a.x;
    float abY = b.y - a.y;
    float adX = d.x - a.x;
    float adY = d.y - a.y;
    float bcX = c.x - b.x;
    float bcY = c.y - b.y;
    float dcX = c.x - d.x;
    float dcY = c.y - d.y;
    float cp1 = cross2(acX, acY, dcX, dcY);
    float cp2 = cross2(bcX, bcY, dcX, dcY);
    float cp3 = cross2(acX, acY, abX, abY);
    float cp4 = cross2(adX, adY, abX, abY);

    return cp1 * cp2 <= 0 && cp3 * cp4 <= 0;
  }

  // 点が矩形内にあるかどうか判定
  static bool isPointInRect(const Point2D& p, const std::array<Point2D, 4>& rect) {
    const Point2D& a = rect[0];
    const Point2D& b = rect[1];
    const Point2D& c = rect[2];
    const Point2D& d = rect[3];
    float apX = p.x - a.x;
    float apY = p.y - a.y;
    float abX = b.x - a.x;
    float abY = b.y - a.y;
    float bpX = p.x - b.x;
    float bpY = p.y - b.y;
    float bcX = c.x - b.x;
    float bcY = c.y - b.y;
    float cpX = p.x - c.x;
    float cpY = p.y - c.y;
    float cdX = d.x - c.x;
    float cdY = d.y - c.y;
    float dpX = p.x - d.x;
    float dpY = p.y - d.y;
    float daX = a.x - d.x;
    float daY = a.y - d.y;
    float cp1 = cross2(apX, apY, abX, abY);
    float cp2 = cross2(bpX, bpY, bcX, bcY);
    float cp3 = cross2(cpX, cpY, cdX, cdY);
    float cp4 = cross2(dpX, dpY, daX, daY);
    // 全て負なら矩形内に含まれる
    // 矩形は必ず時計回りなので全て正の場合を調べる必要はない
    return cp1 <= 0 && cp2 <= 0 && cp3 <= 0 && cp4 <= 0;
  }

  static bool isIntersectedCircleSegment(float cx, float cy, float r, const Point2D& a, const Point2D& b) {
    float abX = b.x - a.x;
    float abY = b.y - a.y;
    float acX = cx - a.x;
    float acY = cy - a.y;
    float bcX = cx - b.x;
    float bcY = cy - b.y;
    // d : 円の中心からABを通る直線への垂線の長さ
    float d = abs(cross2(abX, abY, acX, acY)) / norm(abX, abY);
    if (d > r) return false;
    if (dot2(abX, abY, acX, acY) * dot2(abX, abY, bcX, bcY) <= 0) return true;
    return norm(acX, acY) <= r || norm(bcX, bcY) <= r;
  }

  // 弾幕風のLine = Rect
  bool isIntersectedLineCircle(float x1, float y1, float x2, float y2, float width, float cx, float cy, float r) {
    const auto rect = lineToRect(x1, y1, x2, y2, width);
    // 円が矩形の辺と交わっている場合
    for (int i = 0; i < 4; i++) {
      if (isIntersectedCircleSegment(cx, cy, r, rect[i], rect[(i + 1) & 3])) return true;
    }
    // 円が矩形に入っている場合
    return isPointInRect(Point2D{ cx, cy }, rect);
  }

  Shape::Shape(float x, float y, float r) :
    type(Type::CIRCLE)
  {
    params.Circle.x = x;
    params.Circle.y = y;
    params.Circle.r = r;
    updateBoundingBox();
  }

  Shape::Shape(float x1, float y1, float x2, float y2, float width) :
    type(Type::RECT)
  {
    params.Rect.x1 = x1;
    params.Rect.y1 = y1;
    params.Rect.x2 = x2;
    params.Rect.y2 = y2;
    params.Rect.width = width;
    updateBoundingBox();
  }

  bool Shape::isIntersected(const Shape& other) const {
    if (!boundingBox.isIntersected(other.boundingBox)) return false;
    if (type == Type::CIRCLE && other.type == Type::CIRCLE) {
      float dx = params.Circle.x - other.params.Circle.x;
      float dy = params.Circle.y - other.params.Circle.y;
      float d = params.Circle.r + other.params.Circle.r;
      return dx * dx + dy * dy <= d * d;
    } else if (type == Type::CIRCLE && other.type == Type::RECT) {
      float x1 = other.params.Rect.x1;
      float y1 = other.params.Rect.y1;
      float x2 = other.params.Rect.x2;
      float y2 = other.params.Rect.y2;
      float width = other.params.Rect.width;
      float cx = params.Circle.x;
      float cy = params.Circle.y;
      float r = params.Circle.r;
      return isIntersectedLineCircle(x1, y1, x2, y2, width, cx, cy, r);
    } else if (type == Type::RECT && other.type == Type::CIRCLE) {
      float x1 = params.Rect.x1;
      float y1 = params.Rect.y1;
      float x2 = params.Rect.x2;
      float y2 = params.Rect.y2;
      float width = params.Rect.width;
      float cx = other.params.Circle.x;
      float cy = other.params.Circle.y;
      float r = other.params.Circle.r;
      return isIntersectedLineCircle(x1, y1, x2, y2, width, cx, cy, r);
    } else if (type == Type::RECT && other.type == Type::RECT) {
      const auto rect1 = lineToRect(params.Rect.x1, params.Rect.y1, params.Rect.x2, params.Rect.y2, params.Rect.width);
      const auto rect2 = lineToRect(other.params.Rect.x1, other.params.Rect.y1, other.params.Rect.x2, other.params.Rect.y2, other.params.Rect.width);
      // 辺同士が交わってる場合
      for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
          if (isIntersectedSegmentSegment(rect1[i], rect1[(i + 1) & 3], rect2[j], rect2[(j + 1) & 3])) return true;
        }
      }
      // 矩形が片方の矩形に完全に含まれる場合
      return isPointInRect(rect1[0], rect2) || isPointInRect(rect2[0], rect1);
    }
    return false;
  }

  const BoundingBox & Shape::getBoundingBox() const { return boundingBox; }

  void Shape::trans(float dx, float dy) {
    if (type == Type::CIRCLE) {
      params.Circle.x += dx;
      params.Circle.y += dy;
    } else if (type == Type::RECT) {
      params.Rect.x1 += dx; params.Rect.x2 += dx;
      params.Rect.y1 += dy; params.Rect.y2 += dy;
    }
    transBoundingBox(dx, dy);
  }

  void Shape::setWidth(float width) {
    width = abs(width);
    if (type == Type::CIRCLE) {
      params.Circle.r = width / 2;
    } else if (type == Type::RECT) {
      params.Rect.width = width;
    }
    updateBoundingBox();
  }

  void Shape::render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const {
    const D3DCOLOR color = D3DCOLOR_ARGB(128, 255, 0, 0);
    if (type == Type::CIRCLE) {
      static constexpr int vertexNum = 66;
      static constexpr int way = vertexNum - 2;
      static bool isInitialized = false;
      static std::array<Vertex, vertexNum> vertices;
      if (!isInitialized) {
        float x = 0;
        float y = 1;
        float c = cos(2 * D3DX_PI / way);
        float s = sin(2 * D3DX_PI / way);
        for (int i = 0; i < vertexNum; i++) {
          Vertex& v = vertices[i];
          if (i == 0) {
            v.x = 0;
            v.y = 0;
          } else {
            v.x = x;
            v.y = y;
            float nx = c * x - s * y;
            float ny = s * x + c * y;
            x = nx;
            y = ny;
          }
          v.color = color;
        }
        isInitialized = true;
      }
      D3DXMATRIX world = rotScaleTrans(params.Circle.x, params.Circle.y, 0, 0, 0, 0, params.Circle.r, params.Circle.r, 1);
      renderer->renderPrim2D(D3DPT_TRIANGLEFAN, vertices.size(), vertices.data(), NULL, BLEND_ALPHA, world, std::shared_ptr<Shader>(), permitCamera);
    } else if (type == Type::RECT) {
      static std::array<Vertex, 4> vertices;
      for (auto& v : vertices) {
        v.color = color;
      }
      const auto rect = lineToRect(params.Rect.x1, params.Rect.y1, params.Rect.x2, params.Rect.y2, params.Rect.width);
      vertices[0].x = rect[0].x;
      vertices[0].y = rect[0].y;
      vertices[1].x = rect[1].x;
      vertices[1].y = rect[1].y;
      vertices[2].x = rect[3].x;
      vertices[2].y = rect[3].y;
      vertices[3].x = rect[2].x;
      vertices[3].y = rect[2].y;
      D3DXMATRIX world;
      D3DXMatrixIdentity(&world);
      renderer->renderPrim2D(D3DPT_TRIANGLESTRIP, vertices.size(), vertices.data(), NULL, BLEND_ALPHA, world, std::shared_ptr<Shader>(), permitCamera);
    }
  }

  Shape::Type Shape::getType() const {
    return type;
  }

  void Shape::getCircle(float & x, float & y, float & r) const {
    x = params.Circle.x;
    y = params.Circle.y;
    r = params.Circle.r;
  }

  void Shape::getRect(float & x1, float & y1, float & x2, float & y2, float & width) const {
    x1 = params.Rect.x1;
    y1 = params.Rect.y1;
    x2 = params.Rect.x2;
    y2 = params.Rect.y2;
    width = params.Rect.width;
  }

  void Shape::updateBoundingBox() {
    if (type == Type::CIRCLE) {
      boundingBox.left = params.Circle.x - params.Circle.r;
      boundingBox.top = params.Circle.y - params.Circle.r;
      boundingBox.right = params.Circle.x + params.Circle.r;
      boundingBox.bottom = params.Circle.y + params.Circle.r;
    } else if (type == Type::RECT) {
      const auto rect = lineToRect(params.Rect.x1, params.Rect.y1, params.Rect.x2, params.Rect.y2, params.Rect.width);
      float minX = rect[0].x;
      float maxX = minX;
      float minY = rect[0].y;
      float maxY = minY;
      for (int i = 1; i < 4; i++) {
        float x = rect[i].x;
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        float y = rect[i].y;
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
      }
      boundingBox.left = minX;
      boundingBox.top = minY;
      boundingBox.right = maxX;
      boundingBox.bottom = maxY;
    }
  }

  void Shape::transBoundingBox(float dx, float dy) {
    boundingBox.left += dx;
    boundingBox.top += dy;
    boundingBox.right += dx;
    boundingBox.bottom += dy;
  }

  Intersection::Intersection(const Shape& shape) :
    shape(shape),
    treeIdx(-1)
  {
  }

  Intersection::~Intersection() {}

  bool Intersection::isIntersected(const std::shared_ptr<Intersection>& isect) const {
    return shape.isIntersected(isect->shape);
  }

  void Intersection::render(const std::shared_ptr<Renderer>& renderer, bool permitCamera) const {
    shape.render(renderer, permitCamera);
  }

  const Shape & Intersection::getShape() const {
    return shape;
  }

  int Intersection::getTreeIndex() const {
    return treeIdx;
  }

  const std::vector<std::weak_ptr<Intersection>>& Intersection::getCollideIntersections() const {
    return collideIsects;
  }

  static inline int calcTreeIndex(int level, int morton) {
    return ((1 << (level << 1)) - 1) / 3 + morton;
  }

  CollisionDetector::CollisionDetector(int width, int height, int maxlv, const CollisionMatrix mat, int dim) :
    fieldWidth(width),
    fieldHeight(height),
    maxLevel(maxlv),
    unitCellWidth(1.0f * width / (1 << maxlv)),
    unitCellHeight(1.0f * height / (1 << maxlv)),
    colMatrix(NULL),
    matrixDim(dim)
  {
    colMatrix = new CollisionFunction[dim*dim];
    memcpy(colMatrix, mat, sizeof(CollisionFunction) * dim * dim);
    quadTree.resize(calcTreeIndex(maxLevel + 1, 0));
  }

  CollisionDetector::~CollisionDetector() {
    safe_delete_array(colMatrix);
  }

  void CollisionDetector::add(const std::shared_ptr<Intersection>& isect) {
    if (isect->treeIdx >= 0) return;
    isect->treeIdx = calcTreeIndexFromBoundingBox(isect->shape.getBoundingBox());
    auto& cell = quadTree[isect->treeIdx];
    isect->posInCell = cell.insert(cell.end(), isect);
  }

  void CollisionDetector::remove(const std::shared_ptr<Intersection>& isect) {
    if (isect->treeIdx >= 0) {
      isect->posInCell->reset();
      isect->treeIdx = -1;
    }
  }

  void CollisionDetector::update(const std::shared_ptr<Intersection>& isect) {
    remove(isect);
    add(isect);
  }

  void CollisionDetector::trans(const std::shared_ptr<Intersection>& isect, float dx, float dy) {
    isect->shape.trans(dx, dy);
    update(isect);
  }

  void CollisionDetector::setWidth(const std::shared_ptr<Intersection>& isect, float width) {
    isect->shape.setWidth(width);
    update(isect);
  }

  void CollisionDetector::run() {
    std::vector<std::vector<std::shared_ptr<Intersection>>> supers;
    supers.resize(matrixDim);
    run(0, supers);
  }

  std::vector<std::weak_ptr<Intersection>> CollisionDetector::getIntersectionsCollideWithIntersection(const std::shared_ptr<Intersection>& isect1) const {
    // 幅優先探索
    std::vector<std::weak_ptr<Intersection>> ret;
    const int startTreeIndex = isect1->getTreeIndex();
    std::deque<int> treeIndices;
    treeIndices.push_back(startTreeIndex);
    const CollisionGroup group1 = isect1->getCollisionGroup();
    while (!treeIndices.empty()) {
      const int treeIdx = treeIndices.front();
      treeIndices.pop_front();
      for (const auto& cell : quadTree.at(treeIdx)) {
        if (auto isect2 = cell.lock()) {
          const CollisionGroup group2 = isect2->getCollisionGroup();
          const CollisionFunction func1 = colMatrix[group1 * matrixDim + group2];
          const CollisionFunction func2 = colMatrix[group2 * matrixDim + group1];
          if (func1 == NULL && func2 == NULL) continue;
          if (isect1->isIntersected(isect2)) ret.push_back(isect2);
        }
      }
      // 下位レベル
      if (treeIdx >= startTreeIndex) {
        if ((treeIdx << 2) + 1 < quadTree.size()) {
          treeIndices.push_back((treeIdx << 2) + 1);
          treeIndices.push_back((treeIdx << 2) + 2);
          treeIndices.push_back((treeIdx << 2) + 3);
          treeIndices.push_back((treeIdx << 2) + 4);
        }
      }
      // 上位レベル
      if (treeIdx <= startTreeIndex) {
        if (treeIdx != 0) {
          treeIndices.push_back(((treeIdx - 1) >> 2));
        }
      }
    }
    return ret;
  }

  std::vector<std::weak_ptr<Intersection>> CollisionDetector::getIntersectionsCollideWithShape(const Shape & shape1) const {
    // 幅優先探索
    std::vector<std::weak_ptr<Intersection>> ret;
    const int startTreeIndex = calcTreeIndexFromBoundingBox(shape1.getBoundingBox());
    std::deque<int> treeIndices;
    treeIndices.push_back(startTreeIndex);
    while (!treeIndices.empty()) {
      const int treeIdx = treeIndices.front();
      treeIndices.pop_front();
      for (const auto& cell : quadTree.at(treeIdx)) {
        if (auto isect2 = cell.lock()) {
          if (shape1.isIntersected(isect2->shape)) ret.push_back(isect2);
        }
      }
      // 下位レベル
      if (treeIdx >= startTreeIndex) {
        if ((treeIdx << 2) + 1 < quadTree.size()) {
          treeIndices.push_back((treeIdx << 2) + 1);
          treeIndices.push_back((treeIdx << 2) + 2);
          treeIndices.push_back((treeIdx << 2) + 3);
          treeIndices.push_back((treeIdx << 2) + 4);
        }
      }
      // 上位レベル
      if (treeIdx <= startTreeIndex) {
        if (treeIdx != 0) {
          treeIndices.push_back(((treeIdx - 1) >> 2));
        }
      }
    }
    return ret;
  }

  void CollisionDetector::run(int idx, std::vector<std::vector<std::shared_ptr<Intersection>>>& supers) {
    // 前のレベルの判定の数をグループごとに覚えておく
    std::vector<int> prevSizes(matrixDim, 0);
    for (int i = 0; i < matrixDim; i++) {
      prevSizes[i] = supers[i].size();
    }
    // 上位レベルに所属する全てのIntersectionと衝突判定を取る（グループごと)
    auto& cell = quadTree[idx];
    auto it = cell.begin();
    while (it != cell.end()) {
      if (auto isect = it->lock()) {
        isect->collideIsects.clear();
        CollisionGroup group1 = isect->getCollisionGroup();
        for (int group2 = 0; group2 < matrixDim; group2++) {
          CollisionFunction func1 = colMatrix[group1 * matrixDim + group2];
          CollisionFunction func2 = colMatrix[group2 * matrixDim + group1];
          // 衝突しないグループは無視
          if (func1 == NULL && func2 == NULL) continue;
          for (auto& super : supers[group2]) {
            if (isect->isIntersected(super)) {
              isect->collideIsects.push_back(super);
              super->collideIsects.push_back(isect);
              // 衝突応答は自分->相手, 相手->自分の2方向分
              // 片方だけ定義してもよい
              if (func1) func1(isect, super);
              if (func2) func2(super, isect);
            }
          }
        }
        // 上位レベルに追加
        supers[group1].push_back(isect);
        ++it;
      } else {
        it = cell.erase(it);
      }
    }
    if ((idx << 2) + 1 < quadTree.size()) {
      run((idx << 2) + 1, supers);
      run((idx << 2) + 2, supers);
      run((idx << 2) + 3, supers);
      run((idx << 2) + 4, supers);
    }
    // このレベルで得たIntersectionを除外
    for (int i = 0; i < matrixDim; i++) {
      supers[i].resize(prevSizes[i]);
    }
  }

  static inline DWORD separateBit(DWORD n) {
    n = (n | (n << 8)) & 0x00ff00ff;
    n = (n | (n << 4)) & 0x0f0f0f0f;
    n = (n | (n << 2)) & 0x33333333;
    return (n | (n << 1)) & 0x55555555;
  }

  static inline DWORD pointToMorton(float x, float y, float unitWidth, float unitHeight) {
    return separateBit(x / unitWidth) | (separateBit(y / unitHeight) << 1);
  }

  int CollisionDetector::calcTreeIndexFromBoundingBox(const BoundingBox & boundingBox) const {
    float left = constrain(boundingBox.left, 0.0f, fieldWidth - 1);
    float right = constrain(boundingBox.right, 0.0f, fieldWidth - 1);
    float top = constrain(boundingBox.top, 0.0f, fieldHeight - 1);
    float bottom = constrain(boundingBox.bottom, 0.0f, fieldHeight - 1);

    DWORD m1 = pointToMorton(left, top, unitCellWidth, unitCellHeight);
    DWORD m2 = pointToMorton(right, bottom, unitCellWidth, unitCellHeight);
    DWORD m = m1 ^ m2;
    int k = 0;
    for (int i = 0; i < maxLevel; m >>= 2, i++) {
      if ((m & 0x3) != 0) { k = i + 1; }
    }
    int level = maxLevel - k;
    DWORD morton = m2 >> (k << 1);
    return calcTreeIndex(level, morton);
  }
}