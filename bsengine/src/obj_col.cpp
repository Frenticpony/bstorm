#include <iterator>

#include <bstorm/intersection.hpp>
#include <bstorm/collision_matrix.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm {
  ObjCol::ObjCol(const std::shared_ptr<GameState>& gameState) : gameState(gameState) { }
  ObjCol::~ObjCol() { }

  void ObjCol::pushIntersection(const std::shared_ptr<Intersection>& isect) {
    isects.push_back(isect);
  }

  void ObjCol::shiftIntersection() {
    isects.pop_front();
  }

  void ObjCol::addTempIntersection(const std::shared_ptr<Intersection>& isect) {
    tempIsects.push_back(isect);
  }

  void ObjCol::transIntersection(float dx, float dy) {
    if (auto state = gameState.lock()) {
      for (auto& isect : isects) {
        state->colDetector->trans(isect, dx, dy);
      }
    }
  }

  void ObjCol::setWidthIntersection(float width) {
    if (auto state = gameState.lock()) {
      for (auto& isect : isects) {
        state->colDetector->setWidth(isect, width);
      }
    }
  }

  void ObjCol::renderIntersection(bool isPermitCamera) const {
    if (auto state = gameState.lock()) {
      if (state->renderIntersectionEnable) {
        for (auto& isect : getIntersections()) {
          isect->render(state->renderer, isPermitCamera);
        }
        for (auto& isect : getTempIntersections()) {
          isect->render(state->renderer, isPermitCamera);
        }
      }
    }
  }

  void ObjCol::clearIntersection() {
    isects.clear();
  }

  void ObjCol::clearOldTempIntersection() {
    oldTempIsects.clear();
    std::swap(tempIsects, oldTempIsects);
  }

  bool ObjCol::isIntersected(const std::shared_ptr<ObjCol>& col) const {
    for (const auto& isect1 : getIntersections()) {
      for (const auto& isect2 : col->getIntersections()) {
        if (isect1->isIntersected(isect2)) return true;
      }

      for (const auto& isect2 : col->getTempIntersections()) {
        if (isect1->isIntersected(isect2)) return true;
      }
    }
    for (const auto& isect1 : getTempIntersections()) {
      for (const auto& isect2 : col->getIntersections()) {
        if (isect1->isIntersected(isect2)) return true;
      }

      for (const auto& isect2 : col->getTempIntersections()) {
        if (isect1->isIntersected(isect2)) return true;
      }
    }
    return false;
  }

  bool ObjCol::isIntersected() const {
    return getIntersectedCount() != 0;
  }

  int ObjCol::getIntersectedCount() const {
    int cnt = 0;
    for (const auto& isect : isects) {
      cnt += isect->getCollideIntersections().size();
    }
    for (const auto& isect : oldTempIsects) {
      cnt += isect->getCollideIntersections().size();
    }
    return cnt;
  }

  std::vector<std::weak_ptr<Intersection>> ObjCol::getCollideIntersections() const {
    std::vector<std::weak_ptr<Intersection>> ret;
    for (const auto& isect : getIntersections()) {
      const auto& tmp = isect->getCollideIntersections();
      std::copy(tmp.begin(), tmp.end(), std::back_inserter(ret));
    }
    for (const auto& isect : getTempIntersections()) {
      const auto& tmp = isect->getCollideIntersections();
      std::copy(tmp.begin(), tmp.end(), std::back_inserter(ret));
    }
    return ret;
  }
}