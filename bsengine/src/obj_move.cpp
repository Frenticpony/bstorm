#include <algorithm>

#include <bstorm/dnh_const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_shot.hpp>

namespace bstorm {
  ObjMove::ObjMove(ObjRender *obj) :
    obj(obj),
    mode(std::make_shared<MoveModeA>())
  {
  }

  float ObjMove::getSpeed() const {
    return mode->getSpeed();
  }

  float ObjMove::getAngle() const {
    return mode->getAngle();
  }

  void ObjMove::setSpeed(float speed) {
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA) {
      modeA = std::make_shared<MoveModeA>();
      setMoveMode(modeA);
    }
    modeA->setSpeed(speed);
  }

  void ObjMove::setAngle(float angle) {
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA) {
      modeA = std::make_shared<MoveModeA>();
      setMoveMode(modeA);
    }
    modeA->setAngle(angle);
  }

  void ObjMove::setAcceleration(float accel) {
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA) {
      modeA = std::make_shared<MoveModeA>();
      setMoveMode(modeA);
    }
    modeA->setAcceleration(accel);
  }

  void ObjMove::setMaxSpeed(float maxSpeed) {
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA) {
      modeA = std::make_shared<MoveModeA>();
      setMoveMode(modeA);
    }
    modeA->setMaxSpeed(maxSpeed);
  }

  void ObjMove::setAngularVelocity(float angularVelocity) {
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA) {
      modeA = std::make_shared<MoveModeA>();
      setMoveMode(modeA);
    }
    modeA->setAngularVelocity(angularVelocity);
  }

  void ObjMove::setDestAtSpeed(float x, float y, float speed) {
    float dx = x - getMoveX();
    float dy = y - getMoveY();
    float dist = norm(dx, dy);
    int frame = (int)(ceil(dist / speed));
    float angle = D3DXToDegree(atan2(dy, dx));
    setMoveMode(std::make_shared<MoveModeAtFrame>(frame, speed, angle));
  }

  void ObjMove::setDestAtFrame(float x, float y, int frame) {
    float dx = x - getMoveX();
    float dy = y - getMoveY();
    float dist = norm(dx, dy);
    float speed = dist / frame;
    float angle = D3DXToDegree(atan2(dy, dx));
    setMoveMode(std::make_shared<MoveModeAtFrame>(frame, speed, angle));
  }

  void ObjMove::setDestAtWeight(float x, float y, float weight, float maxSpeed) {
    float dx = x - getMoveX();
    float dy = y - getMoveY();
    setMoveMode(std::make_shared<MoveModeAtWeight>(x, y, D3DXToDegree(atan2(dy, dx)), weight, maxSpeed));
  }

  void ObjMove::setMoveMode(const std::shared_ptr<MoveMode>& mode) {
    if (mode) this->mode = mode;
  }

  void ObjMove::addMovePattern(const std::shared_ptr<MovePattern>& pattern) {
    if (!pattern) return;
    if (pattern->getTiemrCount() < 0) {
      return;
    } else if (pattern->getTiemrCount() == 0) {
      pattern->apply(this, this->obj);
    } else {
      patterns.push_back(pattern);
    }
  }

  void ObjMove::move() {
    auto it = patterns.begin();
    while (it != patterns.end()) {
      auto pat = *it;
      pat->tickTimerCount();
      if (pat->getTiemrCount() <= 0) {
        pat->apply(this, this->obj);
        it = patterns.erase(it);
      } else ++it;
    }
    float x = getMoveX();
    float y = getMoveY();
    mode->move(x, y);
    setMovePosition(x, y);
  }

  const std::shared_ptr<MoveMode>& ObjMove::getMoveMode() const {
    return mode;
  }

  MoveModeA::MoveModeA() :
    speed(0),
    angle(0),
    accel(0),
    maxSpeed(0),
    angularVelocity(0)
  {
  }
  void MoveModeA::move(float & x, float & y) {
    speed += accel;
    if (accel > 0 && speed > maxSpeed || accel < 0 && speed < maxSpeed) {
      speed = maxSpeed;
    }
    angle += angularVelocity;
    float rad = D3DXToRadian(angle);
    float dx = speed * cos(rad);
    float dy = speed * sin(rad);
    x += dx;
    y += dy;
  }

  MoveModeB::MoveModeB(float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY) :
    speedX(speedX),
    speedY(speedY),
    accelX(accelX),
    accelY(accelY),
    maxSpeedX(maxSpeedX),
    maxSpeedY(maxSpeedY)
  {
  }

  void MoveModeB::move(float & x, float & y) {
    speedX += accelX;
    if (accelX > 0 && speedX > maxSpeedX || accelX < 0 && speedX < maxSpeedX) {
      speedX = maxSpeedX;
    }
    speedY += accelY;
    if (accelY > 0 && speedY > maxSpeedY || accelY < 0 && speedY < maxSpeedY) {
      speedY = maxSpeedY;
    }
    x += speedX;
    y += speedY;
  }

  float MoveModeB::getAngle() const {
    return D3DXToDegree(atan2(speedY, speedX));
  }

  float MoveModeB::getSpeed() const {
    return norm(speedX, speedY);
  }

  MoveModeAtFrame::MoveModeAtFrame(int frame, float speed, float angle) :
    frame(frame),
    speed(speed),
    angle(angle),
    isTimeUp(false),
    cosAngle(cos(D3DXToRadian(angle))),
    sinAngle(sin(D3DXToRadian(angle)))
  {
  }

  void MoveModeAtFrame::move(float & x, float & y) {
    if (isTimeUp) return;
    float dx = speed * cosAngle;
    float dy = speed * sinAngle;
    x += dx;
    y += dy;
    frame--;
    if (frame <= 0) {
      speed = 0;
      isTimeUp = true;
    }
  }

  MoveModeAtWeight::MoveModeAtWeight(float destX, float destY, float angle, float weight, float maxSpeed) :
    destX(destX),
    destY(destY),
    speed(maxSpeed),
    angle(angle),
    weight(weight),
    maxSpeed(maxSpeed),
    isArrived(false),
    cosAngle(cos(D3DXToRadian(angle))),
    sinAngle(sin(D3DXToRadian(angle)))
  {
  }

  void MoveModeAtWeight::move(float & x, float & y) {
    if (isArrived) return;
    float distFromDest = norm(x - destX, y - destY);
    if (distFromDest <= 1) {
      speed = 0;
      isArrived = true;
      return;
    }
    speed = std::min(maxSpeed, distFromDest / weight);
    float dx = speed * cosAngle;
    float dy = speed * sinAngle;
    x += dx;
    y += dy;
  }

  MoveMode::~MoveMode() {}

  MovePattern::MovePattern(int timer) :
    timer(timer)
  {
  }

  MovePattern::~MovePattern() {}

  MovePatternA::MovePatternA(int frame, float speed, float angle, float accel, float angularVelocity, float maxSpeed, const std::shared_ptr<ObjMove>& baseObj, const std::shared_ptr<ShotData>& shotData) :
    MovePattern(frame + 1),
    speed(speed),
    angle(angle),
    accel(accel),
    angularVelocity(angularVelocity),
    maxSpeed(maxSpeed),
    baseObj(baseObj),
    shotData(shotData)
  {
  }

  void MovePatternA::apply(ObjMove* move, ObjRender* obj) {
    float prevSpeed = move->getSpeed();
    float prevAngle = move->getAngle();
    auto modeA = std::make_shared<MoveModeA>();
    // speed
    modeA->setSpeed(speed == NO_CHANGE ? prevSpeed : speed);
    // angle
    modeA->setAngle(angle == NO_CHANGE ? prevAngle : angle);
    // acceleration
    modeA->setAcceleration(accel);
    // angularVelocity
    modeA->setAngularVelocity(angularVelocity);
    // maxSpeed
    modeA->setMaxSpeed(maxSpeed);
    // baseObject
    if (auto base = baseObj.lock()) {
      float baseX = base->getMoveX();
      float baseY = base->getMoveY();
      modeA->setAngle(modeA->getAngle() + D3DXToDegree(atan2(baseY - move->getMoveY(), baseX - move->getMoveX())));
    }
    // shotData
    if (auto shot = dynamic_cast<ObjShot*>(obj)) {
      if (auto data = shotData.lock()) {
        shot->setShotData(data);
      }
    }
    move->setMoveMode(modeA);
  }

  MovePatternB::MovePatternB(int frame, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, const std::shared_ptr<ShotData>& shotData) :
    MovePattern(frame + 1),
    speedX(speedX),
    speedY(speedY),
    accelX(accelX),
    accelY(accelY),
    maxSpeedX(maxSpeedX),
    maxSpeedY(maxSpeedY),
    shotData(shotData)
  {
  }

  void MovePatternB::apply(ObjMove * move, ObjRender * obj) {
    float prevSpeed = move->getSpeed();
    float prevAngle = move->getAngle();
    auto modeB = std::make_shared<MoveModeB>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    // speedX
    modeB->setSpeedX(speedX == NO_CHANGE ? (prevSpeed * cos(D3DXToRadian(prevAngle))) : speedX);
    // speedY
    modeB->setSpeedY(speedY == NO_CHANGE ? (prevSpeed * sin(D3DXToRadian(prevAngle))) : speedY);
    // accelerationX
    modeB->setAccelerationX(accelX);
    // accelerationY
    modeB->setAccelerationY(accelY);
    // maxSpeedX
    modeB->setMaxSpeedX(maxSpeedX);
    // maxSpeedY
    modeB->setMaxSpeedY(maxSpeedY);
    // shotData
    if (auto shot = dynamic_cast<ObjShot*>(obj)) {
      if (auto data = shotData.lock()) {
        shot->setShotData(data);
      }
    }
    move->setMoveMode(modeB);
  }
}