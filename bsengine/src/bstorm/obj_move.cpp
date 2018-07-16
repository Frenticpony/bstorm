#include <bstorm/obj_move.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/obj_shot.hpp>

#include <algorithm>
#include <d3dx9.h>

namespace bstorm
{
ObjMove::ObjMove(ObjRender *obj) :
    obj_(obj),
    mode(std::make_shared<MoveModeA>())
{
}

float ObjMove::GetMoveX() const { return obj_->GetX(); }

void ObjMove::SetMoveX(float x) { obj_->SetX(x); }

float ObjMove::GetMoveY() const { return obj_->GetY(); }

void ObjMove::SetMoveY(float y) { obj_->SetY(y); }

void ObjMove::SetMovePosition(float x, float y) { obj_->SetPosition(x, y, obj_->GetZ()); }

float ObjMove::GetSpeed() const
{
    return mode->GetSpeed();
}

float ObjMove::GetAngle() const
{
    return mode->GetAngle();
}

void ObjMove::SetSpeed(float speed)
{
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA)
    {
        modeA = std::make_shared<MoveModeA>();
        SetMoveMode(modeA);
    }
    modeA->SetSpeed(speed);
}

void ObjMove::SetAngle(float angle)
{
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA)
    {
        modeA = std::make_shared<MoveModeA>();
        SetMoveMode(modeA);
    }
    modeA->SetAngle(angle);
}

void ObjMove::SetAcceleration(float accel)
{
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA)
    {
        modeA = std::make_shared<MoveModeA>();
        SetMoveMode(modeA);
    }
    modeA->SetAcceleration(accel);
}

void ObjMove::SetMaxSpeed(float maxSpeed)
{
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA)
    {
        modeA = std::make_shared<MoveModeA>();
        SetMoveMode(modeA);
    }
    modeA->SetMaxSpeed(maxSpeed);
}

void ObjMove::SetAngularVelocity(float angularVelocity)
{
    auto modeA = std::dynamic_pointer_cast<MoveModeA>(mode);
    if (!modeA)
    {
        modeA = std::make_shared<MoveModeA>();
        SetMoveMode(modeA);
    }
    modeA->SetAngularVelocity(angularVelocity);
}

void ObjMove::SetDestAtSpeed(float x, float y, float speed)
{
    float dx = x - GetMoveX();
    float dy = y - GetMoveY();
    float dist = std::hypotf(dx, dy);
    int frame = (int)(ceil(dist / speed));
    float angle = D3DXToDegree(atan2(dy, dx));
    SetMoveMode(std::make_shared<MoveModeAtFrame>(frame, speed, angle));
}

void ObjMove::SetDestAtFrame(float x, float y, int frame)
{
    float dx = x - GetMoveX();
    float dy = y - GetMoveY();
    float dist = std::hypotf(dx, dy);
    float speed = dist / frame;
    float angle = D3DXToDegree(atan2(dy, dx));
    SetMoveMode(std::make_shared<MoveModeAtFrame>(frame, speed, angle));
}

void ObjMove::SetDestAtWeight(float x, float y, float weight, float maxSpeed)
{
    float dx = x - GetMoveX();
    float dy = y - GetMoveY();
    SetMoveMode(std::make_shared<MoveModeAtWeight>(x, y, D3DXToDegree(atan2(dy, dx)), weight, maxSpeed));
}

void ObjMove::SetMoveMode(const std::shared_ptr<MoveMode>& mode)
{
    if (mode) this->mode = mode;
}

void ObjMove::AddMovePattern(const std::shared_ptr<MovePattern>& pattern)
{
    if (!pattern) return;
    if (pattern->GetTimerCount() < 0)
    {
        return;
    } else if (pattern->GetTimerCount() == 0)
    {
        pattern->Apply(this, this->obj_);
    } else
    {
        patterns_.push_back(pattern);
    }
}

void ObjMove::Move()
{
    auto it = patterns_.begin();
    while (it != patterns_.end())
    {
        auto pat = *it;
        pat->TickTimerCount();
        if (pat->GetTimerCount() <= 0)
        {
            pat->Apply(this, this->obj_);
            it = patterns_.erase(it);
        } else ++it;
    }
    float x = GetMoveX();
    float y = GetMoveY();
    mode->Move(x, y);
    SetMovePosition(x, y);
}

const std::shared_ptr<MoveMode>& ObjMove::GetMoveMode() const
{
    return mode;
}

MoveModeA::MoveModeA() :
    speed_(0),
    angle_(0),
    accel_(0),
    maxSpeed_(0),
    angularVelocity_(0)
{
}
void MoveModeA::Move(float & x, float & y)
{
    speed_ += accel_;
    if (accel_ > 0 && speed_ > maxSpeed_ || accel_ < 0 && speed_ < maxSpeed_)
    {
        speed_ = maxSpeed_;
    }
    angle_ += angularVelocity_;
    float rad = D3DXToRadian(angle_);
    float dx = speed_ * cos(rad);
    float dy = speed_ * sin(rad);
    x += dx;
    y += dy;
}

MoveModeB::MoveModeB(float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY) :
    speedX_(speedX),
    speedY_(speedY),
    accelX_(accelX),
    accelY_(accelY),
    maxSpeedX_(maxSpeedX),
    maxSpeedY_(maxSpeedY)
{
}

void MoveModeB::Move(float & x, float & y)
{
    speedX_ += accelX_;
    if (accelX_ > 0 && speedX_ > maxSpeedX_ || accelX_ < 0 && speedX_ < maxSpeedX_)
    {
        speedX_ = maxSpeedX_;
    }
    speedY_ += accelY_;
    if (accelY_ > 0 && speedY_ > maxSpeedY_ || accelY_ < 0 && speedY_ < maxSpeedY_)
    {
        speedY_ = maxSpeedY_;
    }
    x += speedX_;
    y += speedY_;
}

float MoveModeB::GetAngle() const
{
    return D3DXToDegree(atan2(speedY_, speedX_));
}

float MoveModeB::GetSpeed() const
{
    return std::hypotf(speedX_, speedY_);
}

MoveModeAtFrame::MoveModeAtFrame(int frame, float speed, float angle) :
    frame_(frame),
    speed_(speed),
    angle_(angle),
    isTimeUp_(false),
    cosAngle_(cos(D3DXToRadian(angle))),
    sinAngle_(sin(D3DXToRadian(angle)))
{
}

void MoveModeAtFrame::Move(float & x, float & y)
{
    if (isTimeUp_) return;
    float dx = speed_ * cosAngle_;
    float dy = speed_ * sinAngle_;
    x += dx;
    y += dy;
    frame_--;
    if (frame_ <= 0)
    {
        speed_ = 0;
        isTimeUp_ = true;
    }
}

MoveModeAtWeight::MoveModeAtWeight(float destX, float destY, float angle, float weight, float maxSpeed) :
    destX_(destX),
    destY_(destY),
    speed_(maxSpeed),
    angle_(angle),
    weight_(weight),
    maxSpeed_(maxSpeed),
    isArrived_(false),
    cosAngle_(cos(D3DXToRadian(angle))),
    sinAngle_(sin(D3DXToRadian(angle)))
{
}

void MoveModeAtWeight::Move(float & x, float & y)
{
    if (isArrived_) return;
    float distFromDest = std::hypotf(x - destX_, y - destY_);
    if (distFromDest <= 1)
    {
        speed_ = 0;
        isArrived_ = true;
        return;
    }
    speed_ = std::min(maxSpeed_, distFromDest / weight_);
    float dx = speed_ * cosAngle_;
    float dy = speed_ * sinAngle_;
    x += dx;
    y += dy;
}

MoveMode::~MoveMode() {}

MovePattern::MovePattern(int timer) :
    timer_(timer)
{
}

MovePattern::~MovePattern() {}

MovePatternA::MovePatternA(int frame, float speed, float angle, float accel, float angularVelocity, float maxSpeed, const std::shared_ptr<ObjMove>& baseObj, const std::shared_ptr<ShotData>& shotData) :
    MovePattern(frame + 1),
    speed_(speed),
    angle_(angle),
    accel_(accel),
    angularVelocity_(angularVelocity),
    maxSpeed_(maxSpeed),
    baseObj_(baseObj),
    shotData_(shotData)
{
}

void MovePatternA::Apply(ObjMove* move, ObjRender* obj)
{
    float prevSpeed = move->GetSpeed();
    float prevAngle = move->GetAngle();
    auto modeA = std::make_shared<MoveModeA>();
    // speed_
    modeA->SetSpeed(speed_ == NO_CHANGE ? prevSpeed : speed_);
    // angle_
    modeA->SetAngle(angle_ == NO_CHANGE ? prevAngle : angle_);
    // acceleration
    modeA->SetAcceleration(accel_);
    // angularVelocity
    modeA->SetAngularVelocity(angularVelocity_);
    // maxSpeed_
    modeA->SetMaxSpeed(maxSpeed_);
    // baseObject
    if (auto base = baseObj_.lock())
    {
        float baseX = base->GetMoveX();
        float baseY = base->GetMoveY();
        modeA->SetAngle(modeA->GetAngle() + D3DXToDegree(atan2(baseY - move->GetMoveY(), baseX - move->GetMoveX())));
    }
    // shotData
    if (auto shot = dynamic_cast<ObjShot*>(obj))
    {
        if (auto data = shotData_.lock())
        {
            shot->SetShotData(data);
        }
    }
    move->SetMoveMode(modeA);
}

MovePatternB::MovePatternB(int frame, float speedX, float speedY, float accelX, float accelY, float maxSpeedX, float maxSpeedY, const std::shared_ptr<ShotData>& shotData) :
    MovePattern(frame + 1),
    speedX_(speedX),
    speedY_(speedY),
    accelX_(accelX),
    accelY_(accelY),
    maxSpeedX_(maxSpeedX),
    maxSpeedY_(maxSpeedY),
    shotData_(shotData)
{
}

void MovePatternB::Apply(ObjMove * move, ObjRender * obj)
{
    float prevSpeed = move->GetSpeed();
    float prevAngle = move->GetAngle();
    auto modeB = std::make_shared<MoveModeB>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    // speedX
    modeB->SetSpeedX(speedX_ == NO_CHANGE ? (prevSpeed * cos(D3DXToRadian(prevAngle))) : speedX_);
    // speedY
    modeB->SetSpeedY(speedY_ == NO_CHANGE ? (prevSpeed * sin(D3DXToRadian(prevAngle))) : speedY_);
    // accelerationX
    modeB->SetAccelerationX(accelX_);
    // accelerationY
    modeB->SetAccelerationY(accelY_);
    // maxSpeedX
    modeB->SetMaxSpeedX(maxSpeedX_);
    // maxSpeedY
    modeB->SetMaxSpeedY(maxSpeedY_);
    // shotData
    if (auto shot = dynamic_cast<ObjShot*>(obj))
    {
        if (auto data = shotData_.lock())
        {
            shot->SetShotData(data);
        }
    }
    move->SetMoveMode(modeB);
}
}