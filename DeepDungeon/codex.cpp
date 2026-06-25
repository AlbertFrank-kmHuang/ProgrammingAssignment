#include "codex.h"
#include "character_config.h"
#include "enemy_config.h"
#include "weapon_config.h"

#include <QTabWidget>
#include <QListWidget>
#include <QLabel>
#include <QTextBrowser>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainter>
#include <QPixmap>

namespace {

QString rarityName(Rarity r) {
    switch (r) {
    case Rarity::Common:    return "普通";
    case Rarity::Rare:      return "罕见";
    case Rarity::Epic:      return "史诗";
    case Rarity::Legendary: return "传奇";
    }
    return "";
}

QString rarityColor(Rarity r) {
    switch (r) {
    case Rarity::Common:    return "#cfcfcf";
    case Rarity::Rare:      return "#4aa3ff";
    case Rarity::Epic:      return "#c061ff";
    case Rarity::Legendary: return "#ffae00";
    }
    return "#ffffff";
}

// 敌人立绘路径：按职业归类到对应文件夹
QString enemyImagePath(Enemy* e) {
    QString job = QString::fromStdString(e->getJobName());
    QString name = QString::fromStdString(e->getName());
    if (job.contains("法师"))   return QString(":/images/enemies/enemy_wizard/%1.jpg").arg(name);
    if (job.contains("弓箭手")) return QString(":/images/enemies/enemy_archer/%1.jpg").arg(name);
    if (job.contains("无名"))   return QString(":/images/enemies/enemy_nameless/%1.jpg").arg(name);
    return QString(":/images/enemies/enemy_swordman/%1.jpg").arg(name);
}

QString statBlock(Character* c) {
    QString s;
    s += QString("生命 %1 &nbsp; 法力 %2<br>")
             .arg(c->getMaxHealth()).arg(c->getmaxmana());
    s += QString("攻击 %1 &nbsp; 防御 %2<br>")
             .arg(c->getAttack()).arg(c->getDefense());
    s += QString("暴击率 %1% &nbsp; 暴击伤害 %2%<br>")
             .arg(c->getCritrate() * 100, 0, 'f', 0)
             .arg(c->getCritdamage() * 100, 0, 'f', 0);
    s += QString("闪避率 %1%<br>")
             .arg(c->getMissrate() * 100, 0, 'f', 0);
    return s;
}

QString skillBlock(Character* c) {
    QString s = "<b style='color:#ffd700'>技能</b><br>";
    const auto& skills = c->getSkills();
    for (size_t i = 0; i < skills.size(); ++i) {
        s += QString("• <b>%1</b>：%2<br>")
                 .arg(QString::fromStdString(skills[i]->getname()))
                 .arg(QString::fromStdString(skills[i]->getdescription()));
    }
    return s;
}

void setImage(QLabel* label, const QString& path) {
    QPixmap pix(path);
    if (!pix.isNull()) {
        label->setPixmap(pix.scaled(label->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        label->setText("立绘缺失");
    }
}

} // namespace

Codex::Codex(QWidget* parent) : QDialog(parent) {
    setWindowTitle("图鉴");
    resize(920, 620);

    QTabWidget* tabs = new QTabWidget(this);
    tabs->addTab(buildCharacterTab(), "英雄");
    tabs->addTab(buildEnemyTab(), "敌人");
    tabs->addTab(buildWeaponTab(), "武器");

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->addWidget(tabs);

    // 默认选中第一项
    if (m_charList->count() > 0) m_charList->setCurrentRow(0);
    if (m_enemyList->count() > 0) m_enemyList->setCurrentRow(0);
    if (m_weaponList->count() > 0) m_weaponList->setCurrentRow(0);
}

Codex::~Codex() {
    for (Character* c : m_characters) delete c;
    for (Enemy* e : m_enemies) delete e;
    for (Weapon* w : m_weapons) delete w;
}

QWidget* Codex::buildCharacterTab() {
    const std::vector<std::string> names = {
        "莱昂", "巴尔", "格罗姆", "塞恩", "亚瑟",
        "安娜", "莉娜", "梅林", "维嘉", "吉安娜",
        "艾莉亚", "罗宾", "温蕾萨", "希尔瓦娜斯", "莱戈拉斯",
        "索拉", "无名的王", "影", "光", "混沌"
    };

    QWidget* page = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(page);

    m_charList = new QListWidget(page);
    m_charList->setFixedWidth(200);

    for (const std::string& name : names) {
        Character* c = createCharacterByName(name);
        m_characters.push_back(c);
        m_charList->addItem(QString("%1  [%2]")
                                .arg(QString::fromStdString(c->getName()))
                                .arg(QString::fromStdString(c->getJobName())));
    }

    QVBoxLayout* right = new QVBoxLayout;
    m_charImg = new QLabel(page);
    m_charImg->setFixedSize(240, 240);
    m_charImg->setAlignment(Qt::AlignCenter);
    m_charImg->setStyleSheet("border: 2px solid gold; background-color: rgba(0,0,0,120);");
    m_charInfo = new QTextBrowser(page);

    right->addWidget(m_charImg, 0, Qt::AlignHCenter);
    right->addWidget(m_charInfo, 1);

    layout->addWidget(m_charList);
    layout->addLayout(right, 1);

    connect(m_charList, &QListWidget::currentRowChanged, this, &Codex::onCharacterRow);
    return page;
}

QWidget* Codex::buildEnemyTab() {
    QWidget* page = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(page);

    m_enemyList = new QListWidget(page);
    m_enemyList->setFixedWidth(200);

    auto add = [&](Enemy* e, const QString& tier) {
        m_enemies.push_back(e);
        m_enemyTiers.push_back(tier);
        m_enemyList->addItem(QString("%1  [%2]")
                                 .arg(QString::fromStdString(e->getName())).arg(tier));
    };

    // 普通
    add(new BerserkerWarrior(), "普通"); add(new IronGuard(), "普通");
    add(new FireMage(), "普通");         add(new FrostMage(), "普通");
    add(new Sharpshooter(), "普通");     add(new Ranger(), "普通");
    add(new ShadowWalker(), "普通");     add(new ChaosApostle(), "普通");
    // 精英
    add(new BloodBlade(), "精英");       add(new Breaker(), "精英");
    add(new ShadowMage(), "精英");       add(new ElementalMage(), "精英");
    add(new Sniper(), "精英");           add(new Hunter(), "精英");
    add(new VoidWalker(), "精英");       add(new Elementalist(), "精英");
    // 首领
    add(new SwordMaster(), "首领");      add(new Archmage(), "首领");
    add(new ArrowGod(), "首领");         add(new VoidKing(), "首领");

    QVBoxLayout* right = new QVBoxLayout;
    m_enemyImg = new QLabel(page);
    m_enemyImg->setFixedSize(240, 240);
    m_enemyImg->setAlignment(Qt::AlignCenter);
    m_enemyImg->setStyleSheet("border: 2px solid #e74c3c; background-color: rgba(0,0,0,120);");
    m_enemyInfo = new QTextBrowser(page);

    right->addWidget(m_enemyImg, 0, Qt::AlignHCenter);
    right->addWidget(m_enemyInfo, 1);

    layout->addWidget(m_enemyList);
    layout->addLayout(right, 1);

    connect(m_enemyList, &QListWidget::currentRowChanged, this, &Codex::onEnemyRow);
    return page;
}

QWidget* Codex::buildWeaponTab() {
    QWidget* page = new QWidget;
    QHBoxLayout* layout = new QHBoxLayout(page);

    m_weaponList = new QListWidget(page);
    m_weaponList->setFixedWidth(200);

    auto addList = [&](const std::vector<std::string>& list) {
        for (const std::string& name : list) {
            Weapon* w = createWeaponByName(name);
            if (!w) continue;
            m_weapons.push_back(w);
            m_weaponList->addItem(QString("%1  [%2]")
                                      .arg(QString::fromStdString(w->getName()))
                                      .arg(rarityName(w->getRarity())));
        }
    };
    addList(commonWeaponList);
    addList(rareWeaponList);
    addList(epicWeaponList);
    addList(legendaryWeaponList);

    QVBoxLayout* right = new QVBoxLayout;
    m_weaponImg = new QLabel(page);
    m_weaponImg->setFixedSize(200, 200);
    m_weaponImg->setAlignment(Qt::AlignCenter);
    m_weaponImg->setStyleSheet("border: 2px solid #8a6d3b; background-color: rgba(0,0,0,120);");
    m_weaponInfo = new QTextBrowser(page);

    right->addWidget(m_weaponImg, 0, Qt::AlignHCenter);
    right->addWidget(m_weaponInfo, 1);

    layout->addWidget(m_weaponList);
    layout->addLayout(right, 1);

    connect(m_weaponList, &QListWidget::currentRowChanged, this, &Codex::onWeaponRow);
    return page;
}

void Codex::onCharacterRow(int row) {
    if (row < 0 || row >= (int)m_characters.size()) return;
    Character* c = m_characters[row];
    setImage(m_charImg, QString(":/images/characters/%1.png")
                            .arg(QString::fromStdString(c->getName())));

    QString html;
    html += QString("<h2 style='color:#ffd700'>%1</h2>")
                .arg(QString::fromStdString(c->getName()));
    html += QString("<p>职业：%1</p>").arg(QString::fromStdString(c->getJobName()));
    html += "<hr>";
    html += statBlock(c);
    html += "<hr>";
    html += skillBlock(c);
    m_charInfo->setHtml(html);
}

void Codex::onEnemyRow(int row) {
    if (row < 0 || row >= (int)m_enemies.size()) return;
    Enemy* e = m_enemies[row];
    setImage(m_enemyImg, enemyImagePath(e));

    QString html;
    html += QString("<h2 style='color:#e74c3c'>%1</h2>")
                .arg(QString::fromStdString(e->getName()));
    html += QString("<p>类型：%1 &nbsp; 职业：%2 &nbsp; 等级：%3</p>")
                .arg(m_enemyTiers[row])
                .arg(QString::fromStdString(e->getJobName()))
                .arg(e->getLevel());
    html += "<hr>";
    html += statBlock(e);
    html += "<hr>";
    html += skillBlock(e);
    m_enemyInfo->setHtml(html);
}

void Codex::onWeaponRow(int row) {
    if (row < 0 || row >= (int)m_weapons.size()) return;
    Weapon* w = m_weapons[row];
    setImage(m_weaponImg, QString(":/images/weapons/%1.png")
                              .arg(QString::fromStdString(w->getName())));

    QString html;
    html += QString("<h2 style='color:%1'>%2</h2>")
                .arg(rarityColor(w->getRarity()))
                .arg(QString::fromStdString(w->getName()));
    html += QString("<p>品质：<span style='color:%1'>%2</span></p>")
                .arg(rarityColor(w->getRarity()))
                .arg(rarityName(w->getRarity()));
    html += "<hr><b style='color:#ffd700'>属性加成</b><br>";

    auto pct = [](float mult) { return QString::number((mult - 1.0f) * 100, 'f', 0); };
    if (w->getAttackBonus()    != 1) html += QString("攻击 +%1%<br>").arg(pct(w->getAttackBonus()));
    if (w->getDefenseBonus()   != 1) html += QString("防御 +%1%<br>").arg(pct(w->getDefenseBonus()));
    if (w->getHealthBonus()    != 1) html += QString("生命 +%1%<br>").arg(pct(w->getHealthBonus()));
    if (w->getManaBonus()      != 1) html += QString("法力 +%1%<br>").arg(pct(w->getManaBonus()));
    if (w->getCritrateBonus()  != 0) html += QString("暴击率 +%1%<br>").arg(w->getCritrateBonus() * 100, 0, 'f', 0);
    if (w->getCritdamageBonus()!= 0) html += QString("暴击伤害 +%1%<br>").arg(w->getCritdamageBonus() * 100, 0, 'f', 0);
    if (w->getMissrateBonus()  != 0) html += QString("闪避率 +%1%<br>").arg(w->getMissrateBonus() * 100, 0, 'f', 0);

    const QString desc = QString::fromStdString(w->getDescription());
    if (!desc.isEmpty()) html += QString("<hr><i>%1</i>").arg(desc);
    m_weaponInfo->setHtml(html);
}

void Codex::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    QPixmap bg(":/images/backgrounds/prepare_background.png");
    if (!bg.isNull()) {
        QPixmap scaled = bg.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        painter.drawPixmap(rect(), scaled);
        painter.fillRect(rect(), QColor(0, 0, 0, 120));  // 压暗，保证文字可读
    } else {
        painter.fillRect(rect(), QColor(26, 20, 16));
    }
}
