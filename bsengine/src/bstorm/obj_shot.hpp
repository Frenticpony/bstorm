#pragma once

#include <bstorm/point2D.hpp>
#include <bstorm/vertex.hpp>
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
    ObjShot(bool isPlayerShot, const std::shared_ptr<Package>& package);
    ~ObjShot();

    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;

    bool IsRegistered() const;
    void Regist();
    bool IsPlayerShot() const;

    // intersection
    void AddIntersectionCircleA1(float r);
    void AddIntersectionCircleA2(float x, float y, float r);
    void AddIntersectionLine(float x1, float y1, float x2, float y2, float width);
    void AddTempIntersectionCircleA1(float r);
    void AddTempIntersectionCircleA2(float x, float y, float r);
    void AddTempIntersectionLine(float x1, float y1, float x2, float y2, float width);
    bool IsIntersectionEnabled() const;
    void SetIntersectionEnable(bool enable);
    bool IsTempIntersectionMode() const;

    // shot data
    const NullableSharedPtr<ShotData>& GetShotData() const;
    virtual void SetShotData(const std::shared_ptr<ShotData>& shotData);
    int GetAnimationFrameCount() const;
    int GetAnimationIndex() const;

    double GetDamage() const;
    void SetDamage(double damage);
    int GetPenetration() const;
    void SetPenetration(int penetration);
    int GetDelay() const;
    void SetDelay(int delay);
    bool IsDelay() const;
    int GetSourceBlendType() const;
    void SetSourceBlendType(int blendType);
    float GetAngularVelocity() const;
    void SetAngularVelocity(float angularVelocity);
    bool IsSpellResistEnabled() const;
    void SetSpellResistEnable(bool enable);
    bool IsSpellFactorEnabled() const;
    void SetSpellFactor(bool enable);
    bool IsEraseShotEnabled() const;
    void SetEraseShotEnable(bool enable);
    bool IsItemChangeEnabled() const;
    void SetItemChangeEnable(bool enable);
    bool IsAutoDeleteEnabled() const;
    void SetAutoDeleteEnable(bool enable);

    // delete
    void ToItem();
    void EraseWithSpell();
    void DeleteImmediate();
    void FadeDelete();
    float GetFadeScale() const;
    bool IsFadeDeleteStarted() const;
    bool IsFrameDeleteStarted() const;
    int GetDeleteFrameTimer() const;
    int GetFadeDeleteFrameTimer() const;
    void SetDeleteFrame(int frame);

    // graze
    virtual void Graze();
    virtual bool IsGrazeEnabled() const;

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
    void AddShotA1(int shotObjId, int frame);
    void AddShotA2(int shotObjId, int frame, float dist, float angle);
    int GetFrameCountForAddShot() const;
    const std::list<AddedShot>& GetAddedShot() const;
    virtual void GenerateDefaultBonusItem();
protected:
    void TransIntersection(float dx, float dy) override;
    void RenderIntersection(const std::shared_ptr<Renderer>& renderer);
    void CheckAutoDelete(float x, float y);
    void UpdateAnimationPosition();
    void TickDelayTimer();
    void TickDeleteFrameTimer();
    void TickAddedShotFrameCount();
    void TickFadeDeleteTimer();
    NullableSharedPtr<ShotData> shotData_;
    bool isGrazeInvalid_;
    bool isTempIntersectionMode_;
private:
    void AddIntersection(const std::shared_ptr<ShotIntersection>& isect);
    void AddTempIntersection(const std::shared_ptr<ShotIntersection>& isect);
    const bool isPlayerShot_;
    bool isRegistered_;
    bool isFrameDeleteStarted_;
    bool isFadeDeleteStarted_;
    bool spellResistEnable_;
    bool eraseShotEnable_;
    bool spellFactorEnable_;
    bool itemChangeEnable_;
    bool intersectionEnable_;
    bool autoDeleteEnable_;
    float angularVelocity_;
    double damage_;
    int sourceBlendType_;
    int penetration_;
    int fadeDeleteTimer_;
    int delayTimer_;
    int deleteFrameTimer_;
    int fadeDeleteFrame_;
    int animationFrameCnt_;
    int animationIdx_;
    std::list<AddedShot> addedShots_;
    int addedShotFrameCnt_;
};

class ObjLaser : public ObjShot
{
public:
    ObjLaser(bool isPlayerShot, const std::shared_ptr<Package>& package);
    void SetShotData(const std::shared_ptr<ShotData>& shotData) override;
    void Graze() override;
    bool IsGrazeEnabled() const override;
    float GetLength() const;
    void SetLength(float len); // 上限の長さを設定
    float GetRenderWidth() const;
    virtual void SetRenderWidth(float width);
    float GetIntersectionWidth() const;
    void SetIntersectionWidth(float width);
    float GetGrazeInvalidFrame() const;
    void SetGrazeInvalidFrame(int frame);
    float GetGrazeInvalidTimer() const;
    float GetItemDistance() const;
    void SetItemDistance(float distance);
protected:
    void TransIntersection(float dx, float dy) override {}
    void TickGrazeInvalidTimer();
private:
    float length_;
    float renderWidth_;
    float intersectionWidth_;
    bool hasIntersectionWidth_;
    int grazeInvalidFrame_;
    int grazeInvalidTimer_;
    float itemDistance_;
};

class ObjLooseLaser : public ObjLaser
{
public:
    ObjLooseLaser(bool isPlayerShot, const std::shared_ptr<Package>& package);
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    void GenerateDefaultBonusItem() override;
    float GetInvalidLengthHead() const;
    float GetInvalidLengthTail() const;
    bool IsDefaultInvalidLengthEnabled() const;
    void SetDefaultInvalidLengthEnable(bool enable);
    void SetInvalidLength(float head, float tail);
    virtual Point2D GetHead() const;
    virtual Point2D GetTail() const;
    virtual float GetRenderLength() const;
protected:
    void UpdateIntersection();
    void RenderLaser(float width, float length, float angle, const std::shared_ptr<Renderer>& renderer);
private:
    void Extend();
    bool defaultInvalidLengthEnable_;
    float invalidLengthHead_;
    float invalidLengthTail_;
    float renderLength_; // レーザーの描画時の長さ, 不変条件 : 常に正
};

class ObjStLaser : public ObjLooseLaser
{
public:
    ObjStLaser(bool isPlayerShot, const std::shared_ptr<Package>& package);
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    Point2D GetTail() const override;
    float GetLaserAngle() const;
    void SetLaserAngle(float angle);
    bool HasSource() const;
    void SetSource(bool hasSource);
    float GetRenderLength() const override;
private:
    float laserAngle_;
    bool laserSourceEnable_;
    float laserWidthScale_;
};

class ObjCrLaser : public ObjLaser
{
public:
    ObjCrLaser(bool isPlayerShot, const std::shared_ptr<Package>& package);
    void Update() override;
    void Render(const std::shared_ptr<Renderer>& renderer) override;
    void GenerateDefaultBonusItem() override;
    void SetRenderWidth(float width) override;
    float GetTipDecrement() const;
    void SetTipDecrement(float dec);
    int GetLaserNodeCount() const;
protected:
    void FixVertexDistance_(float width);
    void Extend(float x, float y);
    std::vector<Vertex> trail_;
    float totalLaserLength_;
    std::deque<float> laserNodeLengthList_;
    int tailPos_;
    bool hasHead_;
    const size_t shrinkThresholdOffset_;
    float headX_;
    float headY_;
    float tipDecrement_;
};
}
