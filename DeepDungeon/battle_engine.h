#ifndef BATTLE_ENGINE_H
#define BATTLE_ENGINE_H

#include <vector>
#include <QString>
#include "character_config.h"
#include "enemy_config.h"
#include "card_config.h"
#include <QObject>

enum class BattleState {
    DECISION_PHASE,
    PLAYER_ACTION,
    ENEMY_ACTION,
    CHECK_DEATH1,
    CHECK_DEATH2,
    ROUND_END,
    BATTLE_OVER
};

enum class SkillTargetType {
    SELF,               // 自身
    ALLY_ALL,           // 友方全体
    ALLY_SINGLE,        // 随机友方单体（如命运骰子）
    ALLY_SINGLE_SELECT, // 指定友方单体（需弹出选择窗口）
    ENEMY_ALL,          // 敌方全体
    ENEMY_FRONT,        // 敌方前台
    DEFAULT             // 默认敌方前台
};

class BattleEngine : public QObject {
    Q_OBJECT
private:
    BattleState currentState;
    std::vector<Character*> playerTeam;
    std::vector<Character*> enemyTeam;

    int playerSelectedSkillIndex; // 核心：记录玩家按下了0(普攻), 1(特技1), 还是 2(特技2)
    Skill* enemySelectedSkill;
    bool isPlayerFirst;
    std::vector<QString> battleLogs;
    int playerSelectedTargetIndex = -1;

    void processAction(Character* attacker, Character* defender, bool isPlayer);
    void handleDeathAndSwap();
    bool checkBattleEnd();

signals:
    void stateChanged(BattleState newState);  // 状态变化信号
    void logAdded(const QString& log);        // 日志添加信号
    void battleEnded(bool playerWon);         // 战斗结束信号
    void uiNeedsUpdate();                     // UI需要更新信号

public:
    BattleEngine(const std::vector<Character*>& pTeam, const std::vector<Character*>& eTeam);
    void updateState();
    void submitPlayerAction(int skillIndex); // 接收 UI 传来的 0, 1, 2
    ~BattleEngine();

    BattleState getCurrentState() const { return currentState; }
    Character* getPlayerFront() const { return playerTeam.empty() ? nullptr : playerTeam[0]; }
    Enemy* getEnemyFront() const { return enemyTeam.empty() ? nullptr : static_cast<Enemy*>(enemyTeam[0]); }
    bool isPlayerWin() const;
    void addLog(const QString& msg);
    std::vector<QString> getAndClearLogs();
    void processPreActionBuffs(Character* attacker, Character* defender, bool isPlayer);
    void processPostActionBuffs(Character* attacker, Character* defender, bool isPlayer);
    void processRoundEndBuffs(Character* character, bool isPlayer);
    SkillTargetType getSkillTargetType(const QString& skillName);
    void performNormalAttack(Character* attacker, Character* defender, bool isPlayer);
    void handlePlayerAction(Character* attacker, Character* defender);
    void handlePlayerSkill(Character* attacker, Character* defender);
    void handleEnemyAction(Character* attacker, Character* defender);
    void handleEnemySkill(Character* attacker, Character* defender);
    void executeSkill(Character* attacker, Character* defender, Skill* skill,
                      SkillTargetType targetType, const QString& skillName,
                      const QString& skillDesc, bool isPlayer);
    void triggerSupportArcherAttack(Character* mainAttacker, Character* target, bool isPlayerTeam);
    int calculateInstantKillDamage(Character* attacker);
    int calculateNormalAttackDamage(Character* attacker, Character* defender);
    const std::vector<Character*>& getPlayerTeam() const { return playerTeam; }
    const std::vector<Character*>& getEnemyTeam() const { return enemyTeam; }
    void applyBondEffects();
    void clearBondEffects();
    void endPlayerTurnAfterSwitch();
    void triggerWeaponAttackEffects(Character* attacker, Character* defender,
                                    bool isPlayer, int damageDealt);
    void submitPlayerActionWithTarget(int skillIndex, int targetIndex);
    int getPlayerSelectedTargetIndex() const { return playerSelectedTargetIndex; }
    bool switchCharacterTo(int targetIndex);
};

#endif

