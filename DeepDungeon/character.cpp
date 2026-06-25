#include "character_config.h"
#include <algorithm>
#include <random>
#include <vector>
#include <sstream>
#include <QJsonArray>
#include <QJsonDocument>

// Character 构造函数
Character::Character(const std::string& name, CharacterClass job, int level,
                     float critrate, float critdamage, float missrate,
                     int shield, bool isalive, bool isFrontline)
    : name(name), job(job), level(level),
    critrate(critrate), critdamage(critdamage), missrate(missrate),
    shield(shield), isalive(isalive), isFrontline(isFrontline),
    turnCount(0), currentSkillIndex(0), equippedWeapon(nullptr), attackadd(0),
    defenseadd(0), critrateadd(0), critdamageadd(0), missrateadd(0), healthadd(0), manaadd(0) {

    // 默认拥有普通攻击
    skills.push_back(new Skill(0, "普通攻击", 0, "进行一次基础物理攻击"));
}

// 析构函数：清理动态分配的技能
Character::~Character() {
    for (auto s : skills) {
        delete s;
    }
    skills.clear();
}

void Character::addbuff(const buff& b) {
    // 检查是否已有相同类型的buff
    bool found = false;
    for (auto& existingBuff : buffs) {
        if (existingBuff.buff_has == b.buff_has) {
            // 更新已存在buff的持续时间和数值
            existingBuff.bufftime = b.bufftime;
            existingBuff.buffvalue = b.buffvalue;
            found = true;
            break;
        }
    }

    if (!found) {
        buffs.push_back(b);
    }

    // 根据buff类型立即应用效果到add变量
    switch (b.buff_has) {
    case Status::attackAdd:
        attackadd += b.buffvalue;
        break;
    case Status::attackDec:
        attackadd -= b.buffvalue;        // 减少攻击力，负的add
        break;
    case Status::defenseAdd:
        defenseadd += b.buffvalue;
        break;
    case Status::defenseDec:
        defenseadd -= b.buffvalue;  // 减少防御力，负的add
        break;
    case Status::healthAdd:
        healthadd += b.buffvalue;
        break;
    case Status::critrateAdd:
        critrateadd += float(b.buffvalue)/100;
        break;
    case Status::critdamageAdd:
        critdamageadd += float(b.buffvalue)/100;
        break;
    case Status::manaAdd:
        manaadd += b.buffvalue;
        break;
    case Status::manaDec:
        manaadd -= b.buffvalue;  // 减少法力，负的add
        break;
    case Status::missrateAdd:
        missrateadd += float(b.buffvalue)/100;
        break;
    case Status::shieldAdd:
        shield += b.buffvalue;
        break;
    // 以下buff不直接影响add变量，但会在特定时机生效
    case Status::Penetration:
    case Status::fastchange:
    case Status::fixed:
    case Status::Defend:
    case Status::Poisoned:
    case Status::Paralyzed:
    case Status::Asleep:
    case Status::Invincible:
    case Status::LifeSteal:
    case Status::Normal:
        // 这些buff不直接影响add变量
        break;
    }
}

int Character::takeDamage(int damage, Character* attacker, bool isCounterAttack) {
    // 检查无敌状态
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::Invincible) {
            return 0;  // 无敌状态免疫所有伤害
        }
    }

    // 检查闪避
    float dodgeChance = missrate + missrateadd;
    if (dodgeChance > 0.0f) {
        // 生成0-100的随机数
        int randomValue = std::rand() % 10000;
        if (randomValue < static_cast<int>(dodgeChance * 10000)) {
            // 闪避成功，不受到伤害
            return 0;
        }
    }

    // 计算实际防御力（基础防御 + 防御add）
    int actualDefense = defense + defenseadd;

    // 检查防御姿态
    float damageReduction = 1.0f;
    bool isDefending = false;
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::Defend) {
            isDefending = true;
            damageReduction = 0.5f;  // 防御姿态减伤50%
            break;
        }
    }

    // 优先扣除护盾
    if (shield > 0) {
        if (shield >= damage) {
            shield -= damage;
            return 0;
        } else {
            damage -= shield;
            shield = 0;
        }
    }

    // 计算最终伤害
    int finalDamage = static_cast<int>(damage * damageReduction) - actualDefense;
    if (finalDamage < 0) finalDamage = 0;
    if (finalDamage-healthadd < 0){
        healthadd -= finalDamage;
        finalDamage = 0;
    }
    else{
        finalDamage -= healthadd;
        healthadd = 0;
    }


    // 记录攻击前的生命值，用于反击判断
    int healthBefore = health;
    health -= finalDamage;

    if (health <= 0) {
        health = 0;
        isalive = false;
    }

    // 防御反击逻辑
    // 如果不是反击攻击，且处于防御状态，且攻击者存在且存活，且自己受到攻击后仍然存活
    if (!isCounterAttack && isDefending && attacker && attacker->alive() && isalive && health > 0) {
        int counterDamage = calculateCounterDamage(attacker);

        if (counterDamage > 0) {
            attacker->takeDamage(counterDamage, this, true);
            return counterDamage;  // 返回反击伤害
        }
    }
    return 0;
}

int Character::calculateCounterDamage(Character* target) {
    if (!target || !target->alive()) return 0;

    // 基础反击伤害（使用普通攻击计算） - 改为100%伤害
    int baseDamage = attack + attackadd;

    // 计算暴击
    float actualCritRate = critrate + critrateadd;
    float actualCritDamage = critdamage + critdamageadd;

    bool isCrit = (std::rand() % 10000) < (actualCritRate * 10000);
    if (isCrit) {
        baseDamage = static_cast<int>(baseDamage * actualCritDamage);
    }

    return baseDamage;
}

int Character::calculateDamage(Character* target) {
    if (!target || !target->alive()) return 0;

    // 基础攻击力（基础值 + add值）
    int baseDamage = attack + attackadd;

    // 计算暴击
    float actualCritRate = critrate + critrateadd;
    float actualCritDamage = critdamage + critdamageadd;

    bool isCrit = (rand() % 100) < (actualCritRate * 100);
    if (isCrit) {
        baseDamage = static_cast<int>(baseDamage * actualCritDamage);
    }

    // 计算穿透效果
    float penetrationMultiplier = 1.0f;
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::Penetration) {
            penetrationMultiplier += bf.buffvalue;  // 穿透增加伤害系数
        }
    }

    baseDamage = static_cast<int>(baseDamage * penetrationMultiplier);

    // 计算吸血效果
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::LifeSteal) {
            // buffvalue 以 ×100 编码（如 30 表示 30%），需 ÷100 还原为比例
            int healAmount = static_cast<int>(baseDamage * (bf.buffvalue / 100.0f));
            heal(healAmount);
        }
    }

    return baseDamage;
}


// 检查角色是否是弓箭手
bool Character::isArcher() const {
    return job == CharacterClass::Archer;
}

// 执行协同攻击
void Character::performSupportAttack(Character* target) {
    if (!target || !target->alive() || !isalive || !isArcher()) {
        return;
    }

    // 计算协同攻击伤害（普通攻击的50%）
    int supportDamage = static_cast<int>(calculateDamage(target) * 0.5f);

    if (supportDamage > 0) {
        // 对目标造成伤害
        target->takeDamage(supportDamage, this, false);
    }
}

// 核心功能：治疗
void Character::heal(int amount) {
    health = std::min(maxHealth + healthadd, health + amount);
}

// 核心功能：升级
void Character::levelUp() {
    if (level < 5) {
        level++;
    }
    isalive = true;
}

// 核心功能：获取职业名称
std::string Character::getJobName() const {
    switch (job) {
    case CharacterClass::Warrior: return "战士";
    case CharacterClass::Mage:    return "法师";
    case CharacterClass::Archer:  return "弓箭手";
    case CharacterClass::Nonamer: return "无名之人";
    default: return "未知职业";
    }
}

std::string Character::getInfo() const {
    std::stringstream ss;
    ss << getJobName() << " " << name << " (等级:" << level << "星)";
    ss << "\n生命: " << health << "/" << maxHealth + healthadd;
    ss << "  攻击: " << attack + attackadd << "  防御: " << defense + defenseadd;
    ss << "  法力: " << mana << "/" << maxmana + manaadd;
    ss << "  暴击率: " << (critrate + critrateadd) * 100 << "%";
    ss << "  暴击伤害: " << (critdamage + critdamageadd) * 100 << "%";
    ss << "\n闪避率: " << (missrate + missrateadd) * 100 << "%";
    ss << "  护盾: " << shield;

    if (!buffs.empty()) {
        ss << "\n状态效果:";
        for (const auto& bf : buffs) {
            ss << "\n  • " << getBuffDescription(bf) << " (" << bf.bufftime << "回合)";
        }
    }

    ss << "\n技能列表:";
    for (size_t i = 0; i < skills.size(); i++) {
        ss << "\n " << (i + 1) << ". " << skills[i]->getname() << ": " << skills[i]->getdescription();
    }

    if (equippedWeapon) {
        ss << "\n装备: " << equippedWeapon->getName();
    }

    return ss.str();
}

bool Character::startTurn() {
    turnCount++;

    // 检查麻痹和睡眠状态
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::Paralyzed) {
            // 麻痹：有50%概率跳过本回合
            if (rand() % 100 < 50) {
                return false;
            }
        } else if (bf.buff_has == Status::Asleep) {
            // 睡眠：跳过本回合
            return false;
        }
    }

    return true;
}

void Character::endTurn() {
    if (!isFrontline) {
        return;
    }
    for (auto it = buffs.begin(); it != buffs.end(); ) {
        // 减少buff持续时间
        it->bufftime--;

        // 处理每回合结束时的效果
        switch (it->buff_has) {
        case Status::Poisoned:
            // 中毒：每回合受到伤害
            {
                int damage = it->buffvalue;
                health-=damage;
            }
            break;
        default:
            break;
        }

        // 检查buff是否结束
        if (it->bufftime <= 0) {
            // buff结束时，恢复对应的add变量
            switch (it->buff_has) {
            case Status::attackAdd:
                attackadd -= it->buffvalue;
                break;
            case Status::attackDec:
                attackadd -= it->buffvalue;  // 恢复负的add
                break;
            case Status::defenseAdd:
                defenseadd -= it->buffvalue;
                break;
            case Status::defenseDec:
                defenseadd -= it->buffvalue;  // 恢复负的add
                break;
            case Status::healthAdd:
                healthadd -= it->buffvalue;
                if(healthadd<0){
                    healthadd=0;
                }
                break;
            case Status::critrateAdd:
                critrateadd -= float(it->buffvalue)/100;  // 与 addbuff 中的 ÷100 保持一致
                break;
            case Status::critdamageAdd:
                critdamageadd -= float(it->buffvalue)/100;
                break;
            case Status::manaAdd:
                manaadd -= it->buffvalue;
                if(manaadd<0){
                    manaadd=0;
                }
                break;
            case Status::manaDec:
                mana = std::max(mana+it->buffvalue,maxmana);  // 恢复负的add
                break;
            case Status::missrateAdd:
                missrateadd -= float(it->buffvalue)/100;  // 与 addbuff 中的 ÷100 保持一致
                break;
            default:
                // 其他buff类型不需要恢复操作
                break;
            }

            it = buffs.erase(it);
        } else {
            ++it;
        }
    }
}

void Character::setWeapon(Weapon* weapon) {
    equippedWeapon = weapon;
    if (equippedWeapon) {
        attackadd = (int)(attack * (equippedWeapon->getAttackBonus() - 1));
        defenseadd = (int)(defense * (equippedWeapon->getDefenseBonus() - 1));
        critrateadd = equippedWeapon->getCritrateBonus();
        critdamageadd = equippedWeapon->getCritdamageBonus();
        missrateadd = equippedWeapon->getMissrateBonus();
        healthadd = (int)(maxHealth * (equippedWeapon->getHealthBonus() - 1));
        manaadd = (int)(maxmana * (equippedWeapon->getManaBonus() - 1));
        if(equippedWeapon->hasSpecialEffects()){
            Status tempbuff=equippedWeapon->getSpecialEffects()->getbuff();
            int time=equippedWeapon->getSpecialEffects()->gettime();
            if(tempbuff==Status::fixed){
                this->addbuff(buff(tempbuff,time,0));
            }
            else if(tempbuff==Status::fastchange){
                this->addbuff(buff(tempbuff,time,0));
            }
        }
    } else {
        removeWeapon();
    }
}

void Character::removeWeapon() {
    equippedWeapon = nullptr;
    attackadd = defenseadd = healthadd = manaadd = 0;
    critrateadd = critdamageadd = missrateadd = 0.0f;
    this->getbuffs().clear();
}

void Character::addSkill(Skill* skill) {
    if (skill) skills.push_back(skill);
}

// 基类技能执行（基础伤害逻辑）
bool Skill::execute(Character* caster, Character* target,bool hasFixed) {return true;}

bool healSkill::execute(Character* caster, Character* target,bool hasFixed) {
    if (!caster || !target || !target->alive()) return false;
    if(!hasFixed){
        if(caster->getmana()+caster->getmanaadd() < manacost){
            return false;
        }
        if (manacost<caster->getmanaadd()){
            caster->setmanaadd(caster->getmanaadd()-manacost);
        }
        else{
            caster->setmana(caster->getmana()+caster->getmanaadd() - manacost);
            caster->setmanaadd(0);
        }
    }
    target->heal((int)((target->getMaxHealth() + target->gethealthadd()) * healmount));
    return true;
}

bool buffSkill::execute(Character* caster, Character* target,bool hasFixed) {
    if (!caster || !target || !target->alive() ) return false;
    if(!hasFixed){
        if(caster->getmana()+caster->getmanaadd() < manacost){
            return false;
        }
        if (manacost<caster->getmanaadd()){
            caster->setmanaadd(caster->getmanaadd()-manacost);
        }
        else{
            caster->setmana(caster->getmana()+caster->getmanaadd() - manacost);
            caster->setmanaadd(0);
        }
    }
    int val=0;
    if(buff_add==Status::attackAdd){
        val=int(target->getAttack()*add);
    }
    else if(buff_add==Status::critdamageAdd){
        val=int(add*100);
    }
    else if(buff_add==Status::critrateAdd){
        val=int(add*100);
    }
    else if(buff_add==Status::LifeSteal){
        val=int(add*100);
    }
    else if(buff_add==Status::Poisoned){
        val=int(add*100);
    }
    else if(buff_add==Status::attackDec){
        val=-int(target->getAttack()*add);
    }
    else if(buff_add==Status::defenseAdd){
        val=int(target->getDefense()*add);
    }
    else if(buff_add==Status::healthAdd){
        val=int(target->getMaxHealth()*add);
    }
    else if(buff_add==Status::defenseDec){
        val=-int(target->getDefense()*add);
    }
    else if(buff_add==Status::manaAdd){
        val=int(target->getmaxmana()*add);
    }
    else if(buff_add==Status::manaDec){
        val=-int(target->getmaxmana()*add);
    }
    else if(buff_add==Status::shieldAdd){
        val=int(target->getMaxHealth()*add);
    }
    else if(buff_add==Status::missrateAdd){
        val=int(add*100);  // 闪避率以 ×100 编码，addbuff 内会再 ÷100
    }
    target->addbuff(buff(buff_add, duringtime, val));
    return true;
}

bool Character::useSkill(int skillIndex, Character* target,bool isover) {
    if (skillIndex < 0 || skillIndex >= skills.size() || !target) {
        return false;
    }

    Skill* skill = skills[skillIndex];
    int manaCost = skill->getmanacost();

    if(isover){
        skill->execute(this, target,true);
        return true;
    }

    // 检查fixed状态（不消耗魔法）
    bool hasFixed = false;
    for (const auto& bf : buffs) {
        if (bf.buff_has == Status::fixed) {
            hasFixed = true;
            break;
        }
    }

    if (!hasFixed) {
        if (mana < manaCost) {
            return false;  // 法力不足
        }
    }

    // 执行技能
    skill->execute(this, target,hasFixed);

    return true;
}
std::string Character::getBuffDescription(const buff& b) const {
    std::string desc;
    switch (b.buff_has) {
    case Status::attackAdd:
        return "攻击力+" + std::to_string(b.buffvalue);
    case Status::attackDec:
        return "攻击力-" + std::to_string(b.buffvalue);
    case Status::defenseAdd:
        return "防御力+" + std::to_string(b.buffvalue);
    case Status::defenseDec:
        return "防御力-" + std::to_string(b.buffvalue);
    case Status::healthAdd:
        return "最大生命+" + std::to_string(b.buffvalue);
    case Status::critrateAdd:
        return "暴击率+" + std::to_string(b.buffvalue) + "%";
    case Status::critdamageAdd:
        return "暴击伤害+" + std::to_string(b.buffvalue) + "%";
    case Status::manaAdd:
        return "最大法力+" + std::to_string(b.buffvalue);
    case Status::manaDec:
        return "最大法力-" + std::to_string(b.buffvalue);
    case Status::missrateAdd:
        return "闪避率+" + std::to_string(b.buffvalue) + "%";
    case Status::shieldAdd:
        return "护盾+" + std::to_string(b.buffvalue);
    case Status::Penetration:
        return "穿透+" + std::to_string(b.buffvalue) + "%";
    case Status::fastchange:
        return "快速切换技能";
    case Status::fixed:
        return "技能不消耗法力";
    case Status::Defend:
        return "防御姿态(减伤50%)";
    case Status::Poisoned:
        return "中毒(每回合伤害:" + std::to_string(b.buffvalue) + ")";
    case Status::Paralyzed:
        return "麻痹(50%概率无法行动)";
    case Status::Asleep:
        return "睡眠(无法行动)";
    case Status::Invincible:
        return "无敌";
    case Status::LifeSteal:
        return "吸血(" + std::to_string(b.buffvalue) + "%)";
    case Status::Normal:
        return "正常状态";
    default:
        return "未知状态";
    }
}

// 根据名字创建对应的角色对象（工厂函数）
Character* createCharacterByName(const std::string& name) {
    // 映射关系：名字 -> 对应的具体类构造函数
    // 注意：所有具体类都继承自 Character，且构造函数格式为 ClassName(const std::string& name, int level)
    // 这里 level 默认为 1（一星）
    int level = 1;

    if (name == "莱昂") return new Leon(name, level);
    if (name == "巴尔") return new Barr(name, level);
    if (name == "安娜") return new Anna(name, level);
    if (name == "莉娜") return new Lina(name, level);
    if (name == "艾莉亚") return new Arya(name, level);
    if (name == "罗宾") return new Robin(name, level);
    if (name == "索拉") return new Sora(name, level);
    if (name == "无名的王") return new NamelessKing(name, level);
    if (name == "格罗姆") return new Gromm(name, level);
    if (name == "塞恩") return new Sion(name, level);
    if (name == "亚瑟") return new Arthur(name, level);
    if (name == "梅林") return new Merlin(name, level);
    if (name == "维嘉") return new Veigar(name, level);
    if (name == "吉安娜") return new Jaina(name, level);
    if (name == "温蕾萨") return new Vereesa(name, level);
    if (name == "希尔瓦娜斯") return new Sylvanas(name, level);
    if (name == "莱戈拉斯") return new Legolas(name, level);
    if (name == "影") return new Shadow(name, level);
    if (name == "光") return new Light(name, level);
    if (name == "混沌") return new Chaos(name, level);

    return new Warrior(name, level);
}

bool Character::hasBuffType(Status status) const {
    for (const auto& buff : buffs) {
        if (buff.buff_has == status) {
            return true;
        }
    }
    return false;
}

//用于存档

QJsonObject Character::saveToJson() const {
    QJsonObject obj;

    // 1. 核心标识与等级
    obj["name"] = QString::fromStdString(name);
    obj["level"] = level;

    // 2. 当前数值状态
    obj["health"] = health;
    obj["mana"] = mana;
    obj["shield"] = shield;
    obj["isAlive"] = isalive;
    obj["isFrontline"] = isFrontline;

    // 3. 保存buff列表
    QJsonArray buffsArray;
    for (const buff& bf : buffs) {
        QJsonObject buffObj;
        buffObj["status"] = static_cast<int>(bf.buff_has); // 枚举转int
        buffObj["duration"] = bf.bufftime;
        buffObj["value"] = bf.buffvalue;
        buffsArray.append(buffObj);
    }
    obj["buffs"] = buffsArray;

    // 4. 保存装备的武器名称
    if (equippedWeapon) {
        obj["equippedWeapon"] = QString::fromStdString(equippedWeapon->getName());
    } else {
        obj["equippedWeapon"] = QString(); // 空字符串表示未装备
    }

    return obj;
}

bool Character::loadFromJson(const QJsonObject& json) {
    if (json.isEmpty()) return false;

    // 1. 加载等级并更新属性
    int savedLevel = json["level"].toInt();
    while (level < savedLevel) {
        levelUp(); // 调用升级方法，自动更新属性
    }

    // 2. 加载当前状态
    health = json["health"].toInt();
    mana = json["mana"].toInt();
    shield = json["shield"].toInt();
    isalive = json["isAlive"].toBool();
    isFrontline = json["isFrontline"].toBool();

    // 3. 加载buff列表
    buffs.clear();
    QJsonArray buffsArray = json["buffs"].toArray();
    for (const QJsonValue& bfVal : buffsArray) {
        QJsonObject buffObj = bfVal.toObject();
        Status status = static_cast<Status>(buffObj["status"].toInt());
        int duration = buffObj["duration"].toInt();
        int value = buffObj["value"].toInt();
        buffs.push_back(buff(status, duration, value));
    }

    // 注意：装备的武器名称在这里只保存，不立即关联。
    // 实际的关联操作应在所有武器加载完成后，由上层管理统一处理。
    // 这里只是将武器名称存储到一个临时变量，供外部使用。
    // 例如，你可以在Character类中添加一个成员：std::string pendingWeaponName;
    // 并在加载后由游戏管理器调用 setWeaponByName() 方法。

    return true;
}
// --- 职业具体实现 ---

// 平衡说明：伤害公式为 攻击 - 防御，因此防御压低、攻击拉开，避免出现“互相 0 伤害”。
// 剑士：高生命高防御、攻击偏低（前排肉盾）
Warrior::Warrior(const std::string& name, int level) : Character(name, CharacterClass::Warrior, level) {
    maxHealth = 150 + level * 50;
    maxmana = 50 + level * 10;
    attack = 20 + level * 10;
    defense = 20 + level * 6;
    health = maxHealth;
    mana = maxmana;
}

void Warrior::levelUp() {
    Character::levelUp();
    maxHealth = 150 + level * 50;
    maxmana = 50 + level * 10;
    attack = 20 + level * 10;
    defense = 20 + level * 6;
    health = maxHealth;
    mana = maxmana;
}

// 法师：脆皮高爆发、法力深、防御极低
Mage::Mage(const std::string& name, int level) : Character(name, CharacterClass::Mage, level) {
    maxHealth = 100 + level * 25;
    maxmana = 90 + level * 35;
    attack = 25 + level * 12;
    defense = 5 + level * 2;
    health = maxHealth;
    mana = maxmana;
}

void Mage::levelUp() {
    Character::levelUp();
    maxHealth = 100 + level * 25;
    maxmana = 90 + level * 35;
    attack = 25 + level * 12;
    defense = 5 + level * 2;
    health = maxHealth;
    mana = maxmana;
}

// 弓箭手：高持续输出、生命防御中等（后排可协同）
Archer::Archer(const std::string& name, int level) : Character(name, CharacterClass::Archer, level) {
    maxHealth = 100 + level * 25;
    maxmana = 50 + level * 15;
    attack = 20 + level * 12;
    defense = 5 + level * 3;
    health = maxHealth;
    mana = maxmana;
}

void Archer::levelUp() {
    Character::levelUp();
    maxHealth = 100 + level * 28;
    maxmana = 50 + level * 15;
    attack = 20 + level * 12;
    defense = 5 + level * 3;
    health = maxHealth;
    mana = maxmana;
}

// 无名之人：均衡战士
Nonamer::Nonamer(const std::string& name, int level) : Character(name, CharacterClass::Nonamer, level) {
    maxHealth = 110 + level * 32;
    maxmana = 60 + level * 20;
    attack = 15 + level * 10;
    defense = 10 + level * 4;
    health = maxHealth;
    mana = maxmana;
}

void Nonamer::levelUp() {
    Character::levelUp();
    maxHealth = 110 + level * 32;
    maxmana = 60 + level * 20;
    attack = 15 + level * 10;
    defense = 10 + level * 4;
    health = maxHealth;
    mana = maxmana;
}

// 管理器实现
void CharacterManager::addCharacter(Character* character) {
    if (character) characters.push_back(character);
}

Character* CharacterManager::findCharacter(const std::string& name) const {
    for (auto c : characters) if (c->getName() == name) return c;
    return nullptr;
}

bool CharacterManager::removeCharacter(const std::string& name) {
    auto it = std::remove_if(characters.begin(), characters.end(), [&](Character* c){
        if (c->getName() == name) { delete c; return true; }
        return false;
    });
    if (it != characters.end()) {
        characters.erase(it, characters.end());
        return true;
    }
    return false;
}

CharacterManager::~CharacterManager() {
    for (auto c : characters) delete c;
}

// --- 具体英雄实例化 (Leon, Barr, etc.) ---
Leon::Leon(const std::string& name, int level) : Warrior(name, level) {
    addSkill(new buffSkill(1, "重击", Status::Defend, 2, 2.0f, "消耗20点法力，，自身进入防御姿态2回合，减伤50%，遇到攻击会反击", 20));
    addSkill(new buffSkill(2, "狂暴", Status::attackAdd, 3, 0.3f, "消耗20点法力，自身攻击力提升30%，持续3回合", 20));
}

Barr::Barr(const std::string& name, int level) : Warrior(name, level) {
    addSkill(new buffSkill(1, "盾击", Status::Defend, 2, 0.5f, "消耗20法力，自身进入防御姿态2回合，减伤50%，遇到攻击会反击", 20));
    addSkill(new buffSkill(2, "铁壁", Status::defenseAdd, 2, 0.3f, "消耗20点法力，自身防御力提升30%，持续2回合", 20));
}

Anna::Anna(const std::string& name, int level) : Mage(name, level) {
    addSkill(new buffSkill(1, "冰锥术", Status::Asleep, 2, 0, "消耗50点法力，使前台敌人睡眠2回合", 50));
    addSkill(new healSkill(2, "治愈术", 50, 0.3f, "消耗50点法力，全队回复30%生命值"));
}

Lina::Lina(const std::string& name, int level) : Mage(name, level) {
    addSkill(new buffSkill(1, "火球术", Status::Poisoned, 2, 0.15f, "消耗30点法力，使前台敌人中毒（每回合扣15生命值），持续2回合", 30));
    addSkill(new buffSkill(2, "烈焰护体", Status::shieldAdd, 2, 0.3f, "消耗20法力，对自身施加生命值30%的护盾，持续2回合", 30));
}

Arya::Arya(const std::string& name, int level) : Archer(name, level) {
    addSkill(new buffSkill(1, "精准射击", Status::LifeSteal, 2, 0.3f, "消耗30点法力，自身获得吸血效果，恢复等同于造成伤害30%的血量，持续2回合", 30));
    addSkill(new buffSkill(2, "鹰眼", Status::critrateAdd, 2, 1.0f, "消耗50点法力，自身暴击率提升100%", 50));
}

Robin::Robin(const std::string& name, int level) : Archer(name, level) {
    addSkill(new buffSkill(1, "毒箭", Status::Poisoned, 2, 0.15f, "消耗20点法力，使前台敌人中毒（每回合扣5生命值），持续2回合", 20));
    addSkill(new buffSkill(2, "疾风步", Status::missrateAdd, 2, 0.2f, "消耗20点法力，提升指定角色20%闪避率，持续2回合", 20));
}

Sora::Sora(const std::string& name, int level) : Nonamer(name, level) {
    addSkill(new buffSkill(1, "星光祝福", Status::attackAdd, 3, 0.3f, "消耗40点法力，指定目标提升30%攻击力，持续3回合", 40));
    addSkill(new buffSkill(2, "月之守护", Status::manaAdd, 3, 0.3f, "消耗0点法力，提升指定角色30%法力，持续3回合", 0));
}

NamelessKing::NamelessKing(const std::string& name, int level) : Nonamer(name, level) {
    addSkill(new buffSkill(1, "王权", Status::Invincible, 2, 0, "消耗50点法力，指定目标无敌2回合", 50));
    addSkill(new buffSkill(2, "统御", Status::critdamageAdd, 2, 0.4f, "消耗30点法力，提升指定目标40%暴击伤害，持续2回合", 30));
}

Gromm::Gromm(const std::string& name, int level) : Warrior(name, level) {
    addSkill(new buffSkill(1, "嗜血狂暴", Status::LifeSteal, 2, 0.3f, "消耗20点法力，自身获得吸血效果，吸取等同于造成伤害30%的血量，持续2回合", 20));
    addSkill(new buffSkill(2, "无畏冲锋", Status::attackAdd, 2, 0.3f, "消耗20点法力，提升自身30攻击力，持续2回合", 20));
}

Sion::Sion(const std::string& name, int level) : Warrior(name, level) {
    addSkill(new buffSkill(1, "守护誓言", Status::healthAdd, 2, 0.3f, "消耗30点法力，提升自身30%生命值，持续2回合", 30));
    addSkill(new buffSkill(2, "不屈意志", Status::defenseAdd, 3, 0.3f, "消耗30点法力，提升自身30%防御，持续3回合", 30));
}

Arthur::Arthur(const std::string& name, int level) : Warrior(name, level) {
    addSkill(new buffSkill(1, "休养", Status::fixed, 3, 0, "消耗0点法力，使得后两回合行动不消耗法力", 0));
    addSkill(new buffSkill(2, "王者祝福", Status::attackAdd, 3, 0.5f, "消耗50点法力，使得自身攻击力提升50%，持续2回合", 50));
}

Merlin::Merlin(const std::string& name, int level) : Mage(name, level) {
    addSkill(new buffSkill(1, "法力潮汐", Status::manaAdd, 2, 0.4f, "消耗0点法力，回复指定目标40%法力", 0));
    addSkill(new buffSkill(2, "时光停滞", Status::Paralyzed, 2, 0, "消耗50点法力，使前台敌人麻痹（50%概率无法攻击），持续2回合", 50));
}

Veigar::Veigar(const std::string& name, int level) : Mage(name, level) {
    addSkill(new buffSkill(1, "暗影诅咒", Status::attackDec, 3, -0.3f, "消耗30点法力，降低前台敌人30%攻击，持续3回合", 30));
    addSkill(new healSkill(2, "灵魂汲取", 50, 0.3f, "消耗50点法力，为全体全体治疗30%的生命值"));
}

Jaina::Jaina(const std::string& name, int level) : Mage(name, level) {
    addSkill(new buffSkill(1, "冰火两重天", Status::Poisoned, 2, 0.15f, "消耗50点法力，敌方全体中毒（每回合扣除15点血量）2回合", 50));
    addSkill(new buffSkill(2, "元素护盾", Status::shieldAdd, 2, 0.3f, "消耗50点法力，为全体施加基于自身生命值30%的护盾，持续2回合", 50));
}

Vereesa::Vereesa(const std::string& name, int level) : Archer(name, level) {
    addSkill(new buffSkill(1, "疾风射击", Status::fastchange, 2, 0, "自身获得速切效果，持续2回合", 20));
    addSkill(new buffSkill(2, "风之庇护", Status::missrateAdd, 2, 0.2f, "消耗30法力，指定角色提升20%闪避，持续2回合", 30));
}

Sylvanas::Sylvanas(const std::string& name, int level) : Archer(name, level) {
    addSkill(new buffSkill(1, "黑暗箭雨", Status::Paralyzed, 2, 0, "消耗30点法力，敌方全体麻痹（50%概率无法攻击），持续2回合", 30));
    addSkill(new buffSkill(2, "王之意志", Status::Penetration, 2, 1, "消耗30法力，自身获得穿透，使得伤害变为两倍，持续2回合", 30));
}

Legolas::Legolas(const std::string& name, int level) : Archer(name, level) {
    addSkill(new buffSkill(1, "精准连射", Status::critrateAdd, 3, 0.2f, "消耗40法力，全体提升20%暴击率，持续3回合", 40));
    addSkill(new buffSkill(2, "精灵之眼", Status::Penetration, 2, 1, "消耗30法力，自身获得穿透，使得伤害变为两倍，持续2回合", 30));
}

Shadow::Shadow(const std::string& name, int level) : Nonamer(name, level) {
    addSkill(new buffSkill(1, "暗影步", Status::missrateAdd, 2, 0.3f, "消耗30法力，自身提升30%提升闪避，持续2回合", 30));
    addSkill(new buffSkill(2, "背刺", Status::critdamageAdd, 2, 0.5f, "消耗50法力，全队提升40%暴伤，持续2回合", 50));
}

Light::Light(const std::string& name, int level) : Nonamer(name, level) {
    addSkill(new healSkill(1, "光明治愈", 40, 0.6f, "消耗40法力，指定目标恢复60%生命"));
    addSkill(new buffSkill(2, "神圣净化", Status::Invincible, 2, 0, "消耗50法力，自身无敌2回合", 50));
}

Chaos::Chaos(const std::string& name, int level) : Nonamer(name, level) {
    addSkill(new buffSkill(1, "混沌波动", Status::Asleep, 2, 0, "消耗50法力使前台敌人休眠2回合", 50));
    addSkill(new healSkill(2, "命运骰子", 20, 1.0f, "消耗20法力，随机选择一位队友，使其回复满血"));
}