#pragma once

#include <bstorm/obj_prim.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

namespace bstorm {
  class Intersection;
  class PlayerIntersectionToItem;
  class ObjItem;
  class ObjShot;

  class GlobalPlayerParams {
  public:
    GlobalPlayerParams();
    double life;
    double spell;
    double power;
    int64_t score;
    int64_t graze;
    int64_t point;
  };

  class ObjPlayer : public ObjSprite2D, public ObjMove, public ObjCol {
  public:
    ObjPlayer(const std::shared_ptr<GameState>& gameState, const std::shared_ptr<GlobalPlayerParams>& globalParams);
    ~ObjPlayer();
    void update() override;
    void render() override;
    void addIntersectionCircleA1(float dx, float dy, float r, float dr);
    void addIntersectionCircleA2(float dx, float dy, float r);
    void setNormalSpeed(double speed);
    void setSlowSpeed(double speed);
    void setClip(float left, float top, float right, float bottom);
    void setLife(double life);
    void setSpell(double spell);
    void setPower(double power);
    void setInvincibilityFrame(int frame) { invincibilityFrame = frame; }
    void setDownStateFrame(int frame);
    void setRebirthFrame(int frame);
    void setRebirthLossFrame(int frame);
    void setForbidPlayerShot(bool forbid) { permitPlayerShot = !forbid; }
    void setForbidPlayerSpell(bool forbid) { permitPlayerSpell = !forbid; }
    int getState() const { return state; }
    float getNormalSpeed() const { return normalSpeed; }
    float getSlowSpeed() const { return slowSpeed; }
    float getClipLeft() const { return clipLeft; }
    float getClipTop() const { return clipTop; }
    float getClipRight() const { return clipRight; }
    float getClipBottom() const { return clipBottom; }
    double getLife() const;
    double getSpell() const;
    double getPower() const;
    int getInvincibilityFrame() const { return invincibilityFrame; }
    int getDownStateFrame() const { return downStateFrame; }
    int getRebirthFrame() const { return rebirthFrame; }
    bool isPermitPlayerShot() const { return permitPlayerShot; }
    bool isPermitPlayerSpell() const;
    bool isLastSpellWait() const;
    bool isSpellActive() const;
    int64_t getScore() const;
    int64_t getGraze() const;
    int64_t getPoint() const;
    void addScore(int64_t score);
    void addGraze(int64_t graze);
    void addGraze(int shotObjId, int64_t graze);
    void addPoint(int64_t point);
    float getAutoItemCollectLineY() const { return autoItemCollectLineY; }
    void setAutoItemCollectLineY(float y) { autoItemCollectLineY = y; }
    void hit(int collisionObjId);
    void callSpell();
    void getItem(int itemObjId);
    bool isGrazeEnabled() const;
  protected:
    void transIntersection(float dx, float dy);
    bool isInvincible() const;
    void shootDown();
    void rebirth();
    void moveByKeyInput();
    void applyClip();
    void initPosition();
    int invincibilityFrame;
    int downStateFrame;
    int rebirthFrame;
    int rebirthLossFrame;
    bool permitPlayerShot;
    bool permitPlayerSpell;
    int state;
    float normalSpeed;
    float slowSpeed;
    float clipLeft;
    float clipTop;
    float clipRight;
    float clipBottom;
    std::shared_ptr<GlobalPlayerParams> globalParams;
    float autoItemCollectLineY;
    int hitStateTimer;
    int downStateTimer;
    int64_t currentFrameGrazeCnt;
    std::vector<double> currentFrameGrazeObjIds;
    std::vector<Point2D> currentFrameGrazeShotPoints;
    std::shared_ptr<PlayerIntersectionToItem> isectToItem;
  };
}