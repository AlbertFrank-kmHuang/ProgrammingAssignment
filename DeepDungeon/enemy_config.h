#ifndef ENEMY_CONFIG_H
#define ENEMY_CONFIG_H

#include "character_config.h"
#include <string>

//怪物类型
enum class EnemyType{
    Normal,     // 普通怪物
    Elite,      // 精英怪物
    Boss,       // 首领
};

// 回合制敌人类
class Enemy : public Character {
protected:
    EnemyType entype;                    //怪物类型
public:
    Enemy(const std::string& name, int level,CharacterClass job,EnemyType entype);
    virtual ~Enemy() = default;

    // 怪物AI策略
    virtual Skill* AIStrategy();
};
// 具体角色类：剑士
class Warrior_enemy : public Enemy {
public:
    Warrior_enemy(const std::string& name, EnemyType entype,int level = 1);
    // 重写基类方法
    std::string getJobName() const override { return "敌方剑士"; }
};

// 具体角色类：法师
class Mage_enemy : public Enemy {
private:

public:
    Mage_enemy(const std::string& name, EnemyType entype,int level = 1);
    // 重写基类方法
    std::string getJobName() const override { return "敌方法师"; }
};

// 具体角色类：弓箭手
class Archer_enemy : public Enemy {
public:
    Archer_enemy(const std::string& name, EnemyType entype,int level = 1);
    // 重写基类方法
    std::string getJobName() const override { return "敌方弓箭手"; }
};

// 具体角色类：无名之人
class Nonamer_enemy : public Enemy {
public:
    Nonamer_enemy(const std::string& name, EnemyType entype,int level = 1);
    // 重写基类方法
    std::string getJobName() const override { return "敌方无名之人"; }
};

// 剑士类敌人
class BerserkerWarrior : public Warrior_enemy {
public:
    BerserkerWarrior(const std::string& name = "狂暴剑士",
                     EnemyType entype = EnemyType::Normal,
                     int level = 1);
};

class IronGuard : public Warrior_enemy {
public:
    IronGuard(const std::string& name = "钢铁守卫",
              EnemyType entype = EnemyType::Normal,
              int level = 1);
};

class BloodBlade : public Warrior_enemy {
public:
    BloodBlade(const std::string& name = "血刃战士",
               EnemyType entype = EnemyType::Elite,
               int level = 2);
};

class Breaker : public Warrior_enemy {
public:
    Breaker(const std::string& name = "破城者",
            EnemyType entype = EnemyType::Elite,
            int level = 2);
};

class SwordMaster : public Warrior_enemy {
public:
    SwordMaster(const std::string& name = "剑圣",
                EnemyType entype = EnemyType::Boss,
                int level = 3);
};

// 法师类敌人
class FireMage : public Mage_enemy {
public:
    FireMage(const std::string& name = "火焰法师",
             EnemyType entype = EnemyType::Normal,
             int level = 1);
};

class FrostMage : public Mage_enemy {
public:
    FrostMage(const std::string& name = "冰霜法师",
              EnemyType entype = EnemyType::Normal,
              int level = 1);
};

class ShadowMage : public Mage_enemy {
public:
    ShadowMage(const std::string& name = "暗影法师",
               EnemyType entype = EnemyType::Elite,
               int level = 2);
};

class ElementalMage : public Mage_enemy {
public:
    ElementalMage(const std::string& name = "元素法师",
                  EnemyType entype = EnemyType::Elite,
                  int level = 2);
};

class Archmage : public Mage_enemy {
public:
    Archmage(const std::string& name = "大法师",
             EnemyType entype = EnemyType::Boss,
             int level = 3);
};

// 弓箭手类敌人
class Sharpshooter : public Archer_enemy {
public:
    Sharpshooter(const std::string& name = "神射手",
                 EnemyType entype = EnemyType::Normal,
                 int level = 1);
};

class Ranger : public Archer_enemy {
public:
    Ranger(const std::string& name = "游侠",
           EnemyType entype = EnemyType::Normal,
           int level = 1);
};

class Sniper : public Archer_enemy {
public:
    Sniper(const std::string& name = "狙击手",
           EnemyType entype = EnemyType::Elite,
           int level = 2);
};

class Hunter : public Archer_enemy {
public:
    Hunter(const std::string& name = "猎人",
           EnemyType entype = EnemyType::Elite,
           int level = 2);
};

class ArrowGod : public Archer_enemy {
public:
    ArrowGod(const std::string& name = "箭神",
             EnemyType entype = EnemyType::Boss,
             int level = 3);
};

// 无名之人类敌人
class ShadowWalker : public Nonamer_enemy {
public:
    ShadowWalker(const std::string& name = "暗影行者",
                 EnemyType entype = EnemyType::Normal,
                 int level = 1);
};

class ChaosApostle : public Nonamer_enemy {
public:
    ChaosApostle(const std::string& name = "混沌使徒",
                 EnemyType entype = EnemyType::Normal,
                 int level = 1);
};

class VoidWalker : public Nonamer_enemy {
public:
    VoidWalker(const std::string& name = "虚空行者",
               EnemyType entype = EnemyType::Elite,
               int level = 2);
};

class Elementalist : public Nonamer_enemy {
public:
    Elementalist(const std::string& name = "元素使",
                 EnemyType entype = EnemyType::Elite,
                 int level = 2);
};

class VoidKing : public Nonamer_enemy {
public:
    VoidKing(const std::string& name = "虚无之王",
             EnemyType entype = EnemyType::Boss,
             int level = 3);
};


#endif // ENEMY_CONFIG_H
