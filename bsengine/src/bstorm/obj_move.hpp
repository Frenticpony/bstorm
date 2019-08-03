#pragma once

#include <memory>
#include <list>

namespace bstorm
{
class MoveMode
{
public:
    virtual ~MoveMode();
    virtual void Move(float& x, float& y) = 0;
    virtual float GetSpeed() const = 0;
    virtual float GetAngle() const = 0;
};

class MoveModeA : public MoveMode
{
public:
    MoveModeA();
    void Move(float& x, float& y) override;
    float GetSpeed() const override { return speed_; }
    float GetAngle() const override { return angle_; }
    void SetSpeed(float v) { speed_ = v; }
    void SetAngle(float v) { angle_ = v; }
    float GetAcceleration() const { return accel_; }
    void SetAcceleration(float v) { accel_ = v; }
    float GetMaxSpeed() const { return maxSpeed_; }
    void SetMaxSpeed(float v) { maxSpeed_ = v; }
    float GetAngularVelocity() const { return angularVelocity_; }
    void SetAngularVelocity(float v) { angularVelocity_ = v; }
private:
    float speed_;
    float angle_;
    float accel_;
    float maxSpeed_;
    float angularVelocity_;
};

class MoveModeB : public MoveMode
{
public:
    MoveModeB(float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY);
    void Move(float& x, float& y) override;
    float GetAngle() const override;
    float GetSpeed() const override;
    float GetSpeedX() const { return speedX_; }
    void SetSpeedX(float v) { speedX_ = v; }
    float GetSpeedY() const { return speedY_; }
    void SetSpeedY(float v) { speedY_ = v; }
    float GetAccelerationX() const { return accelX_; }
    void SetAccelerationX(float v) { accelX_ = v; }
    float GetAccelerationY() const { return accelY_; }
    void SetAccelerationY(float v) { accelY_ = v; }
    float GetMaxSpeedX() const { return maxSpeedX_; }
    void SetMaxSpeedX(float v) { maxSpeedX_ = v; }
    float GetMaxSpeedY() const { return maxSpeedY_; }
    void SetMaxSpeedY(float v) { maxSpeedY_ = v; }
private:
    float speedX_;
    float speedY_;
    float accelX_;
    float accelY_;
    float maxSpeedX_;
    float maxSpeedY_;
};

class MoveModeAtFrame : public MoveMode
{
public:
    MoveModeAtFrame(int frame, float speed, float angle);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return angle_; }
    float GetSpeed() const override { return speed_; }
private:
    int frame_;
    float speed_;
    const float angle_;
    bool isTimeUp_;
    const float cosAngle_;
    const float sinAngle_;
};

class MoveModeAtWeight : public MoveMode
{
public:
    MoveModeAtWeight(float destX, float destY, float angle, float weight, float maxSpeed);
    void Move(float& x, float& y) override;
    float GetAngle() const override { return angle_; }
    float GetSpeed() const override { return speed_; }
private:
    const float destX_;
    const float destY_;
    float speed_;
    const float angle_;
    const float weight_;
    const float maxSpeed_;
    bool isArrived_;
    const float cosAngle_;
    const float sinAngle_;
};

class MoveModeECL : public MoveMode
{
public:
	MoveModeECL();
	void Move(float& x, float& y) override;
	float GetSpeed() const override { return speed_; }
	float GetAngle() const override { return angle_; }
	float GetAcceleration() const { return accel_; }
	float GetMinSpeed() const { return minSpeed_; }
	float GetMaxSpeed() const { return maxSpeed_; }
	float GetAngularVelocity() const { return angularVelocity_; }
	float GetGravityAngle() const { return gravAngle_; }
	void SetSpeed(float v) { speed_ = v; }
	void SetAngle(float v) { angle_ = v; }
	void SetAcceleration(float v) { accel_ = v; }
	void SetMinSpeed(float v) { minSpeed_ = v; }
	void SetMaxSpeed(float v) { maxSpeed_ = v; }
	void SetAngularVelocity(float v) { angularVelocity_ = v; }
	void SetGravityAngle(float v) { gravAngle_ = v; }

private:
	float speed_;
	float angle_;
	float accel_;
	float minSpeed_;
	float maxSpeed_;
	float angularVelocity_;
	float gravAngle_;

	float speedX_;
	float speedY_;
	float accelX_;
	float accelY_;
};

class ObjRender;
class ObjMove;
class MovePattern
{
public:
    MovePattern(int timer);
    virtual ~MovePattern();
    virtual void Apply(ObjMove* move, ObjRender* obj) = 0;
    void TickTimerCount() { timer_--; }
    int GetTimerCount() const { return timer_; }
private:
    int timer_;
};

class ShotData;
class MovePatternA : public MovePattern
{
public:
    MovePatternA(int frame, float speed, float angle, float accel, float angularVelocity, float maxSpeed, const std::shared_ptr<ObjMove>& baseObj, const std::shared_ptr<ShotData>& shotData);
    void Apply(ObjMove* move, ObjRender* obj) override;
private:
    float speed_;
    float angle_;
    float accel_;
    float angularVelocity_;
    float maxSpeed_;
    std::weak_ptr<ObjMove> baseObj_;
    std::weak_ptr<ShotData> shotData_;
    int shotDataId_;
};

class MovePatternB : public MovePattern
{
public:
    MovePatternB(int frame, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, const std::shared_ptr<ShotData>& shotData);
    void Apply(ObjMove* move, ObjRender* obj) override;
private:
    float speedX_;
    float speedY_;
    float accelX_;
    float accelY_;
    float maxSpeedX_;
    float maxSpeedY_;
    std::weak_ptr<ShotData> shotData_;
};

class ObjMove
{
public:
    ObjMove(ObjRender *obj);
    float GetMoveX() const;
    void SetMoveX(float x);
    float GetMoveY() const;
    void SetMoveY(float y);
    void SetMovePosition(float x, float y);
    float GetSpeed() const;
    void SetSpeed(float speed);
    float GetAngle() const;
    void SetAngle(float angle);
    void SetAcceleration(float accel);
    void SetMaxSpeed(float maxSpeed);
    void SetAngularVelocity(float angularVelocity);
    void SetDestAtSpeed(float x, float y, float speed);
    void SetDestAtFrame(float x, float y, int frame);
    void SetDestAtWeight(float x, float y, float w, float maxSpeed);
    const std::shared_ptr<MoveMode>& GetMoveMode() const;
    void SetMoveMode(const std::shared_ptr<MoveMode>& mode);
    void AddMovePattern(const std::shared_ptr<MovePattern>& pattern);
protected:
    void Move();
    void MoveFade();
private:
    std::shared_ptr<MoveMode> mode;
    std::list<std::shared_ptr<MovePattern>> patterns_;
    ObjRender *obj_;
};
};