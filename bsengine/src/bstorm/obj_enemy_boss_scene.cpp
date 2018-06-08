#include <bstorm/obj_enemy_boss_scene.hpp>

#include <bstorm/dnh_const.hpp>
#include <bstorm/const.hpp>
#include <bstorm/util.hpp>
#include <bstorm/logger.hpp>
#include <bstorm/script.hpp>
#include <bstorm/dnh_value.hpp>
#include <bstorm/obj_enemy.hpp>
#include <bstorm/package.hpp>

namespace bstorm
{
ObjEnemyBossScene::ObjEnemyBossScene(const std::shared_ptr<Package>& package) :
    Obj(package),
    isRegistered_(false),
    currentStep_(0),
    currentPhase_(-1),
    playerSpellCount_(0),
    playerShootDownCount_(0),
    maxStep_(0),
    lastEnemyBossX_(0),
    lastEnemyBossY_(0)
{
    SetType(OBJ_ENEMY_BOSS_SCENE);
    steps_[0] = std::vector<Phase>();
}

void ObjEnemyBossScene::Update()
{
    if (auto package = GetPackage().lock())
    {
        if (isRegistered_)
        {
            Phase& phase = const_cast<Phase&>(GetCurrentPhase());
            auto enemyBoss = enemyBossObj_.lock();
            if (phase.timerF > 0) phase.timerF--;
            if (phase.timerF == 0)
            {
                phase.life = 0;
                if (enemyBoss) enemyBoss->SetLife(0);
                package->NotifyEventAll(EV_TIMEOUT);
            }
            if (enemyBoss)
            {
                lastEnemyBossX_ = enemyBoss->GetX();
                lastEnemyBossY_ = enemyBoss->GetY();
            } else
            {
                // スペルカードで
                if (IsSpell())
                {
                    // 時間制限内にクリアしたか、または耐久スペルで
                    if (phase.timerF != 0 || IsDurableSpell())
                    {
                        // ノーボムノーミスなら取得
                        if (playerSpellCount_ == 0 && playerShootDownCount_ == 0)
                        {
                            package->NotifyEventAll(EV_GAIN_SPELL);
                        }
                    }
                }
                package->NotifyEventAll(EV_END_BOSS_STEP);
                if (!LoadNext())
                {
                    Die();
                }
            }
        }
    }
}

void ObjEnemyBossScene::Regist(const std::shared_ptr<SourcePos>& srcPos)
{
    if (isRegistered_)
    {
        return;
    }
    LoadInThread(srcPos);
    if (!LoadNext())
    {
        Die();
    }
    isRegistered_ = true;
}

void ObjEnemyBossScene::Add(int step, const std::wstring& path)
{
    if (isRegistered_) return;
    maxStep_ = std::max(step, maxStep_);
    if (steps_.count(step) == 0)
    {
        steps_[step] = std::vector<Phase>();
    }
    steps_[step].emplace_back(path);
}

void ObjEnemyBossScene::LoadInThread(const std::shared_ptr<SourcePos>& srcPos)
{
    auto package = GetPackage().lock();
    if (!package) return;
    if (isRegistered_) return;
    for (auto& entry : steps_)
    {
        for (auto& phase : entry.second)
        {
            if (phase.scriptId < 0)
            {
                phase.scriptId = package->LoadScriptInThread(phase.path, SCRIPT_TYPE_SINGLE, SCRIPT_VERSION_PH3, srcPos)->GetID();
            }
        }
    }
}

int ObjEnemyBossScene::GetTimer() const
{
    if (GetTimerF() < 0) return 99;
    return (int)(GetTimerF() / 60);
}

int ObjEnemyBossScene::GetTimerF() const
{
    if (!ExistPhase()) return -1;
    const Phase& phase = GetCurrentPhase();
    return phase.timerF;
}

int ObjEnemyBossScene::GetOrgTimerF() const
{
    if (!ExistPhase()) return -1;
    const Phase& phase = GetCurrentPhase();
    return phase.orgTimerF;
}

PlayerScore ObjEnemyBossScene::GetSpellScore() const
{
    if (!ExistPhase()) return 0;
    const Phase& phase = GetCurrentPhase();
    if (phase.isDurableSpell || phase.orgTimerF == 0) return phase.spellScore;
    return phase.spellScore * phase.timerF / phase.orgTimerF;
}

bool ObjEnemyBossScene::IsSpell() const
{
    if (!ExistPhase()) return false;
    const Phase& phase = GetCurrentPhase();
    return phase.isSpell;
}

bool ObjEnemyBossScene::IsLastSpell() const
{
    if (!ExistPhase()) return false;
    Phase& phase = const_cast<Phase&>(GetCurrentPhase());
    return phase.isLastSpell;
}

bool ObjEnemyBossScene::IsDurableSpell() const
{
    if (!ExistPhase()) return false;
    Phase& phase = const_cast<Phase&>(GetCurrentPhase());
    return phase.isDurableSpell;
}

bool ObjEnemyBossScene::IsLastStep() const
{
    if (currentStep_ < 0) return false;
    return maxStep_ == currentStep_;
}

int ObjEnemyBossScene::GetRemainStepCount() const
{
    if (currentStep_ < 0) return 0;
    return maxStep_ - currentStep_;
}

int ObjEnemyBossScene::GetActiveStepLifeCount() const
{
    if (!ExistPhase()) return 0;
    return steps_.at(currentStep_).size();
}

double ObjEnemyBossScene::GetActiveStepTotalMaxLife() const
{
    if (!ExistPhase()) return 0;
    double total = 0;
    for (const auto& phase : steps_.at(currentStep_))
    {
        total += phase.maxLife;
    }
    return total;
}

double ObjEnemyBossScene::GetActiveStepTotalLife() const
{
    if (!ExistPhase()) return 0;
    const auto& phases = steps_.at(currentStep_);
    double total = 0;
    for (int i = currentPhase_; i < phases.size(); i++)
    {
        total += phases[i].life;
    }
    return total;
}

int ObjEnemyBossScene::GetPlayerShootDownCount() const
{
    return playerShootDownCount_;
}

int ObjEnemyBossScene::GetPlayerSpellCount() const
{
    return playerSpellCount_;
}

std::vector<double> ObjEnemyBossScene::GetActiveStepLifeRateList() const
{
    if (!ExistPhase()) return std::vector<double>();
    double total = GetActiveStepTotalMaxLife();
    double subTotal = 0;
    std::vector<double> list;
    for (const auto& phase : steps_.at(currentStep_))
    {
        subTotal += phase.maxLife;
        list.push_back(subTotal / total);
    }
    return list;
}

double ObjEnemyBossScene::GetCurrentLife() const
{
    if (!ExistPhase()) return 0;
    const Phase& phase = GetCurrentPhase();
    return phase.life;
}

double ObjEnemyBossScene::GetCurrentLifeMax() const
{
    if (!ExistPhase()) return 0;
    const Phase& phase = GetCurrentPhase();
    return phase.maxLife;
}

void ObjEnemyBossScene::SetTimer(int sec)
{
    if (!ExistPhase()) return;
    Phase& phase = const_cast<Phase&>(GetCurrentPhase());
    phase.timerF = sec * 60;
    phase.orgTimerF = phase.timerF;
}

void ObjEnemyBossScene::StartSpell()
{
    if (!ExistPhase()) return;
    Phase& phase = const_cast<Phase&>(GetCurrentPhase());
    phase.isSpell = true;
    if (auto package = GetPackage().lock())
    {
        package->NotifyEventAll(EV_START_BOSS_SPELL);
    }
}

void ObjEnemyBossScene::AddDamage(double damage)
{
    if (!ExistPhase()) return;
    Phase& phase = const_cast<Phase&>(GetCurrentPhase());
    phase.life -= damage;
    phase.life = std::max(0.0, phase.life);
}

void ObjEnemyBossScene::AddPlayerSpellCount(int c)
{
    playerSpellCount_ += c;
}

void ObjEnemyBossScene::AddPlayerShootDownCount(int c)
{
    playerShootDownCount_ += c;
}

std::shared_ptr<ObjEnemy> ObjEnemyBossScene::GetEnemyBossObject() const
{
    return enemyBossObj_.lock();
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

bool ObjEnemyBossScene::ExistPhase() const
{
    if (steps_.count(currentStep_) == 0) return false;
    if (currentPhase_ < 0 || currentPhase_ >= steps_.at(currentStep_).size()) return false;
    return true;
}

const ObjEnemyBossScene::Phase& ObjEnemyBossScene::GetCurrentPhase() const
{
    if (!ExistPhase())
    {
        throw Log(Log::Level::LV_ERROR).SetMessage("phase not exists, please send bug report.");
    }
    const auto& phases = steps_.at(currentStep_);
    return phases.at(currentPhase_);
}

bool ObjEnemyBossScene::LoadNext()
{
    auto package = GetPackage().lock();
    if (!package) return false;

    currentPhase_++;
    if (currentPhase_ >= steps_[currentStep_].size())
    {
        currentStep_++;
        currentPhase_ = 0;
    }

    if (!ExistPhase()) { return false; }

    if (currentPhase_ == 0)
    {
        // new step
        for (auto& phase : steps_[currentStep_])
        {
            // ステップ内の全てのフェーズのスクリプトを開始させる
            if (auto script = package->GetScript(phase.scriptId))
            {
                // コンパイルと@Loadingが終了してなければブロックして完了させる
                script->Start();

                script->NotifyEvent(EV_REQUEST_LIFE);
                if (script->GetScriptResult()->GetType() == DnhValue::Type::NIL)
                {
                    Logger::WriteLog(Log::Level::LV_WARN, "enemy life hasn't been setted in @Event, set a default value 2000.");
                    phase.maxLife = phase.life = 2000.0f;
                } else
                {
                    phase.maxLife = phase.life = std::max(0.0, script->GetScriptResult()->ToNum());
                }

                script->NotifyEvent(EV_REQUEST_TIMER);
                if (script->GetScriptResult()->GetType() == DnhValue::Type::NIL)
                {
                    phase.timerF = -1; // 無制限
                } else
                {
                    // NOTE: double値を60倍してから切り捨てる
                    phase.timerF = (int)(script->GetScriptResult()->ToNum() * 60);
                    phase.orgTimerF = phase.timerF;
                }

                script->NotifyEvent(EV_REQUEST_IS_SPELL);
                phase.isSpell = script->GetScriptResult()->ToBool();

                script->NotifyEvent(EV_REQUEST_SPELL_SCORE);
                phase.spellScore = script->GetScriptResult()->ToInt();

                script->NotifyEvent(EV_REQUEST_IS_LAST_SPELL);
                phase.isLastSpell = script->GetScriptResult()->ToBool();

                script->NotifyEvent(EV_REQUEST_IS_DURABLE_SPELL);
                phase.isDurableSpell = script->GetScriptResult()->ToBool();
            }
        }
    }

    playerSpellCount_ = playerShootDownCount_ = 0;
    const Phase& phase = GetCurrentPhase();

    if (auto script = package->GetScript(phase.scriptId))
    {
        auto boss = package->CreateObjEnemyBoss();
        enemyBossObj_ = boss;
        boss->Regist();
        boss->SetMovePosition(lastEnemyBossX_, lastEnemyBossY_);
        boss->SetLife(GetCurrentLife());
        script->AddAutoDeleteTargetObjectId(boss->GetID());
        script->RunInitialize();
        package->NotifyEventAll(EV_START_BOSS_STEP);
    }

    return true;
}
}