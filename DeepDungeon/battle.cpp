#include "battle.h"
#include "ui_battle.h"
#include <QPainter>
#include <QPixmap>
#include <QMessageBox>
#include "character_config.h"
#include "effects.h"
#include "soundmanager.h"
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <algorithm>
#include <QInputDialog>
#include <QListWidget>

// 血条/法力条样式：chunk 始终着色，槽位用深色（避免出现灰色空槽）
static QString barStyle(const QString& chunkColor) {
    return QString(
        "QProgressBar{border:1px solid #5a4a30;border-radius:4px;"
        "background-color:#241a12;text-align:center;color:#ffffff;font-weight:bold;}"
        "QProgressBar::chunk{border-radius:3px;background-color:%1;}").arg(chunkColor);
}
// 生命值按比例变色：绿→黄→红
static QString hpColor(double ratio) {
    if (ratio > 0.5) return "#2ecc71";
    if (ratio > 0.25) return "#f1c40f";
    return "#e74c3c";
}

battle::battle(BattleEngine* engine, QWidget *parent)
    : QWidget(parent), ui(new Ui::battle), m_engine(engine)
{

        ui->setupUi(this);
    // 设置状态标签为可点击
        ui->label_playerStatus->setCursor(Qt::PointingHandCursor);
        ui->label_enemyStatus->setCursor(Qt::PointingHandCursor);
        ui->label_playerStatus->setToolTip("点击查看详细状态");
        ui->label_enemyStatus->setToolTip("点击查看详细状态");

    // 连接点击信号
        connect(ui->label_playerStatus, &ClickableLabel::clicked, this, &battle::onPlayerStatusClicked);
        connect(ui->label_enemyStatus, &ClickableLabel::clicked, this, &battle::onEnemyStatusClicked);

    // 绑定切人按钮的点击事件
        connect(ui->change, &QPushButton::clicked, this, &battle::change_clicked);

    // 设置响应式布局
        setupResponsiveLayout();

        if (m_engine) {
            // 连接状态变化信号
            connect(m_engine, &BattleEngine::stateChanged, this, &battle::onStateChanged);

            // 连接日志添加信号
            connect(m_engine, &BattleEngine::logAdded, this, &battle::onLogAdded);

            // 连接战斗结束信号
            connect(m_engine, &BattleEngine::battleEnded, this, &battle::onBattleEnded);

            // 连接UI更新信号
            connect(m_engine, &BattleEngine::uiNeedsUpdate, this, &battle::onUiNeedsUpdate);
        }

    // 绑定按钮点击事件
        connect(ui->btn_attack, &QPushButton::clicked, this, &battle::attack_clicked);
        connect(ui->btn_skill1, &QPushButton::clicked, this, &battle::skill1_clicked);
        connect(ui->btn_skill2, &QPushButton::clicked, this, &battle::skill2_clicked);

    // 初始刷新一次UI
        refreshUI();
}

void battle::onStateChanged(BattleState newState) {
        // 根据状态变化执行特定操作
        switch (newState) {
        case BattleState::DECISION_PHASE:
            break;
        case BattleState::BATTLE_OVER:
            break;
        default:
            break;
        }
}

void battle::setupResponsiveLayout() {
    // ── 立绘：随窗口等比放大 ──
    ui->img_playerFront->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->img_enemyFront->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->img_playerFront->setMinimumSize(90, 90);
    ui->img_playerFront->setMaximumSize(160, 160);
    ui->img_enemyFront->setMinimumSize(90, 90);
    ui->img_enemyFront->setMaximumSize(160, 160);
    ui->img_playerFront->setAlignment(Qt::AlignCenter);
    ui->img_enemyFront->setAlignment(Qt::AlignCenter);

    // ── 立绘框：金边（我方） / 血红边（敌方）──
    ui->img_playerFront->setStyleSheet(
        "QLabel { background-color: rgba(6,4,2,190);"
        " border: 3px solid #c9a227; border-radius: 8px; }");
    ui->img_enemyFront->setStyleSheet(
        "QLabel { background-color: rgba(6,4,2,190);"
        " border: 3px solid #8b1a1a; border-radius: 8px; }");

    // ── 名称标签 ──
    ui->label_playerName->setWordWrap(true);
    ui->label_enemyName->setWordWrap(true);
    ui->label_playerName->setAlignment(Qt::AlignCenter);
    ui->label_enemyName->setAlignment(Qt::AlignCenter);
    ui->label_playerName->setStyleSheet(
        "QLabel { color: #f0d878; font-size: 13pt; font-weight: bold; background: transparent; }");
    ui->label_enemyName->setStyleSheet(
        "QLabel { color: #ff7070; font-size: 13pt; font-weight: bold; background: transparent; }");

    // ── 血量 / 法力条 ──
    ui->bar_playerHP->setMinimumHeight(22);
    ui->bar_playerMP->setMinimumHeight(14);
    ui->bar_enemyHP->setMinimumHeight(22);
    ui->bar_enemyMP->setMinimumHeight(14);
    ui->bar_playerMP->setStyleSheet(barStyle("#3a7abe"));
    ui->bar_enemyMP->setStyleSheet(barStyle("#3a7abe"));

    // ── 状态标签（可点击）──
    ui->label_playerStatus->setWordWrap(true);
    ui->label_enemyStatus->setWordWrap(true);
    ui->label_playerStatus->setStyleSheet(
        "QLabel { color: #c8bb9a; background-color: rgba(0,0,0,165);"
        " border: 1px solid #4a3820; border-radius: 4px; font-size: 11px; padding: 3px 6px; }");
    ui->label_enemyStatus->setStyleSheet(
        "QLabel { color: #c8bb9a; background-color: rgba(0,0,0,165);"
        " border: 1px solid #4a3820; border-radius: 4px; font-size: 11px; padding: 3px 6px; }");

    // ── 技能按钮：地牢石刻风格 ──
    const QString battleBtnStyle =
        "QPushButton {"
        "  background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "    stop:0 #2d2418, stop:1 #1a120a);"
        "  color: #d4aa5a; border: 2px solid #6b4e1e;"
        "  border-radius: 7px; font-size: 11pt; font-weight: bold; }"
        "QPushButton:hover {"
        "  background-color: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "    stop:0 #3d3020, stop:1 #271d10);"
        "  border-color: #c9a227; color: #f5d87a; }"
        "QPushButton:pressed { background-color: #0a0705; border-color: #7a5a20; }"
        "QPushButton:disabled {"
        "  background-color: #141008; color: #4a3f2a; border-color: #2a2018; }";
    for (QPushButton* b : {ui->btn_attack, ui->btn_skill1, ui->btn_skill2, ui->change}) {
        b->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        b->setMinimumHeight(54);
        b->setStyleSheet(battleBtnStyle);
    }

    // ── 战斗日志：暗室卷轴 ──
    ui->textBrowser_log->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    ui->textBrowser_log->setStyleSheet(
        "QTextBrowser { background-color: rgba(6,4,2,215); color: #c0b090;"
        " border: 2px solid #4a3820; border-radius: 7px; padding: 9px; font-size: 11px; }");

    // ── 顶部标题栏 ──
    QLabel* header = new QLabel(this);
    header->setText("⚔  深渊对决  ⚔");
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet(
        "QLabel { color: #c9a227; font-size: 14pt; font-weight: bold;"
        " background-color: rgba(0,0,0,120);"
        " border-bottom: 1px solid #4a3820; padding: 5px; letter-spacing: 3px; }");
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    // ── VS 分隔标签 ──
    QLabel* vsLabel = new QLabel("VS", this);
    vsLabel->setAlignment(Qt::AlignCenter);
    vsLabel->setStyleSheet(
        "QLabel { color: #6a4a20; font-size: 24pt; font-weight: bold; background: transparent; }");
    vsLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    vsLabel->setMinimumWidth(50);

    // ── 玩家列 ──
    QVBoxLayout* playerCol = new QVBoxLayout();
    playerCol->setSpacing(6);
    playerCol->addWidget(ui->label_playerName);
    playerCol->addWidget(ui->img_playerFront, 1);
    playerCol->addWidget(ui->bar_playerHP);
    playerCol->addWidget(ui->bar_playerMP);
    playerCol->addWidget(ui->label_playerStatus);

    // ── 敌人列 ──
    QVBoxLayout* enemyCol = new QVBoxLayout();
    enemyCol->setSpacing(6);
    enemyCol->addWidget(ui->label_enemyName);
    enemyCol->addWidget(ui->img_enemyFront, 1);
    enemyCol->addWidget(ui->bar_enemyHP);
    enemyCol->addWidget(ui->bar_enemyMP);
    enemyCol->addWidget(ui->label_enemyStatus);

    // ── 对战行（玩家 | VS | 敌人）──
    QHBoxLayout* combatRow = new QHBoxLayout();
    combatRow->setSpacing(12);
    combatRow->addLayout(playerCol, 2);
    combatRow->addWidget(vsLabel, 0, Qt::AlignVCenter);
    combatRow->addLayout(enemyCol, 2);

    // ── 技能按钮 2×2 格 ──
    QGridLayout* btnGrid = new QGridLayout();
    btnGrid->setSpacing(8);
    btnGrid->addWidget(ui->btn_attack, 0, 0);
    btnGrid->addWidget(ui->btn_skill1, 0, 1);
    btnGrid->addWidget(ui->btn_skill2, 1, 0);
    btnGrid->addWidget(ui->change,     1, 1);

    // ── 底部行：按钮 + 战斗日志 ──
    QHBoxLayout* bottomRow = new QHBoxLayout();
    bottomRow->setSpacing(14);
    bottomRow->addLayout(btnGrid, 1);
    bottomRow->addWidget(ui->textBrowser_log, 2);

    // ── 根布局 ──
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(16, 10, 16, 16);
    root->setSpacing(8);
    root->addWidget(header);
    root->addLayout(combatRow, 3);
    root->addLayout(bottomRow, 2);

    setMinimumSize(660, 480);
}

battle::~battle() { delete ui; }

void battle::setEngine(BattleEngine* newEngine) {
    m_engine = newEngine;
    if (m_engine) {
        ui->textBrowser_log->clear();
        refreshUI();
    }
}

// 按钮向引擎传达明确的意图：0=普攻，1=特技1，2=特技2
void battle::attack_clicked() {
    if (m_engine && m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
        SoundManager::instance().play("click");
        m_engine->submitPlayerAction(0);
    }
}
void battle::skill1_clicked() {
    if (m_engine && m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
        Character* player = m_engine->getPlayerFront();
        if (!player) { return; }
        auto skills = player->getSkills();
        if (skills.size() <= 1) { return; }
        QString skillName = QString::fromStdString(skills[1]->getname());

        if (m_engine->getSkillTargetType(skillName) == SkillTargetType::ALLY_SINGLE_SELECT) {
            int targetIdx = showTargetSelectionDialog(true);
            if (targetIdx < 0) {
                return;
            }
            int skillIndex = 1;
            if (m_engine && m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
                SoundManager::instance().play("skill");
                m_engine->submitPlayerActionWithTarget(skillIndex, targetIdx);
            }
        } else {
            SoundManager::instance().play("skill");
            m_engine->submitPlayerAction(1);
        }
    }
}
void battle::skill2_clicked() {
    if (m_engine && m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
        Character* player = m_engine->getPlayerFront();
        if (!player) {  return; }
        auto skills = player->getSkills();
        if (skills.size() <= 2) {return; }
        QString skillName = QString::fromStdString(skills[2]->getname());

        if (m_engine->getSkillTargetType(skillName) == SkillTargetType::ALLY_SINGLE_SELECT) {
            int targetIdx = showTargetSelectionDialog(true);
            if (targetIdx < 0) {
                return;
            }
            int skillIndex = 2;

            if (m_engine && m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
                SoundManager::instance().play("skill");
                m_engine->submitPlayerActionWithTarget(skillIndex, targetIdx);
            }
        } else {
            SoundManager::instance().play("skill");
            m_engine->submitPlayerAction(2);
        }
    }
}

// 更新玩家UI
void battle::updatePlayerUI(Character* player) {
    if (!player) return;

    int pMaxHP = std::max(1, player->getMaxHealth() + player->gethealthadd());
    int pHP_with_add = player->getHealth() + player->gethealthadd();
    int pHP = std::clamp(pHP_with_add, 0, pMaxHP);

    ui->bar_playerHP->setMaximum(pMaxHP);
    ui->bar_playerHP->setValue(pHP);
    ui->bar_playerHP->setFormat(QString("%1/%2").arg(pHP).arg(pMaxHP));
    ui->bar_playerHP->setStyleSheet(barStyle(hpColor(double(pHP) / pMaxHP)));

    int pMaxMP = std::max(1, player->getmaxmana() + player->getmanaadd());
    int pMP_with_add = player->getmana() + player->getmanaadd();
    int pMP = std::clamp(pMP_with_add, 0, pMaxMP);

    ui->bar_playerMP->setMaximum(pMaxMP);
    ui->bar_playerMP->setValue(pMP);
    ui->bar_playerMP->setFormat(QString("%1/%2").arg(pMP).arg(pMaxMP));
    ui->bar_playerMP->setStyleSheet(barStyle("#3498db"));

    ui->label_playerName->setText(QString::fromStdString(player->getJobName() + " " + player->getName()));

    // 更新玩家立绘
    QString playerImgPath = QString(":/images/characters/%1.png").arg(QString::fromStdString(player->getName()));
    QPixmap pPix(playerImgPath);
    if (!pPix.isNull()) {
        ui->img_playerFront->setPixmap(pPix);
    } else {
        ui->img_playerFront->setText("立绘缺失");
    }

    // 更新玩家状态
    QString pStatus = "护盾: " + QString::number(player->getShield()) + " | 状态: ";
    if (player->getbuffs().empty()) pStatus += "正常";
    else pStatus += QString::number(player->getbuffs().size()) + "个";

    // 添加羁绊信息
    QString bondInfo = "羁绊加成: ";
    if (player->gethealthadd() > 0) bondInfo += QString("生命+%1 ").arg(player->gethealthadd());
    if (player->getmanaadd() > 0) bondInfo += QString("法力+%1 ").arg(player->getmanaadd());
    if (player->getattackadd() > 0) bondInfo += QString("攻击+%1 ").arg(player->getattackadd());
    if (player->getdefenseadd() > 0) bondInfo += QString("防御+%1 ").arg(player->getdefenseadd());
    if (player->getcritrateadd() > 0) bondInfo += QString("暴率+%1% ").arg(player->getcritrateadd() * 100);
    if (player->getcritdamageadd() > 0) bondInfo += QString("暴伤+%1% ").arg(player->getcritdamageadd() * 100);
    if (player->getmissrateadd() > 0) bondInfo += QString("闪避+%1% ").arg(player->getmissrateadd() * 100);

    ui->label_playerStatus->setText(pStatus + " | " + bondInfo);

    // 更新技能按钮
    updateSkillButtons(player);
}

// 更新敌人UI
void battle::updateEnemyUI(Character* enemy) {
    if (!enemy) return;

    int eMaxHP = std::max(1, enemy->getMaxHealth() + enemy->gethealthadd());
    int eHP = std::clamp(enemy->getHealth(), 0, eMaxHP);

    ui->bar_enemyHP->setMaximum(eMaxHP);
    ui->bar_enemyHP->setValue(eHP);
    ui->bar_enemyHP->setFormat(QString("%1/%2").arg(eHP).arg(eMaxHP));
    ui->bar_enemyHP->setStyleSheet(barStyle(hpColor(double(eHP) / eMaxHP)));

    int eMaxMP = std::max(1, enemy->getmaxmana() + enemy->getmanaadd());
    int eMP = std::clamp(enemy->getmana(), 0, eMaxMP);

    ui->bar_enemyMP->setMaximum(eMaxMP);
    ui->bar_enemyMP->setValue(eMP);
    ui->bar_enemyMP->setFormat(QString("%1/%2").arg(eMP).arg(eMaxMP));
    ui->bar_enemyMP->setStyleSheet(barStyle("#3498db"));

    ui->label_enemyName->setText(QString::fromStdString(enemy->getName()));

    // 更新敌人立绘
    QString eJob = QString::fromStdString(enemy->getJobName());
    QString eName = QString::fromStdString(enemy->getName());
    QString enemyImgPath;

    if (eJob.contains("法师")) {
        enemyImgPath = QString(":/images/enemies/enemy_wizard/%1.jpg").arg(eName);
    } else if (eJob.contains("弓箭手")) {
        enemyImgPath = QString(":/images/enemies/enemy_archer/%1.jpg").arg(eName);
    } else if (eJob.contains("无名")) {
        enemyImgPath = QString(":/images/enemies/enemy_nameless/%1.jpg").arg(eName);
    } else {
        enemyImgPath = QString(":/images/enemies/enemy_swordman/%1.jpg").arg(eName);
    }

    QPixmap ePix(enemyImgPath);
    if (!ePix.isNull()) {
        ui->img_enemyFront->setPixmap(ePix);
    } else {
        ui->img_enemyFront->setText("立绘缺失");
    }

    QString eStatus = "护盾: " + QString::number(enemy->getShield()) + " | 状态: ";
    if (enemy->getbuffs().empty()) eStatus += "正常";
    else eStatus += QString::number(enemy->getbuffs().size()) + "个";
    ui->label_enemyStatus->setText(eStatus);
}

// 检查血量变化并触发效果
void battle::checkHpChanges(Character* player, Character* enemy) {
    if (!player || !enemy) return;

    int curPlayerHP = player->getHealth();
    int curEnemyHP = enemy->getHealth();

    if (m_prevPlayerPtr != player || m_prevEnemyPtr != enemy) {
        m_prevPlayerPtr = player;
        m_prevEnemyPtr = enemy;
        m_prevPlayerHP = curPlayerHP;
        m_prevEnemyHP = curEnemyHP;
        return;
    }

    int playerDelta = m_prevPlayerHP - curPlayerHP;
    int enemyDelta = m_prevEnemyHP - curEnemyHP;

    if (playerDelta != 0 || enemyDelta != 0) {
        // 获取最新的日志来检测暴击/闪避
        std::vector<QString> logs; // 这里需要从BattleEngine获取最新日志
        // 由于日志已经通过信号添加，我们可以记录最近一次的伤害日志
        // 简化实现：这里只是记录血量变化，不触发复杂效果
        m_prevPlayerHP = curPlayerHP;
        m_prevEnemyHP = curEnemyHP;
    }
}

void battle::refreshUI() {
    // 获取前后排角色
    Character* pFront = nullptr;
    Character* eFront = nullptr;

        pFront = m_engine->getPlayerFront();
        eFront = m_engine->getEnemyFront();
        qDebug() << "    玩家前排：" << (pFront ? QString::fromStdString(pFront->getName()) : "空指针");
        qDebug() << "    敌人前排：" << (eFront ? QString::fromStdString(eFront->getName()) : "空指针");

        updatePlayerUI(pFront);
        updateEnemyUI(eFront);
        checkHpChanges(pFront, eFront);
}



// 更新技能按钮状态
void battle::updateSkillButtons(Character* player) {
    auto skills = player->getSkills();
    ui->btn_attack->setText(skills.size() > 0 ? QString::fromStdString(skills[0]->getname()) : "普攻");

    if (skills.size() > 1) {
        ui->btn_skill1->setText(QString::fromStdString(skills[1]->getname()) + "\n(MP:" + QString::number(skills[1]->getmanacost()) + ")");
        int totalMana = player->getmana() + player->getmanaadd();

        // 检查是否有fixed状态
        bool hasFixedBuff = false;
        auto buffs = player->getbuffs();
        for (const auto& buff : buffs) {
            if (buff.buff_has == Status::fixed) {
                hasFixedBuff = true;
                break;
            }
        }

        if (skills[1]->getmanacost() == 0 || hasFixedBuff) {
            ui->btn_skill1->setEnabled(true);
        } else {
            ui->btn_skill1->setEnabled(totalMana >= skills[1]->getmanacost());
        }
    } else {
        ui->btn_skill1->setEnabled(false);
    }

    if (skills.size() > 2) {
        ui->btn_skill2->setText(QString::fromStdString(skills[2]->getname()) + "\n(MP:" + QString::number(skills[2]->getmanacost()) + ")");
        int totalMana = player->getmana() + player->getmanaadd();

        bool hasFixedBuff = false;
        auto buffs = player->getbuffs();
        for (const auto& buff : buffs) {
            if (buff.buff_has == Status::fixed) {
                hasFixedBuff = true;
                break;
            }
        }

        if (skills[2]->getmanacost() == 0 || hasFixedBuff) {
            ui->btn_skill2->setEnabled(true);
        } else {
            ui->btn_skill2->setEnabled(totalMana >= skills[2]->getmanacost());
        }
    } else {
        ui->btn_skill2->setEnabled(false);
    }
}
// 根据技能名推断元素颜色
static QColor elementColorForSkill(const QString& name) {
    if (name.contains("火") || name.contains("焰")) return QColor("#ff5a3c"); // 火
    if (name.contains("冰") || name.contains("霜") || name.contains("寒")) return QColor("#5ad1ff"); // 冰
    if (name.contains("雷") || name.contains("电")) return QColor("#ffe14d"); // 雷
    if (name.contains("毒")) return QColor("#7bd84a"); // 毒
    if (name.contains("治") || name.contains("愈") || name.contains("光") ||
        name.contains("圣") || name.contains("祝福") || name.contains("守护"))
        return QColor("#ffd700"); // 治疗/神圣
    if (name.contains("暗") || name.contains("影") || name.contains("混沌") ||
        name.contains("诅咒") || name.contains("黑暗") || name.contains("背刺"))
        return QColor("#a85cff"); // 暗影
    return QColor("#ff9a3c"); // 默认：物理/增益
}

// 扫描日志识别技能释放（日志形如：……释放了 [技能名]），按元素染色闪屏
void battle::triggerSkillEffects(const std::vector<QString>& newLogs) {
    static const QRegularExpression re(QStringLiteral("释放了 \\[(.+?)\\]"));
    for (const QString& log : newLogs) {
        QRegularExpressionMatch m = re.match(log);
        if (m.hasMatch()) {
            const QString skillName = m.captured(1);
            fx::screenFlash(this, elementColorForSkill(skillName));
            break;  // 一帧只触发一次，避免叠加过亮
        }
    }
}

// 根据本帧的血量变化与新日志，播放浮动数字、抖动、闪光与音效
void battle::playCombatFeedback(const std::vector<QString>& newLogs,
                                int playerDelta, int enemyDelta) {
    // 汇总本帧日志，判断是否暴击/闪避
    bool hasCrit = false, hasDodge = false;
    for (const QString& log : newLogs) {
        if (log.contains("暴击")) hasCrit = true;
        if (log.contains("闪避")) hasDodge = true;
    }

    const QPoint enemyCenter = ui->img_enemyFront->geometry().center();
    const QPoint playerCenter = ui->img_playerFront->geometry().center();

    // 敌人受到伤害
    if (enemyDelta > 0) {
        QColor col = hasCrit ? QColor("#ff8c1a") : QColor("#ff4d4d");
        int fontSize = hasCrit ? 26 : 20;
        QString text = hasCrit ? QString("暴击 %1").arg(enemyDelta) : QString("-%1").arg(enemyDelta);
        fx::floatingText(this, enemyCenter, text, col, fontSize);
        fx::shake(ui->img_enemyFront, hasCrit ? 12 : 8);
        fx::flash(ui->img_enemyFront, hasCrit ? QColor(255, 140, 0) : QColor(255, 0, 0));
        SoundManager::instance().play(hasCrit ? "crit" : "hit");
    } else if (enemyDelta < 0) {
        fx::floatingText(this, enemyCenter, QString("+%1").arg(-enemyDelta), QColor("#7bff7b"), 18);
    }

    // 玩家受到伤害
    if (playerDelta > 0) {
        QColor col = hasCrit ? QColor("#ff8c1a") : QColor("#ff4d4d");
        int fontSize = hasCrit ? 26 : 20;
        QString text = hasCrit ? QString("暴击 %1").arg(playerDelta) : QString("-%1").arg(playerDelta);
        fx::floatingText(this, playerCenter, text, col, fontSize);
        fx::shake(ui->img_playerFront, hasCrit ? 12 : 8);
        fx::flash(ui->img_playerFront, QColor(255, 0, 0));
        SoundManager::instance().play("hurt");
    } else if (playerDelta < 0) {
        fx::floatingText(this, playerCenter, QString("+%1").arg(-playerDelta), QColor("#7bff7b"), 18);
        fx::flash(ui->img_playerFront, QColor(80, 220, 80));
        SoundManager::instance().play("heal");
    }

    // 仅有闪避、无血量变化时给出提示
    if (hasDodge && enemyDelta == 0 && playerDelta == 0) {
        fx::floatingText(this, enemyCenter, "闪避!", QColor("#cfcfcf"), 18);
    }
}

void battle::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    QPixmap background(":/images/backgrounds/battle_background.png");
    if (!background.isNull()) {
        qreal dpr = this->devicePixelRatioF();
        QSize targetSize = size() * dpr;
        QPixmap scaled = background.scaled(targetSize, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        scaled.setDevicePixelRatio(dpr);
        painter.drawPixmap(rect(), scaled);
    }
}

// 实现槽函数
void battle::onPlayerStatusClicked() {
    Character* pFront = m_engine->getPlayerFront();
    if (!pFront) {
        QMessageBox::information(this, "状态详情", "角色不存在");
        return;
    }

    showBuffDetails(pFront, "玩家状态详情");
}

void battle::onEnemyStatusClicked() {
    Enemy* eFront = m_engine->getEnemyFront();
    if (!eFront) {
        QMessageBox::information(this, "状态详情", "敌人不存在");
        return;
    }

    showBuffDetails(eFront, "敌人状态详情");
}


// 辅助函数：将buff类型转换为描述
QString battle::getBuffDescription(const buff& b) {
    switch (b.buff_has) {
    case Status::attackAdd:
        return "攻击加成 - 增加攻击力" + QString::number(b.buffvalue);
    case Status::attackDec:
        return "攻击减少 - 减少攻击力" + QString::number(b.buffvalue);
    case Status::defenseAdd:
        return "防御加成 - 增加防御力" + QString::number(b.buffvalue);
    case Status::defenseDec:
        return "防御减少 - 减少防御力" + QString::number(b.buffvalue);
    case Status::healthAdd:
        return "生命增加 - 增加最大生命" + QString::number(b.buffvalue);
    case Status::critrateAdd:
        return "暴击率加成 - 提高暴击率" + QString::number(b.buffvalue) + "%";
    case Status::critdamageAdd:
        return "暴击伤害加成 - 提高暴击伤害" + QString::number(b.buffvalue) + "%";
    case Status::manaAdd:
        return "法力增强 - 增加最大法力" + QString::number(b.buffvalue);
    case Status::manaDec:
        return "法力减少 - 减少最大法力" + QString::number(b.buffvalue);
    case Status::missrateAdd:
        return "闪避率增强 - 提高闪避率" + QString::number(b.buffvalue) + "%";
    case Status::shieldAdd:
        return "附加护盾 - 获得护盾值" + QString::number(b.buffvalue);
    case Status::Penetration:
        return "穿透 - 无视部分防御" + QString::number(b.buffvalue) + "%";
    case Status::fastchange:
        return "速切 - 可以快速切换技能";
    case Status::fixed:
        return "不消耗魔法 - 技能不消耗魔法值";
    case Status::Normal:
        return "正常 - 无效果";
    case Status::Defend:
        return "防御 - 减伤50%";
    case Status::Poisoned:
        return "中毒 - 每回合受到持续伤害" + QString::number(b.buffvalue);
    case Status::Paralyzed:
        return "麻痹 - 有概率无法行动";
    case Status::Asleep:
        return "睡眠 - 无法行动";
    case Status::Invincible:
        return "无敌 - 免疫所有伤害";
    case Status::LifeSteal:
        return "吸血 - 攻击时回复生命" + QString::number(b.buffvalue) + "%";
    default:
        return QString("未知状态 (%1)").arg(static_cast<int>(b.buff_has));
    }
}
void battle::change_clicked() {
    if (!m_engine) return;
    if (m_engine->getCurrentState() != BattleState::DECISION_PHASE) return;

    // 弹出选择对话框，不允许选择自己（index=0）
    int targetIdx = showTargetSelectionDialog(false); // false=不允许选自己
    if (targetIdx < 0) return;

    bool success = m_engine->switchCharacterTo(targetIdx);
    if (success) {
        SoundManager::instance().play("switch");
        refreshUI();

        Character* newFront = m_engine->getPlayerFront();
        bool hasFastChange = false;
        if (newFront) {
            auto buffs = newFront->getbuffs();
            for (const auto& buff : buffs) {
                if (buff.buff_has == Status::fastchange) {
                    hasFastChange = true;
                    break;
                }
            }
        }
        if (!hasFastChange) {
            m_engine->endPlayerTurnAfterSwitch();
        }
    }
}



// 日志添加时的处理
void battle::onLogAdded(const QString& log) {

        ui->textBrowser_log->append(log);

    // 如果日志包含技能释放，触发技能特效
    static const QRegularExpression re(QStringLiteral("释放了 \\[(.+?)\\]"));
    QRegularExpressionMatch m = re.match(log);
    if (m.hasMatch()) {
        const QString skillName = m.captured(1);
        fx::screenFlash(this, elementColorForSkill(skillName));
    }
}

// 战斗结束时的处理
void battle::onBattleEnded(bool playerWon) {

    if (!m_resultPlayed) {
        m_resultPlayed = true;
        SoundManager::instance().play(playerWon ? "victory" : "defeat");
    }

    // 发射战斗结束信号
    emit battleFinished(playerWon);

    // 延迟关闭窗口，但先检查窗口是否仍然可见
    if (this->isVisible()) {
        QTimer::singleShot(1000, this, [this]() {
            this->close();
        });
    }
}

// UI需要更新时的处理
void battle::onUiNeedsUpdate() {
    refreshUI();
}

int battle::showTargetSelectionDialog(bool allowSelf) {
    if (!m_engine) return -1;
    const auto& team = m_engine->getPlayerTeam();

    // 准备数据
    QStringList items;
    QVector<int> indices;
    for (int i = 0; i < team.size(); ++i) {
        Character* c = team[i];
        if (c && c->alive()) {
            if (!allowSelf && i == 0) continue;
            QString label = QString("%1. %2").arg(i+1).arg(QString::fromStdString(c->getName()));
            items << label;
            indices << i;
        }
    }
    if (items.isEmpty()) return -1;

    // 创建自定义对话框
    QDialog dialog(this);
    dialog.setWindowTitle("选择目标");
    dialog.setFixedSize(320, 260);
    dialog.setStyleSheet(
        "QDialog {"
        "  background-color: #1a1a2e;"       // 深色背景
        "  border: 2px solid #c9a227;"       // 金色边框
        "  border-radius: 8px;"
        "}"
        "QLabel {"
        "  color: #f0d878;"                  // 浅金色文字
        "  font-size: 15px;"
        "  font-weight: bold;"
        "  padding: 6px;"
        "}"
        "QListWidget {"
        "  background-color: #16213e;"       // 稍亮深蓝
        "  color: #e0e0e0;"                  // 浅灰文字
        "  font-size: 14px;"
        "  border: 1px solid #4a3820;"
        "  border-radius: 5px;"
        "  padding: 4px;"
        "}"
        "QListWidget::item:selected {"
        "  background-color: #c9a227;"
        "  color: #1a1a2e;"
        "}"
        "QListWidget::item:hover {"
        "  background-color: #2a2a4e;"
        "}"
        "QPushButton {"
        "  background-color: #2d2418;"
        "  color: #d4aa5a;"
        "  border: 1px solid #6b4e1e;"
        "  border-radius: 5px;"
        "  padding: 8px 28px;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "  background-color: #3d3020;"
        "  border-color: #c9a227;"
        "  color: #f5d87a;"
        "}"
        "QPushButton:pressed {"
        "  background-color: #0a0705;"
        "}"
        );

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(16, 12, 16, 12);
    layout->setSpacing(10);

    QLabel* prompt = new QLabel("请选择目标角色：");
    layout->addWidget(prompt);

    QListWidget* listWidget = new QListWidget();
    listWidget->addItems(items);
    listWidget->setCurrentRow(0);
    layout->addWidget(listWidget);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(12);
    QPushButton* okBtn = new QPushButton("确定");
    QPushButton* cancelBtn = new QPushButton("取消");
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);

    // 连接按钮
    QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    // 双击列表项也视为确认
    QObject::connect(listWidget, &QListWidget::itemDoubleClicked, &dialog, &QDialog::accept);

    // 显示对话框（模态）
    if (dialog.exec() == QDialog::Accepted) {
        int row = listWidget->currentRow();
        if (row >= 0 && row < indices.size()) {
            return indices[row];
        }
    }

    // 用户取消，恢复按钮状态
    if (m_engine->getCurrentState() == BattleState::DECISION_PHASE) {
        Character* player = m_engine->getPlayerFront();
        if (player) updateSkillButtons(player);
    }
    return -1;
}
