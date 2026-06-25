#ifndef PREPARE_H
#define PREPARE_H

#include "shop.h"
#include "review.h"
#include "event.h"
#include "benchlabel.h"
#include <QWidget>
#include <QVector>
#include <QPixmap>
#include <QMap>
#include <QResizeEvent>
#include <QRandomGenerator>

class Character;
class Weapon;

struct BenchItem {
    QString type;        // "character", "weapon", "card"
    QString name;        // 物品名称
    QString imagePath;   // 图片路径
    void* data;          // 指向具体对象（Character* 或 Weapon* 或 nullptr for card）
    int price;
    int starLevel = 1;
    BenchItem* equippedWeapon = nullptr;
    QString pendingEquipWeaponName;  // 用于存档时临时存储装备武器名称
};

//======给battle的信息接口======
struct FighterInfo {
    Character* character;
    int starLevel;
    Weapon* weapon;
};

namespace Ui {
class prepare;
}

class prepare : public QWidget
{
    Q_OBJECT

public:
    explicit prepare(QWidget *parent = nullptr);
    ~prepare();


    bool isBenchFull() const;                   // 判断是否已满（所有槽位都有物品）
    int getMaxBenchSize() const { return MAX_BENCH_SIZE; }
    int getBenchSize() const;                   // 返回非空槽位数量
    BenchItem* getBenchItem(int index) const;
    void setBenchItem(int index, BenchItem* item);

    void addItemToBench(const QString& type, const QString& name,
                        const QString& imagePath, void* data, int price);
    void refreshBenchDisplay();
    void swapBenchItems(int fromIndex, int toIndex);   // 交换两个槽位
    void showBenchItemDetail(int index);               // 显示详情
    void sellFromBench(int benchIndex);         // 出售备战席指定槽位的物品

    // 对战席
    bool isFighterSlotEmpty(int index) const;
    BenchItem* getFighterItem(int index) const;
    void setFighterItem(int index, BenchItem* item);
    void refreshFightersDisplay();
    void moveToFighter(int benchIndex, int fighterIndex);  // 从备战席移动到对战席（或交换）
    void showFighterDetail(int index);
    void moveFromFighterToBench(int fighterIndex, int benchIndex);
    bool moveFighterToBench(int fighterIndex, int benchIndex);
    void swapFighters(int fromIndex, int toIndex);

    // 武器佩戴（源武器在备战席，目标角色在备战席或对战席）
    void equipWeaponToBenchCharacter(int weaponIndex, int characterIndex);
    void equipWeaponToFighterCharacter(int weaponIndex, int fighterIndex);
    // 武器交换（从角色身上卸下并交换）

    //======供battle使用=====
    QVector<FighterInfo> getFightersInfo() const;
    void applyBuffCardToFighter(int cardIndex, int fighterIndex);
    void applyBuffCard(int cardIndex, BenchItem* charItem);

    //独立的怪物生成器
    std::vector<Character*> generateEnemiesForFloor(int floor);
    //事件奖励结算中心
    void applyEventReward(const EventReward& reward);
    bool isEventFloor(int floor); // 预判该层是否为事件层
    // 其他
    void updateMoneyUI();
    void setLineEditsAndTextEditsReadOnly();

    int money = 50;
    int count = 0;

    // 用于存档
    // 存档功能
    bool saveToFile(const QString& filePath = "autosave.json");
    bool loadFromFile(const QString& filePath = "autosave.json");
    // 战败状态
    void setDefeated(bool defeated) { isBattleDefeated = defeated; }
    bool isDefeated() const { return isBattleDefeated; }
    // 存档数据结构
    QJsonObject saveGameState() const;
    bool loadGameState(const QJsonObject& json);
    // 内部辅助函数
    QJsonObject benchItemToJson(const BenchItem* item) const;
    BenchItem* benchItemFromJson(const QJsonObject& json);
    //失败后的处理
    void handleBattleDefeat();
    QString getSaveFilePath() const;
    Character* generateNormalEnemy(QRandomGenerator& prng, int level);
    Character* generateEliteEnemy(QRandomGenerator& prng, int level);
    void handleMerchantCard();
    void handleChallengeCard();
    void updateBondEffects();
    void refreshBondStats();
    //关于武器的更新
    void restoreWeaponEquipment();

signals:
    void backToMain();  // 返回到主界面信号

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_btn_start_clicked();
    void on_btn_shop_clicked();
    void on_btn_review_clicked();
    void onBattleFinished(bool isPlayerWin);
    void on_exitbutton_clicked();  // 新增：退出按钮点击事件

private:
    Ui::prepare *ui;
    shop *shopPage = nullptr;
    review *reviewPage = nullptr;
    unsigned int currentRunSeed; // 【新增】当前这局游戏的唯一随机种子
    void paintEvent(QPaintEvent *event) override;

    // 固定大小8的指针数组，空指针表示该槽位为空
    QVector<BenchItem*> benchItems;
    static constexpr int MAX_BENCH_SIZE = 8;

    // 对战席（固定4个槽位，空指针表示空）
    QVector<BenchItem*> fighters;
    static constexpr int MAX_FIGHTER_SIZE = 4;

    // 共用刷新函数
    void refreshSlot(QLabel* imgLabel, QLabel* nameLabel, BenchItem* item,
                     const QString& slotType, int index);

    // 用于存档
    bool isBattleDefeated = false;  // 是否因战斗失败而退出
    QMap<QWidget*, QRect> m_originalGeometries;  // 初始几何，用于全屏比例缩放
    static constexpr int DESIGN_W = 611;
    static constexpr int DESIGN_H = 462;
};


#endif // PREPARE_H