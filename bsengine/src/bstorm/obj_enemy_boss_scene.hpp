#pragma once

#include <bstorm/obj.hpp>
#include <bstorm/type.hpp>

namespace bstorm
{
struct SourcePos;
class ObjEnemy;
class ObjEnemyBossScene : public Obj
{
public:
    ObjEnemyBossScene(const std::shared_ptr<Package>& package);
    void Update() override;
    void Regist(const std::shared_ptr<SourcePos>& srcPos);
    void Add(int step, const std::wstring& path);
    void LoadInThread(const std::shared_ptr<SourcePos>& srcPos);
    int GetTimer() const;
    int GetTimerF() const;
    int GetOrgTimerF() const;
    GameScore GetSpellScore() const;
    bool IsSpell() const;
    bool IsLastSpell() const;
    bool IsDurableSpell() const;
    bool IsLastStep() const;
    int GetRemainStepCount() const;
    int GetActiveStepLifeCount() const;
    double GetActiveStepTotalMaxLife() const;
    double GetActiveStepTotalLife() const;
    int GetPlayerShootDownCount() const;
    int GetPlayerSpellCount() const;
    std::vector<double> GetActiveStepLifeRateList() const;
    double GetCurrentLife() const;
    double GetCurrentLifeMax() const;
    void SetTimer(int sec);
    void StartSpell();
    void AddDamage(double life);
    void AddPlayerSpellCount(int c);
    void AddPlayerShootDownCount(int c);
    std::shared_ptr<ObjEnemy> GetEnemyBossObject() const;
private:
  // Phase: 独立したライフを持つ単位
    struct Phase
    {
        Phase(const std::wstring& path);
        std::wstring path;
        int scriptId;
        double life;
        double maxLife;
        int timerF;
        int orgTimerF;
        GameScore spellScore;
        bool isSpell;
        bool isLastSpell;
        bool isDurableSpell;
    };
    bool LoadNext();
    const Phase& GetCurrentPhase() const;
    bool ExistPhase() const;
    // ボスシーンは複数のステップから成り
    // ステップは複数のフェーズから成る
    std::map<int, std::vector<Phase>> steps_;
    int maxStep_;
    int currentStep_;
    int currentPhase_;
    bool registerFlag_;
    int playerSpellCount_;
    int playerShootDownCount_;
    std::weak_ptr<ObjEnemy> enemyBossObj_;
    float lastEnemyBossX_;
    float lastEnemyBossY_;
};
}