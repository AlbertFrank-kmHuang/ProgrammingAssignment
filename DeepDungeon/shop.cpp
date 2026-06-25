#include "shop.h"
#include "ui_shop.h"
#include "prepare.h"
#include "card_config.h"
#include <QTimer>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QVector>
#include <QPixmap>
#include <QDebug>
#include <QLabel>
#include <vector>
#include <string>
#include <QLineEdit>
#include <QTextEdit>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QPushButton>
extern std::vector<std::string> commonWeaponList;
extern std::vector<std::string> rareWeaponList;
extern std::vector<std::string> epicWeaponList;
extern std::vector<std::string> legendaryWeaponList;

shop::shop(prepare *parentPrepare, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::shop)
    , mainPrepare(parentPrepare)
    , legendaryWeaponMode(false)  // 初始化传说武器模式为false
{
    ui->setupUi(this);
    // 沿用 gametheme.h 的深渊地牢主题
    this->setStyleSheet(
        "QLabel { color: #e8dcc0; font-weight: bold; } "
        "QMessageBox { background-color: #0e0b07; } "
        "QMessageBox QLabel { color: #d8cba8; } "
        "QMessageBox QPushButton { min-width: 90px; min-height: 32px; }"
        );

    // 设置所有 QLineEdit 和 QTextEdit 为只读
    setLineEditsAndTextEditsReadOnly();

    updateMoneyUI();

    refreshCharacters();
    refreshWeapons();
    refreshCards();

    // 对所有可能会显示商品的标签安装事件过滤器
    // 角色部分
    QVector<QLabel*> charImgs = {ui->character1, ui->character2, ui->character3, ui->character4, ui->character5};
    QVector<QLabel*> charNames = {ui->character1_name, ui->character2_name, ui->character3_name, ui->character4_name, ui->character5_name};
    for (auto lbl : charImgs) lbl->installEventFilter(this);
    for (auto lbl : charNames) lbl->installEventFilter(this);

    // 武器部分
    QVector<QLabel*> wepImgs = {ui->weapon1, ui->weapon2, ui->weapon3, ui->weapon4, ui->weapon5};
    QVector<QLabel*> wepNames = {ui->weapon1_name, ui->weapon2_name, ui->weapon3_name, ui->weapon4_name, ui->weapon5_name};
    for (auto lbl : wepImgs) lbl->installEventFilter(this);
    for (auto lbl : wepNames) lbl->installEventFilter(this);

    // 卡片部分
    QVector<QLabel*> cardImgs = {ui->card1, ui->card2, ui->card3, ui->card4, ui->card5};
    QVector<QLabel*> cardNames = {ui->card1_name, ui->card2_name, ui->card3_name, ui->card4_name, ui->card5_name};
    for (auto lbl : cardImgs) lbl->installEventFilter(this);
    for (auto lbl : cardNames) lbl->installEventFilter(this);

    // 捕获初始几何用于全屏比例缩放
    for (QObject* obj : children()) {
        if (QWidget* w = qobject_cast<QWidget*>(obj))
            m_originalGeometries[w] = w->geometry();
    }
}

void shop::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (m_originalGeometries.isEmpty()) return;
    const double sx = double(width())  / double(DESIGN_W);
    const double sy = double(height()) / double(DESIGN_H);
    for (auto it = m_originalGeometries.constBegin(); it != m_originalGeometries.constEnd(); ++it) {
        QWidget* w = it.key();
        if (!w) continue;
        const QRect& orig = it.value();
        w->setGeometry(qRound(orig.x()*sx), qRound(orig.y()*sy),
                       qRound(orig.width()*sx), qRound(orig.height()*sy));
    }
}

shop::~shop()
{
    delete ui;
}

void shop::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap background(":/images/backgrounds/shop_background.png");
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

void shop::updateMoneyUI() {
    if (mainPrepare != nullptr) {
        ui->show_money->setText("Money: " + QString::number(mainPrepare->money));
    }

}

void shop::notEnoughMoney() {

    ui->show_money->setText("Not enough money");

    QTimer::singleShot(1000, this, [this]() {
        this->updateMoneyUI();
    });

    return;

}


void shop::on_exitshop_clicked()
{
    // 重置传说武器模式
    legendaryWeaponMode = false;

    emit backToPrepare();
    this->hide();
    mainPrepare->updateMoneyUI();

    // 刷新prepare界面
    mainPrepare->refreshBenchDisplay();
    mainPrepare->refreshFightersDisplay();
}


//---characters---

//refresh
void shop::refreshCharacters() {
    currentCharacters.clear();
    QStringList charPool = {"罗宾", "索拉", "无名的王", "格罗姆", "塞恩", "亚瑟", "梅林", "维嘉",
        "吉安娜", "温蕾萨", "莱昂", "巴尔", "安娜", "艾莉亚", "影", "光", "混沌", "希尔瓦娜斯", "莱戈拉斯", "莉娜"};

    for(int i = 0; i < 5; ++i) {
        int randIdx = QRandomGenerator::global()->bounded(charPool.size());
        QString name = charPool[randIdx];

        ShopSlot slot;
        slot.name = name;
        slot.price = 5;
        slot.imagePath = QString(":/images/characters/%1.png").arg(name);
        slot.type = "character";

        currentCharacters.append(slot);
    }

    displayCharacters();
}

void shop::displayCharacters() {
    QVector<QLabel*> imgLabels = {ui->character1, ui->character2, ui->character3, ui->character4, ui->character5};
    QVector<QLabel*> txtLabels = {ui->character1_name, ui->character2_name, ui->character3_name, ui->character4_name, ui->character5_name};

    // 获取 QLabel 的设备像素比（Retina 屏幕通常为 2.0）
    qreal dpr = imgLabels[0]->devicePixelRatioF();
    // 目标物理尺寸 = 逻辑尺寸 × 像素比
    QSize targetSize = imgLabels[0]->size() * dpr;

    for(int i = 0; i < currentCharacters.size(); ++i) {
        if(currentCharacters[i].isSoldOut) {
            imgLabels[i]->setText("已售罄");
            txtLabels[i]->setText("");
            continue;
        }

        // 1. 显示名字和价格
        txtLabels[i]->setText(QString("%1\n%2金币").arg(currentCharacters[i].name).arg(currentCharacters[i].price));

        // 2. 显示图片
        QPixmap pix(currentCharacters[i].imagePath);
        if(pix.isNull()) {
            imgLabels[i]->setText("[" + currentCharacters[i].name + "]");
        } else {
            // 高质量缩放原图到目标物理尺寸
            QPixmap scaled = pix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // 设置 Pixmap 的像素比，使其与 QLabel 匹配
            scaled.setDevicePixelRatio(dpr);
            imgLabels[i]->setPixmap(scaled);
        }

        //3.
        m_imageIndex[imgLabels[i]] = i;
        m_imageType[imgLabels[i]] = "character";
        m_nameIndex[txtLabels[i]] = i;
        m_nameType[txtLabels[i]] = "character";
    }
}



void shop::on_new_character_clicked()
{
    if(mainPrepare->money < 2) {
        notEnoughMoney();
    }

    else{
        mainPrepare->money -= 2;
        updateMoneyUI();
        refreshCharacters();
    }
}
//




//---weapon---

//refresh
void shop::refreshWeapons() {
    currentWeapons.clear();

    std::vector<std::string> allWeapons;
    std::vector<int> weaponPrices; // 新增：记录每个武器的价格

    if (legendaryWeaponMode) {
        // 传说武器模式：只从传说武器列表中选取
        allWeapons.insert(allWeapons.end(), legendaryWeaponList.begin(), legendaryWeaponList.end());
        // 传说武器价格：100
        for (size_t i = 0; i < legendaryWeaponList.size(); ++i) {
            weaponPrices.push_back(100);
        }
    } else {
        // 普通模式：从所有武器列表中选取，并设置对应价格

        // 普通武器：价格 10
        for (const auto& weapon : commonWeaponList) {
            allWeapons.push_back(weapon);
            weaponPrices.push_back(10);
        }

        // 稀有武器：价格 25
        for (const auto& weapon : rareWeaponList) {
            allWeapons.push_back(weapon);
            weaponPrices.push_back(25);
        }

        // 史诗武器：价格 60
        for (const auto& weapon : epicWeaponList) {
            allWeapons.push_back(weapon);
            weaponPrices.push_back(60);
        }

    }

    // 如果列表为空，添加默认武器
    if (allWeapons.empty()) {
        allWeapons.push_back("木剑");
        weaponPrices.push_back(5); // 默认普通武器价格
    }

    for(int i = 0; i < 5; ++i) {
        int randIdx = QRandomGenerator::global()->bounded((int)allWeapons.size());
        QString wName = QString::fromStdString(allWeapons[randIdx]);

        ShopSlot slot;
        slot.name = wName;
        slot.price = weaponPrices[randIdx]; // 使用对应的价格

        slot.imagePath = QString(":/images/weapons/%1.png").arg(wName);
        slot.type = "weapon";

        currentWeapons.append(slot);
    }

    displayWeapons();
}

void shop::displayWeapons() {
    QVector<QLabel*> imgLabels = {ui->weapon1, ui->weapon2, ui->weapon3, ui->weapon4, ui->weapon5};
    QVector<QLabel*> txtLabels = {ui->weapon1_name, ui->weapon2_name, ui->weapon3_name, ui->weapon4_name, ui->weapon5_name};

    // 获取 QLabel 的设备像素比（Retina 屏幕通常为 2.0）
    qreal dpr = imgLabels[0]->devicePixelRatioF();
    // 目标物理尺寸 = 逻辑尺寸 × 像素比
    QSize targetSize = imgLabels[0]->size() * dpr;

    for(int i = 0; i < currentWeapons.size(); ++i) {
        if(currentWeapons[i].isSoldOut) {
            imgLabels[i]->setText("已售罄");
            txtLabels[i]->setText("");
            continue;
        }
        txtLabels[i]->setText(QString("%1\n%2金币").arg(currentWeapons[i].name).arg(currentWeapons[i].price));
        QPixmap pix(currentWeapons[i].imagePath);
        if(pix.isNull()) {
            imgLabels[i]->setText("[武器: " + currentWeapons[i].name + "]");
        } else {
            // 高质量缩放原图到目标物理尺寸
            QPixmap scaled = pix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // 设置 Pixmap 的像素比，使其与 QLabel 匹配
            scaled.setDevicePixelRatio(dpr);
            imgLabels[i]->setPixmap(scaled);
        }

        m_imageIndex[imgLabels[i]] = i;
        m_imageType[imgLabels[i]] = "weapon";
        m_nameIndex[txtLabels[i]] = i;
        m_nameType[txtLabels[i]] = "weapon";
    }
}

// 修改刷新武器按钮的点击事件
void shop::on_new_weapons_clicked()
{
    if(mainPrepare->money < 2) {
        notEnoughMoney();
    } else {
        mainPrepare->money -= 2;
        updateMoneyUI();

        // 重置为普通模式（商人卡片效果只持续一次刷新）
        legendaryWeaponMode = false;
        refreshWeapons();
    }
}


//---card---

//refresh
void shop::refreshCards() {
    currentCards.clear();
    QStringList cardPool = {"火球术", "冰锥术", "闪电链", "力量祝福", "守护之光", "迅捷之风",
                            "神秘宝箱", "挑战卡牌", "商人卡牌", "装备补给", "史诗装备", "神器召唤"};

    for(int i = 0; i < 5; ++i) {
        int randIdx = QRandomGenerator::global()->bounded(cardPool.size());
        QString cName = cardPool[randIdx];

        ShopSlot slot;
        slot.name = cName;

        // 根据卡牌名称设置不同的价格
        if (cName == "火球术" || cName == "冰锥术" || cName == "闪电链" ||
            cName == "力量祝福" || cName == "守护之光" || cName == "迅捷之风") {
            slot.price = 10;  // 普通技能卡牌
        } else if (cName == "神秘宝箱" || cName == "挑战卡牌" || cName == "商人卡牌" || cName == "装备补给") {
            slot.price = 20;  // 特殊事件和装备补给卡牌
        } else if (cName == "史诗装备") {
            slot.price = 50;  // 史诗装备卡牌
        } else if (cName == "神器召唤") {
            slot.price = 90;  // 神器召唤卡牌
        } else {
            slot.price = 10;  // 默认价格
        }

        slot.imagePath = QString(":/images/cards/%1.jpg").arg(cName);
        slot.type = "card";

        currentCards.append(slot);
    }
    displayCards();
}

void shop::displayCards() {
    QVector<QLabel*> imgLabels = {ui->card1, ui->card2, ui->card3, ui->card4, ui->card5};
    QVector<QLabel*> txtLabels = {ui->card1_name, ui->card2_name, ui->card3_name, ui->card4_name, ui->card5_name};

    // 获取 QLabel 的设备像素比（Retina 屏幕通常为 2.0）
    qreal dpr = imgLabels[0]->devicePixelRatioF();
    // 目标物理尺寸 = 逻辑尺寸 × 像素比
    QSize targetSize = imgLabels[0]->size() * dpr;

    for(int i = 0; i < currentCards.size(); ++i) {
        if(currentCards[i].isSoldOut) {
            imgLabels[i]->setText("已售罄");
            txtLabels[i]->setText("");
            continue;
        }
        txtLabels[i]->setText(QString("%1\n%2金币").arg(currentCards[i].name).arg(currentCards[i].price));
        QPixmap pix(currentCards[i].imagePath);
        if(pix.isNull()) {
            imgLabels[i]->setText("[" + currentCards[i].name + "]");
        } else {
            // 高质量缩放原图到目标物理尺寸
            QPixmap scaled = pix.scaled(targetSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            // 设置 Pixmap 的像素比，使其与 QLabel 匹配
            scaled.setDevicePixelRatio(dpr);
            imgLabels[i]->setPixmap(scaled);
        }

        m_imageIndex[imgLabels[i]] = i;
        m_imageType[imgLabels[i]] = "card";
        m_nameIndex[txtLabels[i]] = i;
        m_nameType[txtLabels[i]] = "card";
    }
}

void shop::on_new_card_clicked() {
    if(mainPrepare->money < 2) { notEnoughMoney(); }
    else {
        mainPrepare->money -= 2;
        updateMoneyUI();
        refreshCards();
    }
}
//

bool shop::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QLabel *lbl = qobject_cast<QLabel*>(obj);
        if (!lbl) return QWidget::eventFilter(obj, event);

        // 检查是否是图片标签
        if (m_imageIndex.contains(lbl)) {
            int idx = m_imageIndex[lbl];
            QString type = m_imageType[lbl];
            ShopSlot slot;
            if (type == "character") slot = currentCharacters[idx];
            else if (type == "weapon") slot = currentWeapons[idx];
            else slot = currentCards[idx];
            showDetail(slot);   // 点击图片 → 显示详情
            return true;
        }
        // 检查是否是名字标签
        else if (m_nameIndex.contains(lbl)) {
            int idx = m_nameIndex[lbl];
            QString type = m_nameType[lbl];
            ShopSlot slot;
            if (type == "character") slot = currentCharacters[idx];
            else if (type == "weapon") slot = currentWeapons[idx];
            else slot = currentCards[idx];
            buyItem(slot, idx, type);   // 点击名字 → 购买
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}


void shop::showDetail(const ShopSlot &slot) {
    QDialog dlg(this);
    dlg.setWindowTitle(slot.name + "  —  详情");
    dlg.resize(460, 420);
    dlg.setMinimumSize(380, 360);

    QVBoxLayout* root = new QVBoxLayout(&dlg);
    root->setContentsMargins(16, 16, 16, 14);
    root->setSpacing(10);

    // ── 顶部：图片 + 名称/价格 ──
    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->setSpacing(14);

    QLabel* imgLbl = new QLabel(&dlg);
    imgLbl->setFixedSize(128, 128);
    imgLbl->setAlignment(Qt::AlignCenter);
    imgLbl->setStyleSheet(
        "background-color: rgba(0,0,0,160); border: 2px solid #6b4e1e; border-radius: 7px;");

    QPixmap pix(slot.imagePath);
    if (!pix.isNull()) {
        imgLbl->setPixmap(pix.scaled(124, 124, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        imgLbl->setText(slot.name.left(2));
        imgLbl->setStyleSheet(imgLbl->styleSheet() +
            " color: #c9a227; font-size: 24px; font-weight: bold;");
    }

    QVBoxLayout* infoCol = new QVBoxLayout();
    infoCol->setSpacing(6);

    QLabel* nameLbl = new QLabel(slot.name, &dlg);
    nameLbl->setStyleSheet("font-size: 15pt; font-weight: bold; color: #f0d878;");
    nameLbl->setWordWrap(true);

    QString typeStr = (slot.type == "character") ? "角色" :
                      (slot.type == "weapon")    ? "武器" : "卡牌";
    QLabel* priceLbl = new QLabel(
        QString("类型：%1     价格：%2 金币").arg(typeStr).arg(slot.price), &dlg);
    priceLbl->setStyleSheet("color: #c9a227; font-size: 11px;");

    QLabel* hintLbl = new QLabel("点击名称标签可购买", &dlg);
    hintLbl->setStyleSheet("color: #7a6040; font-size: 10px; font-style: italic;");

    infoCol->addWidget(nameLbl);
    infoCol->addWidget(priceLbl);
    infoCol->addWidget(hintLbl);
    infoCol->addStretch();

    topRow->addWidget(imgLbl);
    topRow->addLayout(infoCol, 1);

    // ── 详细信息文本 ──
    QTextBrowser* detail = new QTextBrowser(&dlg);
    detail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString infoText;
    if (slot.type == "character") {
        Character* c = createCharacterByName(slot.name.toStdString());
        if (c) { infoText = QString::fromStdString(c->getInfo()); delete c; }
        else infoText = "无法获取详细属性";
    } else if (slot.type == "weapon") {
        Weapon* w = createWeaponByName(slot.name.toStdString());
        if (w) { infoText = QString::fromStdString(w->getDescription()); delete w; }
        else infoText = "无法获取详细属性";
    } else {
        Card* card = createCardByName(slot.name.toStdString());
        if (card) { infoText = QString::fromStdString(card->getDescription()); delete card; }
        else infoText = "无法获取卡牌信息";
    }
    detail->setPlainText(infoText);

    // ── 关闭按钮 ──
    QPushButton* okBtn = new QPushButton("关闭", &dlg);
    okBtn->setFixedHeight(42);
    connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);

    root->addLayout(topRow);
    root->addWidget(detail, 1);
    root->addWidget(okBtn);

    dlg.exec();
}

//  购买
void shop::buyItem(const ShopSlot &slot, int index, const QString &type) {
    if (slot.isSoldOut) {
        QMessageBox::warning(this, "提示", "该商品已售罄！");
        return;
    }

    if (mainPrepare->isBenchFull()) {
        QMessageBox::warning(this, "备战席已满",
                             QString("备战席最多容纳8个卡牌，请先出售或上阵后再购买！"));
        return;
    }

    if (mainPrepare->money < slot.price) {
        notEnoughMoney();
        return;
    }

    mainPrepare->money -= slot.price;
    updateMoneyUI();

    if (type == "character") {
        Character* c = createCharacterByName(slot.name.toStdString());
        mainPrepare->addItemToBench("character", slot.name, slot.imagePath, c, slot.price);
        currentCharacters[index].isSoldOut = true;
    } else if (type == "weapon") {
        Weapon* w = createWeaponByName(slot.name.toStdString());
        mainPrepare->addItemToBench("weapon", slot.name, slot.imagePath, w, slot.price);
        currentWeapons[index].isSoldOut = true;
    } else if (type == "card") {
        Card* c = createCardByName(slot.name.toStdString());
        mainPrepare->addItemToBench("card", slot.name, slot.imagePath, nullptr, slot.price);
        currentCards[index].isSoldOut = true;
    }

    if (type == "character") displayCharacters();
    else if (type == "weapon") displayWeapons();
    else displayCards();
}

// 新增：设置传说武器模式
void shop::setLegendaryWeaponMode(bool legendaryMode)
{
    legendaryWeaponMode = legendaryMode;
    if (legendaryMode) {
        // 如果切换到传说武器模式，立即刷新武器
        refreshWeapons();
    }
}

void shop::setLineEditsAndTextEditsReadOnly()
{
    // 获取商店窗口中的所有 QLineEdit 控件
    QList<QLineEdit*> lineEdits = this->findChildren<QLineEdit*>();

    // 设置所有 QLineEdit 为只读
    for (QLineEdit* lineEdit : lineEdits) {
        lineEdit->setReadOnly(true);
    }

    // 获取商店窗口中的所有 QTextEdit 控件
    QList<QTextEdit*> textEdits = this->findChildren<QTextEdit*>();

    // 设置所有 QTextEdit 为只读
    for (QTextEdit* textEdit : textEdits) {
        textEdit->setReadOnly(true);
    }
}
