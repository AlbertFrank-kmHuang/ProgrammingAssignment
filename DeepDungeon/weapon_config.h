#ifndef WEAPON_CONFIG_H
#define WEAPON_CONFIG_H

#include <vector>
#include <string>
#include <QJsonObject>

class buffSkill;

// 装备品质
enum class Rarity {
    Common,     // 普通
    Rare,       // 罕见
    Epic,       // 史诗
    Legendary   // 传奇
};

// 装备基础类
class Weapon {
protected:
    std::string name;
    std::string description;
    Rarity rarity;

    // 属性加成
    float healthBonus = 1;
    float attackBonus = 1;
    float defenseBonus = 1;
    float critrateBonus = 0;
    float critdamageBonus = 0;
    float missrateBonus = 0;
    float manaBonus = 1;

    // 特殊效果
    buffSkill* specialEffects;

public:
    Weapon(const std::string& name, Rarity rarity);
    virtual ~Weapon() = default;

    float getHealthBonus() const {return healthBonus;}
    float getAttackBonus() const {return attackBonus;}
    float getDefenseBonus() const {return defenseBonus;}
    float getCritrateBonus() const {return critrateBonus;}
    float getCritdamageBonus() const {return critdamageBonus;}
    float getMissrateBonus() const {return missrateBonus;}
    float getManaBonus() const {return manaBonus;}
    std::string getName() const { return name; }
    std::string getDescription() const {return description;}
    Rarity getRarity() const { return rarity; }
    buffSkill* getSpecialEffects() const { return specialEffects; }
    bool hasSpecialEffects() const { return (specialEffects!=nullptr); }

    //用于存档
    virtual QJsonObject toJson() const;
    virtual bool fromJson(const QJsonObject& json);
};

Weapon* createWeaponByName(const std::string& name);

extern std::vector<std::string> commonWeaponList;
extern std::vector<std::string> rareWeaponList;
extern std::vector<std::string> epicWeaponList;
extern std::vector<std::string> legendaryWeaponList;

Weapon* createWeaponByName(const std::string& name);


#endif // WEAPON_CONFIG_H
