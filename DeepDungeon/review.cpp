#include "review.h"
#include "ui_review.h"
#include "prepare.h"
#include "enemy_config.h"
#include <QPainter>
#include <QLabel>

review::review(prepare *parentPrepare, QWidget *parent)
    : QWidget(parent), ui(new Ui::review), mainPrepare(parentPrepare)
{
    ui->setupUi(this);
    connect(ui->btn_back, &QPushButton::clicked, this, &review::on_btn_back_clicked);

    // 捕获初始几何用于全屏比例缩放
    for (QObject* obj : children()) {
        if (QWidget* w = qobject_cast<QWidget*>(obj))
            m_originalGeometries[w] = w->geometry();
    }
}

void review::resizeEvent(QResizeEvent* event)
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

review::~review() {
    delete ui;
}

void review::on_btn_back_clicked() {
    emit backToPrepare();
    this->hide();
}

// ========= 核心情报刷新逻辑 =========
void review::updateIntel(int currentFloor) {
    QVector<QLabel*> imgLabels = {ui->img_enemy1, ui->img_enemy2, ui->img_enemy3, ui->img_enemy4};
    QVector<QLabel*> infoLabels = {ui->info_enemy1, ui->info_enemy2, ui->info_enemy3, ui->info_enemy4};

    // 先清空所有槽位
    for (int i = 0; i < 4; ++i) {
        imgLabels[i]->clear(); infoLabels[i]->clear();
        imgLabels[i]->hide(); infoLabels[i]->hide();
    }

    // =====================================
    // 核心新增：如果下一层是事件层，显示神秘问号！
    // =====================================
    if (mainPrepare->isEventFloor(currentFloor)) {
        ui->label_title->setText("--- 下一层：未知奇遇 ---");

        imgLabels[1]->show();  // 借用中间的怪物槽位
        infoLabels[1]->show();

        imgLabels[1]->setStyleSheet("background-color: rgba(0,0,0,150); border: 2px dashed #f1c40f; color: #f1c40f; font-size: 60px; font-weight: bold;");
        imgLabels[1]->setAlignment(Qt::AlignCenter);
        imgLabels[1]->setText("?");

        infoLabels[1]->setText("<span style='font-size:16px; color:#f1c40f;'>神秘房间</span><br><br>前方是一个未知的房间。<br>里面可能藏着宝藏，也可能是致命的危机...");
        return; // 直接返回，不再渲染怪物
    }

    // =====================================
    // 否则，渲染怪物情报
    // =====================================
    ui->label_title->setText("--- 下一层敌方情报 ---");
    std::vector<Character*> mockEnemies = mainPrepare->generateEnemiesForFloor(currentFloor);

    for (size_t i = 0; i < mockEnemies.size() && i < 4; ++i) {
        Character* e = mockEnemies[i];
        imgLabels[i]->show(); infoLabels[i]->show();

        // 恢复原有样式（防止被问号房覆盖）
        imgLabels[i]->setStyleSheet("background-color: rgba(0,0,0,150); border: 2px solid #c0392b;");

        QString infoText = QString("<span style='font-size:14px; color:gold;'>[%1]</span><br>"
                                   "HP: %2 | MP: %3<br>攻击: %4 | 防御: %5<br><hr>技能:<br>")
                               .arg(QString::fromStdString(e->getName()))
                               .arg(e->getMaxHealth()).arg(e->getmaxmana())
                               .arg(e->getAttack()).arg(e->getDefense());

        for (auto skill : e->getSkills()) {
            infoText += QString("- %1<br>").arg(QString::fromStdString(skill->getname()));
        }
        infoLabels[i]->setText(infoText);

        QString eJob = QString::fromStdString(e->getJobName());
        QString eName = QString::fromStdString(e->getName());
        QString imgPath;

        if (eJob.contains("法师")) imgPath = QString(":/images/enemies/enemy_wizard/%1.jpg").arg(eName);
        else if (eJob.contains("弓箭手")) imgPath = QString(":/images/enemies/enemy_archer/%1.jpg").arg(eName);
        else if (eJob.contains("无名")) imgPath = QString(":/images/enemies/enemy_nameless/%1.jpg").arg(eName);
        else imgPath = QString(":/images/enemies/enemy_swordman/%1.jpg").arg(eName);

        QPixmap pix(imgPath);
        if (!pix.isNull()) {
            imgLabels[i]->setPixmap(pix.scaled(imgLabels[i]->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
            imgLabels[i]->setText("");
        } else {
            imgLabels[i]->setText("无图像");
            imgLabels[i]->setAlignment(Qt::AlignCenter);
        }
    }

    for (Character* e : mockEnemies) delete e;
    mockEnemies.clear();
}

void review::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    // 这里如果想换背景图可以换，不换的话就用我上面 UI 里的纯色深色背景
    QPixmap background(":/images/backgrounds/event_background.png");
    if (!background.isNull()) {
        painter.drawPixmap(rect(), background.scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        painter.fillRect(rect(), QColor(44, 62, 80)); // 默认高级深灰蓝
    }
}
