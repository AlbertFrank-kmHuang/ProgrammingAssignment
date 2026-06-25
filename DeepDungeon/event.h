#ifndef EVENT_H
#define EVENT_H

#include <QWidget>
#include <QDialog>
#include <QString>
#include <vector>
#include <string>
#include <random>

// 前向声明
class Character;
class Weapon;

namespace Ui {
class event;
}

// 事件类型
enum class EventType {
    Treasure,      // 宝箱
    Merchant,      // 商人
    Trap,          // 陷阱
    Challenge      // 挑战
};

// 奖励类型
enum class RewardType {
    StarUpgrade,       // 角色升星
    HealAll,           // 全队回血
    Gold,              // 金币
    Weapon,            // 武器
};

// 事件奖励结构
struct EventReward {
    RewardType type = RewardType::Gold;
    int value = 0;             // 用于金币数量、回血量
    Weapon* weapon = nullptr;  // 如果是武器奖励
    Character* target = nullptr;     // 如果是针对特定角色
    int cost=0;
};

// EventWidget 类，继承自 QDialog
class EventWidget : public QDialog
{
    Q_OBJECT

public:
    explicit EventWidget(QWidget *parent = nullptr);
    ~EventWidget();

    // 设置事件信息
    void setEventInfo(EventType type, const std::vector<Character*>& party);

    // 获取事件结果
    bool isAccepted() const { return accepted; }
    bool requiresFight() const { return needFight; }
    EventReward getReward() const { return reward; }
    std::vector<Character*> getParty() const { return party; }
    void setEventImage(EventType type);

signals:
    void eventAccepted();
    void eventRejected();
    void fightTriggered(EventReward reward);
    void rewardGranted(EventReward reward);

private slots:
    void onAcceptClicked();
    void onRejectClicked();

private:
    void generateEventContent();
    void createTreasureEvent();
    void createMerchantEvent();
    void createTrapEvent();
    void createChallengeEvent();
    void resizeEvent(QResizeEvent* event) override;

    // 辅助函数
    QString getEventTypeName(EventType type) const;
    QString getEventDescription(EventType type) const;
    QString getRewardDescription() const;

    Ui::event *ui;
    EventType currentType;
    bool accepted = false;
    bool needFight = false;
    EventReward reward;
    std::vector<Character*> party;
    // 保存子控件的初始几何信息
    QMap<QWidget*, QRect> m_originalGeometries;

    // EventWidget的设计基准尺寸（根据event.ui的实际设计尺寸设置）
    static constexpr int DESIGN_WIDTH = 560;
    static constexpr int DESIGN_HEIGHT = 620;
};

// 事件管理器
class EventManager {
private:
    std::mt19937 rng;

public:
    EventManager() {
        std::random_device rd;
        rng = std::mt19937(rd());
    }

    // 判断是否触发事件
    bool triggerEventAfterBattle() {
        std::uniform_int_distribution<int> dist(1, 100);
        return dist(rng) <= 30; // 30%概率触发
    }

    // 随机生成事件类型
    EventType generateRandomEvent() {
        std::uniform_int_distribution<int> dist(0, 3);
        int type = dist(rng);
        return static_cast<EventType>(type);
    }

    // 获取事件描述
    std::string getEventName(EventType type) {
        switch(type) {
        case EventType::Treasure: return "神秘宝箱";
        case EventType::Merchant: return "旅行商人";
        case EventType::Trap: return "致命陷阱";
        case EventType::Challenge: return "强者挑战";
        default: return "未知事件";
        }
    }
};

#endif // EVENT_H