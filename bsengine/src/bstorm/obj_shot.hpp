#pragma once

#include <bstorm/type.hpp>
#include <bstorm/obj_render.hpp>
#include <bstorm/obj_move.hpp>
#include <bstorm/obj_col.hpp>

#include <list>
#include <deque>

namespace bstorm
{
class ShotData;
class ShotIntersection;

class ShotCounter
{
public:
    ShotCounter();
    int playerShotCount;
    int enemyShotCount;
};

class ObjShot : public ObjRender, public ObjMove, public ObjCol, public std::enable_shared_from_this<ObjShot>
{
public:
    ObjShot(bool isPlayerShot, const std::shared_ptr<GameState>& gameState);
    ~ObjShot();

    void update() override;
    void render() override;

    bool isRegistered() const;
    void regist();
    bool isPlayerShot() const;

    // intersection
    void addIntersectionCircleA1(float r);
    void addIntersectionCircleA2(float x, float y, float r);
    void addIntersectionLine(float x1, float y1, float x2, float y2, float width);
    void addTempIntersectionCircleA1(float r);
    void addTempIntersectionCircleA2(float x, float y, float r);
    void addTempIntersectionLine(float x1, float y1, float x2, float y2, float width);
    bool isIntersectionEnabled() const;
    void setIntersectionEnable(bool enable);
    bool isTempIntersectionMode() const;

    // shot data
    const std::shared_ptr<ShotData>& getShotData() const;
    virtual void setShotData(const std::shared_ptr<ShotData>& shotData);
    int getAnimationFrameCount() const;
    int getAnimationIndex() const;

    double getDamage() const;
    void setDamage(double damage);
    int getPenetration() const;
    void setPenetration(int penetration);
    int getDelay() const;
    void setDelay(int delay);
    bool isDelay() const;
    int getSourceBlendType() const;
    void setSourceBlendType(int blendType);
    float getAngularVelocity() const;
    void setAngularVelocity(float angularVelocity);
    bool isSpellResistEnabled() const;
    void setSpellResistEnable(bool enable);
    bool isSpellFactorEnabled() const;
    void setSpellFactor(bool enable);
    bool isEraseShotEnabled() const;
    void setEraseShotEnable(bool enable);
    bool isItemChangeEnabled() const;
    void setItemChange(bool enable);
    bool isAutoDeleteEnabled() const;
    void setAutoDeleteEnable(bool enable);

    // delete
    void toItem();
    void eraseWithSpell();
    void deleteImmediate();
    void fadeDelete();
    float getFadeScale() const;
    bool isFadeDeleteStarted() const;
    bool isFrameDeleteStarted() const;
    int getDeleteFrameTimer() const;
    int getFadeDeleteFrameTimer() const;
    void setDeleteFrame(int frame);

    // graze
    virtual void graze();
    virtual bool isGrazeEnabled() const;

    // spawn
    class AddedShot
    {
    public:
        AddedShot(int objId, int frame);
        AddedShot(int objId, int frame, float dist, float angle);
        enum class Type
        {
            A1,
            A2
        };
        const Type type;
        const int objId;
        int frame;
        const float dist;
        const float angle;
    };
    void addShotA1(int shotObjId, int frame);
    void addShotA2(int shotObjId, int frame, float dist, float angle);
    int getFrameCountForAddShot() const;
    const std::list<AddedShot>& getAddedShot() const;
    virtual void generateDefaultBonusItem();
protected:
    void transIntersection(float dx, float dy) override;
    void renderIntersection();
    void checkAutoDelete(float x, float y);
    void updateAnimationPosition();
    void tickDelayTimer();
    void tickDeleteFrameTimer();
    void tickAddedShotFrameCount();
    void tickFadeDeleteTimer();
    std::shared_ptr<ShotData> shotData;
    bool grazeInvalidFlag;
    bool useTempIntersectionFlag;
private:
    void addIntersection(const std::shared_ptr<ShotIntersection>& isect);
    void addTempIntersection(const std::shared_ptr<ShotIntersection>& isect);
    const bool playerShotFlag;
    bool registerFlag;
    bool useDeleteFrameFlag;
    bool fadeDeleteFlag;
    bool spellResistEnable;
    bool eraseShotEnable;
    bool spellFactorEnable;
    bool itemChangeEnable;
    bool intersectionEnable;
    bool autoDeleteEnable;
    float angularVelocity;
    double damage;
    int sourceBlendType;
    int penetration;
    int fadeDeleteTimer;
    int delayTimer;
    int deleteFrameTimer;
    int fadeDeleteFrame;
    int animationFrameCnt;
    int animationIdx;
    std::list<AddedShot> addedShots;
    int addedShotFrameCnt;
};

class ObjLaser : public ObjShot
{
public:
    ObjLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState);
    void setShotData(const std::shared_ptr<ShotData>& shotData) override;
    bool isGrazeEnabled() const override;
    void graze() override;
    float getLength() const;
    void setLength(float limit);
    float getRenderWidth() const;
    virtual void setRenderWidth(float width);
    float getIntersectionWidth() const;
    void setIntersectionWidth(float width);
    float getGrazeInvalidFrame() const;
    void setGrazeInvalidFrame(int frame);
    float getGrazeInvalidTimer() const;
    float getItemDistance() const;
    void setItemDistance(float distance);
protected:
    void transIntersection(float dx, float dy) override {}
    void tickGrazeInvalidTimer();
private:
    float length;
    float renderWidth;
    float intersectionWidth;
    bool hasIntersectionWidth;
    int grazeInvalidFrame;
    int grazeInvalidTimer;
    float itemDistance;
};

class ObjLooseLaser : public ObjLaser
{
public:
    ObjLooseLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState);
    void update() override;
    void render() override;
    void generateDefaultBonusItem() override;
    float getInvalidLengthHead() const;
    float getInvalidLengthTail() const;
    bool isDefaultInvalidLengthEnabled() const;
    void setDefaultInvalidLengthEnable(bool enable);
    void setInvalidLength(float head, float tail);
    virtual Point2D getHead() const;
    virtual Point2D getTail() const;
    virtual float getRenderLength() const;
protected:
    void updateIntersection();
    void renderLaser(float width, float length, float angle);
private:
    void extend();
    bool defaultInvalidLengthEnable;
    float invalidLengthHead;
    float invalidLengthTail;
    float renderLength; // レーザーの描画時の長さ, 不変条件 : 常に正
};

class ObjStLaser : public ObjLooseLaser
{
public:
    ObjStLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState);
    void update() override;
    void render() override;
    Point2D getTail() const override;
    float getLaserAngle() const;
    void setLaserAngle(float angle);
    bool hasSource() const;
    void setSource(bool hasSource);
    float getRenderLength() const override;
private:
    float laserAngle;
    bool laserSourceEnable;
    float laserWidthScale;
};

class ObjCrLaser : public ObjLaser
{
public:
    ObjCrLaser(bool isPlayerShot, const std::shared_ptr<GameState>& gameState);
    void update() override;
    void render() override;
    void generateDefaultBonusItem() override;
    void setRenderWidth(float width) override;
    float getTipDecrement() const;
    void setTipDecrement(float dec);
    int getLaserNodeCount() const;
protected:
    void fixVertexDistance(float width);
    void extend(float x, float y);
    std::vector<Vertex> trail;
    float totalLaserLength;
    std::deque<float> laserNodeLengthList;
    int tailPos;
    bool hasHead;
    const size_t shrinkThresholdOffset;
    float headX;
    float headY;
    float tipDecrement;
};
}
