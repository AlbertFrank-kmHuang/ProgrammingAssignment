#include "battle_engine.h"
#include "character_config.h"
#include <cstdlib>
#include <ctime>
#include <vector>
#include <QDebug>

BattleEngine::BattleEngine(const std::vector<Character*>& pTeam, const std::vector<Character*>& eTeam)
    : currentState(BattleState::DECISION_PHASE), playerTeam(pTeam), enemyTeam(eTeam),
    playerSelectedSkillIndex(-1), enemySelectedSkill(nullptr), isPlayerFirst(true)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // 添加羁绊效果
    applyBondEffects();
    // 开局随机决定整场战斗的先后手顺序
    isPlayerFirst = (std::rand() % 2 == 0);
    addLog(QString("【全局顺序】%1 将在战斗中先手")
               .arg(isPlayerFirst ? "玩家" : "敌人"));
    addLog("--- 战斗开始 ---");
    // 发射初始状态信号
    emit stateChanged(currentState);
    // 发射UI更新信号
    emit uiNeedsUpdate();
}

void BattleEngine::addLog(const QString& msg) {
    battleLogs.push_back(msg);
    emit logAdded(msg);  // 发出日志添加信号
}
std::vector<QString> BattleEngine::getAndClearLogs() {
    std::vector<QString> temp = battleLogs;
    battleLogs.clear();
    return temp;
}

// 应用羁绊效果
void BattleEngine::applyBondEffects() {
    if (playerTeam.empty()) return;

    // 统计各职业数量
    int mageCount = 0;      // 法师
    int warriorCount = 0;   // 剑士
    int archerCount = 0;    // 弓箭手
    int namelessCount = 0;  // 无名之人

    for (Character* c : playerTeam) {
        CharacterClass career = c->getJob();

        if (career == CharacterClass::Mage) {
            mageCount++;
        } else if (career == CharacterClass::Warrior) {
            warriorCount++;
        } else if (career == CharacterClass::Archer) {
            archerCount++;
        } else if (career == CharacterClass::Nonamer) {
            namelessCount++;
        }
    }

    // 记录羁绊效果
    addLog("=== 羁绊效果生效 ===");

    // 应用法师羁绊
    if (mageCount >= 2) {
        addLog(QString("【%1法师羁绊】所有人法力增加50点").arg(mageCount));
        for (Character* c : playerTeam) {
            c->setmanaadd(c->getmanaadd() + 50);
        }

        if (mageCount >= 4) {
            addLog(QString("【%1法师羁绊】所有人法力增加100点，生命值增加100点").arg(mageCount));
            for (Character* c : playerTeam) {
                c->setmanaadd(c->getmanaadd() + 100);
                c->sethealthadd(c->gethealthadd() + 100);
            }
        }
    }

    // 应用剑士羁绊
    if (warriorCount >= 2) {
        addLog(QString("【%1剑士羁绊】所有人生命增加50点").arg(warriorCount));
        for (Character* c : playerTeam) {
            c->sethealthadd(c->gethealthadd() + 50);
        }

        if (warriorCount >= 4) {
            addLog(QString("【%1剑士羁绊】所有人生命增加100点，防御力增加50点").arg(warriorCount));
            for (Character* c : playerTeam) {
                c->sethealthadd(c->gethealthadd() + 100);
                c->setdefenseadd(c->getdefenseadd() + 50);
            }
        }
    }

    // 应用弓箭手羁绊
    if (archerCount >= 2) {
        addLog(QString("【%1弓箭手羁绊】所有人攻击力增加20点").arg(archerCount));
        for (Character* c : playerTeam) {
            c->setattackadd(c->getattackadd() + 20);
        }

        if (archerCount >= 4) {
            addLog(QString("【%1弓箭手羁绊】所有人攻击力增加50点，闪避率增加30%").arg(archerCount));
            for (Character* c : playerTeam) {
                c->setattackadd(c->getattackadd() + 50);
                c->setmissrateadd(c->getmissrateadd() + 0.3f);
            }
        }
    }

    // 应用无名之人羁绊
    if (namelessCount >= 2) {
        addLog(QString("【%1无名之人羁绊】所有人暴击伤害增加50点").arg(namelessCount));
        for (Character* c : playerTeam) {
            c->setcritdamageadd(c->getcritdamageadd() + 0.5f);
        }

        if (namelessCount >= 4) {
            addLog(QString("【%1无名之人羁绊】所有人暴击率增加50点，暴击伤害增加80点").arg(namelessCount));
            for (Character* c : playerTeam) {
                c->setcritrateadd(c->getcritrateadd() + 0.5f);
                c->setcritdamageadd(c->getcritdamageadd() + 0.8f);
            }
        }
    }

    // ===== 组合羁绊（跨职业搭配，奖励多样化阵容） =====
    bool hasW = warriorCount >= 1, hasM = mageCount >= 1,
        hasA = archerCount >= 1, hasN = namelessCount >= 1;

    // 四职同辉：四种职业各至少一人 —— 全能加成
    if (hasW && hasM && hasA && hasN) {
        addLog("【四职同辉羁绊】职业齐全！全队攻击+30，防御+30，暴击率+20%");
        for (Character* c : playerTeam) {
            c->setattackadd(c->getattackadd() + 30);
            c->setdefenseadd(c->getdefenseadd() + 30);
            c->setcritrateadd(c->getcritrateadd() + 0.2f);
        }
    }

    // 战法相济：剑士 + 法师 —— 攻击与法力
    if (hasW && hasM) {
        addLog("【战法相济羁绊】剑士与法师并肩！全队攻击+20，法力+30");
        for (Character* c : playerTeam) {
            c->setattackadd(c->getattackadd() + 20);
            c->setmanaadd(c->getmanaadd() + 30);
        }
    }

    // 疾影连射：弓箭手 + 无名之人 —— 暴击伤害与闪避
    if (hasA && hasN) {
        addLog("【疾影连射羁绊】游猎与暗影！全队暴击伤害+30%，闪避+10%");
        for (Character* c : playerTeam) {
            c->setcritdamageadd(c->getcritdamageadd() + 0.3f);
            c->setmissrateadd(c->getmissrateadd() + 0.1f);
        }
    }

    // 铁壁神佑：剑士≥2 且 法师≥1 —— 坚不可摧
    if (warriorCount >= 2 && mageCount >= 1) {
        addLog("【铁壁神佑羁绊】坚盾与圣光！全队生命+60，防御+20");
        for (Character* c : playerTeam) {
            c->sethealthadd(c->gethealthadd() + 60);
            c->setdefenseadd(c->getdefenseadd() + 20);
        }
    }

    // 显示最终属性
    for (Character* c : playerTeam) {
        addLog(QString("  %1: 生命+%2, 法力+%3, 攻击+%4, 防御+%5")
                   .arg(QString::fromStdString(c->getName()))
                   .arg(c->gethealthadd())
                   .arg(c->getmanaadd())
                   .arg(c->getattackadd())
                   .arg(c->getdefenseadd()));
    }
}

// 接收 UI 传来的 0, 1, 2
void BattleEngine::submitPlayerAction(int skillIndex) {
    if (currentState == BattleState::DECISION_PHASE) {
        playerSelectedSkillIndex = skillIndex;

        if (!enemyTeam.empty() && enemyTeam[0]->alive()) {
            Enemy* enemyPtr = static_cast<Enemy*>(enemyTeam[0]);
            enemySelectedSkill = enemyPtr->AIStrategy();
        }
        // 直接进入先手方的行动状态
        currentState = isPlayerFirst ? BattleState::PLAYER_ACTION : BattleState::ENEMY_ACTION;
        updateState();
    }
}

// 清除羁绊效果
void BattleEngine::clearBondEffects() {
    // 在战斗结束时清除所有羁绊加成
    for (Character* c : playerTeam) {
        // 重置所有加成
        c->sethealthadd(0);
        c->setmanaadd(0);
        c->setattackadd(0);
        c->setdefenseadd(0);
        c->setcritrateadd(0);
        c->setcritdamageadd(0);
        c->setmissrateadd(0);
    }
}

void BattleEngine::updateState() {

    bool needContinue = true;
    BattleState oldState = currentState;  // 记录旧状态

    while (needContinue) {
        needContinue = false;
        switch (currentState) {
        case BattleState::DECISION_PHASE:
            break;
        case BattleState::PLAYER_ACTION:
            // 执行玩家行动
            if (playerTeam[0]->alive() && playerTeam[0]->startTurn()) {
                if (playerSelectedSkillIndex != -2) {
                    processAction(playerTeam[0], enemyTeam[0], true);
                } else {
                    addLog("玩家切换角色结束回合，跳过玩家行动。");
                }
            }
            if(isPlayerFirst){
                currentState = BattleState::CHECK_DEATH1;
            }
            else{
                currentState = BattleState::CHECK_DEATH2;
            }
            needContinue = true;
            break;
        case BattleState::ENEMY_ACTION:
            // 执行敌人行动
            if (enemyTeam[0]->alive() && enemyTeam[0]->startTurn()) {
                processAction(enemyTeam[0], playerTeam[0], false);
            }
            if(!isPlayerFirst){
                currentState = BattleState::CHECK_DEATH1;
            }
            else{
                currentState = BattleState::CHECK_DEATH2;
            }
            needContinue = true;
            break;
        case BattleState::CHECK_DEATH1:
            handleDeathAndSwap();
            if (checkBattleEnd()) {
                currentState = BattleState::BATTLE_OVER;
            } else {
                if(isPlayerFirst){
                    currentState = BattleState::ENEMY_ACTION;
                }
                else{
                    currentState = BattleState::PLAYER_ACTION;
                }
            }
            needContinue = true;
            break;
        case BattleState::CHECK_DEATH2:
            handleDeathAndSwap();
            if (checkBattleEnd()) {
                currentState = BattleState::BATTLE_OVER;
            } else {
                currentState = BattleState::ROUND_END;
            }
            needContinue = true;
            break;
        case BattleState::ROUND_END:
            if (playerTeam[0]->alive()) {
                playerTeam[0]->endTurn();
                processRoundEndBuffs(playerTeam[0], true);
            }
            if (enemyTeam[0]->alive()) {
                enemyTeam[0]->endTurn();
                processRoundEndBuffs(enemyTeam[0], false);
            }
            playerSelectedSkillIndex = -1;
            playerSelectedTargetIndex = -1;
            currentState = BattleState::DECISION_PHASE;
            break;
        case BattleState::BATTLE_OVER:
            clearBondEffects();
            addLog("--- 战斗结束 ---");
            break;
        }
    }

    // 状态变化时发出信号
    if (oldState != currentState) {
        emit stateChanged(currentState);
    }

    // 总是发出UI更新信号
    emit uiNeedsUpdate();

    // 检查战斗是否结束
    if (currentState == BattleState::BATTLE_OVER) {
        bool playerWon = isPlayerWin();
        emit battleEnded(playerWon);
    }
}

// 计算普通攻击伤害
int BattleEngine::calculateNormalAttackDamage(Character* attacker, Character* defender) {
    if (!attacker || !defender) return 0;

    // 基础攻击力
    int baseAttack = attacker->getAttack() + attacker->getattackadd();

    // 计算暴击
    float critRate = attacker->getCritrate() + attacker->getcritrateadd();
    float critDamage = attacker->getCritdamage() + attacker->getcritdamageadd();

    bool isCrit = (std::rand() % 10000) < (critRate * 10000);
    int finalDamage = baseAttack;

    if (isCrit) {
        finalDamage = static_cast<int>(finalDamage * critDamage);
        addLog(QString("  【暴击！】%1 触发了暴击！")
                   .arg(QString::fromStdString(attacker->getName())));
    }

    return finalDamage;  // 返回原始伤害值
}

// 触发协同攻击
void BattleEngine::triggerSupportArcherAttack(Character* mainAttacker, Character* target, bool isPlayerTeam) {
    if (!mainAttacker || !target || !target->alive()) {
        return;  // 攻击者或目标无效，不触发协同攻击
    }

    std::vector<Character*>& team = isPlayerTeam ? playerTeam : enemyTeam;
    int supportCount = 0;

    for (size_t i = 1; i < team.size(); i++) {  // 从第二个角色开始（第一个是前台）
        Character* character = team[i];

        // 检查弓箭手是否存活且可进行协同攻击
        if (character && character->alive() && character->isArcher()) {
            // 计算协同攻击伤害（普通攻击的50%）
            int baseDamage = calculateNormalAttackDamage(character, target);
            int supportDamage = static_cast<int>(baseDamage * 0.5);

            if (supportDamage > 0) {
                // 使用协同攻击函数
                character->performSupportAttack(target);
                supportCount++;

                addLog(QString("  【协同攻击】%1 发动了协同攻击，造成 %2 点伤害")
                           .arg(QString::fromStdString(character->getName()))
                           .arg(supportDamage));
            } else {
                addLog(QString("  【协同攻击】%1 尝试发动协同攻击，但被闪避了")
                           .arg(QString::fromStdString(character->getName())));
            }
        }
    }

    if (supportCount > 0) {
        addLog(QString("【协同支援】%1 名弓箭手发动了协同攻击")
                   .arg(supportCount));
    }
}

// 核心执行区
void BattleEngine::processAction(Character* attacker, Character* defender, bool isPlayer) {
    if (!attacker || !attacker->alive()) return;

    if (isPlayer) {
        handlePlayerAction(attacker, defender);
    } else {
        handleEnemyAction(attacker, defender);
    }
}

// 处理玩家行动
void BattleEngine::handlePlayerAction(Character* attacker, Character* defender) {
    // 检查是否是切换角色结束回合的特殊标记
    if (playerSelectedSkillIndex == -1) {
        addLog("玩家未选择行动，跳过。");
        return;
    }
    if (playerSelectedSkillIndex == -2) {
        addLog(QString("  %1 刚刚切换上场，不进行行动。").arg(QString::fromStdString(attacker->getName())));
        return;  // 直接返回，不执行任何行动
    }
    // 检查玩家选择的技能索引
    if (playerSelectedSkillIndex < 0 || playerSelectedSkillIndex >= static_cast<int>(attacker->getSkills().size())) {
        // 如果索引无效，默认为普攻
        playerSelectedSkillIndex = 0;
    }

    // 分离普攻和技能
    if (playerSelectedSkillIndex == 0) {
        // 普通攻击
        if (!defender || !defender->alive()) {
            addLog("错误：攻击目标不存在！");
            return;
        }
        performNormalAttack(attacker, defender, true);
    } else {
        // 技能攻击
        handlePlayerSkill(attacker, defender);
    }
}

// 处理玩家技能
void BattleEngine::handlePlayerSkill(Character* attacker, Character* defender) {
    Skill* skill = attacker->getSkills()[playerSelectedSkillIndex];
    QString skillName = QString::fromStdString(skill->getname());
    SkillTargetType targetType = getSkillTargetType(skillName);

    if (targetType == SkillTargetType::ALLY_SINGLE_SELECT) {
        // 指定单体目标（已由UI选择）
        if (playerSelectedTargetIndex < 0 || playerSelectedTargetIndex >= (int)playerTeam.size()) {
            addLog("错误：未选择有效目标");
            return;
        }
        Character* target = playerTeam[playerSelectedTargetIndex];
        if (!target || !target->alive()) {
            addLog("错误：目标已死亡");
            return;
        }
        addLog(QString("【%1】对 %2 释放了 [%3]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(QString::fromStdString(target->getName()))
                   .arg(skillName));
        QString skillDesc = QString::fromStdString(skill->getdescription());
        addLog(QString("  技能效果：%1").arg(skillDesc));
        processPreActionBuffs(attacker, target, true);
        attacker->useSkill(playerSelectedSkillIndex, target);
        processPostActionBuffs(attacker, target, true);
        playerSelectedSkillIndex = -1;
        playerSelectedTargetIndex = -1;
    } else {
        // 其他类型走 executeSkill
        QString skillDesc = QString::fromStdString(skill->getdescription());
        executeSkill(attacker, defender, skill, targetType, skillName, skillDesc, true);
    }
}

// 处理敌人行动
void BattleEngine::handleEnemyAction(Character* attacker, Character* defender) {
    if (!enemySelectedSkill) {
        // 普通攻击
        if (!defender || !defender->alive()) return;
        performNormalAttack(attacker, defender, false);
        return;
    }

    // 检查是否是普攻
    std::vector<Skill*> skills = attacker->getSkills();
    int skillIndex = -1;
    for (size_t i = 0; i < skills.size(); ++i) {
        if (skills[i] == enemySelectedSkill) {
            skillIndex = static_cast<int>(i);
            break;
        }
    }

    if (skillIndex == 0) {
        // 普通攻击
        if (!defender || !defender->alive()) return;
        performNormalAttack(attacker, defender, false);
    } else {
        // 技能攻击
        handleEnemySkill(attacker, defender);
    }
}

// 处理敌人技能
void BattleEngine::handleEnemySkill(Character* attacker, Character* defender) {
    QString skillName = QString::fromStdString(enemySelectedSkill->getname());
    QString skillDesc = QString::fromStdString(enemySelectedSkill->getdescription());

    // 根据技能名称确定目标类型
    SkillTargetType targetType = getSkillTargetType(skillName);

    // 执行技能
    executeSkill(attacker, defender, enemySelectedSkill, targetType, skillName, skillDesc, false);
}

// 执行技能
void BattleEngine::executeSkill(Character* attacker, Character* defender, Skill* skill,
                                SkillTargetType targetType, const QString& skillName,
                                const QString& skillDesc, bool isPlayer) {
    switch (targetType) {
    case SkillTargetType::SELF: {
        // 对自己释放的技能
        addLog(QString("【%1】对自己释放了 [%2]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(skillName));
        addLog(QString("  技能效果：%1").arg(skillDesc));

        processPreActionBuffs(attacker, attacker, isPlayer);
        attacker->useSkill(playerSelectedSkillIndex, attacker,false);
        processPostActionBuffs(attacker, attacker, isPlayer);
        break;
    }
    case SkillTargetType::ALLY_ALL: {
        // 全体友方技能
        addLog(QString("【%1】对全体友方释放了 [%2]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(skillName));
        addLog(QString("  技能效果：%1").arg(skillDesc));

        bool i=true;
        std::vector<Character*> team=playerTeam;
        for (Character* ally : team) {
            if (ally && ally->alive()) {
                processPreActionBuffs(attacker, ally, isPlayer);
                if (i){
                    attacker->useSkill(playerSelectedSkillIndex, ally,false);
                    i=false;
                }
                else{
                    attacker->useSkill(playerSelectedSkillIndex, ally,true);
                }
                processPostActionBuffs(attacker, ally, isPlayer);
            }
        }
        break;
    }
    case SkillTargetType::ENEMY_ALL: {
        // 全体敌方技能
        addLog(QString("【%1】对全体敌方释放了 [%2]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(skillName));
        addLog(QString("  技能效果：%1").arg(skillDesc));

        // 遍历所有存活的敌方角色
        std::vector<Character*>& team = isPlayer ? enemyTeam : playerTeam;
        bool i=true;
        for (Character* enemy : team) {
            if (enemy && enemy->alive()) {
                processPreActionBuffs(attacker, enemy, isPlayer);
                if (i){
                    attacker->useSkill(playerSelectedSkillIndex, enemy,false);
                    i=false;
                }
                else{
                    attacker->useSkill(playerSelectedSkillIndex, enemy,true);
                }
                processPostActionBuffs(attacker, enemy, isPlayer);
            }
        }
        break;
    }
    case SkillTargetType::ALLY_SINGLE: {
        // 随机友方技能（命运骰子）
        if (skillName == "命运骰子") {
            // 随机选择一个存活的友方角色
            std::vector<Character*>& team = isPlayer ? playerTeam : enemyTeam;
            std::vector<Character*> aliveAllies;
            for (Character* ally : team) {
                if (ally && ally->alive()) {
                    aliveAllies.push_back(ally);
                }
            }

            if (!aliveAllies.empty()) {
                int randomIndex = std::rand() % aliveAllies.size();
                Character* target = aliveAllies[randomIndex];

                addLog(QString("【%1】对 %2 释放了 [%3]")
                           .arg(QString::fromStdString(attacker->getName()))
                           .arg(QString::fromStdString(target->getName()))
                           .arg(skillName));
                addLog(QString("  技能效果：%1").arg(skillDesc));

                processPreActionBuffs(attacker, target, isPlayer);

                // 特殊处理：直接恢复满血
                int maxHealth = target->getMaxHealth() + target->gethealthadd();
                target->setHealth(maxHealth);
                addLog(QString("  %1 的生命值已完全恢复！")
                           .arg(QString::fromStdString(target->getName())));

                processPostActionBuffs(attacker, target, isPlayer);
            }
        } else {
            // 其他随机友方技能，默认对自己
            addLog(QString("【%1】对自己释放了 [%2]")
                       .arg(QString::fromStdString(attacker->getName()))
                       .arg(skillName));
            addLog(QString("  技能效果：%1").arg(skillDesc));

            processPreActionBuffs(attacker, attacker, isPlayer);
            attacker->useSkill(playerSelectedSkillIndex, attacker,false);
            processPostActionBuffs(attacker, attacker, isPlayer);
        }
        break;
    }
    case SkillTargetType::ENEMY_FRONT: {
        // 敌方前台技能
        if (!defender || !defender->alive()) return;

        addLog(QString("【%1】对 %2 释放了 [%3]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(QString::fromStdString(defender->getName()))
                   .arg(skillName));
        addLog(QString("  技能效果：%1").arg(skillDesc));

        processPreActionBuffs(attacker, defender, isPlayer);
        attacker->useSkill(playerSelectedSkillIndex, defender,false);
        processPostActionBuffs(attacker, defender, isPlayer);
        break;
    }
    default: {
        // 默认：敌方前台
        if (!defender || !defender->alive()) return;

        addLog(QString("【%1】释放了 [%2]")
                   .arg(QString::fromStdString(attacker->getName()))
                   .arg(skillName));
        addLog(QString("  技能效果：%1").arg(skillDesc));

        processPreActionBuffs(attacker, defender, isPlayer);
        attacker->useSkill(playerSelectedSkillIndex, defender,false);
        processPostActionBuffs(attacker, defender, isPlayer);
        break;
    }
    }
}

SkillTargetType BattleEngine::getSkillTargetType(const QString& skillName) {
    // ===== 友方全体技能 =====
    static const QStringList allyAllSkills = {
        "治愈术", "灵魂汲取", "元素护盾", "精准连射", "背刺"
    };

    // ===== 指定友方单体技能（弹出选择窗口） =====
    static const QStringList allySingleSelectSkills = {
        "星光祝福", "疾风步", "月之守护", "王权", "统御",
        "法力潮汐", "风之庇护", "光明治愈"
    };

    // ===== 随机友方单体技能 =====
    static const QStringList randomAllySkills = {"命运骰子"};

    // ===== 敌方全体技能 =====
    static const QStringList enemyAllSkills = {"冰火两重天", "黑暗箭雨"};

    // ===== 自身buff技能（排除已移出的） =====
    static const QStringList selfBuffSkills = {
        "暗影步", "神圣净化", "重击", "狂暴", "盾击", "铁壁",
        "烈焰护体", "精准射击", "鹰眼", "嗜血狂暴", "无畏冲锋",
        "守护誓言", "不屈意志", "休养", "王者祝福",
        "疾风射击", "王之意志", "精灵之眼","防御反击","治疗","buff"
    };

    // ===== 敌方前台debuff技能 =====
    static const QStringList enemyFrontDebuffSkills = {
        "冰锥术", "火球术", "毒箭", "时光停滞", "暗影诅咒", "混沌波动"
    };

    if (allyAllSkills.contains(skillName)) {
        return SkillTargetType::ALLY_ALL;
    } else if (allySingleSelectSkills.contains(skillName)) {
        return SkillTargetType::ALLY_SINGLE_SELECT;
    } else if (randomAllySkills.contains(skillName)) {
        return SkillTargetType::ALLY_SINGLE;
    } else if (enemyAllSkills.contains(skillName)) {
        return SkillTargetType::ENEMY_ALL;
    } else if (selfBuffSkills.contains(skillName)) {
        return SkillTargetType::SELF;
    } else if (enemyFrontDebuffSkills.contains(skillName)) {
        return SkillTargetType::ENEMY_FRONT;
    } else {
        return SkillTargetType::DEFAULT; // 默认敌方前台
    }
}

void BattleEngine::handleDeathAndSwap() {
    if (!playerTeam.empty() && !playerTeam[0]->alive()) {
        for (size_t i = 1; i < playerTeam.size(); ++i) {
            if (playerTeam[i]->alive()) {
                std::swap(playerTeam[0], playerTeam[i]);
                break;
            }
        }
    }
    if (!enemyTeam.empty() && !enemyTeam[0]->alive()) {
        for (size_t i = 1; i < enemyTeam.size(); ++i) {
            if (enemyTeam[i]->alive()) {
                std::swap(enemyTeam[0], enemyTeam[i]);
                break;
            }
        }
    }
}

bool BattleEngine::checkBattleEnd() {
    bool pAlive = false; for (auto c : playerTeam) if (c->alive()) pAlive = true;
    bool eAlive = false; for (auto e : enemyTeam) if (e->alive()) eAlive = true;
    return !pAlive || !eAlive;
}
bool BattleEngine::isPlayerWin() const {
    for (auto c : playerTeam) if (c->alive()) return true;
    return false;
}

// 处理攻击前的buff效果
void BattleEngine::processPreActionBuffs(Character* attacker, Character* defender, bool isPlayer) {
    // 处理攻击者的增益buff
    auto attackerBuffs = attacker->getbuffs();
    for (const auto& buff : attackerBuffs) {
        switch (buff.buff_has) {
        case Status::attackAdd:
            addLog(QString("  【攻击加成】%1 攻击力增加 %2").arg(QString::fromStdString(attacker->getName())).arg(buff.buffvalue));
            break;
        case Status::attackDec:
            addLog(QString("  【攻击减少】%1 攻击力减少 %2").arg(QString::fromStdString(attacker->getName())).arg(buff.buffvalue));
            break;
        case Status::critrateAdd:
            addLog(QString("  【暴击加成】%1 暴击率增加 %2%").arg(QString::fromStdString(attacker->getName())).arg(buff.buffvalue));
            break;
        case Status::critdamageAdd:
            addLog(QString("  【暴伤加成】%1 暴击伤害增加 %2%").arg(QString::fromStdString(attacker->getName())).arg(buff.buffvalue));
            break;
        case Status::Penetration:
            addLog(QString("  【穿透效果】%1 无视部分防御").arg(QString::fromStdString(attacker->getName())));
            break;
        case Status::fastchange:
            addLog(QString("  【速切效果】%1 可以快速切换技能").arg(QString::fromStdString(attacker->getName())));
            break;
        case Status::fixed:
            addLog(QString("  【不耗魔法】%1 技能不消耗魔法值").arg(QString::fromStdString(attacker->getName())));
            break;
        case Status::LifeSteal:
            addLog(QString("  【吸血效果】%1 攻击时回复生命").arg(QString::fromStdString(attacker->getName())));
            break;
        default:
            break;
        }
    }

    // 处理防御者的防御buff
    auto defenderBuffs = defender->getbuffs();
    for (const auto& buff : defenderBuffs) {
        switch (buff.buff_has) {
        case Status::defenseAdd:
            addLog(QString("  【防御加成】%1 防御力增加 %2").arg(QString::fromStdString(defender->getName())).arg(buff.buffvalue));
            break;
        case Status::defenseDec:
            addLog(QString("  【防御减少】%1 防御力减少 %2").arg(QString::fromStdString(defender->getName())).arg(buff.buffvalue));
            break;
        case Status::missrateAdd:
            addLog(QString("  【闪避加成】%1 闪避率增加 %2%").arg(QString::fromStdString(defender->getName())).arg(buff.buffvalue));
            break;
        case Status::Invincible:
            addLog(QString("  【无敌状态】%1 处于无敌状态").arg(QString::fromStdString(defender->getName())));
            break;
        case Status::Paralyzed:
            addLog(QString("  【麻痹状态】%1 被麻痹，可能无法行动").arg(QString::fromStdString(defender->getName())));
            break;
        case Status::Asleep:
            addLog(QString("  【睡眠状态】%1 正在睡眠，无法行动").arg(QString::fromStdString(defender->getName())));
            break;
        case Status::Defend:
            addLog(QString("  【防御姿态】%1 处于防御姿态，减伤50%").arg(QString::fromStdString(defender->getName())));
            break;
        default:
            break;
        }
    }
}

// 处理攻击后的buff效果
void BattleEngine::processPostActionBuffs(Character* attacker, Character* defender, bool isPlayer) {
    auto defenderBuffs = defender->getbuffs();
    for (const auto& buff : defenderBuffs) {
        switch (buff.buff_has) {
        case Status::Poisoned:
            addLog(QString("  【中毒效果】%1 受到中毒伤害 %2").arg(QString::fromStdString(defender->getName())).arg(buff.buffvalue));
            break;
        default:
            break;
        }
    }

    auto attackerBuffs = attacker->getbuffs();
    for (const auto& buff : attackerBuffs) {
        switch (buff.buff_has) {
        case Status::LifeSteal:
            addLog(QString("  【吸血效果】%1 回复生命值 %2").arg(QString::fromStdString(attacker->getName())).arg(buff.buffvalue));
            break;
        default:
            break;
        }
    }
}

// 新增函数：处理回合结束时的buff效果
void BattleEngine::processRoundEndBuffs(Character* character, bool isPlayer) {
    if (!character || !character->alive()) return;

    auto buffs = character->getbuffs();
    QString charName = QString::fromStdString(character->getName());

    for (const auto& buff : buffs) {
        switch (buff.buff_has) {
        case Status::Poisoned:
            addLog(QString("【回合结束】%1 受到中毒伤害 %2").arg(charName).arg(buff.buffvalue));
            break;
        default:
            break;
        }
    }

    // 记录buff持续时间
    if (!buffs.empty()) {
        addLog(QString("【状态更新】%1 剩余buff数量：%2").arg(charName).arg(buffs.size()));
    }
}

// 处理普通攻击
void BattleEngine::performNormalAttack(Character* attacker, Character* defender, bool isPlayer) {
    if (!attacker || !attacker->alive() || !defender || !defender->alive()) {
        addLog(QString("攻击失败：目标不存在或已死亡"));
        return;
    }

    addLog(QString("【%1】对 %2 进行了普通攻击")
               .arg(QString::fromStdString(attacker->getName()))
               .arg(QString::fromStdString(defender->getName())));

    // 处理攻击前的buff效果
    processPreActionBuffs(attacker, defender, isPlayer);

    // 计算普通攻击伤害
    int damage = calculateNormalAttackDamage(attacker, defender);

    // 应用伤害
    if (damage > 0) {
        int counterDamage = defender->takeDamage(damage, attacker, false);

        addLog(QString("  %1 受到了 %2 点伤害")
                   .arg(QString::fromStdString(defender->getName()))
                   .arg(damage));

        // 如果有反击，输出日志
        if (counterDamage > 0) {
            addLog(QString("  【防御反击】%1 反击了 %2，造成 %3 点伤害")
                       .arg(QString::fromStdString(defender->getName()))
                       .arg(QString::fromStdString(attacker->getName()))
                       .arg(counterDamage));
        }

        // 触发武器普通攻击效果
        triggerWeaponAttackEffects(attacker, defender, isPlayer, damage);

        // 触发协同攻击
        if (defender->alive()) {
            triggerSupportArcherAttack(attacker, defender, isPlayer);
        }
    } else {
        addLog(QString("  %1 的攻击被闪避了")
                   .arg(QString::fromStdString(attacker->getName())));
    }

    // 处理攻击后的buff效果
    processPostActionBuffs(attacker, defender, isPlayer);
}

void BattleEngine::endPlayerTurnAfterSwitch() {
    // 记录日志
    addLog(QString("【战术换人】%1 被替换上场，结束当前回合。")
               .arg(QString::fromStdString(getPlayerFront() ? getPlayerFront()->getName() : "未知角色")));

    // 直接进入回合结束流程
    playerSelectedSkillIndex = -2;  // 使用特殊值标记是切换角色结束回合
    currentState = BattleState::ENEMY_ACTION;

    // 更新状态，进入敌人回合
    updateState();
}

BattleEngine::~BattleEngine() {
    // 清理敌人对象
    for (Character* enemy : enemyTeam) {
        if (enemy) {
            delete enemy;
        }
    }
    enemyTeam.clear();

    // 注意：playerTeam 中的角色对象由 prepare 管理，这里不删除

    // 清理战斗日志
    battleLogs.clear();
}
void BattleEngine::triggerWeaponAttackEffects(Character* attacker, Character* defender,
                                              bool isPlayer, int damageDealt) {
    if (!attacker || !defender) return;

    // 检查攻击者是否有武器特殊效果
    if (!attacker->getWeapon()) return;
    if (!attacker->getWeapon()->hasSpecialEffects()) return;

    buffSkill* effect = attacker->getWeapon()->getSpecialEffects();

    Status temp=effect->getbuff();
    int time=effect->gettime();

        // 根据效果名称判断触发条件
        if (temp == Status::Poisoned) {
            // 中毒效果
            defender->addbuff(buff(temp, time, static_cast<int>(damageDealt * 0.15)));
            addLog(QString("  【武器特效】%1 的武器使目标中毒，每回合受到 %2 点伤害")
                       .arg(QString::fromStdString(attacker->getName()))
                       .arg(static_cast<int>(damageDealt * 0.15)));
        }
        else if (temp == Status::Paralyzed) {
            // 麻痹效果
            defender->addbuff(buff(temp, time, 0));
            addLog(QString("  【武器特效】%1 的武器使目标麻痹，有50%概率无法行动")
                       .arg(QString::fromStdString(attacker->getName())));
        }
        else if (temp == Status::LifeSteal) {
            // 吸血效果
            int healAmount = static_cast<int>(damageDealt * 0.3);
            attacker->heal(healAmount);
            addLog(QString("  【武器特效】%1 的武器触发吸血，恢复 %2 点生命值")
                       .arg(QString::fromStdString(attacker->getName()))
                       .arg(healAmount));
        }
        else if (temp == Status::shieldAdd) {
            // 护盾效果
            int shieldAmount = static_cast<int>(attacker->getMaxHealth() * 0.3);
            attacker->addbuff(buff(temp, time, shieldAmount));
            addLog(QString("  【武器特效】%1 的武器提供护盾，获得 %2 点护盾")
                       .arg(QString::fromStdString(attacker->getName()))
                       .arg(shieldAmount));
        }
        else if (temp == Status::Penetration) {
            // 穿透效果
            attacker->addbuff(buff(temp, time, 1.0f));
            addLog(QString("  【武器特效】%1 的武器提供穿透效果，造成两倍伤害")
                       .arg(QString::fromStdString(attacker->getName())));
        }
        else if (temp == Status::Defend) {
            // 反击效果
            attacker->addbuff(buff(temp, time, 0));
            addLog(QString("  【武器特效】%1 的武器进入防御姿态，减伤50%，受到攻击会反击")
                       .arg(QString::fromStdString(attacker->getName())));
        }
}

void BattleEngine::submitPlayerActionWithTarget(int skillIndex, int targetIndex) {
    if (currentState == BattleState::DECISION_PHASE) {
        playerSelectedSkillIndex = skillIndex;
        playerSelectedTargetIndex = targetIndex;  // 记录目标索引

        // 敌人 AI
        if (!enemyTeam.empty() && enemyTeam[0]->alive()) {
            Enemy* enemyPtr = static_cast<Enemy*>(enemyTeam[0]);
            enemySelectedSkill = enemyPtr->AIStrategy();
        }
        currentState = isPlayerFirst ? BattleState::PLAYER_ACTION : BattleState::ENEMY_ACTION;
        updateState();
    }
}

bool BattleEngine::switchCharacterTo(int targetIndex) {
    if (playerTeam.size() < 2) return false;
    if (targetIndex <= 0 || targetIndex >= (int)playerTeam.size()) return false;
    Character* target = playerTeam[targetIndex];
    if (!target || !target->alive()) return false;

    // 简单交换：前排与目标角色互换位置
    std::swap(playerTeam[0], playerTeam[targetIndex]);

    addLog(QString("【战术换人】%1 被替换上场！")
               .arg(QString::fromStdString(playerTeam[0]->getName())));

    // 检查速切buff
    bool hasFastChange = false;
    auto buffs = playerTeam[0]->getbuffs();
    for (const auto& buff : buffs) {
        if (buff.buff_has == Status::fastchange) {
            hasFastChange = true;
            break;
        }
    }
    if (hasFastChange) {
        addLog("速切效果生效：切换角色不消耗回合");
    } else {
        addLog("正常换人：切换角色消耗当前回合");
    }
    return true;
}