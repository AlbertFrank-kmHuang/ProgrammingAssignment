#ifndef EVENT_CONFIG_H
#define EVENT_CONFIG_H


#include <memory>
#include <random>

// 事件类型
enum class EventType {
    Treasure,      // 宝箱
    Merchant,      // 商人
    Shrine,        // 祭坛
    Trap,          // 陷阱
    Rest,          // 休息点
    Mystery,       // 神秘事件
    Challenge      // 挑战
};

// 随机事件
class RandomEvent {
protected:
    std::string name;
    std::string description;
    EventType type;
    std::mt19937 rng;

public:
    RandomEvent(const std::string& name, EventType type);
    virtual ~RandomEvent() = default;

    // 触发事件
    virtual bool trigger(std::vector<std::shared_ptr<Character>>& party,
                                std::vector<std::shared_ptr<Card>>& deck,
                                std::vector<std::shared_ptr<Equipment>>& inventory) = 0;

    std::string getName() const { return name; }
    EventType getType() const { return type; }
};

// 具体事件实现
class TreasureEvent : public RandomEvent {
private:
    std::vector<std::shared_ptr<Equipment>> possibleLoot;
    std::vector<std::shared_ptr<Card>> possibleCards;

public:
    TreasureEvent();
    EventResult trigger(std::vector<std::shared_ptr<Character>>& party,
                        std::vector<std::shared_ptr<Card>>& deck,
                        std::vector<std::shared_ptr<Equipment>>& inventory) override;
};

class MerchantEvent : public RandomEvent {
private:
    struct ShopItem {
        std::shared_ptr<Equipment> equipment = nullptr;
        std::shared_ptr<Card> card = nullptr;
        int price = 0;
    };

    std::vector<ShopItem> shopItems;

public:
    MerchantEvent();
    EventResult trigger(std::vector<std::shared_ptr<Character>>& party,
                        std::vector<std::shared_ptr<Card>>& deck,
                        std::vector<std::shared_ptr<Equipment>>& inventory) override;
};

// 事件生成器
class EventGenerator {
private:
    std::vector<std::unique_ptr<RandomEvent>> eventPool;

public:
    EventGenerator();

    // 根据关卡生成事件
    std::unique_ptr<RandomEvent> generateEvent(int stageLevel,
                                               float eventChance = 0.3f);

    // 添加事件权重
    void addEvent(std::unique_ptr<RandomEvent> event, float weight = 1.0f);
};

#endif // EVENT_CONFIG_H
