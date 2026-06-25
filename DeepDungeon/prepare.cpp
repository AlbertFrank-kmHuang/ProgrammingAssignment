#include "prepare.h"
#include "ui_prepare.h"
#include <QMessageBox>
#include <QDebug>
#include <QEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
#include <algorithm>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDataStream>
#include <QGraphicsDropShadowEffect>
#include <QRandomGenerator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QRandomGenerator>
#include <QStandardPaths>

#include "character_config.h"
#include "weapon_config.h"
#include "benchlabel.h"
#include "battle_engine.h"
#include "enemy_config.h"
#include "battle.h"
#include "event.h"
#include "card_config.h"
prepare::prepare(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::prepare)
{
    ui->setupUi(this);
    // 沿用 gametheme.h 的深渊地牢主题，仅针对备战席补充必要的覆盖
    this->setStyleSheet(
        "QLabel { color: #e8dcc0; font-weight: bold; } "
        "QMessageBox { background-color: #0e0b07; } "
        "QMessageBox QLabel { color: #d8cba8; } "
        "QMessageBox QPushButton { min-width: 90px; min-height: 32px; }"
        );
    ui->money_show->setText(QString::number(money));
    ui->count_show->setText(QString::number(count));
    ui->money_show->setStyleSheet("font-size: 16pt; font-weight: bold; color: #f0d878; background: transparent;");
    ui->count_show->setStyleSheet("font-size: 16pt; font-weight: bold; color: #f0d878; background: transparent;");
    // 【新增】利用系统全局真随机，生成这一局的唯一种子
    currentRunSeed = QRandomGenerator::global()->generate();
    // 初始化备战席（8个空槽位）
    benchItems.resize(MAX_BENCH_SIZE, nullptr);
    // 初始化对战席（4个空槽位）
    fighters.resize(MAX_FIGHTER_SIZE, nullptr);

    // 设置备战席标签
    QVector<BenchLabel*> benchLabels = {ui->bench1, ui->bench2, ui->bench3, ui->bench4,
                                         ui->bench5, ui->bench6, ui->bench7, ui->bench8};
    for (int i = 0; i < benchLabels.size(); ++i) {
        benchLabels[i]->setIndex(i);
        benchLabels[i]->setText("空");
        benchLabels[i]->setAlignment(Qt::AlignCenter);
        benchLabels[i]->installEventFilter(this); // 备战席已经自己处理了拖拽，但为了统一也安装
    }

    // 初始化对战席（使用 FighterLabel）
    QVector<FighterLabel*> fighterLabels = {ui->fighter1, ui->fighter2, ui->fighter3, ui->fighter4};
    for (int i = 0; i < fighterLabels.size(); ++i) {
        fighterLabels[i]->setIndex(i);
        fighterLabels[i]->installEventFilter(this);
    }

    // 为对战席图片和名字标签启用接收拖拽
    QVector<QLabel*> fighterImgs = {ui->fighter1, ui->fighter2, ui->fighter3, ui->fighter4};
    for (auto lbl : fighterImgs) {
        lbl->setAcceptDrops(true);   // 覆盖 FighterLabel 默认的 false
    }
    QVector<QLabel*> fighterNames = {ui->fighter1_name, ui->fighter2_name, ui->fighter3_name, ui->fighter4_name};
    for (auto lbl : fighterNames) {
        lbl->setAcceptDrops(true);
    }

    // 为出售区控件安装事件过滤器
    if (ui->drag_sell) {
        ui->drag_sell->installEventFilter(this);
        ui->drag_sell->setAcceptDrops(true);
    }

    setAcceptDrops(true);

    // 刷新显示
    refreshBenchDisplay();
    refreshFightersDisplay();
    // 【新增】设置所有 QLineEdit 和 QTextEdit 为只读
    setLineEditsAndTextEditsReadOnly();

    // 捕获各子控件的初始几何，用于全屏/窗口缩放时等比还原
    for (QObject* obj : children()) {
        if (QWidget* w = qobject_cast<QWidget*>(obj)) {
            m_originalGeometries[w] = w->geometry();
        }
    }
}

void prepare::setLineEditsAndTextEditsReadOnly()
{
    // 获取 prepare 窗口中的所有 QLineEdit 控件
    QList<QLineEdit*> lineEdits = this->findChildren<QLineEdit*>();

    // 设置所有 QLineEdit 为只读
    for (QLineEdit* lineEdit : lineEdits) {
        lineEdit->setReadOnly(true);
    }

    // 获取 prepare 窗口中的所有 QTextEdit 控件
    QList<QTextEdit*> textEdits = this->findChildren<QTextEdit*>();

    // 设置所有 QTextEdit 为只读
    for (QTextEdit* textEdit : textEdits) {
        textEdit->setReadOnly(true);
    }
}

prepare::~prepare()
{
    // 释放所有角色装备的武器（它们不在数组中）
    auto releaseEquippedWeapons = [](BenchItem* item) {
        if (item && item->type == "character" && item->equippedWeapon) {
            if (item->equippedWeapon->data) delete static_cast<Weapon*>(item->equippedWeapon->data);
            delete item->equippedWeapon;
            item->equippedWeapon = nullptr;
        }
    };
    for (auto ptr : benchItems) releaseEquippedWeapons(ptr);
    for (auto ptr : fighters) releaseEquippedWeapons(ptr);

    // 释放备战席中的指针（此时角色装备武器已释放，避免 double free）
    for (auto ptr : benchItems) {
        if (ptr) {
            if (ptr->type == "character" && ptr->data) delete static_cast<Character*>(ptr->data);
            else if (ptr->type == "weapon" && ptr->data) delete static_cast<Weapon*>(ptr->data);
            delete ptr;
        }
    }
    // 释放对战席中的指针
    for (auto ptr : fighters) {
        if (ptr) {
            if (ptr->type == "character" && ptr->data) delete static_cast<Character*>(ptr->data);
            else if (ptr->type == "weapon" && ptr->data) delete static_cast<Weapon*>(ptr->data);
            delete ptr;
        }
    }
    delete ui;
}

void prepare::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/images/backgrounds/prepare_background.png");
    if (!background.isNull()) {
        // 获取当前窗口的设备像素比（Retina 屏幕通常为 2.0）
        qreal dpr = this->devicePixelRatioF();
        // 目标物理尺寸 = 窗口逻辑尺寸 × 像素比
        QSize targetSize = size() * dpr;
        // 将原图缩放到物理尺寸
        QPixmap scaled = background.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        // 设置 Pixmap 的像素比，使其与窗口匹配
        scaled.setDevicePixelRatio(dpr);
        // 绘制到窗口（rect() 是逻辑矩形，但 pixmap 的高分辨率会自动适配）
        painter.drawPixmap(rect(), scaled);
    } else {
        painter.fillRect(rect(), Qt::darkGray);
    }
}

// 发光效果
void setBorderGlow(QLabel* label, int starLevel) {
    if (!label) return;
    // 移除旧的特效（避免内存泄漏，先删除旧的）
    QGraphicsDropShadowEffect *oldEffect = qobject_cast<QGraphicsDropShadowEffect*>(label->graphicsEffect());
    if (oldEffect) {
        delete oldEffect;
    }
    if (starLevel == 2) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(label);
        effect->setColor(QColor(0, 100, 255, 200)); // 蓝色光
        effect->setBlurRadius(80);
        effect->setOffset(0, 0);
        label->setGraphicsEffect(effect);
    } else if (starLevel == 3) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(label);
        effect->setColor(QColor(255, 165, 0, 200)); // 橙色光
        effect->setBlurRadius(80);
        effect->setOffset(0, 0);
        label->setGraphicsEffect(effect);
    } else if (starLevel == 4) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(label);
        effect->setColor(QColor(138, 43, 226, 200)); // 紫色光
        effect->setBlurRadius(80);
        effect->setOffset(0, 0);
        label->setGraphicsEffect(effect);
    } else if (starLevel == 5) {
        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(label);
        effect->setColor(QColor(255, 0, 0, 200)); // 红色光
        effect->setBlurRadius(100); // 5星光效更亮
        effect->setOffset(0, 0);
        label->setGraphicsEffect(effect);
    } else {
        label->setGraphicsEffect(nullptr);
    }
}


void prepare::refreshSlot(QLabel* imgLabel, QLabel* nameLabel, BenchItem* item,
                          const QString& slotType, int index) {
    Q_UNUSED(slotType);
    Q_UNUSED(index);
    if (!imgLabel) return;

    // 获取 QLabel 的设备像素比
    qreal dpr = imgLabel->devicePixelRatioF();
    QSize targetSize = imgLabel->size() * dpr;

    // 1. 空槽位处理
    if (!item) {
        imgLabel->clear();
        imgLabel->setText("空");
        imgLabel->setAlignment(Qt::AlignCenter);
        imgLabel->setScaledContents(false);
        if (nameLabel) nameLabel->setText("");
        QLabel *star = imgLabel->findChild<QLabel*>("starLabel");
        if (star) star->deleteLater();
        QLabel *weapon = imgLabel->findChild<QLabel*>("weaponLabel");
        if (weapon) weapon->deleteLater();
        imgLabel->setGraphicsEffect(nullptr);
        return;
    }

    // 2. 有物品：显示基础图片或文字
    QPixmap pix(item->imagePath);
    if (!pix.isNull()) {
        // 高质量缩放
        QPixmap scaled = pix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        imgLabel->setPixmap(scaled);
        imgLabel->setText("");
    } else {
        imgLabel->setPixmap(QPixmap());
        if (item->type == "card") {
            imgLabel->setText(item->name);
        } else {
            imgLabel->setText(QString("[%1]").arg(item->name));
        }
        imgLabel->setAlignment(Qt::AlignCenter);
    }

    if (nameLabel) nameLabel->setText(item->name);

    // 3. 角色专用处理（特效、星级、武器图标）
    if (item->type == "character") {
        // 边框发光效果
        setBorderGlow(imgLabel, item->starLevel);

        // 右下角星级标签
        QLabel *starLabel = imgLabel->findChild<QLabel*>("starLabel");
        if (!starLabel) {
            starLabel = new QLabel(imgLabel);
            starLabel->setObjectName("starLabel");
            starLabel->setAlignment(Qt::AlignCenter);
            starLabel->setStyleSheet("background-color: rgba(0,0,0,120); color: gold; font-weight: bold; border-radius: 10px;");
            starLabel->setFixedSize(20, 16);
        }
        starLabel->setText(QString("★%1").arg(item->starLevel));
        int x = imgLabel->width() - starLabel->width() - 3;
        int y = imgLabel->height() - starLabel->height() - 3;
        starLabel->move(x, y);
        starLabel->raise();
        starLabel->show();

        // 左下角武器图标
        QLabel *weaponLabel = imgLabel->findChild<QLabel*>("weaponLabel");
        if (!weaponLabel) {
            weaponLabel = new QLabel(imgLabel);
            weaponLabel->setObjectName("weaponLabel");
            weaponLabel->setFixedSize(20, 20);
            weaponLabel->setStyleSheet("background-color: rgba(0,0,0,80); border-radius: 4px;");
            weaponLabel->setAlignment(Qt::AlignCenter);
        }

        if (item->equippedWeapon) {
            QPixmap weaponPix(item->equippedWeapon->imagePath);
            if (!weaponPix.isNull()) {
                qreal weaponDpr = weaponLabel->devicePixelRatioF();
                QSize weaponTargetSize = weaponLabel->size() * weaponDpr;
                QPixmap scaledWeapon = weaponPix.scaled(weaponTargetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                scaledWeapon.setDevicePixelRatio(weaponDpr);
                weaponLabel->setPixmap(scaledWeapon);
                weaponLabel->setText("");
            } else {
                weaponLabel->setText(item->equippedWeapon->name);
            }
        } else {
            weaponLabel->clear();
            weaponLabel->setText("");
        }

        weaponLabel->move(5, imgLabel->height() - weaponLabel->height() - 3);
        weaponLabel->raise();
        weaponLabel->show();
    } else {
        // 非角色物品：清除角标和特效
        QLabel *star = imgLabel->findChild<QLabel*>("starLabel");
        if (star) star->deleteLater();
        QLabel *weapon = imgLabel->findChild<QLabel*>("weaponLabel");
        if (weapon) weapon->deleteLater();
        imgLabel->setGraphicsEffect(nullptr);
    }
}

// prepare.cpp
// 在合适位置添加以下代码

// 计算并显示羁绊效果
void prepare::updateBondEffects() {
    if (!ui->magic_done) {
        return;
    }

    // 统计各职业数量
    int mageCount = 0;      // 法师
    int warriorCount = 0;   // 剑士
    int archerCount = 0;    // 弓箭手
    int namelessCount = 0;  // 无名之人

    // 遍历出战席
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        if (fighters[i] && fighters[i]->type == "character" && fighters[i]->data) {
            Character* c = static_cast<Character*>(fighters[i]->data);
            CharacterClass career = c->getJob();
            if (career == CharacterClass::Mage) {
                mageCount++;
            } else if (career == CharacterClass::Warrior) {
                warriorCount++;
            } else if (career == CharacterClass::Archer) {
                archerCount++;
            } else if (career == CharacterClass::Nonamer) {
                namelessCount++;
            }
        }
    }

    // 构建羁绊描述
    QString bondText = "当前羁绊效果：\n";
    bool hasBond = false;

    // 法师羁绊
    if (mageCount >= 2) {
        hasBond = true;
        bondText += QString("【%1法师】所有人法力增加50点\n").arg(mageCount);
        if (mageCount >= 4) {
            bondText += "  → 激活四法师：所有人法力增加100点，生命值增加100点\n";
        }
    }

    // 剑士羁绊
    if (warriorCount >= 2) {
        hasBond = true;
        bondText += QString("【%1剑士】所有人生命增加50点\n").arg(warriorCount);
        if (warriorCount >= 4) {
            bondText += "  → 激活四剑士：所有人生命增加100点，防御力增加50点\n";
        }
    }

    // 弓箭手羁绊
    if (archerCount >= 2) {
        hasBond = true;
        bondText += QString("【%1弓箭手】所有人攻击力增加20点\n").arg(archerCount);
        if (archerCount >= 4) {
            bondText += "  → 激活四弓箭手：所有人攻击力增加50点，闪避率增加30%\n";
        }
    }

    // 无名之人羁绊
    if (namelessCount >= 2) {
        hasBond = true;
        bondText += QString("【%1无名之人】所有人暴击伤害增加50点\n").arg(namelessCount);
        if (namelessCount >= 4) {
            bondText += "  → 激活四无名之人：所有人暴击率增加50点，暴击伤害增加80点\n";
        }
    }

    // ===== 新增：组合羁绊显示 =====
    bool hasW = warriorCount >= 1, hasM = mageCount >= 1,
        hasA = archerCount >= 1, hasN = namelessCount >= 1;

    // 四职同辉：四种职业各至少一人
    if (hasW && hasM && hasA && hasN) {
        hasBond = true;
        bondText += "【四职同辉】四种职业齐全！全队攻击+30，防御+30，暴击率+20%\n";
    }

    // 战法相济：剑士 + 法师
    if (hasW && hasM) {
        hasBond = true;
        bondText += "【战法相济】剑士与法师并肩！全队攻击+20，法力+30\n";
    }

    // 疾影连射：弓箭手 + 无名之人
    if (hasA && hasN) {
        hasBond = true;
        bondText += "【疾影连射】游猎与暗影！全队暴击伤害+30%，闪避+10%\n";
    }

    // 铁壁神佑：剑士≥2 且 法师≥1
    if (warriorCount >= 2 && mageCount >= 1) {
        hasBond = true;
        bondText += "【铁壁神佑】坚盾与圣光！全队生命+60，防御+20\n";
    }

    if (!hasBond) {
        bondText = "当前无羁绊效果\n";
    }

    // 更新显示
    ui->magic_done->setPlainText(bondText);
}

// 更新羁绊统计
void prepare::refreshBondStats() {
    updateBondEffects();
}

// ---------- 备战席函数 ----------
bool prepare::isBenchFull() const {
    for (auto ptr : benchItems) if (!ptr) return false;
    return true;
}

int prepare::getBenchSize() const {
    int cnt = 0;
    for (auto ptr : benchItems) if (ptr) cnt++;
    return cnt;
}

BenchItem* prepare::getBenchItem(int index) const {
    if (index < 0 || index >= MAX_BENCH_SIZE) return nullptr;
    return benchItems[index];
}

void prepare::setBenchItem(int index, BenchItem* item) {
    if (index < 0 || index >= MAX_BENCH_SIZE) return;
    benchItems[index] = item;
}

void prepare::addItemToBench(const QString& type, const QString& name,
                             const QString& imagePath, void* data, int price) {
    int emptyIndex = -1;
    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        if (!benchItems[i]) { emptyIndex = i; break; }
    }
    if (emptyIndex == -1) {
        QMessageBox::warning(this, "备战席已满", QString("最多只能容纳 %1 个物品！").arg(MAX_BENCH_SIZE));
        return;
    }
    BenchItem* newItem = new BenchItem;
    newItem->equippedWeapon = nullptr;
    newItem->type = type;
    newItem->name = name;
    newItem->imagePath = imagePath;
    newItem->data = data;
    newItem->price = price;
    benchItems[emptyIndex] = newItem;
    refreshBenchDisplay();
}


void prepare::refreshBenchDisplay() {
    QVector<QLabel*> benchLabels = {ui->bench1, ui->bench2, ui->bench3, ui->bench4,
                                     ui->bench5, ui->bench6, ui->bench7, ui->bench8};
    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        // 备战席没有单独的名字标签，传入 nullptr
        refreshSlot(benchLabels[i], nullptr, benchItems[i], "bench", i);
    }
}

void prepare::swapBenchItems(int fromIndex, int toIndex) {
    // 边界检查
    if (fromIndex < 0 || fromIndex >= MAX_BENCH_SIZE ||
        toIndex < 0 || toIndex >= MAX_BENCH_SIZE)
        return;
    if (fromIndex == toIndex) return;

    BenchItem* fromItem = benchItems[fromIndex];
    BenchItem* toItem = benchItems[toIndex];

    // ---------- 合并升星分支 ----------
    if (fromItem && toItem &&
        fromItem->type == "character" && toItem->type == "character" &&
        fromItem->name == toItem->name &&
        fromItem->starLevel == toItem->starLevel &&
        fromItem->starLevel < 5) {

        // 1. 处理源角色的武器：将其放到源槽位（脱下武器，变成独立物品）
        if (fromItem->equippedWeapon) {
            benchItems[fromIndex] = fromItem->equippedWeapon;  // 武器占据源槽位
            fromItem->equippedWeapon = nullptr;               // 角色不再持有武器
        } else {
            benchItems[fromIndex] = nullptr;                  // 无武器则槽位清空
        }

        // 2. 目标角色升星（目标角色的武器保留不变）
        toItem->starLevel++;
        // 同步角色对象的等级
        Character* targetChar = static_cast<Character*>(toItem->data);
        if (targetChar) {
            targetChar->levelUp();  // 这会增加角色的攻击、生命等属性
            // 【新增】如果达到5星，给予额外属性奖励
            if (toItem->starLevel == 5) {
                QMessageBox::information(this, "满星突破",
                                         QString("%1 已达到最高星级！")
                                             .arg(QString::fromStdString(targetChar->getName())));
            }
        }

        // 3. 释放源角色的动态数据（Character 对象）和 BenchItem 本身
        if (fromItem->data) delete static_cast<Character*>(fromItem->data);
        delete fromItem;

        // 4. 刷新备战席显示（武器图标、星级等会更新）
        refreshBenchDisplay();

        // 5. 更新每个备战席标签的索引（拖拽时需要）
        QVector<BenchLabel*> benchLabels = {ui->bench1, ui->bench2, ui->bench3, ui->bench4,
                                             ui->bench5, ui->bench6, ui->bench7, ui->bench8};
        for (int i = 0; i < benchLabels.size(); ++i) {
            if (benchLabels[i]) benchLabels[i]->setIndex(i);
        }
        return;
    }

    // ---------- 普通交换分支 ----------
    std::swap(benchItems[fromIndex], benchItems[toIndex]);
    refreshBenchDisplay();

    // 更新索引
    QVector<BenchLabel*> benchLabels = {ui->bench1, ui->bench2, ui->bench3, ui->bench4,
                                         ui->bench5, ui->bench6, ui->bench7, ui->bench8};
    for (int i = 0; i < benchLabels.size(); ++i) {
        if (benchLabels[i]) benchLabels[i]->setIndex(i);
    }
}

void prepare::showBenchItemDetail(int index) {
    BenchItem* item = getBenchItem(index);
    if (!item) return;

    QDialog dlg(this);
    dlg.setWindowTitle(item->name + " - " + item->type);
    dlg.resize(460, 420);
    dlg.setMinimumSize(380, 340);

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(14, 14, 14, 12);
    root->setSpacing(10);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(14);

    QLabel* imgLbl = new QLabel(&dlg);
    imgLbl->setFixedSize(100, 100);
    imgLbl->setAlignment(Qt::AlignCenter);
    imgLbl->setStyleSheet("background-color: rgba(0,0,0,160); border: 2px solid #6b4e1e; border-radius: 6px;");

    QPixmap pix(item->imagePath);
    if (!pix.isNull()) {
        imgLbl->setPixmap(pix.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        imgLbl->setText(item->name.left(2));
        imgLbl->setStyleSheet(imgLbl->styleSheet() + " color: #c9a227; font-size: 20px; font-weight: bold;");
    }

    QVBoxLayout* infoCol = new QVBoxLayout();
    infoCol->setSpacing(5);

    QLabel* nameLbl = new QLabel(item->name, &dlg);
    nameLbl->setStyleSheet("font-size: 14pt; font-weight: bold; color: #f0d878;");
    nameLbl->setWordWrap(true);

    QString typeStr = (item->type == "character") ? QString::fromUtf8("\xe8\xa7\x92\xe8\x89\xb2") :
                      (item->type == "weapon")    ? QString::fromUtf8("\xe6\xad\xa6\xe5\x99\xa8") :
                                                    QString::fromUtf8("\xe5\x8d\xa1\xe7\x89\x8c");
    QString starStr2 = (item->starLevel > 0) ? QString(" %1").arg(item->starLevel) : QString();
    QLabel* subLbl = new QLabel(typeStr + starStr2 + "   " + QString::fromUtf8("\xe4\xbb\xb7\xe6\xa0\xbc") + ": " +
                                QString::number(item->price), &dlg);
    subLbl->setStyleSheet("color: #c9a227; font-size: 11px;");

    infoCol->addWidget(nameLbl);
    infoCol->addWidget(subLbl);
    if (item->type == "character" && item->equippedWeapon) {
        QLabel* eqLbl = new QLabel(QString::fromUtf8("\xe8\xa3\x85\xe5\xa4\x87") + ": " +
                                   item->equippedWeapon->name, &dlg);
        eqLbl->setStyleSheet("color: #a0c8ff; font-size: 11px;");
        infoCol->addWidget(eqLbl);
    }
    infoCol->addStretch();

    topRow->addWidget(imgLbl);
    topRow->addLayout(infoCol, 1);

    QTextBrowser* detail = new QTextBrowser(&dlg);
    detail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString infoText;
    if (item->type == "character" && item->data) {
        Character* c = static_cast<Character*>(item->data);
        if (item->equippedWeapon && item->equippedWeapon->data) {
            Weapon* w = static_cast<Weapon*>(item->equippedWeapon->data);
            infoText += QString::fromUtf8("[ \xe6\xad\xa6\xe5\x99\xa8\xe6\x95\x88\xe6\x9e\x9c: ") +
                        QString::fromStdString(w->getDescription()) + " ]\n\n";
        }
        infoText += QString::fromStdString(c->getInfo());
    } else if (item->type == "weapon" && item->data) {
        Weapon* w = static_cast<Weapon*>(item->data);
        infoText = QString::fromStdString(w->getDescription());
    } else if (item->type == "card") {
        Card* card = createCardByName(item->name.toStdString());
        if (card) { infoText = QString::fromStdString(card->getDescription()); delete card; }
        else infoText = QString::fromUtf8("\xe6\x97\xa0\xe6\xb3\x95\xe8\x8e\xb7\xe5\x8f\x96\xe5\x8d\xa1\xe7\x89\x8c\xe4\xbf\xa1\xe6\x81\xaf");
    }
    detail->setPlainText(infoText);

    QPushButton* okBtn = new QPushButton(QString::fromUtf8("\xe5\x85\xb3\xe9\x97\xad"), &dlg);
    okBtn->setFixedHeight(40);
    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    root->addLayout(topRow);
    root->addWidget(detail, 1);
    root->addWidget(okBtn);

    dlg.exec();
}

//出售
void prepare::sellFromBench(int benchIndex) {
    BenchItem* item = getBenchItem(benchIndex);
    if (!item) return;

    // 处理角色佩戴武器的情况
    if (item->type == "character" && item->equippedWeapon) {
        // 将武器留在原地（原槽位），然后删除角色
        BenchItem* weapon = item->equippedWeapon;
        item->equippedWeapon = nullptr;  // 解除角色与武器的关联

        // 删除角色对象（包括 Character 数据和 BenchItem）
        if (item->data) delete static_cast<Character*>(item->data);
        delete item;
        // 原槽位现在指向武器（武器成为独立物品）
        benchItems[benchIndex] = weapon;
        refreshBenchDisplay();
        return;
    }

    // 正常出售（非角色 或 角色无武器）
    int sellPrice = static_cast<int>(std::ceil(item->price * 0.5));
    money += sellPrice;
    updateMoneyUI();

    // 释放资源
    if (item->type == "character" && item->data) delete static_cast<Character*>(item->data);
    else if (item->type == "weapon" && item->data) delete static_cast<Weapon*>(item->data);
    delete item;
    benchItems[benchIndex] = nullptr;
    refreshBenchDisplay();
}

// ===========对战席函数==========
bool prepare::isFighterSlotEmpty(int index) const {
    return (index >= 0 && index < MAX_FIGHTER_SIZE && fighters[index] == nullptr);
}

BenchItem* prepare::getFighterItem(int index) const {
    if (index < 0 || index >= MAX_FIGHTER_SIZE) return nullptr;
    return fighters[index];
}

void prepare::setFighterItem(int index, BenchItem* item) {
    if (index < 0 || index >= MAX_FIGHTER_SIZE) return;
    fighters[index] = item;
}

void prepare::refreshFightersDisplay() {
    QVector<QLabel*> imgLabels = {ui->fighter1, ui->fighter2, ui->fighter3, ui->fighter4};
    QVector<QLabel*> nameLabels = {ui->fighter1_name, ui->fighter2_name, ui->fighter3_name, ui->fighter4_name};
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        refreshSlot(imgLabels[i], nameLabels[i], fighters[i], "fighter", i);
    }
}

void prepare::moveToFighter(int benchIndex, int fighterIndex) {
    BenchItem* source = getBenchItem(benchIndex);
    if (!source) return;
    // 只允许角色上阵
    if (source->type != "character") {
        QMessageBox::warning(this, "上阵失败", "只有角色可以上阵！");
        return;
    }
    BenchItem* target = fighters[fighterIndex];
    if (target) {
        // 【新增】只允许与角色交换
        if (target->type != "character") {
            QMessageBox::warning(this, "交换失败", "只能与角色交换位置！");
            return;
        }
        // 交换：将对战席的角色放回备战席原位置
        fighters[fighterIndex] = source;
        benchItems[benchIndex] = target;
    } else {
        // 空位：直接移动
        fighters[fighterIndex] = source;
        benchItems[benchIndex] = nullptr;
    }
    refreshBenchDisplay();
    refreshFightersDisplay();
    refreshBondStats();  // 新增：刷新羁绊显示
}

void prepare::swapFighters(int fromIndex, int toIndex) {
    if (fromIndex < 0 || fromIndex >= MAX_FIGHTER_SIZE ||
        toIndex < 0 || toIndex >= MAX_FIGHTER_SIZE) return;
    if (fromIndex == toIndex) return;
    std::swap(fighters[fromIndex], fighters[toIndex]);
    refreshFightersDisplay();
    refreshBondStats();  // 新增：刷新羁绊显示
}




// ---------- 武器佩戴与调整 ----------
void prepare::equipWeaponToBenchCharacter(int weaponIndex, int characterIndex) {
    BenchItem* weapon = getBenchItem(weaponIndex);
    if (!weapon || weapon->type != "weapon") {
        return;
    }

    BenchItem* character = getBenchItem(characterIndex);
    if (!character || character->type != "character") {
        return;
    }


    // 1. 获取旧武器
    BenchItem* oldWeapon = character->equippedWeapon;

    // 2. 装备新武器
    character->equippedWeapon = weapon;

    // 3. 从原位置移除新武器
    benchItems[weaponIndex] = nullptr;

    // 4. 将旧武器放回原武器槽位
    if (oldWeapon) {
        benchItems[weaponIndex] = oldWeapon;
    }

    // 5. 更新角色对象的武器状态
    if (character->data && weapon->data) {
        Character* c = static_cast<Character*>(character->data);
        Weapon* w = static_cast<Weapon*>(weapon->data);

        // 先移除旧武器（如果有）
        if (oldWeapon && oldWeapon->data) {
            Weapon* oldW = static_cast<Weapon*>(oldWeapon->data);
        }

        // 装备新武器
        c->setWeapon(w);
    }

    // 6. 如果旧武器是另一个角色的装备，需要更新那个角色的状态
    if (oldWeapon && oldWeapon->data) {
        // 检查是否有其他角色装备了这把旧武器
        for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
            if (benchItems[i] && benchItems[i] != character && benchItems[i]->type == "character") {
                if (benchItems[i]->equippedWeapon == oldWeapon) {
                    benchItems[i]->equippedWeapon = nullptr;
                    if (benchItems[i]->data) {
                        Character* otherC = static_cast<Character*>(benchItems[i]->data);
                        otherC->removeWeapon();
                    }
                }
            }
        }
        for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
            if (fighters[i] && fighters[i] != character && fighters[i]->type == "character") {
                if (fighters[i]->equippedWeapon == oldWeapon) {
                    fighters[i]->equippedWeapon = nullptr;
                    if (fighters[i]->data) {
                        Character* otherC = static_cast<Character*>(fighters[i]->data);
                        otherC->removeWeapon();
                    }
                }
            }
        }
    }

    refreshBenchDisplay();
    refreshFightersDisplay();  // 强制刷新所有UI

}

void prepare::equipWeaponToFighterCharacter(int weaponIndex, int fighterIndex) {
    BenchItem* weapon = getBenchItem(weaponIndex);
    if (!weapon || weapon->type != "weapon") {
        return;
    }

    BenchItem* character = fighters[fighterIndex];
    if (!character || character->type != "character") {
        return;
    }


    BenchItem* oldWeapon = character->equippedWeapon;
    character->equippedWeapon = weapon;
    benchItems[weaponIndex] = oldWeapon;  // 将旧武器放回武器槽

    // 更新角色对象的武器状态
    if (character->data && weapon->data) {
        Character* c = static_cast<Character*>(character->data);
        Weapon* w = static_cast<Weapon*>(weapon->data);

        // 先移除旧武器（如果有）
        if (oldWeapon && oldWeapon->data) {
            Weapon* oldW = static_cast<Weapon*>(oldWeapon->data);
        }

        // 装备新武器
        c->setWeapon(w);
    }

    // 如果旧武器是另一个角色的装备，需要更新那个角色的状态
    if (oldWeapon && oldWeapon->data) {
        // 检查是否有其他角色装备了这把旧武器
        for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
            if (benchItems[i] && benchItems[i] != character && benchItems[i]->type == "character") {
                if (benchItems[i]->equippedWeapon == oldWeapon) {
                    benchItems[i]->equippedWeapon = nullptr;
                    if (benchItems[i]->data) {
                        Character* otherC = static_cast<Character*>(benchItems[i]->data);
                        otherC->removeWeapon();
                    }
                }
            }
        }
        for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
            if (fighters[i] && fighters[i] != character && fighters[i]->type == "character") {
                if (fighters[i]->equippedWeapon == oldWeapon) {
                    fighters[i]->equippedWeapon = nullptr;
                    if (fighters[i]->data) {
                        Character* otherC = static_cast<Character*>(fighters[i]->data);
                        otherC->removeWeapon();
                    }
                }
            }
        }
    }

    refreshBenchDisplay();
    refreshFightersDisplay();

}

std::vector<Character*> prepare::generateEnemiesForFloor(int floor) {

    std::vector<Character*> enemies;
    int enemiesGenerated = 0;  // 计数器

    // 使用层数作为随机种子
    QRandomGenerator prng(currentRunSeed + floor * 999);

    // 1. 确定敌人星级
    int enemyLevel = 1;
    if (floor <= 10) {
        enemyLevel = 1;  // 前10关为1星
    } else if (floor <= 20) {
        enemyLevel = 2;  // 10-20关为2星
    } else {
        enemyLevel = 3;  // 20关以上为3星
    }

    // 2. 确定敌人总数
    int totalEnemies=floor/10+2;

    // 3. 判断关卡类型
    bool isBossFloor = (floor % 10 == 0);  // 10的倍数关卡
    bool isEliteFloor = (floor % 5 == 0) && !isBossFloor;  // 5的倍数但不是10的倍数

    // 4. 根据不同情况生成敌人
    if (isBossFloor) {

        int bossCount = 0;
        int otherCount = 0;

        // 确定Boss数量和其他敌人数量
        if (floor <= 30) {
            // 前30层boss一定只生成一个，剩下的怪为小怪
            bossCount = 1;
            otherCount = totalEnemies - 1;
        } else if (floor <= 60) {
            // 30-60层boss2个，剩下的怪随机从小怪和精英怪里面生成
            bossCount = 2;
            otherCount = totalEnemies - 2;
        } else if (floor <= 90) {
            // 60-90层boss3个，剩下的怪全是精英怪
            bossCount = 3;
            otherCount = totalEnemies - 3;
        } else {
            // 90层以上生成四个boss
            bossCount = 4;
            otherCount = totalEnemies - 4;
        }

        // 确保不会出现负数
        if (otherCount < 0) otherCount = 0;


        // 生成Boss
        for (int i = 0; i < bossCount; ++i) {
            int bossType = prng.bounded(4);
            Character* boss = nullptr;


            switch (bossType) {
            case 0:
                boss = new SwordMaster("剑圣", EnemyType::Boss, enemyLevel);
                break;
            case 1:
                boss = new Archmage("大法师", EnemyType::Boss, enemyLevel);
                break;
            case 2:
                boss = new ArrowGod("箭神", EnemyType::Boss, enemyLevel);
                break;
            case 3:
                boss = new VoidKing("虚无之王", EnemyType::Boss, enemyLevel);
                break;
            default:
                boss = new SwordMaster("剑圣", EnemyType::Boss, enemyLevel);
                break;
            }

            if (boss) {
                enemies.push_back(boss);
                enemiesGenerated++;
            }
        }

        // 生成其他敌人
        for (int i = 0; i < otherCount; ++i) {
            Character* enemy = nullptr;

            if (floor <= 30) {
                // 前30层：剩下的怪为小怪
                enemy = generateNormalEnemy(prng, enemyLevel);
            } else if (floor <= 60) {
                // 30-60层：剩下的怪随机从小怪和精英怪里面生成
                if (prng.bounded(2) == 0) {  // 50%概率生成精英怪
                    enemy = generateEliteEnemy(prng, enemyLevel);
                } else {
                    enemy = generateNormalEnemy(prng, enemyLevel);
                }
            } else{
                // 剩下的怪全是精英怪
                enemy = generateEliteEnemy(prng, enemyLevel);
            }

            if (enemy) {
                enemies.push_back(enemy);
                enemiesGenerated++;
            }
        }
    }
    else if (isEliteFloor) {
        // 精英关卡：全部生成精英怪
        for (int i = 0; i < totalEnemies; ++i) {
            Character* enemy = generateEliteEnemy(prng, enemyLevel);
            if (enemy) {
                enemies.push_back(enemy);
                enemiesGenerated++;
            }
        }
    }
    else {
        // 普通关卡：全部生成普通怪
        for (int i = 0; i < totalEnemies; ++i) {
            Character* enemy = generateNormalEnemy(prng, enemyLevel);
            if (enemy) {
                enemies.push_back(enemy);
                enemiesGenerated++;
            }
        }
    }

    return enemies;
}

// 生成普通敌人
Character* prepare::generateNormalEnemy(QRandomGenerator& prng, int level) {
    int type = prng.bounded(4);
    Character* enemy = nullptr;

        switch (type) {
        case 0:
            enemy = new BerserkerWarrior("狂暴剑士", EnemyType::Normal, level);
            break;
        case 1:
            enemy = new FrostMage("冰霜法师", EnemyType::Normal, level);
            break;
        case 2:
            enemy = new Ranger("游侠", EnemyType::Normal, level);
            break;
        case 3:
            enemy = new ChaosApostle("混沌使徒", EnemyType::Normal, level);
            break;
        default:
            qWarning() << "      无效的敌人类型，使用默认";
            enemy = new BerserkerWarrior("狂暴剑士", EnemyType::Normal, level);
            break;
        }

    return enemy;
}

// 生成精英敌人
Character* prepare::generateEliteEnemy(QRandomGenerator& prng, int level) {
    int type = prng.bounded(4);
    Character* enemy = nullptr;

        switch (type) {
        case 0:
            enemy = new BloodBlade("血刃战士", EnemyType::Elite, level);
            break;
        case 1:
            enemy = new ElementalMage("元素法师", EnemyType::Elite, level);
            break;
        case 2:
            enemy = new Sniper("狙击手", EnemyType::Elite, level);
            break;
        case 3:
            enemy = new VoidWalker("虚空行者", EnemyType::Elite, level);
            break;
        default:
            qWarning() << "      无效的精英敌人类型，使用默认";
            enemy = new BloodBlade("血刃战士", EnemyType::Elite, level);
            break;
        }

    return enemy;
}

// ==========================================
// 2. 真实处理 EventWidget 发来的奖励
// ==========================================
void prepare::applyEventReward(const EventReward& reward) {
    if (reward.cost > 0) {
        if (this->money < reward.cost) {
            QMessageBox::warning(this, "金币不足",
                                 QString("你的金币不足，需要%1金币，但只有%2金币。").arg(reward.cost).arg(this->money));

            // 如果奖励是武器，且需要购买，但金币不足，需要删除武器对象避免内存泄漏
            if (reward.type == RewardType::Weapon && reward.weapon) {
                delete reward.weapon;
            }
            return;
        }
        // 扣除金币
        this->money -= reward.cost;
        updateMoneyUI();
    }
    switch(reward.type) {
    case RewardType::Gold:
        this->money += reward.value;
        this->updateMoneyUI();
        QMessageBox::information(this, "事件奖励", QString("你获得了 %1 金币！").arg(reward.value));
        break;

    case RewardType::HealAll:
        for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
            if (fighters[i] && fighters[i]->type == "character" && fighters[i]->data) {
                Character* c = static_cast<Character*>(fighters[i]->data);
                int newHp = std::min(c->getHealth() + reward.value, c->getMaxHealth() + c->gethealthadd());
                c->setHealth(newHp);
            }
        }
        QMessageBox::information(this, "事件奖励", QString("你对战席上的队伍恢复了 %1 点生命值！").arg(reward.value));
        break;

    case RewardType::StarUpgrade:
        if (reward.target&&reward.target->getLevel()<5) {
            reward.target->levelUp(); // 调用你们写好的升级函数
            // 寻找UI数据同步星级显示
            for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
                if (fighters[i] && fighters[i]->data == reward.target) fighters[i]->starLevel++;
            }
            QMessageBox::information(this, "实力飞跃", QString("%1 获得了突破，属性大幅提升！").arg(QString::fromStdString(reward.target->getName())));
            refreshFightersDisplay();
        }
        else{
            QMessageBox::information(this, "已满星", QString("%1 已满星，升级无效！").arg(QString::fromStdString(reward.target->getName())));
        }
        break;

    case RewardType::Weapon:
        if (reward.weapon) {
            QString wName = QString::fromStdString(reward.weapon->getName());
            QString wImg = QString(":/images/weapons/%1.png").arg(wName);
            this->addItemToBench("weapon", wName, wImg, reward.weapon, 5);
            QMessageBox::information(this, "稀世珍宝", QString("你获得了装备【%1】，已放入备战席！").arg(wName));
        }
        break;
    }
}
bool prepare::isEventFloor(int floor) {
    if (floor % 5 == 0) return false; // Boss层绝对不可能是事件
    // 使用特殊的乘数作为种子，保证每层的事件判定是固定的！
    QRandomGenerator prng(currentRunSeed+floor * 777);
    return prng.bounded(100) < 20; // 20% 概率
}
// ---------- 其他函数 ----------
void prepare::updateMoneyUI() {
    ui->money_show->setText(QString::number(money));
}

// ==========================================
// 3. 点击"开始游戏"：掷骰子决定是奇遇还是实战
// ==========================================
void prepare::on_btn_start_clicked() {

    QVector<FighterInfo> pInfo = getFightersInfo();

    if (pInfo.isEmpty()) {
        QMessageBox::warning(this, "阵容空虚", "至少需要放置一名角色在对战席上！");
        return;
    }

    std::vector<Character*> playerTeam;
    for (auto info : pInfo) {
        Character* c = info.character;

        if (info.weapon) {
            c->setWeapon(info.weapon);
        } else {
            c->removeWeapon();
        }
        playerTeam.push_back(c);
    }


    int currentFloor = (count == 0) ? 1 : count;

    // ----- 20% 概率触发随机事件 (Boss层除外) -----
    if (isEventFloor(currentFloor)) {

        EventType eType = static_cast<EventType>(QRandomGenerator::global()->bounded(4));

        EventWidget* eventPage = new EventWidget(this);

        eventPage->setEventInfo(eType, playerTeam);
        eventPage->setAttribute(Qt::WA_DeleteOnClose);

        // 信号1：直接拿奖励（宝箱/商人）
        connect(eventPage, &EventWidget::rewardGranted, this, [=, this](EventReward reward) {
            applyEventReward(reward);
            this->count++; // 算作推进一层
            ui->count_show->setText(QString::number(this->count));
            this->show();
        });

        // 信号2：要打架（陷阱/挑战）
        connect(eventPage, &EventWidget::fightTriggered, this, [=, this](EventReward reward) {
                std::vector<Character*> enemyTeam = generateEnemiesForFloor(currentFloor);

                BattleEngine* engine = new BattleEngine(playerTeam, enemyTeam);

                battle* battlePage = new battle(engine, nullptr);

                battlePage->setAttribute(Qt::WA_DeleteOnClose);

                // 设置界面大小和位置
                battlePage->resize(this->size());
                battlePage->move(this->pos());
                if (this->isFullScreen()) {
                    battlePage->showFullScreen();
                } else {
                    battlePage->show();
                }

                connect(battlePage, &battle::battleFinished, this, [=, this](bool isWin) {

                    if (isWin) {
                        this->count++;
                        ui->count_show->setText(QString::number(this->count));
                        applyEventReward(reward);

                        this->money += 10;
                        updateMoneyUI();
                        saveToFile(getSaveFilePath());

                    } else {
                        QMessageBox::critical(this, "陨落", "你在事件挑战中牺牲了...");
                        handleBattleDefeat();
                    }

                    this->show();
                });

                eventPage->close();
        });

        // 信号3：拒绝事件（避战）
        connect(eventPage, &EventWidget::eventRejected, this, [=, this]() {
            this->count++;
            ui->count_show->setText(QString::number(this->count));
            this->show();
        });

        // 显示事件页面
        eventPage->resize(this->size());
        eventPage->move(this->pos());
        if (this->isFullScreen()) {
            eventPage->showFullScreen();
        } else {
            eventPage->show();
        }
        this->hide();

        return;
    }

    // ----- 普通战斗流程 -----

        std::vector<Character*> enemyTeam = generateEnemiesForFloor(currentFloor);
        // ... 生成敌人 ...

        BattleEngine* engine = new BattleEngine(playerTeam, enemyTeam);
        battle* battlePage = new battle(engine, nullptr);

        // 【关键修复】设置窗口属性
        battlePage->setAttribute(Qt::WA_DeleteOnClose);
        battlePage->setWindowModality(Qt::ApplicationModal);  // 设置为模态窗口

        // 设置界面
        battlePage->resize(this->size());
        battlePage->move(this->pos());
        if (this->isFullScreen()) {
            battlePage->showFullScreen();
        } else {
            battlePage->show();
        }

        // 【关键修复】正确连接战斗结束信号
        connect(battlePage, &battle::battleFinished, this, [=, this, battlePage, engine](bool isWin) {

            // 【重要】先处理战斗结果
            onBattleFinished(isWin);

            // 【重要】延迟关闭战斗窗口
            QTimer::singleShot(1000, this, [battlePage, engine]() {
                battlePage->close();  // 关闭战斗窗口
                battlePage->deleteLater();  // 确保窗口被删除

                // 清理战斗引擎
                if (engine) {
                    delete engine;
                }
            });
        });

        this->hide();
}

void prepare::on_btn_shop_clicked() {
    if (shopPage == nullptr) {
        shopPage = new shop(this);
        connect(shopPage, &shop::backToPrepare, this, &prepare::show);
    }
    shopPage->updateMoneyUI();
    // 【新增】设置商店界面的大小和位置，跟随prepare界面
    shopPage->resize(this->size());
    shopPage->move(this->pos());

    // 【新增】如果是全屏状态，也将商店界面设置为全屏
    if (this->isFullScreen()) {
        shopPage->showFullScreen();
    } else {
        shopPage->show();
    }
    this->hide();
}

void prepare::on_btn_review_clicked() {
    if (reviewPage == nullptr) {
        reviewPage = new review(this); // 确保传 this 进去
        connect(reviewPage, &review::backToPrepare, this, &prepare::show);
    }

    // 计算即将面对的层数（如果是刚开始，count=0，面对的是第一层）
    int currentFloor = (count == 0) ? 1 : count;

    // 让情报界面生成这一层的情报
    reviewPage->updateIntel(currentFloor);

    // 【修改】设置review窗口的大小、位置和全屏状态，与prepare窗口保持一致
    reviewPage->resize(this->size());
    reviewPage->move(this->pos());

    if (this->isFullScreen()) {
        reviewPage->showFullScreen();
    } else {
        reviewPage->show();
    }

    this->hide();
}

bool prepare::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
        if (dragEvent->mimeData()->hasFormat("application/x-benchindex")) {
            dragEvent->setDropAction(Qt::MoveAction);
            dragEvent->accept();
            return true;
        }
    }
    else if (event->type() == QEvent::Drop) {
        QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
        if (dropEvent->mimeData()->hasFormat("application/x-benchindex")) {
            QByteArray data = dropEvent->mimeData()->data("application/x-benchindex");
            QDataStream stream(&data, QIODevice::ReadOnly);
            QString sourceType;
            int fromIndex;
            stream >> sourceType >> fromIndex;

            // ---- 目标：备战席 ----
            QVector<BenchLabel*> benchLabels = {ui->bench1, ui->bench2, ui->bench3, ui->bench4,
                                                 ui->bench5, ui->bench6, ui->bench7, ui->bench8};
            int benchIdx = -1;
            for (int i = 0; i < benchLabels.size(); ++i) {
                if (obj == benchLabels[i]) {
                    benchIdx = i;
                    break;
                }
            }
            if (benchIdx != -1) {
                if (sourceType == "bench") {
                    BenchItem* srcItem = getBenchItem(fromIndex);
                    BenchItem* dstItem = benchItems[benchIdx];
                    // 武器佩戴到备战席角色
                    if (srcItem && srcItem->type == "weapon" && dstItem && dstItem->type == "character") {
                        equipWeaponToBenchCharacter(fromIndex, benchIdx);
                    }
                    else if (srcItem && srcItem->type == "card" && dstItem && dstItem->type == "character") {
                        applyBuffCard(fromIndex, dstItem);
                    }
                    else {
                        swapBenchItems(fromIndex, benchIdx);
                    }
                } else if (sourceType == "fighter") {
                    moveFromFighterToBench(fromIndex, benchIdx);
                }
                dropEvent->accept();
                return true;
            }

            // ---- 目标：对战席 ----
            QVector<QLabel*> fighterImgs = {ui->fighter1, ui->fighter2, ui->fighter3, ui->fighter4};
            QVector<QLabel*> fighterNames = {ui->fighter1_name, ui->fighter2_name, ui->fighter3_name, ui->fighter4_name};
            int fighterIdx = -1;
            for (int i = 0; i < fighterImgs.size(); ++i) {
                if (obj == fighterImgs[i] || obj == fighterNames[i]) {
                    fighterIdx = i;
                    break;
                }
            }
            if (fighterIdx != -1) {
                if (sourceType == "bench") {
                    BenchItem* srcItem = getBenchItem(fromIndex);
                    BenchItem* dstItem = fighters[fighterIdx];
                    // 武器佩戴到对战席角色
                    if (srcItem && srcItem->type == "weapon" && dstItem && dstItem->type == "character") {
                        equipWeaponToFighterCharacter(fromIndex, fighterIdx);
                    }
                    //卡牌对对战席角色使用
                    else if (srcItem && srcItem->type == "card" && dstItem && dstItem->type == "character") {
                        applyBuffCardToFighter(fromIndex, fighterIdx);
                    }
                    else {
                        moveToFighter(fromIndex, fighterIdx);
                    }
                } else if (sourceType == "fighter") {
                    swapFighters(fromIndex, fighterIdx);
                }
                dropEvent->accept();
                return true;
            }

            // ---- 目标：出售区 ----
            if (obj == ui->drag_sell) {
                if (sourceType == "bench") {
                    sellFromBench(fromIndex);
                } else if (sourceType == "fighter") {
                    QMessageBox::warning(this, "提示", "对战席角色不能直接出售，请先移回备战席。");
                }
                dropEvent->accept();
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void prepare::showFighterDetail(int index) {
    BenchItem* item = getFighterItem(index);
    if (!item) return;

    QDialog dlg(this);
    dlg.setWindowTitle(item->name + " - " + QString::fromUtf8("\xe5\xaf\xb9\xe6\x88\x98\xe5\xb8\xad\xe8\xaf\xa6\xe6\x83\x85"));
    dlg.resize(460, 420);
    dlg.setMinimumSize(380, 340);

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(14, 14, 14, 12);
    root->setSpacing(10);

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(14);

    QLabel* imgLbl = new QLabel(&dlg);
    imgLbl->setFixedSize(100, 100);
    imgLbl->setAlignment(Qt::AlignCenter);
    imgLbl->setStyleSheet("background-color: rgba(0,0,0,160); border: 2px solid #c9a227; border-radius: 6px;");

    QPixmap pix(item->imagePath);
    if (!pix.isNull()) {
        imgLbl->setPixmap(pix.scaled(96, 96, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        imgLbl->setText(item->name.left(2));
        imgLbl->setStyleSheet(imgLbl->styleSheet() + " color: #f0d878; font-size: 20px; font-weight: bold;");
    }

    QVBoxLayout* infoCol = new QVBoxLayout();
    infoCol->setSpacing(5);

    QLabel* nameLbl = new QLabel(item->name, &dlg);
    nameLbl->setStyleSheet("font-size: 14pt; font-weight: bold; color: #f0d878;");
    nameLbl->setWordWrap(true);

    QLabel* subLbl = new QLabel(QString::fromUtf8("\xe6\x98\x9f\xe7\xba\xa7: %1").arg(item->starLevel) +
        "   " + QString::fromUtf8("\xe4\xbb\xb7\xe6\xa0\xbc") + ": " + QString::number(item->price), &dlg);
    subLbl->setStyleSheet("color: #c9a227; font-size: 11px;");

    infoCol->addWidget(nameLbl);
    infoCol->addWidget(subLbl);
    if (item->equippedWeapon) {
        QLabel* eqLbl = new QLabel(QString::fromUtf8("\xe8\xa3\x85\xe5\xa4\x87") + ": " +
                                   item->equippedWeapon->name, &dlg);
        eqLbl->setStyleSheet("color: #a0c8ff; font-size: 11px;");
        infoCol->addWidget(eqLbl);
    }
    infoCol->addStretch();

    topRow->addWidget(imgLbl);
    topRow->addLayout(infoCol, 1);

    QTextBrowser* detail = new QTextBrowser(&dlg);
    detail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString infoText;
    if (item->type == "character" && item->data) {
        Character* c = static_cast<Character*>(item->data);
        if (item->equippedWeapon && item->equippedWeapon->data) {
            Weapon* w = static_cast<Weapon*>(item->equippedWeapon->data);
            infoText += QString::fromUtf8("[ \xe6\xad\xa6\xe5\x99\xa8\xe6\x95\x88\xe6\x9e\x9c: ") +
                        QString::fromStdString(w->getDescription()) + " ]\n\n";
        }
        infoText += QString::fromStdString(c->getInfo());
    } else if (item->type == "weapon" && item->data) {
        Weapon* w = static_cast<Weapon*>(item->data);
        infoText = QString::fromStdString(w->getDescription());
    }
    detail->setPlainText(infoText);

    QPushButton* okBtn = new QPushButton(QString::fromUtf8("\xe5\x85\xb3\xe9\x97\xad"), &dlg);
    okBtn->setFixedHeight(40);
    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    root->addLayout(topRow);
    root->addWidget(detail, 1);
    root->addWidget(okBtn);

    dlg.exec();
}

void prepare::moveFromFighterToBench(int fighterIndex, int benchIndex) {
    if (fighterIndex < 0 || fighterIndex >= MAX_FIGHTER_SIZE) return;
    if (benchIndex < 0 || benchIndex >= MAX_BENCH_SIZE) return;
    BenchItem* source = fighters[fighterIndex];
    if (!source) return;
    // 只能移动角色（可根据需求放宽）
    if (source->type != "character") {
        QMessageBox::warning(this, "移动失败", "只有角色可以移动到备战席！");
        return;
    }
    BenchItem* target = benchItems[benchIndex];
    if (target) {
        // 【新增】只允许与角色交换
        if (target->type != "character") {
            QMessageBox::warning(this, "交换失败", "只能与角色交换位置！");
            return;
        }
        // 交换
        fighters[fighterIndex] = target;
        benchItems[benchIndex] = source;
    } else {
        // 直接移动
        fighters[fighterIndex] = nullptr;
        benchItems[benchIndex] = source;
    }
    refreshBenchDisplay();
    refreshFightersDisplay();
    refreshBondStats();  // 新增：刷新羁绊显示
}

//=====提取Fighter（即对战席上角色）信息，供battle使用=====
QVector<FighterInfo> prepare::getFightersInfo() const {
    QVector<FighterInfo> result;
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        BenchItem* item = fighters[i];
        if (item && item->type == "character") {
            FighterInfo info;
            info.character = static_cast<Character*>(item->data);
            info.starLevel = item->starLevel;
            info.weapon = (item->equippedWeapon && item->equippedWeapon->type == "weapon")
                              ? static_cast<Weapon*>(item->equippedWeapon->data) : nullptr;
            result.append(info);
        }
    }
    return result;
}


// =========================================================
// 卡牌作用于 出战席角色
// =========================================================

void prepare::handleMerchantCard() {

    QMessageBox::information(this, "商人卡牌",
                             "你使用了商人卡牌，召唤了神秘的旅行商人！\n"
                             "现在可以在商店中购买传奇级别的武器。\n"
                             "注意：传奇武器价格更高，但属性更强！");

    // 打开商店界面
    if (shopPage == nullptr) {
        shopPage = new shop(this);
        connect(shopPage, &shop::backToPrepare, this, [this]() {
            this->show();
            refreshBenchDisplay();
            refreshFightersDisplay();
        });
    }

    // 设置商店为传说武器模式
    shopPage->setLegendaryWeaponMode(true);

    // 设置商店为模态窗口
    shopPage->setWindowModality(Qt::ApplicationModal);
    shopPage->setWindowFlags(shopPage->windowFlags() | Qt::WindowStaysOnTopHint);

    // 连接关闭信号
    connect(shopPage, &shop::backToPrepare, this, [this]() {
        this->show();
        shopPage->hide();
    });

    shopPage->resize(this->size());
    shopPage->move(this->pos());
    if (this->isFullScreen()) {
        shopPage->showFullScreen();
    } else {
        shopPage->show();
    }
    this->hide();
}

void prepare::handleChallengeCard() {
    QMessageBox::information(this, "挑战卡牌",
                             "你使用了挑战卡牌，触发了强者挑战事件！\n"
                             "准备好迎接强大的敌人吧！\n"
                             "胜利将获得丰厚奖励！");

    // 获取当前出战队伍
    QVector<FighterInfo> pInfo = getFightersInfo();
    std::vector<Character*> playerTeam;
    for (auto info : pInfo) {
        Character* c = info.character;
        playerTeam.push_back(c);
    }

    // 生成挑战事件
    EventWidget* eventPage = new EventWidget(this);
    eventPage->setEventInfo(EventType::Challenge, playerTeam);

    // 【关键修复1】移除过于严格的模态设置
    // 只保留基本的模态，避免阻塞整个应用程序
    eventPage->setWindowModality(Qt::WindowModal);  // 改为窗口模态，而不是应用模态
    eventPage->setAttribute(Qt::WA_DeleteOnClose);  // 确保窗口关闭时自动删除

    // 连接信号
    connect(eventPage, &EventWidget::rewardGranted, this, [=](EventReward reward) {
        applyEventReward(reward);
        this->show();  // 显示prepare窗口
        eventPage->close();  // 关闭事件窗口
    });

    connect(eventPage, &EventWidget::fightTriggered, this, [=](EventReward reward) {
        // 【关键修复2】在战斗开始前立即关闭事件窗口
        eventPage->close();

        int currentFloor = (count == 0) ? 1 : count;
        // 挑战事件使用更强的敌人
        std::vector<Character*> enemyTeam = generateEnemiesForFloor(currentFloor);
        BattleEngine* engine = new BattleEngine(playerTeam, enemyTeam);
        battle* battlePage = new battle(engine, nullptr);

        // 【关键修复3】设置战斗窗口的模态和属性
        battlePage->setWindowModality(Qt::WindowModal);
        battlePage->setAttribute(Qt::WA_DeleteOnClose);

        // 【关键修复4】连接战斗结束信号
        connect(battlePage, &battle::battleFinished, this, [=](bool isWin) {
            if (isWin) {
                this->count++;
                ui->count_show->setText(QString::number(this->count));
                applyEventReward(reward);
                for (Character* c : playerTeam) {
                    c->setalive(true);
                    c->setHealth(c->getMaxHealth() + c->gethealthadd());
                }
                QMessageBox::information(this, "挑战胜利",
                                         "你成功完成了挑战！\n"
                                         "获得额外奖励！");
            } else {
                QMessageBox::critical(this, "挑战失败",
                                      "你在挑战中败下阵来...\n"
                                      "但没有惩罚，可以重新挑战。");
            }

            // 【关键修复5】清理资源
            delete engine;
            this->show();  // 确保显示prepare窗口
        });

        // 【关键修复6】连接窗口关闭信号
        connect(battlePage, &battle::destroyed, this, [this]() {
            this->show();  // 确保战斗窗口关闭后显示prepare窗口
        });

        battlePage->resize(this->size());
        battlePage->move(this->pos());
        if (this->isFullScreen()) {
            battlePage->showFullScreen();
        } else {
            battlePage->show();
        }
        this->hide();  // 隐藏prepare窗口
    });

    connect(eventPage, &EventWidget::eventRejected, this, [=]() {
        QMessageBox::information(this, "放弃挑战", "你选择了放弃挑战。");
        this->show();  // 显示prepare窗口
        eventPage->close();  // 关闭事件窗口
    });

    eventPage->resize(this->size());
    eventPage->move(this->pos());
    if (this->isFullScreen()) {
        eventPage->showFullScreen();
    } else {
        eventPage->show();
    }
    this->hide();  // 隐藏prepare窗口
}

void prepare::applyBuffCardToFighter(int cardIndex, int fighterIndex) {
    applyBuffCard(cardIndex, fighters[fighterIndex]);
}

void prepare::applyBuffCard(int cardIndex, BenchItem* charItem) {
    BenchItem* cardItem = getBenchItem(cardIndex);

    if (!cardItem || cardItem->type != "card") {
        return;
    }
    if (!charItem || charItem->type != "character" || !charItem->data) {
        return;
    }

    // 现场生成卡牌
    Card* card = createCardByName(cardItem->name.toStdString());
    if (!card) {
        QMessageBox::warning(this, "系统错误", "无法识别的卡牌：" + cardItem->name);
        return;
    }

    Character* targetChar = static_cast<Character*>(charItem->data);

    // 2. 根据卡牌类型和名称进行不同处理
    QString cardName = cardItem->name;
    CardType cardType = card->getType();
    bool cardUsed = false;
    bool shouldDeleteCard = true;

    // 获取出战队伍
    QVector<FighterInfo> pInfo = getFightersInfo();
    std::vector<Character*> playerTeam;
    for (auto info : pInfo) {
        Character* c = info.character;
        playerTeam.push_back(c);
    }

    int currentFloor = (count == 0) ? 1 : count;

    if (cardName == "火球术" || cardName == "冰锥术" || cardName == "闪电链") {
        // 攻击法术卡牌：询问是否立即进入战斗

        std::string cardDescription = card->getDescription();
        QString description = QString::fromStdString(cardDescription);  // description 在这里定义
        QMessageBox::StandardButton reply = QMessageBox::question(this, "攻击法术",
                                                                  QString("你使用了【%1】\n\n%2\n\n"
                                                                          "攻击法术只能在战斗中使用，是否立即开始一场战斗来施放这个法术？")
                                                                      .arg(cardName)
                                                                      .arg(description),
                                                                  QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // 生成敌人
            std::vector<Character*> enemyTeam = generateEnemiesForFloor(currentFloor);

            if (enemyTeam.empty()) {
                QMessageBox::warning(this, "错误", "没有生成敌人，无法使用攻击法术！");
                delete card;
                return;
            }

            // 对第一个敌人使用攻击卡片，应用伤害和麻痹效果
            Character* firstEnemy = enemyTeam[0];
            AttackCard* attackCard = dynamic_cast<AttackCard*>(card);

            if (attackCard) {
                // 使用攻击卡片的use方法，这会应用伤害和麻痹效果
                bool success = attackCard->use(firstEnemy);

                if (success) {
                    int currentHealth = firstEnemy->getHealth();

                    // 如果敌人死亡，从队伍中移除
                    if (!firstEnemy->alive()) {
                        delete firstEnemy;
                        enemyTeam.erase(enemyTeam.begin());
                    }
                }
            }

            // 创建战斗引擎
            BattleEngine* engine = new BattleEngine(playerTeam, enemyTeam);
            battle* battlePage = new battle(engine, nullptr);

            // 【新增】设置战斗界面的大小和位置，跟随prepare界面
            battlePage->resize(this->size());
            battlePage->move(this->pos());

            // 如果是全屏状态，也将战斗界面设置为全屏
            if (this->isFullScreen()) {
                battlePage->showFullScreen();
            } else {
                battlePage->show();
            }

            connect(battlePage, &battle::battleFinished, this, [=](bool isWin) {
                if (isWin) {
                    this->count++;
                    ui->count_show->setText(QString::number(this->count));

                    // 胜利
                    updateMoneyUI();

                    for (Character* c : playerTeam) {
                        c->setalive(true);
                        c->setHealth(c->getMaxHealth() + c->gethealthadd());
                    }

                    QMessageBox::information(this, "战斗胜利",
                                             QString("你成功施放了【%1】并赢得了战斗！").arg(cardName));
                } else {
                    QMessageBox::critical(this, "战斗失败",
                                          QString("你在施放【%1】的战斗中失败了...").arg(cardName));
                }

                delete engine;
                battlePage->deleteLater();
                this->show();

            });

            battlePage->resize(this->size());
            battlePage->move(this->pos());
            if (this->isFullScreen()) {
                battlePage->showFullScreen();
            } else {
                battlePage->show();
            }
            this->hide();
            cardUsed = true;
        } else {
            // 取消使用，不删除卡牌
            QMessageBox::information(this, "取消施法",
                                     "法术施放被取消，卡牌保留。\n"
                                     "你可以在战斗中使用这张卡牌。");
            shouldDeleteCard = false;
        }
    }
    else if (cardName == "力量祝福" || cardName == "守护之光" || cardName == "迅捷之风") {
        // 增益卡牌：检查是否已存在相同类型的buff
        BuffCard* buffCard = dynamic_cast<BuffCard*>(card);
        if (buffCard) {
            Status buffType = buffCard->getBuffType();

            // 检查角色是否已有相同类型的buff
            if (targetChar->hasBuffType(buffType)) {
                QMessageBox::warning(this, "重复增益",
                                     QString("角色【%1】已经拥有【%2】效果，不能重复施加！")
                                         .arg(QString::fromStdString(targetChar->getName()))
                                         .arg(cardName));
                shouldDeleteCard = false;
            } else {
                bool useSuccess = buffCard->use(targetChar);
                if (useSuccess) {
                    QMessageBox::information(this, "获得增益",
                                             QString("%1 获得了【%2】效果，持续%3回合！")
                                                 .arg(QString::fromStdString(targetChar->getName()))
                                                 .arg(cardName)
                                                 .arg(buffCard->getDuration()));
                    cardUsed = true;
                    shouldDeleteCard = true;
                } else {
                    QMessageBox::warning(this, "使用失败", "增益卡牌使用失败！");
                    shouldDeleteCard = false;
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "增益卡牌转换失败！");
            shouldDeleteCard = false;
        }
    }
    else if (cardName == "商人卡牌" || cardName == "挑战卡牌" || cardName == "神秘宝箱") {
        // 事件卡牌：立即生成相应事件界面
        EventCard* eventCard = dynamic_cast<EventCard*>(card);
        if (eventCard) {
            if (cardName == "商人卡牌") {
                // 商人卡牌：生成商店界面
                handleMerchantCard();
                cardUsed = true;
            }
            else if (cardName == "挑战卡牌") {
                // 挑战卡牌：生成挑战事件
                handleChallengeCard();
                cardUsed = true;
            }
            else if (cardName == "神秘宝箱") {
                // 神秘宝箱：使用原有事件逻辑
                bool useSuccess = eventCard->use(targetChar);
                if (useSuccess) {
                    EventReward reward = eventCard->getReward();
                    // 确保金币奖励不为0
                    if (reward.type == RewardType::Gold && reward.value <= 0) {
                        reward.value = 0 + QRandomGenerator::global()->bounded(31); // 0-30金币
                    }

                    applyEventReward(reward);
                    cardUsed = true;
                } else {
                    shouldDeleteCard = false;
                }
            }
        } else {
            QMessageBox::warning(this, "错误", "事件卡牌转换失败！");
            shouldDeleteCard = false;
        }
    }
    else if (cardName == "装备补给" || cardName == "史诗装备" || cardName == "神器召唤") {
        // 装备补给卡牌
        WeaponCard* weaponCard = dynamic_cast<WeaponCard*>(card);
        if (weaponCard) {
            bool useSuccess = weaponCard->use(targetChar);
            if (useSuccess) {
                Weapon* newWep = weaponCard->getWeapon();
                if (newWep) {
                    QString wName = QString::fromStdString(newWep->getName());

                    QMessageBox::information(this, "神器降临",
                                             QString("成功召唤了武器【%1】！\n已放入备战席的卡牌槽位。")
                                                 .arg(wName));

                    // 创建武器的BenchItem
                    BenchItem* wItem = new BenchItem;
                    wItem->type = "weapon";
                    wItem->name = wName;
                    wItem->imagePath = QString(":/images/weapons/%1.png").arg(wName);
                    wItem->data = newWep;
                    wItem->price = 0;  // 装备的武器不标价
                    wItem->starLevel = 0;
                    wItem->equippedWeapon = nullptr;

                    // 关键：删除原卡片的BenchItem
                    if (cardItem) {
                        delete cardItem;  // 删除卡片的BenchItem
                    }

                    // 用武器的BenchItem替换槽位中的卡片BenchItem
                    benchItems[cardIndex] = wItem;

                    // 刷新显示
                    refreshBenchDisplay();
                    refreshFightersDisplay();

                    cardUsed = true;
                    shouldDeleteCard = true;
                } else {
                    QMessageBox::warning(this, "错误", "装备生成失败！");
                    shouldDeleteCard = false;
                }
            } else {
                QMessageBox::warning(this, "使用失败", "装备卡牌使用失败！");
                shouldDeleteCard = false;
            }
        } else {
            QMessageBox::warning(this, "错误", "装备卡牌转换失败！");
            shouldDeleteCard = false;
        }
    }
    else {
        // 其他未知卡牌类型
        QMessageBox::warning(this, "未知卡牌",
                             QString("未知的卡牌类型：%1\n无法使用此卡牌。")
                                 .arg(cardName));
        shouldDeleteCard = false;
    }

    // 清理卡片
    if (cardUsed && shouldDeleteCard) {

        // 删除卡片对象
        delete card;

        // 重要：对于装备卡牌，我们已经删除了cardItem并替换为武器
        // 对于其他卡牌，需要删除benchItems[cardIndex]并设置为nullptr
        if (cardName == "装备补给" || cardName == "史诗装备" || cardName == "神器召唤") {
        } else {
            // 其他卡牌：删除卡片的BenchItem
            if (cardIndex >= 0 && cardIndex < MAX_BENCH_SIZE && benchItems[cardIndex]) {
                delete benchItems[cardIndex];
                benchItems[cardIndex] = nullptr;
            }
        }
    } else if (!cardUsed && shouldDeleteCard) {

        delete card;

        if (cardIndex >= 0 && cardIndex < MAX_BENCH_SIZE && benchItems[cardIndex]) {
            delete benchItems[cardIndex];
            benchItems[cardIndex] = nullptr;

            refreshBenchDisplay();
            QCoreApplication::processEvents();
        }
    } else {
        // 卡片未使用，保留卡片
        delete card;  // 只删除创建的临时Card对象
    }

    // 强制刷新界面
    refreshBenchDisplay();
    refreshFightersDisplay();

    // 确保界面更新
    QCoreApplication::processEvents();

}

//用于存档
// 保存游戏到文件
bool prepare::saveToFile(const QString& filePath)
{
    if (isBattleDefeated) {
        return false;
    }

    QJsonObject gameState = saveGameState();
    if (gameState.isEmpty()) {
        return false;
    }

    QJsonDocument doc(gameState);
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    return true;
}

QJsonObject prepare::saveGameState() const {
    QJsonObject state;

    // 添加版本号
    state["version"] = 1;

    // 1. 保存游戏元数据
    state["currentRunSeed"] = static_cast<qint64>(currentRunSeed);
    state["money"] = money;
    state["count"] = count;

    // 2. 保存备战席
    QJsonArray benchArray;
    for (const BenchItem* item : benchItems) {
        benchArray.append(item ? benchItemToJson(item) : QJsonValue());
    }
    state["bench"] = benchArray;

    // 3. 保存对战席
    QJsonArray fighterArray;
    for (const BenchItem* item : fighters) {
        fighterArray.append(item ? benchItemToJson(item) : QJsonValue());
    }
    state["fighters"] = fighterArray;

    // 4. 保存全局武器列表（只保存名称，加载时重建）
    std::vector<QString> allWeaponNames;
    //遍历benchItems和fighters，收集所有Weapon对象的名称
    QJsonArray weaponNameArray;
    for (const auto& name : allWeaponNames) {
        weaponNameArray.append(name);
    }
    state["weaponRegistry"] = weaponNameArray;

    return state;
}

bool prepare::loadGameState(const QJsonObject& json) {
    if (json.isEmpty()) return false;

    // 验证版本
    int version = json["version"].toInt();
    if (version != 1) {
        return false;
    }

    // 清除当前状态
    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        if (benchItems[i]) {
            delete benchItems[i];
        }
        benchItems[i] = nullptr;
    }
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        if (fighters[i]) {
            delete fighters[i];
        }
        fighters[i] = nullptr;
    }

    // 1. 加载元数据
    currentRunSeed = json["currentRunSeed"].toVariant().toUInt();
    money = json["money"].toInt();
    count = json["count"].toInt();
    updateMoneyUI();
    ui->count_show->setText(QString::number(count));

    // 2. 加载备战席
    QJsonArray benchArray = json["bench"].toArray();
    for (int i = 0; i < benchArray.size() && i < MAX_BENCH_SIZE; ++i) {
        if (!benchArray[i].isNull()) {
            benchItems[i] = benchItemFromJson(benchArray[i].toObject());
        } else {
            benchItems[i] = nullptr;
        }
    }

    // 3. 加载对战席
    QJsonArray fighterArray = json["fighters"].toArray();
    for (int i = 0; i < fighterArray.size() && i < MAX_FIGHTER_SIZE; ++i) {
        if (!fighterArray[i].isNull()) {
            fighters[i] = benchItemFromJson(fighterArray[i].toObject());
        } else {
            fighters[i] = nullptr;
        }
    }

    // 4. 恢复武器装备关系
    restoreWeaponEquipment();

    // 5. 刷新界面
    refreshBenchDisplay();
    refreshFightersDisplay();

    return true;
}

QJsonObject prepare::benchItemToJson(const BenchItem* item) const {
    QJsonObject obj;
    obj["type"] = item->type;
    obj["name"] = item->name;
    obj["imagePath"] = item->imagePath;
    obj["price"] = item->price;
    obj["starLevel"] = item->starLevel;

    // 保存具体数据
    if (item->type == "character") {
        Character* c = static_cast<Character*>(item->data);
        obj["characterData"] = c->saveToJson();
    } else if (item->type == "weapon") {
        Weapon* w = static_cast<Weapon*>(item->data);
        obj["weaponData"] = w->toJson();
    }

    // 保存装备的武器（通过名称引用）
    if (item->equippedWeapon) {
        obj["equippedWeapon"] = item->equippedWeapon->name;
    }

    return obj;
}

BenchItem* prepare::benchItemFromJson(const QJsonObject& json) {
    BenchItem* item = new BenchItem;
    item->type = json["type"].toString();
    item->name = json["name"].toString();
    item->imagePath = json["imagePath"].toString();
    item->price = json["price"].toInt();
    item->starLevel = json["starLevel"].toInt();

    // 重建具体数据
    if (item->type == "character") {
        Character* c = createCharacterByName(item->name.toStdString());
        if (c) {
            c->loadFromJson(json["characterData"].toObject());
            item->data = c;
        }
    } else if (item->type == "weapon") {
        Weapon* w = createWeaponByName(item->name.toStdString());
        if (w) {
            w->fromJson(json["weaponData"].toObject());
            item->data = w;
        }
    } else {
        item->data = nullptr;
    }

    // 保存待装备的武器名称
    if (json.contains("equippedWeapon") && !json["equippedWeapon"].isNull()) {
        item->pendingEquipWeaponName = json["equippedWeapon"].toString();
    } else {
        item->pendingEquipWeaponName = "";
    }

    return item;
}

void prepare::handleBattleDefeat()
{
    isBattleDefeated = true;

    // 清除当前进度
    count = 1;  // 重置到第一关
    money = 0;  // 清空金币

    // 清空备战席和对战席
    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        delete benchItems[i];
        benchItems[i] = nullptr;
    }
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        delete fighters[i];
        fighters[i] = nullptr;
    }

    // 刷新界面
    refreshBenchDisplay();
    refreshFightersDisplay();
    updateMoneyUI();
    ui->count_show->setText(QString::number(count));

    // 显示失败消息
    QMessageBox::information(this, "游戏结束", "您的队伍全军覆没！游戏进度已重置。");

    // 返回主界面
    this->hide();
    emit backToMain();
}

// 修改战斗结束处理
void prepare::onBattleFinished(bool isPlayerWin)
{
    if (isPlayerWin) {
        // 战斗胜利
        isBattleDefeated = false;

        // 增加关卡计数
        count++;
        ui->count_show->setText(QString::number(count));
        money += 10;  // 新增：每次战斗胜利获得20金币
        // 恢复所有对战席角色的状态
        for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
            if (fighters[i] && fighters[i]->type == "character" && fighters[i]->data) {
                Character* c = static_cast<Character*>(fighters[i]->data);

                //恢复法力值
                int maxMana = c->getmaxmana();
                c->setmana(maxMana);

                //清除战斗中的临时buff
                c->getbuffs().clear();
            }
        }
        updateMoneyUI();

        // 保存游戏进度
        saveToFile(getSaveFilePath());

        // 显示准备界面
        this->show();
    } else {
        // 战斗失败
        handleBattleDefeat();
    }
}

QString prepare::getSaveFilePath() const {
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // 如果路径为空（某些极简系统），回退到当前可执行文件目录
    if (appDataPath.isEmpty()) {
        appDataPath = QCoreApplication::applicationDirPath();
    }

    QDir dir(appDataPath);
    // 确保目标目录存在
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    // 存档文件命名为 autosave.json
    return dir.absoluteFilePath("autosave.json");
}

bool prepare::loadFromFile(const QString& filePath) {
    QFile file(filePath);

    if (!file.exists()) {
        return false;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        return false;
    }

    return loadGameState(doc.object());
}

void prepare::restoreWeaponEquipment()
{
    // 1. 收集所有独立武器（type == "weapon" 的 BenchItem），记录其位置
    struct WeaponEntry {
        BenchItem* item;
        int benchIndex;   // -1 表示在 fighters 中
        int fighterIndex; // -1 表示在 bench 中
    };
    QMap<QString, QVector<WeaponEntry>> weaponMap;

    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        if (benchItems[i] && benchItems[i]->type == "weapon") {
            weaponMap[benchItems[i]->name].append({benchItems[i], i, -1});
        }
    }
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        if (fighters[i] && fighters[i]->type == "weapon") {
            weaponMap[fighters[i]->name].append({fighters[i], -1, i});
        }
    }

    // 2. 处理所有角色（备战席和出战席）
    auto processCharacter = [&](BenchItem* charItem) {
        if (!charItem || charItem->type != "character") return;
        if (charItem->pendingEquipWeaponName.isEmpty()) return;

        QString weaponName = charItem->pendingEquipWeaponName;
        charItem->pendingEquipWeaponName.clear(); // 清除标记

        // 查找是否有同名独立武器
        auto it = weaponMap.find(weaponName);
        if (it != weaponMap.end() && !it->isEmpty()) {
            // 取出第一个同名武器
            WeaponEntry entry = it->first();
            it->removeFirst(); // 从 map 中移除该条目

            // 从原位置移除该武器（置空）
            if (entry.benchIndex != -1) {
                benchItems[entry.benchIndex] = nullptr;
            } else if (entry.fighterIndex != -1) {
                fighters[entry.fighterIndex] = nullptr;
            }

            // 装备给角色
            charItem->equippedWeapon = entry.item;
            // 同步数据到 Character 对象
            if (charItem->data && entry.item->data) {
                Character* c = static_cast<Character*>(charItem->data);
                Weapon* w = static_cast<Weapon*>(entry.item->data);
                c->setWeapon(w);
            }
        } else {
            // 没有独立武器，创建新的武器实例并直接装备（不放入任何槽位）
            Weapon* weapon = createWeaponByName(weaponName.toStdString());
            if (!weapon) return;

            BenchItem* weaponItem = new BenchItem;
            weaponItem->type = "weapon";
            weaponItem->name = weaponName;
            weaponItem->imagePath = QString(":/images/weapons/%1.png").arg(weaponName);
            weaponItem->data = weapon;
            weaponItem->price = 0;
            weaponItem->starLevel = 0;
            weaponItem->equippedWeapon = nullptr;

            charItem->equippedWeapon = weaponItem;
            if (charItem->data) {
                Character* c = static_cast<Character*>(charItem->data);
                c->setWeapon(weapon);
            }
        }
    };

    // 处理备战席角色
    for (int i = 0; i < MAX_BENCH_SIZE; ++i) {
        processCharacter(benchItems[i]);
    }
    // 处理出战席角色
    for (int i = 0; i < MAX_FIGHTER_SIZE; ++i) {
        processCharacter(fighters[i]);
    }

    // 3. 未被任何角色装备的独立武器保持原位（已在 benchItems/fighters 中），无需额外操作
}

// 窗口全屏/缩放时，按照 611×462 的设计基准等比缩放所有子控件
void prepare::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_originalGeometries.isEmpty()) return;

    const double sx = double(width())  / double(DESIGN_W);
    const double sy = double(height()) / double(DESIGN_H);

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
void prepare::on_exitbutton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              "退出游戏",
                                                              "确定要退出游戏吗？\n未保存的进度将会丢失！",
                                                              QMessageBox::Yes | QMessageBox::No,
                                                              QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 先保存游戏进度
        saveToFile(getSaveFilePath());

        // 退出整个应用程序
        QApplication::quit();
    }
}