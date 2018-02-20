#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/script.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/game_state.hpp>
#include <bstorm/obj_enemy_boss_scene.hpp>

namespace bstorm {
  ObjEnemyBossScene::ObjEnemyBossScene(const std::shared_ptr<GameState>& gameState) :
    Obj(gameState),
    registerFlag(false),
    currentStep(0),
    currentPhase(-1),
    playerSpellCount(0),
    playerShootDownCount(0),
    maxStep(0),
    lastEnemyBossX(0),
    lastEnemyBossY(0)
  {
    setType(OBJ_ENEMY_BOSS_SCENE);
    steps[0] = std::vector<Phase>();
  }

  void ObjEnemyBossScene::update() {
    if (auto state = getGameState()) {
      if (registerFlag) {
        Phase& phase = const_cast<Phase&>(getCurrentPhase());
        auto enemyBoss = enemyBossObj.lock();
        if (phase.timerF > 0) phase.timerF--;
        if (phase.timerF == 0) {
          phase.life = 0;
          if (enemyBoss) enemyBoss->setLife(0);
          state->scriptManager->notifyEventAll(EV_TIMEOUT);
        }
        if (enemyBoss) {
          lastEnemyBossX = enemyBoss->getX();
          lastEnemyBossY = enemyBoss->getY();
        } else {
          // スペルカードで
          if (isSpell()) {
            // 時間制限内にクリアしたか、または耐久スペルで
            if (phase.timerF != 0 || isDurableSpell()) {
              // ノーボムノーミスなら取得
              if (playerSpellCount == 0 && playerShootDownCount == 0) {
                state->scriptManager->notifyEventAll(EV_GAIN_SPELL);
              }
            }
          }
          state->scriptManager->notifyEventAll(EV_END_BOSS_STEP);
          if (!loadNext()) {
            die();
          }
        }
      }
    }
  }

  void ObjEnemyBossScene::regist(const std::shared_ptr<SourcePos>& srcPos) {
    if (registerFlag) {
      return;
    }
    loadInThread(srcPos);
    if (!loadNext()) {
      die();
    }
    registerFlag = true;
  }

  void ObjEnemyBossScene::add(int step, const std::wstring& path) {
    if (registerFlag) return;
    maxStep = std::max(step, maxStep);
    if (steps.count(step) == 0) {
      steps[step] = std::vector<Phase>();
    }
    steps[step].emplace_back(path);
  }

  void ObjEnemyBossScene::loadInThread(const std::shared_ptr<SourcePos>& srcPos) {
    auto state = getGameState();
    if (!state) return;
    if (registerFlag) return;
    for (auto& entry : steps) {
      for (auto& phase : entry.second) {
        if (phase.scriptId < 0)
          phase.scriptId = state->scriptManager->compileInThread(phase.path, SCRIPT_TYPE_SINGLE, state->stageMainScriptInfo.version, srcPos)->getID();
      }
    }
  }

  int ObjEnemyBossScene::getTimer() const {
    if (getTimerF() < 0) return 99;
    return (int)(getTimerF() / 60);
  }

  int ObjEnemyBossScene::getTimerF() const {
    if (!existPhase()) return -1;
    const Phase& phase = getCurrentPhase();
    return phase.timerF;
  }

  int ObjEnemyBossScene::getOrgTimerF() const {
    if (!existPhase()) return -1;
    const Phase& phase = getCurrentPhase();
    return phase.orgTimerF;
  }

  int64_t ObjEnemyBossScene::getSpellScore() const {
    if (!existPhase()) return 0;
    const Phase& phase = getCurrentPhase();
    if (phase.isDurableSpell || phase.orgTimerF == 0) return phase.spellScore;
    return phase.spellScore * phase.timerF / phase.orgTimerF;
  }

  bool ObjEnemyBossScene::isSpell() const {
    if (!existPhase()) return false;
    const Phase& phase = getCurrentPhase();
    return phase.isSpell;
  }

  bool ObjEnemyBossScene::isLastSpell() const {
    if (!existPhase()) return false;
    Phase& phase = const_cast<Phase&>(getCurrentPhase());
    return phase.isLastSpell;
  }

  bool ObjEnemyBossScene::isDurableSpell() const {
    if (!existPhase()) return false;
    Phase& phase = const_cast<Phase&>(getCurrentPhase());
    return phase.isDurableSpell;
  }

  bool ObjEnemyBossScene::isLastStep() const {
    if (currentStep < 0) return false;
    return maxStep == currentStep;
  }

  int ObjEnemyBossScene::getRemainStepCount() const {
    if (currentStep < 0) return 0;
    return maxStep - currentStep;
  }

  int ObjEnemyBossScene::getActiveStepLifeCount() const {
    if (!existPhase()) return 0;
    return steps.at(currentStep).size();
  }

  double ObjEnemyBossScene::getActiveStepTotalMaxLife() const {
    if (!existPhase()) return 0;
    double total = 0;
    for (const auto& phase : steps.at(currentStep)) {
      total += phase.maxLife;
    }
    return total;
  }

  double ObjEnemyBossScene::getActiveStepTotalLife() const {
    if (!existPhase()) return 0;
    const auto& phases = steps.at(currentStep);
    double total = 0;
    for (int i = currentPhase; i < phases.size(); i++) {
      total += phases[i].life;
    }
    return total;
  }

  int ObjEnemyBossScene::getPlayerShootDownCount() const {
    return playerShootDownCount;
  }

  int ObjEnemyBossScene::getPlayerSpellCount() const {
    return playerSpellCount;
  }

  std::vector<double> ObjEnemyBossScene::getActiveStepLifeRateList() const {
    if (!existPhase()) return std::vector<double>();
    double total = getActiveStepTotalMaxLife();
    double subTotal = 0;
    std::vector<double> list;
    for (const auto& phase : steps.at(currentStep)) {
      subTotal += phase.maxLife;
      list.push_back(subTotal / total);
    }
    return list;
  }

  double ObjEnemyBossScene::getCurrentLife() const {
    if (!existPhase()) return 0;
    const Phase& phase = getCurrentPhase();
    return phase.life;
  }

  double ObjEnemyBossScene::getCurrentLifeMax() const {
    if (!existPhase()) return 0;
    const Phase& phase = getCurrentPhase();
    return phase.maxLife;
  }

  void ObjEnemyBossScene::setTimer(int sec) {
    if (!existPhase()) return;
    Phase& phase = const_cast<Phase&>(getCurrentPhase());
    phase.timerF = sec * 60;
    phase.orgTimerF = phase.timerF;
  }

  void ObjEnemyBossScene::startSpell() {
    if (!existPhase()) return;
    Phase& phase = const_cast<Phase&>(getCurrentPhase());
    phase.isSpell = true;
    if (auto state = getGameState()) {
      state->scriptManager->notifyEventAll(EV_START_BOSS_SPELL);
    }
  }

  void ObjEnemyBossScene::addDamage(double damage) {
    if (!existPhase()) return;
    Phase& phase = const_cast<Phase&>(getCurrentPhase());
    phase.life -= damage;
    phase.life = std::max(0.0, phase.life);
  }

  void ObjEnemyBossScene::addPlayerSpellCount(int c) {
    playerSpellCount += c;
  }

  void ObjEnemyBossScene::addPlayerShootDownCount(int c) {
    playerShootDownCount += c;
  }

  std::shared_ptr<ObjEnemy> ObjEnemyBossScene::getEnemyBossObject() const {
    return enemyBossObj.lock();
  }

  ObjEnemyBossScene::Phase::Phase(const std::wstring& path) :
    path(path),
    scriptId(-1),
    life(0),
    timerF(-1),
    orgTimerF(-1),
    spellScore(0),
    isSpell(false),
    isLastSpell(false),
    isDurableSpell(false)
  {
  }

  bool ObjEnemyBossScene::existPhase() const {
    if (steps.count(currentStep) == 0) return false;
    if (currentPhase < 0 || currentPhase >= steps.at(currentStep).size()) return false;
    return true;
  }

  const ObjEnemyBossScene::Phase& ObjEnemyBossScene::getCurrentPhase() const {
    if (!existPhase()) {
      throw Log(Log::Level::LV_ERROR).setMessage("phase not exists, please send bug report.");
    }
    const auto& phases = steps.at(currentStep);
    return phases.at(currentPhase);
  }

  bool ObjEnemyBossScene::loadNext() {
    auto state = getGameState();
    if (!state) return false;

    currentPhase++;
    if (currentPhase >= steps[currentStep].size()) {
      currentStep++;
      currentPhase = 0;
    }

    if (!existPhase()) { return false; }

    if (currentPhase == 0) {
      // new step
      for (auto& phase : steps[currentStep]) {
        // ステップ内の全てのフェーズのスクリプトを開始させる
        if (auto script = state->scriptManager->get(phase.scriptId)) {
          // コンパイルと@Loadingが終了してなければブロックして完了させる
          script->start();

          script->notifyEvent(EV_REQUEST_LIFE);
          if (script->getScriptResult()->getType() == DnhValue::Type::NIL) {
            Logger::WriteLog(Log::Level::LV_WARN, "enemy life hasn't been setted in @Event, set a default value 2000.");
            phase.maxLife = phase.life = 2000.0f;
          } else {
            phase.maxLife = phase.life = std::max(0.0, script->getScriptResult()->toNum());
          }

          script->notifyEvent(EV_REQUEST_TIMER);
          if (script->getScriptResult()->getType() == DnhValue::Type::NIL) {
            phase.timerF = -1; // 無制限
          } else {
            // NOTE: double値を60倍してから切り捨てる
            phase.timerF = (int)(script->getScriptResult()->toNum() * 60);
            phase.orgTimerF = phase.timerF;
          }

          script->notifyEvent(EV_REQUEST_IS_SPELL);
          phase.isSpell = script->getScriptResult()->toBool();

          script->notifyEvent(EV_REQUEST_SPELL_SCORE);
          phase.spellScore = script->getScriptResult()->toInt();

          script->notifyEvent(EV_REQUEST_IS_LAST_SPELL);
          phase.isLastSpell = script->getScriptResult()->toBool();

          script->notifyEvent(EV_REQUEST_IS_DURABLE_SPELL);
          phase.isDurableSpell = script->getScriptResult()->toBool();
        }
      }
    }

    playerSpellCount = playerShootDownCount = 0;
    const Phase& phase = getCurrentPhase();

    if (auto script = state->scriptManager->get(phase.scriptId)) {
      auto boss = state->objTable->create<ObjEnemy>(true, state);
      state->objLayerList->setRenderPriority(boss, DEFAULT_ENEMY_RENDER_PRIORITY);
      boss->regist();
      boss->setMovePosition(lastEnemyBossX, lastEnemyBossY);
      boss->setLife(getCurrentLife());
      enemyBossObj = boss;
      script->runInitialize();
      state->scriptManager->notifyEventAll(EV_START_BOSS_STEP);
    }

    return true;
  }
}