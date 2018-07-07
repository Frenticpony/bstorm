#pragma once

#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>
#include <bstorm/point2D.hpp>
#include <bstorm/stage_common_player_params.hpp>

namespace bstorm
{
class Intersection;
class PlayerIntersectionToItem;
class ObjItem;
class ObjShot;

class ObjPlayer : public ObjSprite2D, public ObjMove, public ObjCol, public std::enable_shared_from_this<ObjPlayer>
{
public:
    ObjPlayer(const std::shared_ptr<CollisionDetector>& colDetector, const std::shared_ptr<Package>& package);
    ~ObjPlayer();
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    void AddIntersectionCircleA1(float dx, float dy, float r, float dr);
    void AddIntersectionCircleA2(float dx, float dy, float r);
    void AddIntersectionToItem();
    void ClearIntersection();
    void SetNormalSpeed(double speed);
    void SetSlowSpeed(double speed);
    void SetClip(float left, float top, float right, float bottom);
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
    int GetInvincibilityFrame() const { return invincibilityFrame_; }
    int GetDownStateFrame() const { return downStateFrame_; }
    int GetRebirthFrame() const { return rebirthFrame_; }
    bool IsPermitPlayerShot() const { return permitPlayerShot_; }
    bool IsPermitPlayerSpell() const;
    bool IsLastSpellWait() const;
    bool IsSpellActive() const;

    void SetLife(PlayerLife life);
    void SetSpell(PlayerSpell spell);
    void SetPower(PlayerPower power);
    void SetScore(PlayerScore score);
    void SetGraze(PlayerGraze graze);
    void SetPoint(PlayerPoint point);
    PlayerScore GetScore() const;
    PlayerGraze GetGraze() const;
    PlayerPoint GetPoint() const;
    PlayerLife GetLife() const;
    PlayerSpell GetSpell() const;
    PlayerPower GetPower() const;

    void GrazeToShot(int shotObjId, PlayerGraze grazeCnt);
    float GetAutoItemCollectLineY() const { return autoItemCollectLineY_; }
    void SetAutoItemCollectLineY(float y) { autoItemCollectLineY_ = y; }
    void Hit(int collisionObjId);
    void CallSpell();
    void ObtainItem(int itemObjId);
    bool IsGrazeEnabled() const;
protected:
    void OnTrans(float dx, float dy) override;
private:
    // develop only
    bool IsForceInvincible() const;
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
    float autoItemCollectLineY_;
    int hitStateTimer_;
    int downStateTimer_;
    PlayerGraze currentFrameGrazeCnt_;
    std::vector<double> currentFrameGrazeObjIds_;
    std::vector<Point2D> currentFrameGrazeShotPoints_;
};
}