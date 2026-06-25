#ifndef CHARACTER_CONFIG_H
#define CHARACTER_CONFIG_H

#include "weapon_config.h"
#include <string>
#include <vector>
#include <QJsonObject>
#include <QJsonArray>

// 前向声明
class Character;

// 枚举：角色职业
enum class CharacterClass {
    Warrior,    // 剑士
    Mage,       // 法师
    Archer,     // 弓箭手
    Nonamer,    // 无名之人
};

// 角色状态
enum class Status {
    attackAdd,      // 攻击增强
    defenseAdd,     // 防御增强
    attackDec,      // 攻击减少
    defenseDec,     // 防御减少
    healthAdd,      // 生命增加
    critrateAdd,    // 暴击率增强
    critdamageAdd,  // 暴击伤害增强
    manaAdd,        // 法力增强
    manaDec,        // 法力减少
    missrateAdd,    // 闪避率增强
    shieldAdd,      // 附加护盾
    Penetration,    // 穿透
    fastchange,     // 速切
    fixed,          // 不消耗魔法
    Normal,         // 正常
    Defend,         // 防御
    Poisoned,       // 中毒
    Paralyzed,      // 麻痹
    Asleep,         // 睡眠
    Invincible,     // 无敌
    LifeSteal       // 吸血
};

// buff类
class buff {
public:
    Status buff_has;
    int bufftime;
    int buffvalue;
    buff(Status buff_has, int bufftime, int buffvalue)
        : buff_has(buff_has), bufftime(bufftime), buffvalue(buffvalue) {}
};

// 技能基类
class Skill {
protected:
    int id;
    std::string name;
    std::string description;
    int manacost;
public:
    Skill(int id, std::string name, int manacost, std::string description)
        : id(id), name(name), manacost(manacost), description(description) {}
    virtual ~Skill() = default;
    int getmanacost() { return manacost; }
    std::string getname() { return name; }
    std::string getdescription() { return description; }
    virtual bool execute(Character* caster, Character* target,bool hasFixed);
};

class healSkill : public Skill {
protected:
    float healmount;
public:
    healSkill(int id, std::string name, int manacost, float healmount, std::string description) :
        Skill(id, name, manacost, description), healmount(healmount) {}
    bool execute(Character* caster, Character* target,bool hasFixed) override;
};

class buffSkill : public Skill {
protected:
    Status buff_add;
    int duringtime;
    float add;
public:
    buffSkill(int id, std::string name, Status buff_add, int duringtime, float add, std::string description, int manacost = 0) :
        Skill(id, name, manacost, description),
        buff_add(buff_add), duringtime(duringtime), add(add) {}
    bool execute(Character* caster, Character* target,bool hasFixed) override;
    Status getbuff(){return buff_add;};
    int gettime(){return duringtime;};
};

// 基类：角色
class Character {
protected:
    std::string name;
    CharacterClass job;
    int level;
    int health;
    int healthadd;
    int maxHealth;
    int attack;
    int attackadd;
    int defense;
    int defenseadd;
    int maxmana;
    int mana;
    int manaadd;
    float critrate;
    float critrateadd;
    float critdamage;
    float critdamageadd;
    float missrate;
    float missrateadd;
    int shield;
    bool isalive;
    bool isFrontline;
    int turnCount;
    int currentSkillIndex;
    Weapon* equippedWeapon;
    std::vector<Skill*> skills;
    std::vector<buff> buffs;

public:
    Character(const std::string& name, CharacterClass job, int level = 1,
              float critrate = 0.2, float critdamage = 1.5, float missrate = 0,
              int shield = 0, bool isalive = true, bool isFrontline = false);

    virtual ~Character();

    // Getters
    std::string getName() const { return name; }
    CharacterClass getJob() const { return job; }
    int getLevel() const { return level; }
    int getHealth() const { return health; }
    int gethealthadd() const { return healthadd; }
    int getattackadd() const { return attackadd; }
    int getdefenseadd() const { return defenseadd; }
    float getcritrateadd() const { return critrateadd; }
    float getcritdamageadd() const { return critdamageadd; }
    float getmissrateadd() const { return missrateadd; }
    int getMaxHealth() const { return maxHealth; }
    int getAttack() const { return attack; }
    int getDefense() const { return defense; }
    int getmaxmana() const { return maxmana; }
    int getmana() const { return mana; }
    int getmanaadd() const { return manaadd; }
    float getCritrate() const { return critrate; }
    float getCritdamage() const { return critdamage; }
    float getMissrate() const { return missrate; }
    int getShield() const { return shield; }
    bool alive() const { return isalive; }
    bool isFront() const { return isFrontline; }
    int getTurnCount() const { return turnCount; }
    int getcurrentSkillIndex() const { return currentSkillIndex; }
    const Weapon* getWeapon() const { return equippedWeapon; }
    const std::vector<Skill*>& getSkills() const { return skills; }
    std::vector<buff>& getbuffs() { return buffs; }

    // Setters & Methods
    void setWeapon(Weapon* weapon);
    void removeWeapon();
    void setName(const std::string& newname) { name = newname; }
    void setFrontline() { isFrontline = true; }
    void setcurrentSkillIndex(int current) { currentSkillIndex = current; }
    void addSkill(Skill* skill);
    void addbuff(const buff& b); // 确保实现此函数
    bool useSkill(int skillIndex, Character* target,bool isover=false);
    bool startTurn();
    void endTurn();
    void setLevel(int newlevel) { level = newlevel; }
    void setHealth(int h) { health = h; }
    void sethealthadd(int add) { healthadd = add; }
    void setattackadd(int add) { attackadd = add; }
    void setdefenseadd(int add) { defenseadd = add; }
    void setcritrateadd(float add) { critrateadd = add; }
    void setcritdamageadd(float add) { critdamageadd = add; }
    void setmissrateadd(float add) { missrateadd = add; }
    void setMaxHealth(int newmaxhealth) { maxHealth = newmaxhealth; }
    void setAttack(int newattack) { attack = newattack; }
    void setDefense(int newdefense) { defense = newdefense; }
    void setmaxmana(int newmaxmana) { maxmana = newmaxmana; }
    void setmana(int newmana) { mana = newmana; }
    void setmanaadd(int add) { manaadd = add; }
    void setCritrate(float newcritrate) { critrate = newcritrate; }
    void setCritdamage(float newcritdamage) { critdamage = newcritdamage; }
    void setMissrate(float newmissrate) { missrate = newmissrate; }
    void setShield(int newshield) { shield = newshield; }
    void setalive(bool newalive) { isalive = newalive; }
    void setisFront(bool front) { isFrontline = front; }
    void setTurnCount(int turn) { turnCount = turn; }

    virtual int takeDamage(int damage, Character* attacker, bool isCounterAttack);
    virtual void heal(int amount);
    virtual void levelUp();
    virtual std::string getInfo() const;
    virtual std::string getJobName() const; // 确保在 .cpp 中实现
    bool hasBuffType(Status status) const;
    int calculateDamage(Character* target);
    std::string getBuffDescription(const buff& b) const;
    int calculateCounterDamage(Character* target);
    bool isArcher() const;
    void performSupportAttack(Character* target);
    //用于存档
    QJsonObject saveToJson() const;               // 保存当前状态
    bool loadFromJson(const QJsonObject& json);   // 从JSON加载状态
};

// 职业类
class Warrior : public Character {
public:
    Warrior(const std::string& name, int level = 1);
    void levelUp() override;
    std::string getJobName() const override { return "剑士"; }
};

class Mage : public Character {
public:
    Mage(const std::string& name, int level = 1);
    void levelUp() override;
    std::string getJobName() const override { return "法师"; }
};

class Archer : public Character {
public:
    Archer(const std::string& name, int level = 1);
    void levelUp() override;
    std::string getJobName() const override { return "弓箭手"; }
};

class Nonamer : public Character {
public:
    Nonamer(const std::string& name, int level = 1);
    void levelUp() override;
    std::string getJobName() const override { return "无名之人"; }
};

// 管理器
class CharacterManager {
private:
    std::vector<Character*> characters;
public:
    void addCharacter(Character* character);
    Character* findCharacter(const std::string& name) const;
    std::vector<Character*> getAllCharacters() const { return characters; }
    bool removeCharacter(const std::string& name);
    bool saveToFile(const std::string& filename) const;
    bool loadFromFile(const std::string& filename);
    ~CharacterManager();
};

// --- 下面是具体角色的子类定义 ---
class Leon : public Warrior { public: Leon(const std::string& name = "莱昂", int level = 1); };
class Barr : public Warrior { public: Barr(const std::string& name = "巴尔", int level = 1); };
class Anna : public Mage { public: Anna(const std::string& name = "安娜", int level = 1); };
class Lina : public Mage { public: Lina(const std::string& name = "莉娜", int level = 1); };
class Arya : public Archer { public: Arya(const std::string& name = "艾莉亚", int level = 1); };
class Robin : public Archer { public: Robin(const std::string& name = "罗宾", int level = 1); };
class Sora : public Nonamer { public: Sora(const std::string& name = "索拉", int level = 1); };
class NamelessKing : public Nonamer { public: NamelessKing(const std::string& name = "无名的王", int level = 1); };
class Gromm : public Warrior { public: Gromm(const std::string& name = "格罗姆", int level = 1); };
class Sion : public Warrior { public: Sion(const std::string& name = "塞恩", int level = 1); };
class Arthur : public Warrior { public: Arthur(const std::string& name = "亚瑟", int level = 1); };
class Merlin : public Mage { public: Merlin(const std::string& name = "梅林", int level = 1); };
class Veigar : public Mage { public: Veigar(const std::string& name = "维嘉", int level = 1); };
class Jaina : public Mage { public: Jaina(const std::string& name = "吉安娜", int level = 1); };
class Vereesa : public Archer { public: Vereesa(const std::string& name = "温蕾萨", int level = 1); };
class Sylvanas : public Archer { public: Sylvanas(const std::string& name = "希尔瓦娜斯", int level = 1); };
class Legolas : public Archer { public: Legolas(const std::string& name = "莱戈拉斯", int level = 1); };
class Shadow : public Nonamer { public: Shadow(const std::string& name = "影", int level = 1); };
class Light : public Nonamer { public: Light(const std::string& name = "光", int level = 1); };
class Chaos : public Nonamer { public: Chaos(const std::string& name = "混沌", int level = 1); };

Character* createCharacterByName(const std::string& name);

#endif // CHARACTER_CONFIG_H