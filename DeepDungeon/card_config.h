#ifndef CARD_CONFIG_H
#define CARD_CONFIG_H

#include "character_config.h"
#include "weapon_config.h"
#include "event.h"  // 包含事件系统
#include <string>

// 卡牌类型
enum class CardType {
    Attack,     // 攻击卡
    Skill,      // 技能卡
    Spell,      // 法术卡
    Equipment,  // 装备卡
    Event       // 事件卡
};

// 卡牌类
class Card {
private:
    std::string name;
    std::string description;
    Character* target;
    CardType type;

public:
    Card(const std::string& name, const std::string& description, Character* target, CardType type);
    virtual ~Card() = default;

    // 使用卡牌
    virtual bool use(Character* target = nullptr);

    // 获取属性
    std::string getDescription() const { return description; }
    std::string getName() const { return name; }
    CardType getType() const { return type; }
    Character* getTarget() const { return target; }

    // 设置目标
    void setTarget(Character* newTarget) { target = newTarget; }
};

// 攻击卡
class AttackCard : public Card {
private:
    int Damage;           // 基础伤害
    Status additionalEffect;  // 附加效果
    int effectDuration;       // 效果持续时间

public:
    AttackCard(const std::string& name, const std::string& description, Character* target,
               int Damage = 0, Status additionalEffect = Status::Normal, int effectDuration = 0);

    bool use(Character* target) override;

    // 获取属性
    int getDamage() const { return Damage; }
    Status getAdditionalEffect() const { return additionalEffect; }
    int getEffectDuration() const { return effectDuration; }
};

// 增益卡
class BuffCard : public Card {
private:
    Status buffType;          // 增益类型
    float buffValue;          // 增益数值（百分比）
    int duration;             // 持续时间
    bool isTeamwide;          // 是否为全队增益

public:
    BuffCard(const std::string& name, const std::string& description, Character* target,
             Status buffType, float buffValue, int duration, bool isTeamwide = false);

    bool use(Character* target) override;

    // 获取属性
    Status getBuffType() const { return buffType; }
    float getBuffValue() const { return buffValue; }
    int getDuration() const { return duration; }
    bool isTeamwideBuff() const { return isTeamwide; }
};

// 事件卡
class EventCard : public Card {
private:
    EventType eventType;      // 事件类型
    EventReward reward;       // 事件奖励
    bool requiresFight;       // 是否需要战斗

public:
    EventCard(const std::string& name, const std::string& description, Character* target,
              EventType eventType, bool requiresFight = false);

    // 获取属性
    EventType getEventType() const { return eventType; }
    EventReward getReward() const { return reward; }
    bool requiresFightToResolve() const { return requiresFight; }

    // 设置奖励
    void setReward(const EventReward& newReward) { reward = newReward; }
};

// 装备卡
class WeaponCard : public Card {
private:
    Weapon* weapon;           // 装备对象
    Rarity rarity;            // 装备品质
    bool isRandom;            // 是否为随机装备
    std::string specificWeaponName;  // 指定装备名称

public:
    // 构造函数1：指定具体装备
    WeaponCard(const std::string& name, const std::string& description, Character* target,
               const std::string& weaponName);

    // 构造函数2：随机品质装备
    WeaponCard(const std::string& name, const std::string& description, Character* target,
               Rarity rarity);

    ~WeaponCard();

    bool use(Character* target) override;

    // 获取属性
    Weapon* getWeapon() const { return weapon; }
    Rarity getRarity() const { return rarity; }
    bool isRandomWeapon() const { return isRandom; }
    std::string getSpecificWeaponName() const { return specificWeaponName; }
};

// 卡牌管理类
class CardManager {
private:
    std::vector<Card*> allCards;  // 所有卡牌
    std::vector<Card*> playerDeck; // 玩家卡组

public:
    CardManager();
    ~CardManager();

    // 卡牌管理
    void addCard(Card* card);
    void removeCard(Card* card);
    Card* getCardByName(const std::string& name) const;

    // 卡组管理
    void addToDeck(Card* card);
    void removeFromDeck(Card* card);
    void shuffleDeck();
    Card* drawCard();
    void discardCard(Card* card);
    void reshuffleDiscardPile();

    // 获取信息
    std::vector<Card*> getAllCards() const { return allCards; }
    std::vector<Card*> getPlayerDeck() const { return playerDeck; }
    int getDeckSize() const { return playerDeck.size(); }

    // 卡牌生成
    Card* generateRandomCard();
    Card* generateCardByType(CardType type);
    std::vector<Card*> generateStarterDeck();

    // 保存/加载
    bool saveDeckToFile(const std::string& filename) const;
    bool loadDeckFromFile(const std::string& filename);
};

// 卡牌工厂函数声明
Card* createCardByName(const std::string& cardName);
AttackCard* createAttackCard(const std::string& cardName);
BuffCard* createBuffCard(const std::string& cardName);
EventCard* createEventCard(const std::string& cardName);
WeaponCard* createWeaponCard(const std::string& cardName);

// 获取卡牌列表函数声明
std::vector<std::string> getAllCardNames();
std::vector<std::string> getCardsByType(CardType type);

#endif // CARD_CONFIG_H
