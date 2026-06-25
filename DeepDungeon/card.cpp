#include "card_config.h"
#include "weapon_config.h"
#include <random>
#include <vector>
#include <algorithm>
#include <fstream>

// 全局随机数生成器
std::random_device rd;
std::mt19937 rng(rd());

// ============ Card基类实现 ============
Card::Card(const std::string& name, const std::string& description, Character* target, CardType type)
    : name(name), description(description), target(target), type(type) {}

bool Card::use(Character* target) {
    // 基类默认实现，记录卡牌使用(具体记录方法待实现)
    return true;
}

// ============ AttackCard类实现 ============
AttackCard::AttackCard(const std::string& name, const std::string& description, Character* target,
                       int Damage, Status additionalEffect, int effectDuration)
    : Card(name, description, target, CardType::Attack),
    Damage(Damage), additionalEffect(additionalEffect), effectDuration(effectDuration) {}

bool AttackCard::use(Character* target) {
    if (!target || !target->alive()) {
        return false;
    }

    // 应用伤害
    target->takeDamage(Damage,nullptr,false);

    // 如果有附加效果
    if (additionalEffect != Status::Normal && effectDuration > 0) {
        target->addbuff(buff(additionalEffect, effectDuration,0));
    }

    return true;
}

// ============ BuffCard类实现 ============
BuffCard::BuffCard(const std::string& name, const std::string& description, Character* target,
                   Status buffType, float buffValue, int duration, bool isTeamwide)
    : Card(name, description, target, CardType::Skill),
    buffType(buffType), buffValue(buffValue), duration(duration), isTeamwide(isTeamwide) {}

bool BuffCard::use(Character* target) {
    if (!target || !target->alive()) {
        return false;
    }
    int buffmount=0;
    // 计算增益数值（具体生效由 addbuff 统一处理，避免重复叠加并覆盖装备/羁绊加成）
    switch (buffType) {
    case Status::attackAdd:
        buffmount=int(target->getAttack() * buffValue);
        break;
    case Status::defenseAdd:
        buffmount=int(target->getDefense() * buffValue);
        break;
    case Status::healthAdd:
        buffmount=int(target->getMaxHealth() * buffValue);
        break;
    case Status::critrateAdd:
    case Status::critdamageAdd:
    case Status::missrateAdd:
        buffmount=int(buffValue*100);
        break;
    case Status::shieldAdd:
        buffmount=int(target->getMaxHealth() * buffValue);
        break;
    case Status::manaAdd:
        buffmount=int(target->getmaxmana() * buffValue);
        break;
    default:
        return false;
    }

    // 添加buff（addbuff 内部会立即应用效果并记录持续时间）
    target->addbuff(buff(buffType, duration,buffmount));

    return true;
}

// ============ EventCard类实现 ============
EventCard::EventCard(const std::string& name, const std::string& description, Character* target,
                     EventType eventType, bool requiresFight)
    : Card(name, description, target, CardType::Event),
    eventType(eventType), requiresFight(requiresFight) {
    // 初始化默认奖励
    reward.type = RewardType::Gold;
    reward.value = 0;
    reward.weapon = nullptr;
    reward.target = nullptr;
}

// ============ WeaponCard类实现 ============
WeaponCard::WeaponCard(const std::string& name, const std::string& description, Character* target,
                       const std::string& weaponName)
    : Card(name, description, target, CardType::Equipment),
    isRandom(false), specificWeaponName(weaponName) {
    weapon = createWeaponByName(weaponName);
    if (weapon) {
        rarity = weapon->getRarity();
    } else {
        rarity = Rarity::Rare;
    }
}

WeaponCard::WeaponCard(const std::string& name, const std::string& description, Character* target,
                       Rarity rarity)
    : Card(name, description, target, CardType::Equipment),
    rarity(rarity), isRandom(true), specificWeaponName("") {
    weapon = nullptr; // 在使用时随机生成
}

WeaponCard::~WeaponCard() {

}

bool WeaponCard::use(Character* target) {
    if (!target) {
        return false;
    }

    // 如果是随机装备，生成随机装备
    if (isRandom && !weapon) {
        std::vector<std::string>* weaponList = nullptr;

        switch (rarity) {
        case Rarity::Common:
            weaponList = &commonWeaponList;
            break;
        case Rarity::Rare:
            weaponList = &rareWeaponList;
            break;
        case Rarity::Epic:
            weaponList = &epicWeaponList;
            break;
        case Rarity::Legendary:
            weaponList = &legendaryWeaponList;
            break;
        }

        if (!weaponList || weaponList->empty()) {
            return false;
        }

        std::uniform_int_distribution<int> dist(0, weaponList->size() - 1);
        std::string weaponName = (*weaponList)[dist(rng)];
        weapon = createWeaponByName(weaponName);
    }

    if (!weapon) {
        return false;
    }

    // 为目标装备武器
    target->setWeapon(weapon);
    return true;
}

// ============ 具体卡牌实现 ============

// 攻击卡1：火球术
class FireballCard : public AttackCard {
public:
    FireballCard(Character* target = nullptr)
        : AttackCard("火球术", "造成30点火焰伤害", target, 30, Status::Normal, 0) {}
};

// 攻击卡2：冰锥术
class IceSpikeCard : public AttackCard {
public:
    IceSpikeCard(Character* target = nullptr)
        : AttackCard("冰锥术", "造成20点伤害，麻痹敌人1回合",
                     target, 20, Status::Paralyzed, 1) {}
};

// 攻击卡3：闪电链
class LightningChainCard : public AttackCard {
public:
    LightningChainCard(Character* target = nullptr)
        : AttackCard("闪电链", "造成15点伤害，麻痹敌人2回合",
                     target, 15, Status::Paralyzed, 2) {}
};

// 增益卡1：力量祝福
class StrengthBlessingCard : public BuffCard {
public:
    StrengthBlessingCard(Character* target = nullptr)
        : BuffCard("力量祝福", "攻击力+30%，持续3回合",
                   target, Status::attackAdd, 0.3f, 3, false) {}
};

// 增益卡2：守护之光
class GuardianLightCard : public BuffCard {
public:
    GuardianLightCard(Character* target = nullptr)
        : BuffCard("守护之光", "获得相当于最大生命50%的护盾，持续2回合",
                   target, Status::shieldAdd, 0.5f, 2, false) {}
};

// 增益卡3：迅捷之风
class SwiftWindCard : public BuffCard {
public:
    SwiftWindCard(Character* target = nullptr)
        : BuffCard("迅捷之风", "闪避率+30%，持续2回合",
                   target, Status::missrateAdd, 0.3f, 2, false) {}
};

// 事件卡1：神秘宝箱
class MysteryTreasureCard : public EventCard {
public:
    MysteryTreasureCard(Character* target = nullptr)
        : EventCard("神秘宝箱", "开启宝箱，随机获得金币 / 装备 / 治疗",
                    target, EventType::Treasure, false) {}
};

// 事件卡2：命运骰子
class FateDiceCard : public EventCard {
public:
    FateDiceCard(Character* target = nullptr)
        : EventCard("挑战卡牌", "挑战强敌，胜利后获得丰厚奖励",
                    target, EventType::Challenge, true) {}
};

// 事件卡3：商人
class MercantCard : public EventCard {
public:
    MercantCard(Character* target = nullptr)
        : EventCard("商人卡牌", "立即开启一次商店",
                    target, EventType::Merchant, false) {}
};

// 装备卡1：普通装备补给
class RareEquipmentCard : public WeaponCard {
public:
    RareEquipmentCard(Character* target = nullptr)
        : WeaponCard("装备补给", "随机获得 1 件罕见品质装备",
                     target, Rarity::Rare) {}
};

// 装备卡2：史诗装备补给
class EpicEquipmentCard : public WeaponCard {
public:
    EpicEquipmentCard(Character* target = nullptr)
        : WeaponCard("史诗装备", "随机获得 1 件史诗品质装备",
                     target, Rarity::Epic) {}
};

// 装备卡3：传奇装备召唤
class LegendaryWeaponCard : public WeaponCard {
public:
    LegendaryWeaponCard(Character* target = nullptr)
        : WeaponCard("神器召唤", "随机获得 1 件传奇品质装备",
                     target, Rarity::Legendary) {}
};

// ============ 卡牌工厂函数 ============
Card* createCardByName(const std::string& cardName) {
    if (cardName == "火球术") return new FireballCard();
    if (cardName == "冰锥术") return new IceSpikeCard();
    if (cardName == "闪电链") return new LightningChainCard();
    if (cardName == "力量祝福") return new StrengthBlessingCard();
    if (cardName == "守护之光") return new GuardianLightCard();
    if (cardName == "迅捷之风") return new SwiftWindCard();
    if (cardName == "神秘宝箱") return new MysteryTreasureCard();
    if (cardName == "挑战卡牌") return new FateDiceCard();
    if (cardName == "商人卡牌") return new MercantCard();
    if (cardName == "装备补给") return new RareEquipmentCard();
    if (cardName == "史诗装备") return new EpicEquipmentCard();
    if (cardName == "神器召唤") return new LegendaryWeaponCard();

    return nullptr;
}

AttackCard* createAttackCard(const std::string& cardName) {
    if (cardName == "火球术") return new FireballCard();
    if (cardName == "冰锥术") return new IceSpikeCard();
    if (cardName == "闪电链") return new LightningChainCard();
    return nullptr;
}

BuffCard* createBuffCard(const std::string& cardName) {
    if (cardName == "力量祝福") return new StrengthBlessingCard();
    if (cardName == "守护之光") return new GuardianLightCard();
    if (cardName == "迅捷之风") return new SwiftWindCard();
    return nullptr;
}

EventCard* createEventCard(const std::string& cardName) {
    if (cardName == "神秘宝箱") return new MysteryTreasureCard();
    if (cardName == "挑战卡牌") return new FateDiceCard();
    if (cardName == "商人卡牌") return new MercantCard();
    return nullptr;
}

WeaponCard* createWeaponCard(const std::string& cardName) {
    if (cardName == "装备补给") return new RareEquipmentCard();
    if (cardName == "史诗装备") return new EpicEquipmentCard();
    if (cardName == "神器召唤") return new LegendaryWeaponCard();
    return nullptr;
}

// ============ 卡牌列表函数 ============
std::vector<std::string> getAllCardNames() {
    std::vector<std::string> cardList;

    // 攻击卡
    cardList.push_back("火球术");
    cardList.push_back("冰锥术");
    cardList.push_back("闪电链");

    // 增益卡
    cardList.push_back("力量祝福");
    cardList.push_back("守护之光");
    cardList.push_back("迅捷之风");

    // 事件卡
    cardList.push_back("神秘宝箱");
    cardList.push_back("挑战卡牌");
    cardList.push_back("商人卡牌");

    // 装备卡
    cardList.push_back("装备补给");
    cardList.push_back("史诗装备");
    cardList.push_back("神器召唤");

    return cardList;
}

std::vector<std::string> getCardsByType(CardType type) {
    std::vector<std::string> result;

    switch (type) {
    case CardType::Attack:
        result = {"火球术", "冰锥术", "闪电链"};
        break;
    case CardType::Skill:
        result = {"力量祝福", "守护之光", "迅捷之风"};
        break;
    case CardType::Event:
        result = {"神秘宝箱", "挑战卡牌", "商人卡牌"};
        break;
    case CardType::Equipment:
        result = {"装备补给", "史诗装备", "神器召唤"};
        break;
    default:
        result = {};
        break;
    }

    return result;
}

// ============ CardManager类实现 ============
CardManager::CardManager() {
    // 初始化时创建所有可用卡牌
    std::vector<std::string> allCardNames = getAllCardNames();
    for (const auto& cardName : allCardNames) {
        Card* card = createCardByName(cardName);
        if (card) {
            allCards.push_back(card);
        }
    }
}

CardManager::~CardManager() {
    // 清理内存
    for (Card* card : allCards) {
        delete card;
    }
    allCards.clear();

    playerDeck.clear();
}

void CardManager::addCard(Card* card) {
    if (card) {
        allCards.push_back(card);
    }
}

void CardManager::removeCard(Card* card) {
    auto it = std::find(allCards.begin(), allCards.end(), card);
    if (it != allCards.end()) {
        allCards.erase(it);
    }
}

Card* CardManager::getCardByName(const std::string& name) const {
    for (Card* card : allCards) {
        if (card->getName() == name) {
            return card;
        }
    }
    return nullptr;
}

void CardManager::addToDeck(Card* card) {
    if (card) {
        playerDeck.push_back(card);
    }
}

void CardManager::removeFromDeck(Card* card) {
    auto it = std::find(playerDeck.begin(), playerDeck.end(), card);
    if (it != playerDeck.end()) {
        playerDeck.erase(it);
    }
}

Card* CardManager::generateRandomCard() {
    std::vector<std::string> allNames = getAllCardNames();
    if (allNames.empty()) {
        return nullptr;
    }

    std::uniform_int_distribution<int> dist(0, allNames.size() - 1);
    std::string randomCardName = allNames[dist(rng)];
    return createCardByName(randomCardName);
}

Card* CardManager::generateCardByType(CardType type) {
    std::vector<std::string> cardsOfType = getCardsByType(type);
    if (cardsOfType.empty()) {
        return nullptr;
    }

    std::uniform_int_distribution<int> dist(0, cardsOfType.size() - 1);
    std::string randomCardName = cardsOfType[dist(rng)];
    return createCardByName(randomCardName);
}

std::vector<Card*> CardManager::generateStarterDeck() {
    std::vector<Card*> starterDeck;

    // 添加一些基础卡牌
    starterDeck.push_back(createCardByName("火球术"));
    starterDeck.push_back(createCardByName("火球术"));
    starterDeck.push_back(createCardByName("冰锥术"));
    starterDeck.push_back(createCardByName("力量祝福"));
    starterDeck.push_back(createCardByName("守护之光"));
    starterDeck.push_back(createCardByName("装备补给"));
    starterDeck.push_back(createCardByName("装备补给"));
    starterDeck.push_back(createCardByName("神秘宝箱"));

    return starterDeck;
}

bool CardManager::saveDeckToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    for (Card* card : playerDeck) {
        file << card->getName() << std::endl;
    }

    file.close();
    return true;
}

bool CardManager::loadDeckFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }

    // 清空当前卡组
    for (Card* card : playerDeck) {
        delete card;
    }
    playerDeck.clear();

    std::string cardName;
    while (std::getline(file, cardName)) {
        Card* card = createCardByName(cardName);
        if (card) {
            playerDeck.push_back(card);
        }
    }

    file.close();
    return true;
}