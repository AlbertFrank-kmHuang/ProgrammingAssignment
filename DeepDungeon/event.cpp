#include "event.h"
#include "ui_event.h"
#include "weapon_config.h"     // 包含武器配置
#include "character_config.h"  // 包含角色配置
#include <QMessageBox>
#include <random>
#include <QString>
#include <QTextEdit>
#include <QLabel>
#include <vector>
#include <QPixmap>



EventWidget::EventWidget(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::event)
    , accepted(false)
    , needFight(false)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("随机事件");
    setModal(true);

    // 连接按钮信号
    connect(ui->accept_button, &QPushButton::clicked, this, &EventWidget::onAcceptClicked);
    connect(ui->cancel_button, &QPushButton::clicked, this, &EventWidget::onRejectClicked);
    // 【新增】捕获各子控件的初始几何，用于全屏/窗口缩放时等比还原
    // 递归遍历所有子控件
    auto allWidgets = this->findChildren<QWidget*>();
    for (QWidget* w : allWidgets) {
        // 跳过顶层窗口自身
        if (w == this) continue;
        // 跳过布局中的spacer等非widget对象
        if (w->parent() && w->parent() != this) {
            // 只保存直接子控件，或者根据需要调整
            if (w->parent() == this || qobject_cast<QDialog*>(w->parent()) == this) {
                m_originalGeometries[w] = w->geometry();
            }
        } else {
            m_originalGeometries[w] = w->geometry();
        }
    }
}

EventWidget::~EventWidget() {
    // 【核心修复】：移交所有权管理
    // 1. 如果 accepted 为 true，说明玩家接受了事件，武器已经发到了备战席。
    //    此时武器的所有权归 prepare(备战席) 所有，绝对不能在这里 delete！
    // 2. 只有当 accepted 为 false（玩家点击了拒绝/避让），武器生成了但没发出去，
    //    此时才需要销毁它，防止内存泄漏。

    if (!accepted && reward.weapon != nullptr) {
        delete reward.weapon;
        reward.weapon = nullptr;
    }

    delete ui;
}

void EventWidget::setEventInfo(EventType type, const std::vector<Character*>& party) {
    this->party = party;
    this->currentType = type;

    // 清空文本框
    ui->text_show->clear();

    // 生成事件内容
    generateEventContent();
}

void EventWidget::generateEventContent() {
    // 设置标题
    QString title = QString("触发%1事件！").arg(getEventTypeName(currentType));
    ui->label->setText(QString("<html><head/><body><p align=\"center\"><span style=\" font-size:14pt; font-weight:700;\">%1</span></p></body></html>").arg(title));

    // 清空文本框
    ui->text_show->clear();

    // 设置事件图片
    setEventImage(currentType);

    // 显示事件信息
    ui->text_show->append(QString("事件类型: %1").arg(getEventTypeName(currentType)));
    ui->text_show->append("");
    ui->text_show->append("事件描述:");
    ui->text_show->append(getEventDescription(currentType));
    ui->text_show->append("");

    // 根据事件类型生成具体内容
    switch(currentType) {
    case EventType::Treasure:
        createTreasureEvent();
        break;
    case EventType::Merchant:
        createMerchantEvent();
        break;
    case EventType::Trap:
        createTrapEvent();
        break;
    case EventType::Challenge:
        createChallengeEvent();
        break;
    }

    ui->text_show->append("奖励信息:");
    ui->text_show->append(getRewardDescription());

    if (needFight) {
        ui->text_show->append("\n注意：这个事件需要先进行战斗！");
    }
}

QString EventWidget::getEventTypeName(EventType type) const {
    switch(type) {
    case EventType::Treasure: return "神秘宝箱";
    case EventType::Merchant: return "旅行商人";
    case EventType::Trap: return "致命陷阱";
    case EventType::Challenge: return "强者挑战";
    default: return "未知事件";
    }
}

QString EventWidget::getEventDescription(EventType type) const {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> descDist(0, 2);
    int descIndex = descDist(rng);

    switch(type) {
    case EventType::Treasure: {
        QString descs[] = {
            "你发现了一个古老的宝箱，上面覆盖着灰尘和蜘蛛网。",
            "一个闪烁着神秘光芒的宝箱静静地躺在角落里。",
            "这个宝箱看起来年代久远，但保存得相当完好。"
        };
        return descs[descIndex];
    }
    case EventType::Merchant: {
        QString descs[] = {
            "一位戴着兜帽的旅行商人向你招手，他的摊位上摆满了各种商品。",
            "神秘的商人似乎在这里等候多时，他的眼神中透露着智慧。",
            "商人微笑着展示他的商品，每一样都看起来价值不菲。"
        };
        return descs[descIndex];
    }
    case EventType::Trap: {
        QString descs[] = {
            "脚下传来机械运转的声音，你意识到自己踩到了陷阱！",
            "墙壁上的机关突然启动，暗箭从四面八方射来！",
            "地板突然下陷，一群怪物从黑暗中涌出！"
        };
        return descs[descIndex];
    }
    case EventType::Challenge: {
        QString descs[] = {
            "一位强大的战士拦住了你的去路，他的眼中闪烁着战意。",
            "挑战者向你发起决斗，胜利者将获得丰厚的奖励。",
            "竞技场的门在你面前打开，观众席上传来欢呼声。"
        };
        return descs[descIndex];
    }
    default: return "这是一个神秘的事件。";
    }
}

QString EventWidget::getRewardDescription() const {
    switch(reward.type) {
    case RewardType::StarUpgrade: {
        if (reward.target) {
            return QString("%1 可以升星！").arg(QString::fromStdString(reward.target->getName()));
        }
        return "一名随机角色可以升星！";
    }
    case RewardType::HealAll: {
        return QString("全队恢复 %1 点生命值").arg(reward.value);
    }
    case RewardType::Gold: {
        return QString("获得 %1 金币").arg(reward.value);
    }
    case RewardType::Weapon: {
        if (reward.weapon) {
            std::string rarityName = "";
            switch(reward.weapon->getRarity()) {
            case Rarity::Common: rarityName = "普通"; break;
            case Rarity::Rare: rarityName = "稀有"; break;
            case Rarity::Epic: rarityName = "史诗"; break;
            case Rarity::Legendary: rarityName = "传奇"; break;
            }
            return QString("获得%1武器: %2").arg(QString::fromStdString(rarityName), QString::fromStdString(reward.weapon->getName()));
        }
        return "获得随机武器";
    }
    default: return "未知奖励";
    }
}

void EventWidget::onAcceptClicked() {
    accepted = true;

    if (!needFight) {
        // 直接应用奖励
        emit rewardGranted(reward);
    } else {
        // 需要战斗
        emit fightTriggered(reward);
    }

    accept();  // 关闭对话框
}

void EventWidget::onRejectClicked() {
    accepted = false;
    emit eventRejected();
    reject();  // 关闭对话框
}

void EventWidget::createTreasureEvent() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 3);

    int choice = dist(rng);
    needFight = false;

    switch(choice) {
    case 0: { // 金币
        std::uniform_int_distribution<int> goldDist(50, 150);
        reward.type = RewardType::Gold;
        reward.value = goldDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        reward.cost=0;
        break;
    }
    case 1: { // 全队回血
        std::uniform_int_distribution<int> healDist(20, 50);
        reward.type = RewardType::HealAll;
        reward.value = healDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        reward.cost=0;
        break;
    }
    case 2: { // 角色升星
        reward.type = RewardType::StarUpgrade;
        reward.value = 0;
        reward.weapon = nullptr;
        reward.cost=0;
        if (!party.empty()) {
            std::uniform_int_distribution<int> charDist(0, party.size() - 1);
            reward.target = party[charDist(rng)];
        } else {
            reward.target = nullptr;
        }
        break;
    }
    case 3: { // 史诗品质武器
        reward.type = RewardType::Weapon;
        reward.value = 0;
        reward.target = nullptr;
        reward.cost=0;

        // 从史诗武器列表中随机选择一个
        std::uniform_int_distribution<int> weaponDist(0, epicWeaponList.size() - 1);
        int weaponIndex = weaponDist(rng);
        std::string weaponName = epicWeaponList[weaponIndex];
        reward.weapon = createWeaponByName(weaponName);

        if (reward.weapon) {
            // 添加额外描述
            ui->text_show->append(QString("发现了一件史诗品质的装备：%1").arg(QString::fromStdString(weaponName)));
        }
        break;
    }
    }
}

void EventWidget::createMerchantEvent() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 1);

    int choice = dist(rng);
    needFight = false;

    switch(choice) {
    case 0: { // 出售回血药
        reward.type = RewardType::HealAll;
        std::uniform_int_distribution<int> healDist(50, 100);
        reward.value = healDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        ui->text_show->append(QString("商人正在出售回血药（全队回血）"));
        reward.cost = reward.value / 2;  // 价格设为回血量的一半
        ui->text_show->append(QString("代价: 需要支付%1金币").arg(reward.cost));
        break;
    }
    case 1: { // 出售传奇品质装备
        reward.type = RewardType::Weapon;
        reward.value = 0;
        reward.cost = 100;  // 传奇武器固定价格
        reward.target = nullptr;

        // 从传奇武器列表中随机选择一个
        std::uniform_int_distribution<int> weaponDist(0, legendaryWeaponList.size() - 1);
        int weaponIndex = weaponDist(rng);
        std::string weaponName = legendaryWeaponList[weaponIndex];
        reward.weapon = createWeaponByName(weaponName);

        if (reward.weapon) {
            int price = 100;  // 传奇武器固定价格
            ui->text_show->append(QString("商人正在出售一件传奇装备：%1").arg(QString::fromStdString(weaponName)));
            ui->text_show->append(QString("代价: 需要支付%1金币").arg(price));
        }
        break;
    }
    }
}

void EventWidget::createTrapEvent() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 2);

    int choice = dist(rng);
    needFight = true;

    switch(choice) {
    case 0: { // 宝箱怪物
        std::uniform_int_distribution<int> goldDist(40, 60);
        reward.type = RewardType::Gold;
        reward.value = goldDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        reward.cost=0;
        ui->text_show->append("警告: 有怪物守护着宝藏！胜利可获得金币奖励！");
        break;
    }
    case 1: { // 传奇品质武器守卫
        reward.type = RewardType::Weapon;
        reward.value = 0;
        reward.target = nullptr;
        reward.cost=0;

        // 从传奇武器列表中随机选择一个
        std::uniform_int_distribution<int> weaponDist(0, legendaryWeaponList.size() - 1);
        int weaponIndex = weaponDist(rng);
        std::string weaponName = legendaryWeaponList[weaponIndex];
        reward.weapon = createWeaponByName(weaponName);

        if (reward.weapon) {
            ui->text_show->append(QString("警告: 强大的怪物守护着一件传奇装备：%1").arg(QString::fromStdString(weaponName)));
        }
        break;
    }
    case 2: { // 陷阱
        std::uniform_int_distribution<int> healDist(50, 100);
        reward.type = RewardType::HealAll;
        reward.value = healDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        reward.cost=0;
        ui->text_show->append("警告: 你掉进了陷阱！战斗胜利可恢复生命值！");
        break;
    }
    }
}

void EventWidget::createChallengeEvent() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 2);

    int choice = dist(rng);
    needFight = true;

    switch(choice) {
    case 0: { // 强者挑战
        reward.type = RewardType::StarUpgrade;
        reward.value = 0;
        reward.weapon = nullptr;
        reward.cost=0;
        if (!party.empty()) {
            std::uniform_int_distribution<int> charDist(0, party.size() - 1);
            reward.target = party[charDist(rng)];
        } else {
            reward.target = nullptr;
        }
        ui->text_show->append("挑战: 击败强大的对手！胜利可使随机角色升星！");
        break;
    }
    case 1: { // 竞技场传奇品质武器挑战
        reward.type = RewardType::Weapon;
        reward.value = 0;
        reward.target = nullptr;
        reward.cost=0;

        // 从传奇武器列表中随机选择一个
        std::uniform_int_distribution<int> weaponDist(0, legendaryWeaponList.size() - 1);
        int weaponIndex = weaponDist(rng);
        std::string weaponName = legendaryWeaponList[weaponIndex];
        reward.weapon = createWeaponByName(weaponName);

        if (reward.weapon) {
            ui->text_show->append(QString("挑战: 赢得竞技场比赛，获得传奇装备：%1！").arg(QString::fromStdString(weaponName)));
        }
        break;
    }
    case 2: { // 守卫挑战
        std::uniform_int_distribution<int> goldDist(40, 60);
        reward.type = RewardType::Gold;
        reward.value = goldDist(rng);
        reward.weapon = nullptr;
        reward.target = nullptr;
        reward.cost=0;
        ui->text_show->append("挑战: 击败强盗守卫！胜利可获得金币奖励！");
        break;
    }
    }
}
void EventWidget::setEventImage(EventType type) {
    QString imagePath;

    switch(type) {
    case EventType::Treasure:
        imagePath = ":/images/events/Treasure.jpg";
        break;
    case EventType::Merchant:
        imagePath = ":/images/events/Merchant.jpg";
        break;
    case EventType::Trap:
        imagePath = ":/images/events/Trap.jpg";
        break;
    case EventType::Challenge:
        imagePath = ":/images/events/Challenge.jpg";
        break;
    default:
        imagePath = "";  // 默认不显示图片
        break;
    }

    if (!imagePath.isEmpty()) {
        QPixmap eventPixmap(imagePath);
        if (!eventPixmap.isNull()) {
            // 设置图片显示
            ui->image_show->setPixmap(eventPixmap);
            ui->image_show->setScaledContents(true);  // 缩放图片以适应QLabel
        } else {
            // 图片加载失败，显示文本提示
            ui->image_show->setText("事件图片加载失败");
            ui->image_show->setAlignment(Qt::AlignCenter);
        }
    } else {
        ui->image_show->clear();  // 清空图片
    }
}
void EventWidget::resizeEvent(QResizeEvent* event)
{
    QDialog::resizeEvent(event);

    // 如果还没有保存初始几何信息，先保存
    if (m_originalGeometries.isEmpty()) {
        for (QObject* obj : children()) {
            if (QWidget* w = qobject_cast<QWidget*>(obj)) {
                m_originalGeometries[w] = w->geometry();
            }
        }
    }

    if (m_originalGeometries.isEmpty()) return;

    // 计算缩放比例
    const double sx = double(width())  / double(DESIGN_WIDTH);
    const double sy = double(height()) / double(DESIGN_HEIGHT);

    // 缩放所有保存的控件
    for (auto it = m_originalGeometries.constBegin(); it != m_originalGeometries.constEnd(); ++it) {
        QWidget* w = it.key();
        if (!w) continue;

        const QRect& orig = it.value();
        w->setGeometry(
            qRound(orig.x()      * sx),
            qRound(orig.y()      * sy),
            qRound(orig.width()  * sx),
            qRound(orig.height() * sy)
            );
    }
}