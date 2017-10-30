#pragma once

#include <bstorm/obj.hpp>

namespace bstorm {
  class ObjEnemy;
  class ObjEnemyBossScene : public Obj {
  public:
    ObjEnemyBossScene(const std::shared_ptr<GameState>& gameState);
    void update() override;
    void regist();
    void add(int step, const std::wstring& path);
    void loadInThread();
    int getTimer() const;
    int getTimerF() const;
    int getOrgTimerF() const;
    int64_t getSpellScore() const;
    bool isSpell() const;
    bool isLastSpell() const;
    bool isDurableSpell() const;
    bool isLastStep() const;
    int getRemainStepCount() const;
    int getActiveStepLifeCount() const;
    double getActiveStepTotalMaxLife() const;
    double getActiveStepTotalLife() const;
    int getPlayerShootDownCount() const;
    int getPlayerSpellCount() const;
    std::vector<double> getActiveStepLifeRateList() const;
    double getCurrentLife() const;
    double getCurrentLifeMax() const;
    void setTimer(int sec);
    void startSpell();
    void addDamage(double life);
    void addPlayerSpellCount(int c);
    void addPlayerShootDownCount(int c);
    std::shared_ptr<ObjEnemy> getEnemyBossObject() const;
  protected:
    // Phase: 独立したライフを持つ単位
    struct Phase {
      Phase(const std::wstring& path);
      std::wstring path;
      int scriptId;
      double life;
      double maxLife;
      int timerF;
      int orgTimerF;
      int64_t spellScore;
      bool isSpell;
      bool isLastSpell;
      bool isDurableSpell;
    };
    bool loadNext();
    const Phase& getCurrentPhase() const;
    bool existPhase() const;
    // ボスシーンは複数のステップから成り
    // ステップは複数のフェーズから成る
    std::map<int, std::vector<Phase>> steps;
    int maxStep;
    int currentStep;
    int currentPhase;
    bool registerFlag;
    int playerSpellCount;
    int playerShootDownCount;
    std::weak_ptr<ObjEnemy> enemyBossObj;
    float lastEnemyBossX;
    float lastEnemyBossY;
  };
}