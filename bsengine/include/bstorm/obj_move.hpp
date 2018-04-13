#pragma once

#include <memory>
#include <list>

namespace bstorm {
  class MoveMode {
  public:
    virtual ~MoveMode();
    virtual void move(float& x, float& y) = 0;
    virtual float getSpeed() const = 0;
    virtual float getAngle() const = 0;
  };

  class MoveModeA : public MoveMode {
  public:
    MoveModeA();
    void move(float& x, float& y) override;
    float getSpeed() const override { return speed; }
    float getAngle() const override { return angle; }
    void setSpeed(float v) { speed = v; }
    void setAngle(float v) { angle = v; }
    float getAcceleration() const { return accel; }
    void setAcceleration(float v) { accel = v; }
    float getMaxSpeed() const { return maxSpeed; }
    void setMaxSpeed(float v) { maxSpeed = v; }
    float getAngularVelocity() const { return angularVelocity; }
    void setAngularVelocity(float v) { angularVelocity = v; }
  private:
    float speed;
    float angle;
    float accel;
    float maxSpeed;
    float angularVelocity;
  };

  class MoveModeB : public MoveMode {
  public:
    MoveModeB(float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY);
    void move(float& x, float& y) override;
    float getAngle() const override;
    float getSpeed() const override;
    float getSpeedX() const { return speedX; }
    void setSpeedX(float v) { speedX = v; }
    float getSpeedY() const { return speedY; }
    void setSpeedY(float v) { speedY = v; }
    float getAccelerationX() const { return accelX; }
    void setAccelerationX(float v) { accelX = v; }
    float getAccelerationY() const { return accelY; }
    void setAccelerationY(float v) { accelY = v; }
    float getMaxSpeedX() const { return maxSpeedX; }
    void setMaxSpeedX(float v) { maxSpeedX = v; }
    float getMaxSpeedY() const { return maxSpeedY; }
    void setMaxSpeedY(float v) { maxSpeedY = v; }
  private:
    float speedX;
    float speedY;
    float accelX;
    float accelY;
    float maxSpeedX;
    float maxSpeedY;
  };

  class MoveModeAtFrame : public MoveMode {
  public:
    MoveModeAtFrame(int frame, float speed, float angle);
    void move(float& x, float& y) override;
    float getAngle() const override { return angle; }
    float getSpeed() const override { return speed; }
  private:
    int frame;
    float speed;
    float angle;
    bool isTimeUp;
    float cosAngle;
    float sinAngle;
  };

  class MoveModeAtWeight : public MoveMode {
  public:
    MoveModeAtWeight(float destX, float destY, float angle, float weight, float maxSpeed);
    void move(float& x, float& y) override;
    float getAngle() const override { return angle; }
    float getSpeed() const override { return speed; }
  private:
    float destX;
    float destY;
    float speed;
    float angle;
    float weight;
    float maxSpeed;
    bool isArrived;
    float cosAngle;
    float sinAngle;
  };

  class ObjRender;
  class ObjMove;
  class MovePattern {
  public:
    MovePattern(int timer);
    virtual ~MovePattern();
    virtual void apply(ObjMove* move, ObjRender* obj) = 0;
    void tickTimerCount() { timer--; }
    int getTiemrCount() const { return timer; }
  protected:
    int timer;
  };

  class ShotData;
  class MovePatternA : public MovePattern {
  public:
    MovePatternA(int frame, float speed, float angle, float accel, float angularVelocity, float maxSpeed, const std::shared_ptr<ObjMove>& baseObj, const std::shared_ptr<ShotData>& shotData);
    void apply(ObjMove* move, ObjRender* obj) override;
  private:
    float speed;
    float angle;
    float accel;
    float angularVelocity;
    float maxSpeed;
    std::weak_ptr<ObjMove> baseObj;
    std::weak_ptr<ShotData> shotData;
    int shotDataId;
  };

  class MovePatternB : public MovePattern {
  public:
    MovePatternB(int frame, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, const std::shared_ptr<ShotData>& shotData);
    void apply(ObjMove* move, ObjRender* obj) override;
  private:
    float speedX;
    float speedY;
    float accelX;
    float accelY;
    float maxSpeedX;
    float maxSpeedY;
    std::weak_ptr<ShotData> shotData;
  };

  class ObjMove {
  public:
    ObjMove(ObjRender *obj);
    float getMoveX() const;
    void setMoveX(float x);
    float getMoveY() const;
    void setMoveY(float y);
    void setMovePosition(float x, float y);
    float getSpeed() const;
    void setSpeed(float speed);
    float getAngle() const;
    void setAngle(float angle);
    void setAcceleration(float accel);
    void setMaxSpeed(float maxSpeed);
    void setAngularVelocity(float angularVelocity);
    void setDestAtSpeed(float x, float y, float speed);
    void setDestAtFrame(float x, float y, int frame);
    void setDestAtWeight(float x, float y, float w, float maxSpeed);
    const std::shared_ptr<MoveMode>& getMoveMode() const;
    void setMoveMode(const std::shared_ptr<MoveMode>& mode);
    void addMovePattern(const std::shared_ptr<MovePattern>& pattern);
  protected:
    void move();
  private:
    std::shared_ptr<MoveMode> mode;
    std::list<std::shared_ptr<MovePattern>> patterns;
    ObjRender *obj;
  };
};