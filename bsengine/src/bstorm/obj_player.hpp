#pragma once

#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm
{
class Intersection;
class PlayerIntersectionToItem;
class ObjItem;
class ObjShot;

class GlobalPlayerParams
{
public:
    GlobalPlayerParams();
    double life;
    double spell;
    double power;
    int64_t score;
    int64_t graze;
    int64_t point;
};

class ObjPlayer : public ObjSprite2D, public ObjMove, public ObjCol, public std::enable_shared_from_this<ObjPlayer>
{
public:
    ObjPlayer(const std::shared_ptr<GameState>& gameState, const std::shared_ptr<GlobalPlayerParams>& globalParams);
    ~ObjPlayer();
    void Update() override;
    void Render() override;
    void AddIntersectionCircleA1(float dx, float dy, float r, float dr);
    void AddIntersectionCircleA2(float dx, float dy, float r);
    void AddIntersectionToItem();
    void SetNormalSpeed(double speed);
    void SetSlowSpeed(double speed);
    void SetClip(float left, float top, float right, float bottom);
    void SetLife(double life);
    void SetSpell(double spell);
    void SetPower(double power);
    void SetInvincibilityFrame(int frame) { invincibilityFrame_ = frame; }
    void SetDownStateFrame(int frame);
    void SetRebirthFrame(int frame);
    void SetRebirthLossFrame(int frame);
    void SetForbidPlayerShot(bool forbid) { permitPlayerShot_ = !forbid; }
    void SetForbidPlayerSpell(bool forbid) { permitPlayerSpell_ = !forbid; }
    int GetState() const { return state_; }
    float GetNormalSpeed() const { return normalSpeed_; }
    float GetSlowSpeed() const { return slowSpeed_; }
    float GetClipLeft() const { return clipLeft_; }
    float GetClipTop() const { return clipTop_; }
    float GetClipRight() const { return clipRight_; }
    float GetClipBottom() const { return clipBottom_; }
    double GetLife() const;
    double GetSpell() const;
    double GetPower() const;
    int GetInvincibilityFrame() const { return invincibilityFrame_; }
    int GetDownStateFrame() const { return downStateFrame_; }
    int GetRebirthFrame() const { return rebirthFrame_; }
    bool IsPermitPlayerShot() const { return permitPlayerShot_; }
    bool IsPermitPlayerSpell() const;
    bool IsLastSpellWait() const;
    bool IsSpellActive() const;
    int64_t GetScore() const;
    int64_t GetGraze() const;
    int64_t GetPoint() const;
    void AddScore(int64_t score);
    void AddGraze(int64_t graze);
    void AddGraze(int shotObjId, int64_t graze);
    void AddPoint(int64_t point);
    float GetAutoItemCollectLineY() const { return autoItemCollectLineY_; }
    void SetAutoItemCollectLineY(float y) { autoItemCollectLineY_ = y; }
    void Hit(int collisionObjId);
    void CallSpell();
    void ObtainItem(int itemObjId);
    bool IsGrazeEnabled() const;
protected:
    void TransIntersection(float dx, float dy);
private:
    bool IsInvincible() const;
    void ShootDown();
    void Rebirth();
    void MoveByKeyInput();
    void ApplyClip();
    void InitPosition();
    int invincibilityFrame_;
    int downStateFrame_;
    int rebirthFrame_;
    int rebirthLossFrame_;
    bool permitPlayerShot_;
    bool permitPlayerSpell_;
    int state_;
    float normalSpeed_;
    float slowSpeed_;
    float clipLeft_;
    float clipTop_;
    float clipRight_;
    float clipBottom_;
    std::shared_ptr<GlobalPlayerParams> globalParams_;
    float autoItemCollectLineY_;
    int hitStateTimer_;
    int downStateTimer_;
    int64_t currentFrameGrazeCnt_;
    std::vector<double> currentFrameGrazeObjIds_;
    std::vector<Point2D> currentFrameGrazeShotPoints_;
    std::shared_ptr<PlayerIntersectionToItem> isectToItem_;
};
}