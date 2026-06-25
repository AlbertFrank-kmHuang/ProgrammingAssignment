#include "weapon_config.h"
#include "character_config.h"
#include <vector>
// Weapon 类的构造函数
Weapon::Weapon(const std::string& name, Rarity rarity)
    : name(name), rarity(rarity),specialEffects(nullptr) {}

QJsonObject Weapon::toJson() const {
    QJsonObject obj;
    // 当前只保存足以重建武器的标识信息：名称
    obj["name"] = QString::fromStdString(name);
    return obj;
}

bool Weapon::fromJson(const QJsonObject& json) {
    if (json.isEmpty()) return false;
    return true;
}

std::vector<std::string> commonWeaponList={"木剑","木法杖","木弓","木盾"};
std::vector<std::string> rareWeaponList={"匕首","魔杖","硬木弓","骑士盾"};
std::vector<std::string> epicWeaponList={"斩龙者","毒蛇之牙","雷光魔杖","虚空行者","狮心盾","守护者之盾","逐日者","霜语者"};
std::vector<std::string> legendaryWeaponList={"诸神黄昏","命运之剑","命运穿刺","灭世者","命运的纺锤","天上的盾","绝对防御","泰坦之弓","弑神者"};
// 说明：attackBonus / defenseBonus / healthBonus / manaBonus 为“倍数”（1 表示不变，1.2 表示 +20%）；
//      critrateBonus / critdamageBonus / missrateBonus 为“加值”（0 表示不变）。
//可直接拿来用的武器列表:
class woodSword:public Weapon{
public:
    woodSword(const std::string& name="木剑", Rarity rarity=Rarity::Common):
        Weapon(name,rarity){
        attackBonus = 1.2;
        description="普通的木剑，增加少量攻击（1.2倍），无法增加弓箭手后台攻击伤害";
    }
};

class WoodStaff:public Weapon{
public:
    WoodStaff(const std::string& name="木法杖", Rarity rarity=Rarity::Common):
        Weapon(name,rarity){
        manaBonus = 1.2;
        description="普通的木法杖，增加少量法力（1.2倍）";
    }
};

class WoodBow:public Weapon{
public:
    WoodBow(const std::string& name="木弓", Rarity rarity=Rarity::Common):
        Weapon(name,rarity){
        attackBonus=1.2;
        description="普通的木弓，增加少量攻击（1.2倍），可以增加弓箭手后台攻击伤害";
    }
};

class WoodShield:public Weapon{
public:
    WoodShield(const std::string& name="木盾", Rarity rarity=Rarity::Common):
        Weapon(name,rarity){
        defenseBonus=1.1;
        description="普通的木盾牌，增加少量防御（1.1倍）";
    }
};

class Dagger:public Weapon{
public:
    Dagger(const std::string& name="匕首", Rarity rarity=Rarity::Rare):
        Weapon(name,rarity){
        attackBonus=1.2;
        critdamageBonus=0.2;
        description="罕见的匕首，增加少量攻击（1.2倍），无法增加弓箭手后台攻击伤害，增加0.2的暴击伤害";
    }
};

class Wand:public Weapon{
public:
    Wand(const std::string& name="魔杖", Rarity rarity=Rarity::Rare):
        Weapon(name,rarity){
        manaBonus = 1.2;
        healthBonus=1.2;
        description="神奇的魔杖，增加少量法力（1.2倍），增加少量生命（1.2倍）";
    }
};

class HardwoodBow:public Weapon{
public:
    HardwoodBow(const std::string& name="硬木弓", Rarity rarity=Rarity::Rare):
        Weapon(name,rarity){
        attackBonus=1.2;
        critdamageBonus=0.2;
        description="硬木做成的弓，增加少量攻击力（1.2倍），可以增加弓箭手后台攻击伤害，增加0.2的暴击伤害";
    }
};

class KnightShield:public Weapon{
public:
    KnightShield(const std::string& name="骑士盾", Rarity rarity=Rarity::Rare):
        Weapon(name,rarity){
        defenseBonus=1.1;
        healthBonus=1.2;
        description="一位骑士的盾牌，增加少量防御力（1.1倍），增加少量生命值（1.2倍）";
    }
};

class DragonSlayer:public Weapon{
public:
    DragonSlayer(const std::string& name="斩龙者", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        attackBonus=1.5;
        critdamageBonus=0.5;
        critrateBonus=0.2;
        description="沾染龙血的英雄之剑，大幅增加攻击力（1.5倍），无法增加弓箭手后台攻击伤害,增加0.5的暴击伤害，0.2的暴击率";
    }
};

class ViperFang:public Weapon{
public:
    ViperFang(const std::string& name="毒蛇之牙", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        attackBonus=1.3;
        critdamageBonus=0.5;
        critrateBonus=0.2;
        description="浸透毒液的匕首，来自毒蛇的牙齿，增加0.3倍攻击，0.5的暴击伤害，0.2的暴击率，攻击时让敌人陷入中毒，持续一回合";
        buffSkill* skill=new buffSkill(1,"中毒",Status::Poisoned,1,0.15,"攻击后，使目标陷入持续1回合的中毒状态");
        specialEffects=skill;
    }
};

class LightningRod:public Weapon{
public:
    LightningRod(const std::string& name="雷光魔杖", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        manaBonus = 1.5;
        attackBonus=1.5;
        description="闪电制成的魔杖，大幅提升法力（1.5倍），大幅增加攻击力（1.5倍），无法增加弓箭手后台攻击伤害，攻击时让敌人陷入麻痹，持续一回合";
        buffSkill* skill=new buffSkill(1,"麻痹",Status::Paralyzed,1,0,"攻击后，使敌人陷入1回合的麻痹状态");
        specialEffects=skill;
    }
};

class Voidwalker:public Weapon{
public:
    Voidwalker(const std::string& name="虚空行者", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        attackBonus=1.5;
        manaBonus = 1.5;
        healthBonus=1.5;
        description="皇帝的新法杖，大幅增加攻击力（1.5倍），无法增加弓箭手后台攻击伤害，大幅提升法力（1.5倍），大幅提升生命值（1.5倍）";
    }
};

class LionheartShield:public Weapon{
public:
    LionheartShield(const std::string& name="狮心盾", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        defenseBonus=1.3;
        healthBonus=1.5;
        missrateBonus=0.2;
        description="中间镶嵌的宝钻如图跳动的狮心，大幅增加防御力（1.3倍），大幅提升生命值（1.5倍），增加0.2的闪避率";
    }
};

class  GuardianAegis:public Weapon{
public:
     GuardianAegis(const std::string& name="守护者之盾", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        defenseBonus=1.3;
        healthBonus=1.5;
        description="守护了整个世界的奇盾，大幅增加防御力（1.3倍），大幅提升生命值（1.5倍），攻击后获得反击，敌人攻击自己时可以反击";
        buffSkill* skill=new buffSkill(1,"反击",Status::Defend, 2, 0.5,"进入防御姿态，减伤50%，受到攻击后反击");
        specialEffects=skill;
    }
};

class Sunseeker:public Weapon{
public:
    Sunseeker(const std::string& name="逐日者", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        attackBonus=1.5;
        critdamageBonus=0.5;
        critrateBonus=0.2;
        description="太阳为之颤抖，大幅增加攻击力（1.5倍），可以增加弓箭手的后台伤害，增加0.5的暴击伤害，0.2的暴击率";
    }
};

class Frostwhisper:public Weapon{
public:
    Frostwhisper(const std::string& name="霜语者", Rarity rarity=Rarity::Epic):
        Weapon(name,rarity){
        attackBonus=1.3;
        critdamageBonus=0.5;
        critrateBonus=0.2;
        description="冰霜为箭，增加0.3倍攻击，0.5的暴击伤害，0.2的暴击率，攻击后让敌人陷入麻痹，持续一回合";
        buffSkill* skill=new buffSkill(1,"麻痹",Status::Paralyzed,1,0,"攻击后，使敌人陷入1回合的麻痹状态");
        specialEffects=skill;
    }
};

class Ragnarok:public Weapon{
public:
    Ragnarok(const std::string& name="诸神黄昏", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.5;
        critdamageBonus=0.5;
        critrateBonus=1.0;
        description="古老战神的遗物，增加0.5的暴击伤害，1.0的暴击率，大幅提升攻击力（1.5倍），无法增加弓箭手后台攻击伤害，攻击后附加一回合吸血效果，本次攻击立即触发一次";
        buffSkill* skill=new buffSkill(1,"吸血",Status::LifeSteal,1,0.3,"攻击后，立即触发吸血效果");
        specialEffects=skill;
    }
};

class SwordofDestiny:public Weapon{
public:
    SwordofDestiny(const std::string& name="命运之剑", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.5;
        healthBonus=2.0;
        defenseBonus=1.8;
        description="裁决命运的圣剑，大幅提升防御力（1.8倍），大幅提升生命值（2倍），大幅提升攻击力（1.5倍），无法增加弓箭手后台攻击伤害，攻击后为自身附加护盾，持续一回合";
        buffSkill* skill=new buffSkill(1,"护盾",Status::shieldAdd,2,0.3,"攻击后附加护盾（生命值30%），护盾持续2个回合");
        specialEffects=skill;
    }
};

class Fatepiercer:public Weapon{
public:
    Fatepiercer(const std::string& name="命运穿刺", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.3;
        critdamageBonus=0.5;
        critrateBonus=0.3;
        description="刺进他心脏的一剑，中幅提升攻击力（1.3倍），增加0.5的暴击伤害，0.3的暴击率，无法增加弓箭手后台攻击伤害，为装备者附加一回合的穿透，本次攻击也算穿透";
        buffSkill* skill=new buffSkill(1,"穿透",Status::Penetration,2,1,"攻击后附加两回合的穿透");
        specialEffects=skill;
    }
};

class Worldender:public Weapon{
public:
    Worldender(const std::string& name="灭世者", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.5;
        manaBonus = 1.5;
        healthBonus=2.0;
        description="诞生于毁灭，大幅提升攻击力（1.5倍），大幅提升生命值（2倍），大幅提升法力（1.5倍），无法增加弓箭手后台攻击伤害，释放技能后可以直接切人而不消耗回合";
        buffSkill* skill=new buffSkill(1,"速切",Status::fastchange,10000000,0,"可以直接切人而不消耗回合");
        specialEffects=skill;
    }
};

class SpindleofFate:public Weapon{
public:
    SpindleofFate(const std::string& name="命运的纺锤", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.3;
        critdamageBonus=0.5;
        critrateBonus=0.3;
        description="独自编织命运，中幅提升攻击力（1.3倍），无法增加弓箭手后台攻击伤害，增加0.5的暴击伤害，0.3的暴击率，释放技能不再消耗法力";
        buffSkill* skill=new buffSkill(1,"无消耗",Status::fixed,100000000,0,"释放技能不再消耗法力");
        specialEffects=skill;
    }
};

class CelestialShield:public Weapon{
public:
    CelestialShield(const std::string& name="天上的盾", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        defenseBonus=2.0;
        healthBonus=2.0;
        missrateBonus=0.3;
        description="取自蓝天，大幅增加防御力（2倍），大幅提升生命值（2倍），增加0.3的闪避率";
    }
};

class AbsoluteDefense:public Weapon{
public:
    AbsoluteDefense(const std::string& name="绝对防御", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        defenseBonus=2.0;
        healthBonus=1.5;
        missrateBonus=0.5;
        description="无坚不摧，大幅增加防御力（2倍），提升生命值（1.5倍），增加0.5的闪避率，攻击后为自身附加护盾，持续一回合";
        buffSkill* skill=new buffSkill(1,"护盾",Status::shieldAdd,2,0.3,"攻击后附加护盾（生命值30%），护盾持续2个回合");
        specialEffects=skill;
    }
};

class TitanString:public Weapon{
public:
    TitanString(const std::string& name="泰坦之弓", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=1.5;
        critdamageBonus=0.5;
        critrateBonus=0.3;
        description="气贯长虹，大幅增加攻击力（1.5倍），可以增加弓箭手后台攻击伤害，增加0.5的暴击伤害，0.3的暴击率，攻击后为前台施加吸血效果，持续一回合";
        buffSkill* skill=new buffSkill(1,"吸血",Status::LifeSteal,2,0.3,"攻击后，附加两回合的吸血效果");
        specialEffects=skill;
    }
};

class Godslayer:public Weapon{
public:
    Godslayer(const std::string& name="弑神者", Rarity rarity=Rarity::Legendary):
        Weapon(name,rarity){
        attackBonus=2.0;
        critdamageBonus=0.3;
        critrateBonus=1.0;
        description="瞄即必杀，大幅增加攻击力（2倍），可以增加弓箭手后台攻击伤害，增加0.3的暴击伤害，1.0的暴击率";
    }
};

//根据名字的武器生成器
Weapon* createWeaponByName(const std::string& name) {
    if (name == "木剑") return new woodSword();
    if (name == "木法杖") return new WoodStaff();
    if (name == "木弓") return new WoodBow();
    if (name == "木盾") return new WoodShield();
    if (name == "匕首") return new Dagger();
    if (name == "魔杖") return new Wand();
    if (name == "硬木弓") return new HardwoodBow();
    if (name == "骑士盾") return new KnightShield();

    // 史诗武器
    if (name == "斩龙者") return new DragonSlayer();
    if (name == "毒蛇之牙") return new ViperFang();
    if (name == "雷光魔杖") return new LightningRod();
    if (name == "虚空行者") return new Voidwalker();
    if (name == "狮心盾") return new LionheartShield();
    if (name == "守护者之盾") return new GuardianAegis();
    if (name == "逐日者") return new Sunseeker();
    if (name == "霜语者") return new Frostwhisper();

    // 传奇武器
    if (name == "诸神黄昏") return new Ragnarok();
    if (name == "命运之剑") return new SwordofDestiny();
    if (name == "命运穿刺") return new Fatepiercer();
    if (name == "灭世者") return new Worldender();
    if (name == "命运的纺锤") return new SpindleofFate();
    if (name == "天上的盾") return new CelestialShield();
    if (name == "绝对防御") return new AbsoluteDefense();
    if (name == "泰坦之弓") return new TitanString();
    if (name == "弑神者") return new Godslayer();

    return nullptr; // 未知武器
}
