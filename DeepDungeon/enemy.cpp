#include "enemy_config.h"
#include "character_config.h"
#include <random>

// 构造函数
Enemy::Enemy(const std::string& name,int level,CharacterClass job,EnemyType entype)
    : Character(name, job, level, 0.2f, 1.0f, 0, 0,true, false),
    entype(entype){}

//敌方剑士（高血高防、攻击中等）
Warrior_enemy::Warrior_enemy(const std::string& name, EnemyType entype,int level):
    Enemy::Enemy(name,level,CharacterClass::Warrior,entype){
    if(entype==EnemyType::Normal){
        if(level==1){ maxHealth=280; maxmana=100; attack=40; defense=20; }
        else if(level==2){ maxHealth=320; maxmana=140; attack=60; defense=40; }
        else if(level==3){ maxHealth=400; maxmana=200; attack=90; defense=60; }
    }
    else if(entype==EnemyType::Elite){
        if(level==1){ maxHealth=360; maxmana=100; attack=60; defense=30; }
        else if(level==2){ maxHealth=480; maxmana=140; attack=90; defense=50; }
        else if(level==3){ maxHealth=500; maxmana=200; attack=100; defense=70; }
    }
    else if(entype==EnemyType::Boss){
        if(level==1){ maxHealth=600; maxmana=140; attack=100; defense=70; }
        else if(level==2){ maxHealth=800; maxmana=200; attack=120; defense=90; }
        else if(level==3){ maxHealth=1200; maxmana=300; attack=150; defense=120; }
    }
    if(level==1){ critrate=0.2; critdamage=1.2; missrate=0.1; }
    else if(level==2){ critrate=0.4; critdamage=1.4; missrate=0.2; }
    else { critrate=0.6; critdamage=1.6; missrate=0.3; }
    mana=maxmana;
    health=maxHealth;
}

// 敌方法师（高爆发、防御极低、血量薄）
Mage_enemy::Mage_enemy(const std::string& name, EnemyType entype,int level):
    Enemy::Enemy(name,level,CharacterClass::Mage,entype){
    if(entype==EnemyType::Normal){
        if(level==1){ maxHealth=180; maxmana=200; attack=60; defense=10; }
        else if(level==2){ maxHealth=210; maxmana=240; attack=80; defense=30; }
        else if(level==3){ maxHealth=300; maxmana=300; attack=100; defense=50; }
    }
    else if(entype==EnemyType::Elite){
        if(level==1){ maxHealth=240; maxmana=200; attack=100; defense=20; }
        else if(level==2){ maxHealth=300; maxmana=240; attack=120; defense=40; }
        else if(level==3){ maxHealth=400; maxmana=300; attack=140; defense=60; }
    }
    else if(entype==EnemyType::Boss){
        if(level==1){ maxHealth=500; maxmana=240; attack=140; defense=40; }
        else if(level==2){ maxHealth=700; maxmana=300; attack=160; defense=60; }
        else if(level==3){ maxHealth=1000; maxmana=400; attack=200; defense=90; }
    }
    if(level==1){ critrate=0.2; critdamage=1.2; missrate=0.1; }
    else if(level==2){ critrate=0.4; critdamage=1.4; missrate=0.2; }
    else { critrate=0.6; critdamage=1.6; missrate=0.3; }
    health=maxHealth;
    mana=maxmana;
}

// 敌方弓箭手（高输出、防御低、血量中下）
Archer_enemy::Archer_enemy(const std::string& name, EnemyType entype,int level):
    Enemy::Enemy(name,level,CharacterClass::Archer,entype){
    if(entype==EnemyType::Normal){
        if(level==1){ maxHealth=180; maxmana=100; attack=60; defense=5; }
        else if(level==2){ maxHealth=210; maxmana=140; attack=80; defense=25; }
        else if(level==3){ maxHealth=300; maxmana=200; attack=100; defense=40; }
    }
    else if(entype==EnemyType::Elite){
        if(level==1){ maxHealth=240; maxmana=100; attack=100; defense=10; }
        else if(level==2){ maxHealth=300; maxmana=140; attack=120; defense=30; }
        else if(level==3){ maxHealth=400; maxmana=200; attack=140; defense=50; }
    }
    else if(entype==EnemyType::Boss){
        if(level==1){ maxHealth=500; maxmana=140; attack=140; defense=30; }
        else if(level==2){ maxHealth=700; maxmana=200; attack=160; defense=50; }
        else if(level==3){ maxHealth=1000; maxmana=300; attack=200; defense=80; }
    }
    if(level==1){ critrate=0.2; critdamage=1.2; missrate=0.1; }
    else if(level==2){ critrate=0.4; critdamage=1.4; missrate=0.2; }
    else { critrate=0.6; critdamage=1.6; missrate=0.3; }
    mana=maxmana;
    health=maxHealth;
}

// 敌方无名之人（均衡）
Nonamer_enemy::Nonamer_enemy(const std::string& name, EnemyType entype,int level):
    Enemy::Enemy(name,level,CharacterClass::Nonamer,entype){
    if(entype==EnemyType::Normal){
        if(level==1){ maxHealth=220; maxmana=200; attack=50; defense=10; }
        else if(level==2){ maxHealth=250; maxmana=240; attack=60; defense=30; }
        else if(level==3){ maxHealth=350; maxmana=300; attack=80; defense=50; }
    }
    else if(entype==EnemyType::Elite){
        if(level==1){ maxHealth=300; maxmana=200; attack=80; defense=25; }
        else if(level==2){ maxHealth=350; maxmana=240; attack=100; defense=40; }
        else if(level==3){ maxHealth=450; maxmana=300; attack=120; defense=60; }
    }
    else if(entype==EnemyType::Boss){
        if(level==1){ maxHealth=550; maxmana=240; attack=120; defense=40; }
        else if(level==2){ maxHealth=750; maxmana=300; attack=140; defense=60; }
        else if(level==3){ maxHealth=1100; maxmana=400; attack=180; defense=90; }
    }
    if(level==1){ critrate=0.2; critdamage=1.2; missrate=0.1; }
    else if(level==2){ critrate=0.4; critdamage=1.4; missrate=0.2; }
    else { critrate=0.6; critdamage=1.6; missrate=0.3; }
    mana=maxmana;
    health=maxHealth;
}

// AI策略
Skill* Enemy::AIStrategy() {
    // 血量低于50%,50%概率治疗
    if (getHealth() < getMaxHealth() * 0.5f) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        float randomValue = dis(gen);
        if(randomValue<=0.5){
            for (int i = 0; i < (int)skills.size(); ++i) {
                if (skills[i]->getname() == "治疗"&&skills[i]->getmanacost()<mana+manaadd) {
                    return skills[i];
                }
            }
        }
    }

    // 如果血量低于50%,50%概率防御
    if (getHealth() < getMaxHealth() * 0.5f) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0.0, 1.0);
        float randomValue = dis(gen);
        if(randomValue<=0.5){
            for (int i = 0; i < (int)skills.size(); ++i) {
                if (skills[i]->getname() == "防御反击"&&skills[i]->getmanacost()<mana+manaadd) {
                    return skills[i];
                }
            }
        }
    }

    //50%概率上buff
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    float randomValue = dis(gen);
    if(randomValue<=0.5){
        for (int i = 0; i < (int)skills.size(); ++i) {
            if (skills[i]->getname() == "buff"&&skills[i]->getmanacost()<mana+manaadd) {
                return skills[i];
            }
        }
    }

    //都不行则普通攻击
    if (!skills.empty()) return skills[0];
    return nullptr;
}

// 剑士类敌人实现
BerserkerWarrior::BerserkerWarrior(const std::string& name, EnemyType entype, int level)
    : Warrior_enemy(name, entype, level) {

    // 治疗技能：狂怒治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：狂战士之怒
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：狂暴之力
    buffSkill* buff = new buffSkill(3, "buff", Status::attackAdd, 3, 0.3f,
                                    "消耗20点法力，提升自身30%攻击力，持续3回合", 20);
    skills.push_back(buff);
}

IronGuard::IronGuard(const std::string& name, EnemyType entype, int level)
    : Warrior_enemy(name, entype, level) {

    // 治疗技能：钢铁意志
    healSkill* heal = new healSkill(1, "治疗", 30, 0.3f, "消耗30点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：钢铁壁垒
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态2回合，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：钢铁守护
    buffSkill* buff = new buffSkill(3, "buff", Status::defenseAdd, 3, 0.3f,
                                    "消耗20点法力，提升自身30%防御力，持续3回合", 20);
    skills.push_back(buff);
}

BloodBlade::BloodBlade(const std::string& name, EnemyType entype, int level)
    : Warrior_enemy(name, entype, level) {

    // 治疗技能：嗜血恢复
    healSkill* heal = new healSkill(1, "治疗", 30, 0.2f, "消耗30点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：鲜血壁垒
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：嗜血狂怒
    buffSkill* buff = new buffSkill(3, "buff", Status::LifeSteal, 2, 0.3f,
                                    "消耗30点法力，自身获得2回合30%吸血效果", 30);
    skills.push_back(buff);
}

Breaker::Breaker(const std::string& name, EnemyType entype, int level)
    : Warrior_enemy(name, entype, level) {

    // 治疗技能：坚韧恢复
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：不动如山
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：破甲之势
    buffSkill* buff = new buffSkill(3, "buff", Status::Penetration, 2, 0,
                                    "消耗30点法力，自身获得2回合穿透效果", 30);
    skills.push_back(buff);
}

SwordMaster::SwordMaster(const std::string& name, EnemyType entype, int level)
    : Warrior_enemy(name, entype, level) {

    // 治疗技能：宗师冥想
    healSkill* heal = new healSkill(1, "治疗", 30, 0.3f, "消耗30点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：剑心通明
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：剑意爆发
    buffSkill* buff = new buffSkill(3, "buff", Status::critdamageAdd, 3, 0.3f,
                                    "消耗20点法力，提升自身30%暴击伤害，持续3回合", 20);
    skills.push_back(buff);
}

// 法师类敌人实现
FireMage::FireMage(const std::string& name, EnemyType entype, int level)
    : Mage_enemy(name, entype, level) {

    // 治疗技能：火焰治疗
    healSkill* heal = new healSkill(1, "治疗", 30, 0.2f, "消耗30点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：火焰护盾
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：火焰强化
    buffSkill* buff = new buffSkill(3, "buff", Status::manaAdd, 2, 0.3f,
                                    "回复自身30%法力，持续2回合", 0);
    skills.push_back(buff);
}

FrostMage::FrostMage(const std::string& name, EnemyType entype, int level)
    : Mage_enemy(name, entype, level) {

    // 治疗技能：冰霜治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：冰霜屏障
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：冰霜减速
    buffSkill* buff = new buffSkill(3, "buff", Status::missrateAdd, 2, 0.3f,
                                    "消耗20点法力，提升自身30%闪避率，持续2回合", 20);
    skills.push_back(buff);
}

ShadowMage::ShadowMage(const std::string& name, EnemyType entype, int level)
    : Mage_enemy(name, entype, level) {

    // 治疗技能：暗影汲取
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：暗影护盾
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：暗影诅咒
    buffSkill* buff = new buffSkill(3, "buff", Status::attackAdd, 2, 0.3f,
                                    "消耗20点法力，提升自身30%攻击力，持续2回合", 20);
    skills.push_back(buff);
}

ElementalMage::ElementalMage(const std::string& name, EnemyType entype, int level)
    : Mage_enemy(name, entype, level) {

    // 治疗技能：元素治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：元素护盾
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：元素共鸣
    buffSkill* buff = new buffSkill(3, "buff", Status::healthAdd, 2, 0.3f,
                                    "消耗30点法力，提升自身30%生命上限，持续2回合", 30);
    skills.push_back(buff);
}

Archmage::Archmage(const std::string& name, EnemyType entype, int level)
    : Mage_enemy(name, entype, level) {

    // 治疗技能：奥术治疗
    healSkill* heal = new healSkill(1, "治疗", 50, 0.5f, "消耗50点法力，恢复自身50%生命值");
    skills.push_back(heal);

    // 防御反击技能：奥术屏障
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗50点法力，自身进入防御状态，减伤50%并反击", 50);
    skills.push_back(defense);

    // buff技能：奥术智慧
    buffSkill* buff = new buffSkill(3, "buff", Status::manaAdd, 2, 0.4f,
                                    "提升自身40%法力上限，持续2回合", 0);
    skills.push_back(buff);
}

// 弓箭手类敌人实现
Sharpshooter::Sharpshooter(const std::string& name, EnemyType entype, int level)
    : Archer_enemy(name, entype, level) {

    // 治疗技能：精准治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：灵活闪避
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：精准瞄准
    buffSkill* buff = new buffSkill(3, "buff", Status::critrateAdd, 2, 0.3f,
                                    "消耗20点法力，提升自身30%暴击率，持续2回合", 20);
    skills.push_back(buff);
}

Ranger::Ranger(const std::string& name, EnemyType entype, int level)
    : Archer_enemy(name, entype, level) {

    // 治疗技能：自然治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：游侠防御
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：猎人印记
    buffSkill* buff = new buffSkill(3, "buff", Status::missrateAdd, 2, 0.2f,
                                    "消耗20点法力，提升自身20%闪避率，持续2回合", 20);
    skills.push_back(buff);
}

Sniper::Sniper(const std::string& name, EnemyType entype, int level)
    : Archer_enemy(name, entype, level) {

    // 治疗技能：快速包扎
    healSkill* heal = new healSkill(1, "治疗", 30, 0.3f, "消耗30点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：狙击姿态
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：狙击专注
    buffSkill* buff = new buffSkill(3, "buff", Status::critdamageAdd, 2, 0.4f,
                                    "消耗30点法力，提升自身40%暴击伤害，持续2回合", 30);
    skills.push_back(buff);
}

Hunter::Hunter(const std::string& name, EnemyType entype, int level)
    : Archer_enemy(name, entype, level) {

    // 治疗技能：猎手治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身15%生命值");
    skills.push_back(heal);

    // 防御反击技能：猎人防御
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：追踪标记
    buffSkill* buff = new buffSkill(3, "buff", Status::defenseAdd, 2, 0.3f,
                                    "消耗30点法力，增加自己30%防御力，持续2回合", 30);
    skills.push_back(buff);
}

ArrowGod::ArrowGod(const std::string& name, EnemyType entype, int level)
    : Archer_enemy(name, entype, level) {

    // 治疗技能：神佑治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.2f, "消耗20点法力，恢复自身20%生命值");
    skills.push_back(heal);

    // 防御反击技能：神之庇护
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗20点法力，自身进入防御状态，减伤50%并反击", 20);
    skills.push_back(defense);

    // buff技能：箭神祝福
    buffSkill* buff = new buffSkill(3, "buff", Status::attackAdd, 3, 0.4f,
                                    "消耗40点法力，提升自身40%攻击力，持续3回合", 40);
    skills.push_back(buff);
}

// 无名之人类敌人实现
ShadowWalker::ShadowWalker(const std::string& name, EnemyType entype, int level)
    : Nonamer_enemy(name, entype, level) {

    // 治疗技能：暗影恢复
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗25点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：暗影闪避
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：暗影强化
    buffSkill* buff = new buffSkill(3, "buff", Status::missrateAdd, 2, 0.3f,
                                    "消耗20点法力，提升自身30%闪避率，持续2回合", 20);
    skills.push_back(buff);
}

ChaosApostle::ChaosApostle(const std::string& name, EnemyType entype, int level)
    : Nonamer_enemy(name, entype, level) {

    // 治疗技能：混沌治疗
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：混沌护盾
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：混沌加持
    buffSkill* buff = new buffSkill(3, "buff", Status::manaAdd, 2, 0.3f,
                                    "消耗20点法力，增加自身30%法力值，持续2回合", 20);
    skills.push_back(buff);
}

VoidWalker::VoidWalker(const std::string& name, EnemyType entype, int level)
    : Nonamer_enemy(name, entype, level) {

    // 治疗技能：虚空汲取
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：虚空屏障
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：虚空之力
    buffSkill* buff = new buffSkill(3, "buff", Status::shieldAdd, 2, 0.3f,
                                    "消耗30点法力，自身获得30%生命值的护盾，持续2回合", 30);
    skills.push_back(buff);
}

Elementalist::Elementalist(const std::string& name, EnemyType entype, int level)
    : Nonamer_enemy(name, entype, level) {

    // 治疗技能：元素恢复
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：元素守护
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：元素平衡
    buffSkill* buff = new buffSkill(3, "buff", Status::healthAdd, 2, 0.2f,
                                    "消耗20点法力，提升自身20%生命上限，持续2回合", 20);
    skills.push_back(buff);
}

VoidKing::VoidKing(const std::string& name, EnemyType entype, int level)
    : Nonamer_enemy(name, entype, level) {

    // 治疗技能：虚无恢复
    healSkill* heal = new healSkill(1, "治疗", 20, 0.3f, "消耗20点法力，恢复自身30%生命值");
    skills.push_back(heal);

    // 防御反击技能：虚无壁垒
    buffSkill* defense = new buffSkill(2, "防御反击", Status::Defend, 2, 0.5f,
                                       "消耗30点法力，自身进入防御状态，减伤50%并反击", 30);
    skills.push_back(defense);

    // buff技能：虚无之力
    buffSkill* buff = new buffSkill(3, "buff", Status::Penetration, 2, 1,
                                    "消耗40点法力，自身获得2回合穿透效果", 40);
    skills.push_back(buff);
}